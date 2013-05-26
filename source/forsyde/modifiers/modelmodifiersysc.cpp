/*
 * Copyright (c) 2011-2013
 *     Gabriel Hjort Blindell <ghb@kth.se>
 *     George Ungureanu <ugeorge@kth.se>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OF THIS SOFTWARE NOR THE
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "../../frontend/dumper.h"
#include "modelmodifiersysc.h"
#include "../SY/fanoutsy.h"
#include "../SY/delaysy.h"
#include "../../language/cfunction.h"
#include "../../language/cdatatype.h"
#include "../../tools/tools.h"
#include "../../exceptions/castexception.h"
#include "../../exceptions/indexoutofboundsexception.h"
#include <set>
#include <map>
#include <string>
#include <utility>
#include <iterator>
#include <new>
#include <stdexcept>
#include <cmath>

using namespace f2cc;
using namespace f2cc::Forsyde;
using namespace f2cc::Forsyde::SY;
using std::string;
using std::list;
using std::advance;
using std::map;
using std::set;
using std::vector;
using std::bad_alloc;
using std::pair;
using std::make_pair;

ModelModifierSysC::ModelModifierSysC(ProcessNetwork* processnetwork,
		Logger& logger, Config::Costs costs)
        throw(InvalidArgumentException) : processnetwork_(processnetwork), logger_(logger),
        costs_(costs), delay_dependency_(true){
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }
}

ModelModifierSysC::~ModelModifierSysC() throw() {}


void ModelModifierSysC::flattenAndParallelize() throw(
    		RuntimeException, InvalidModelException, InvalidProcessException, OutOfMemoryException){

	logger_.logMessage(Logger::INFO, "Flattening the process network and "
			"extracting data parallel processes ...");

	Composite* root = processnetwork_->getComposite(Id("f2cc0"));
	if(!root){
		THROW_EXCEPTION(InvalidModelException, string("Process network ")
						+ "does not have a root process");
	}

    list<Composite*> list_of_composites  = root->getComposites();
    for (list<Composite*>::iterator it = list_of_composites.begin(); it != list_of_composites.end(); ++it){
		flattenCompositeProcess(*it, root);
    }
    XmlDumper dump1(logger_);
    dump1.dump(processnetwork_, "flattened.xml");

    list<list<Leaf*> > equivalent_combs = extractEquivalentCombs(root);

	list<list<Leaf*> >::iterator equ_it;
	for (equ_it = equivalent_combs.begin(); equ_it != equivalent_combs.end(); ++equ_it){
		createParallelComposite(root,*equ_it);
	}
    XmlDumper dump2(logger_);
    dump2.dump(processnetwork_, "flattened1.xml");

	list<list<Leaf*> > equivalent_procs = extractEquivalentLeafs(root);
	while(equivalent_procs.size() != 0){
		for (equ_it = equivalent_procs.begin(); equ_it != equivalent_procs.end(); ++equ_it){
			createParallelComposite(root,*equ_it);
		}
		equivalent_procs = extractEquivalentLeafs(root);
	}
	removeRedundantZipsUnzips(root);
}

void ModelModifierSysC::optimizePlatform() throw(
		RuntimeException, InvalidModelException, InvalidProcessException, OutOfMemoryException){

	logger_.logMessage(Logger::INFO, "Optimizing the target platform for"
			" individual processes...");

	Composite* root = processnetwork_->getComposite(Id("f2cc0"));
	if(!root){
		THROW_EXCEPTION(InvalidModelException, string("Process network ")
						+ "does not have a root process");
	}

    list<Leaf*> list_of_leaves  = root->getProcesses();
    list<Composite*> list_of_pcomps  = root->getComposites();
    bool optimizing_finished = false;
    while (!optimizing_finished){
    	optimizing_finished = true;
		for (list<Leaf*>::iterator it = list_of_leaves.begin(); it != list_of_leaves.end(); ++it){
			bool is_on_device = (*it)->isMappedToDevice();
			map<CostType, unsigned long long> process_costs;
			int cost_this_platform = 0;
			int cost_switch_platform = 0;
			process_costs = calculateCostInNetwork(*it,is_on_device);
			for (map<CostType, unsigned long long>::iterator cit = process_costs.begin();
					cit != process_costs.end(); ++cit){
				cost_this_platform += cit->second;
			}
			process_costs = calculateCostInNetwork(*it,!is_on_device);
			for (map<CostType, unsigned long long>::iterator cit = process_costs.begin();
					cit != process_costs.end(); ++cit){
				cost_switch_platform += cit->second;
			}

			logger_.logMessage(Logger::DEBUG, string() + "\""
					+ (*it)->getId()->getString() +
					"\" mapped for "
					+ (is_on_device ? "device" : "host")
					+ ", has a total cost of " + tools::toString(cost_this_platform)
					+ "; switching platforms would change cost to "
					+ tools::toString(cost_switch_platform) );

			if (cost_this_platform > cost_switch_platform){
				(*it)->mapToDevice(!is_on_device);
				optimizing_finished = false;
				logger_.logMessage(Logger::DEBUG, string() + "\""
						+ (*it)->getId()->getString() +
						"\" switched platforms to "
						+ (!is_on_device ? "device" : "host") );
			}
		}
		for (list<Composite*>::iterator it = list_of_pcomps.begin(); it != list_of_pcomps.end(); ++it){
			bool is_on_device = (*it)->isMappedToDevice();
			map<CostType, unsigned long long> process_costs;
			int cost_this_platform = 0;
			int cost_switch_platform = 0;
			process_costs = calculateCostInNetwork(*it,is_on_device);
			for (map<CostType, unsigned long long>::iterator cit = process_costs.begin();
					cit != process_costs.end(); ++cit){
				cost_this_platform += cit->second;
			}
			process_costs = calculateCostInNetwork(*it,!is_on_device);
			for (map<CostType, unsigned long long>::iterator cit = process_costs.begin();
					cit != process_costs.end(); ++cit){
				cost_switch_platform += cit->second;
			}
			logger_.logMessage(Logger::DEBUG, string() + "\""
					+ (*it)->getId()->getString() +
					"\" mapped for " + (is_on_device ? "device" : "host")
					+ ", has a total cost of " + tools::toString(cost_this_platform)
					+ "; switching platforms would change cost to "
					+ tools::toString(cost_switch_platform) );

			if (cost_this_platform > cost_switch_platform){
				(*it)->mapToDevice(!is_on_device);
				optimizing_finished = false;
				logger_.logMessage(Logger::DEBUG, string() + "\""
						+ (*it)->getId()->getString() +
						"\" switched platforms to "
						+ (!is_on_device ? "device" : "host") );
			}
		}
    }

	logger_.logMessage(Logger::INFO, "Finished the platform optomizations for"
			" individual processes...");
}

Config::Costs ModelModifierSysC::loadBalance() throw(
		RuntimeException, InvalidModelException, InvalidProcessException, OutOfMemoryException){

	logger_.logMessage(Logger::INFO, "Commencing the load balancing algorithm...");

	Composite* root = processnetwork_->getComposite(Id("f2cc0"));
	if(!root){
		THROW_EXCEPTION(InvalidModelException, string("Process network ")
						+ "does not have a root process");
	}

	logger_.logMessage(Logger::INFO, "Extracting individual data paths");
	list<DataPath> datapaths = extractDataPaths(root);

	quantum_cost_ = 0;
	logger_.logMessage(Logger::INFO, "Finding the maximum (quantum) cost in the process network...");
	string owner = findMaximumCost(root, datapaths);
	logger_.logMessage(Logger::INFO, string() + "Found the maximum cost of "
			+ tools::toString(quantum_cost_) + " belonging to the "
			+ owner);

	logger_.logMessage(Logger::INFO, "Extracting contained paths and sorting them by maximum cost...");
	map<unsigned long long, list<Id> > contained_sections = sortContainedSectionsByCost(datapaths);

	stage_costs_.insert(make_pair(0,0));
	bool split_successful = false;
	logger_.logMessage(Logger::INFO, "Splitting the data paths into pipeline stages");
	while (!split_successful){
		for (map<unsigned long long, list<Id> >::reverse_iterator csit = contained_sections.rbegin();
				csit != contained_sections.rend(); ++csit){
			//std::cout<<"COST: "<<(*csit).first<<"\n";
			std::vector<Id> list_as_vector(csit->second.begin(), csit->second.end());
			split_successful = splitPipelineStages(list_as_vector);
		}
	}

	return costs_;
}


std::list<ModelModifierSysC::DataPath> ModelModifierSysC::extractDataPaths(Composite* root) throw (
 		 RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException){
    if (!root) {
        THROW_EXCEPTION(InvalidArgumentException, "\"root\" must not be NULL");
    }
    visited_processes_.clear();
    list<DataPath> paths;

    list<Composite::IOPort*> outputs  = root->getOutIOPorts();
	for (list<Composite::IOPort*>::iterator oit = outputs.begin(); oit != outputs.end(); ++oit){
		if ((*oit)->getConnectedPortInside()->getProcess() == root){
			THROW_EXCEPTION(InvalidProcessException, string("Port  \"")
							+ (*oit)->getId()->getString()
							+ "\" points to no data path ");
		}

		DataPath new_path;
		new_path.output_process_ = *root->getId();
		list<DataPath> opaths = parsePath((*oit)->getConnectedPortInside()->getProcess(),
				new_path, root);
		logger_.logMessage(Logger::INFO, string() + "Extracted "
				+ tools::toString(opaths.size())
				+" data paths that lead to "
				+ (*oit)->getId()->getString());
		tools::append(paths,opaths);
	}
	return paths;
}

std::list<ModelModifierSysC::DataPath> ModelModifierSysC::parsePath(Process* process, DataPath current_path,
		Composite* root) throw(
  		 RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException){
    if (!process) {
        THROW_EXCEPTION(InvalidArgumentException, "\"process\" must not be NULL");
    }
    list<DataPath> ret_paths;
    //std::cout<<current_path.printDataPath()<<"\n";

	Leaf* leaf = dynamic_cast<Leaf*>(process);
	ParallelComposite* pcomp = dynamic_cast<ParallelComposite*>(process);
	if (!leaf && !pcomp){
		THROW_EXCEPTION(InvalidProcessException, string("Process  \"")
						+ process->getId()->getString()
						+ "\" should not have been a composite at this stage");
	}

	DataPath new_path = current_path;
	new_path.path_.push_front(make_pair(*process->getId(), process->isMappedToDevice()));

	if (leaf){
		list<Leaf::Port*> inports  = leaf->getInPorts();
		for (list<Leaf::Port*>::iterator iit = inports.begin(); iit != inports.end(); ++iit){
			Process* next_proc = (*iit)->getConnectedPort()->getProcess();
			if (next_proc == root) {
				//reached the root's inputs. close the path
				DataPath path_to_introduce = new_path;
				path_to_introduce.input_process_ = *(*iit)->getConnectedPort()->getProcess()->getId();
				//Close the contained section by adding a start;
				ret_paths.push_back(path_to_introduce);
				logger_.logMessage(Logger::DEBUG, string() + "Added a new data path: "
						+  path_to_introduce.printDataPath() );
			}
			else{
				if (new_path.wasVisited(*next_proc->getId())){
					//was visited before
					DataPath path_to_introduce = new_path;
					path_to_introduce.input_process_ =
							*(*iit)->getConnectedPort()->getProcess()->getId();
					path_to_introduce.is_loop_ = true;
					//Close the contained section by adding a start;
					ret_paths.push_back(path_to_introduce);
					logger_.logMessage(Logger::DEBUG, string() + "Added a new data path: "
							+  path_to_introduce.printDataPath() );
				}
				else {
					//was not visited before. Continue parsing
					tools::append(ret_paths,parsePath(next_proc, new_path, root));
				}
			}
		}
	}
	if (pcomp){
		list<Composite::IOPort*> inports  = pcomp->getInIOPorts();
		for (list<Composite::IOPort*>::iterator iit = inports.begin(); iit != inports.end(); ++iit){
			Process* next_proc = (*iit)->getConnectedPortOutside()->getProcess();
			if (next_proc == root) {
				//reached the root's inputs. close the path
				DataPath path_to_introduce = new_path;
				path_to_introduce.input_process_ =
						*(*iit)->getConnectedPortOutside()->getProcess()->getId();
				//Close the contained section by adding a start;
				ret_paths.push_back(path_to_introduce);
				logger_.logMessage(Logger::DEBUG, string() + "Added a new data path: "
						+  path_to_introduce.printDataPath() );
			}
			else{
				if (new_path.wasVisited(*next_proc->getId())){
					//was visited before
					DataPath path_to_introduce = new_path;
					path_to_introduce.input_process_ =
							*(*iit)->getConnectedPortOutside()->getProcess()->getId();
					path_to_introduce.is_loop_ = true;
					//Close the contained section by adding a start;
					ret_paths.push_back(path_to_introduce);
					logger_.logMessage(Logger::DEBUG, string() + "Added a new data path: "
							+  path_to_introduce.printDataPath() );

				}
				else {
					//was not visited before. Continue parsing
					tools::append(ret_paths,parsePath(next_proc, new_path, root));
				}
			}
		}
	}
	return ret_paths;
}

std::string ModelModifierSysC::findMaximumCost(Composite* root, list<DataPath> datapaths) throw (
  		 RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException,
  		InvalidModelException){
    if (!root) {
        THROW_EXCEPTION(InvalidArgumentException, "\"composite\" must not be NULL");
    }

    string maximum_owner;
    unsigned long long max_comm_cost = 0;
    unsigned long long max_comp_cost = 0;

    unsigned long long seq_cost = 0;
    list<Leaf*> leafs  = root->getProcesses();
	for (list<Leaf*>::iterator lit = leafs.begin(); lit != leafs.end(); ++lit){
		if (!(*lit)->isMappedToDevice()) seq_cost += (*lit)->getCost() * costs_.k_SEQ;
		else {
			std::map<CostType, unsigned long long> leaf_costs = calculateCostInNetwork((*lit), true);
			if (leaf_costs[PROCESS_COST] > max_comp_cost){
				max_comp_cost = leaf_costs[PROCESS_COST];
				maximum_owner = string("leaf process \"")
						+ (*lit)->getId()->getString() + "\" executing on device";
			}
			if (leaf_costs[IN_COST] > max_comm_cost){
				max_comm_cost = leaf_costs[IN_COST];
				maximum_owner = string("The input costs for leaf process \"")
						+ (*lit)->getId()->getString() + "\" executing on device";
			}
			if (leaf_costs[OUT_COST] > max_comm_cost){
				max_comm_cost = leaf_costs[OUT_COST];
				maximum_owner = string("The output costs for leaf process \"")
						+ (*lit)->getId()->getString() + "\" executing on device";
			}
		}
	}
    list<Composite*> pcomps  = root->getComposites();
	for (list<Composite*>::iterator cit = pcomps.begin(); cit != pcomps.end(); ++cit){
		ParallelComposite* pcomp = dynamic_cast<ParallelComposite*>(*cit);
		if (!pcomp){
			THROW_EXCEPTION(InvalidProcessException, string("Process  \"")
							+ (*cit)->getId()->getString()
							+ "\" should not have been a composite at this stage");
		}
		else{
			if (!pcomp->isMappedToDevice()) seq_cost += pcomp->getCost() * pcomp->getNumProcesses()
					* costs_.k_SEQ;
			else {
				std::map<CostType, unsigned long long> pcomp_costs = calculateCostInNetwork(pcomp, true);
				if (pcomp_costs[PROCESS_COST] > max_comp_cost){
					max_comp_cost = pcomp_costs[PROCESS_COST];
					maximum_owner = string("leaf process \"")
							+ pcomp->getId()->getString() + "\" executing on device";
				}
				if (pcomp_costs[IN_COST] > max_comm_cost){
					max_comm_cost = pcomp_costs[IN_COST];
					maximum_owner = string("leaf process \"")
							+ pcomp->getId()->getString() + "\" executing on device";
				}
				if (pcomp_costs[OUT_COST] > max_comm_cost){
					max_comm_cost = pcomp_costs[OUT_COST];
					maximum_owner = string("leaf process \"")
							+ pcomp->getId()->getString() + "\" executing on device";
				}
			}
		}
	}
	if (seq_cost >max_comp_cost){
		max_comp_cost = seq_cost;
		maximum_owner = string("processes executing on host");
	}

	for (list<DataPath>::iterator dit = datapaths.begin(); dit != datapaths.end(); ++dit){
		if (dit->is_loop_){
			list<Id> first_contained = dit->getContainedPaths().front();
			if (getIdFromList(dit->input_process_, first_contained) != first_contained.end()){
				unsigned long long loopcost = calculateLoopCost(dit->input_process_, first_contained);
				if (loopcost > max_comp_cost){
					max_comp_cost = loopcost;
					maximum_owner = string("processes the loop started by \"")
							+ dit->input_process_.getString() + "\"";
				}
			}
		}
	}

	if (max_comm_cost > max_comp_cost){
		unsigned long long n_bursts = ceil((float)max_comm_cost / (float)max_comp_cost);
		if (n_bursts > 16) costs_.n_bursts = 16;
		else costs_.n_bursts = unsigned(n_bursts);
		unsigned long long new_cost =  max_comm_cost / costs_.n_bursts + 1;
		if (new_cost < max_comp_cost) quantum_cost_ = max_comp_cost;
		else quantum_cost_ = new_cost;
	}
	else {
		quantum_cost_ = max_comp_cost;
		costs_.n_bursts = 1;
	}

	return maximum_owner;
}

std::map<unsigned long long, std::list<Id> > ModelModifierSysC::sortContainedSectionsByCost(
		std::list<DataPath> datapaths) throw (
	RuntimeException, InvalidProcessException, OutOfMemoryException, InvalidModelException){

	map<unsigned long long, list<Id> > sorted_list;
	map<unsigned long long , unsigned> index;

	for (list<DataPath>::iterator dpit = datapaths.begin(); dpit != datapaths.end(); ++dpit){
		list<list<Id> > list_contained = dpit->getContainedPaths();
		for (list<list<Id> >::iterator lcit = list_contained.begin(); lcit != list_contained.end(); ++lcit){
			unsigned long long contained_cost = 0;
			if(dpit->is_loop_ && lcit == list_contained.begin()){
				if (getIdFromList(dpit->input_process_, *lcit) != (*lcit).end()){
					unsigned long long loopcost = calculateLoopCost(dpit->input_process_, *lcit);
					if (loopcost > contained_cost) contained_cost = loopcost;
					//continue;
				}
			}
			else {
				for (list<Id>::iterator cit = lcit->begin(); cit != lcit->end(); ++cit){
					Process* current = processnetwork_->getProcess(*cit);
					if (!current) current = processnetwork_->getComposite(*cit);
					std::map<CostType, unsigned long long> proc_costs =
							calculateCostInNetwork(current, true);
					if (proc_costs[PROCESS_COST] > contained_cost) contained_cost = proc_costs[PROCESS_COST];
				}
			}
			if (index.find(contained_cost) != index.end()) index[contained_cost]++;
			else index.insert(make_pair(contained_cost, contained_cost * 1000));
			sorted_list.insert(make_pair(index[contained_cost], *lcit));
		}
	}
	return sorted_list;
}

bool ModelModifierSysC::splitPipelineStages(vector<Id> contained_s)
    throw (RuntimeException, InvalidProcessException, OutOfMemoryException, InvalidModelException){

	//std::cout<<"!!!AT THE ENTRANCE: "<<printVector(contained_s)<<"\n";
	vector<Id> already_assigned;
	bool is_optimized = false;

	logger_.logMessage(Logger::DEBUG, string() + "Commencing load balancing for the contained "
			"section... ");

	while(!is_optimized){
		is_optimized = true;
		//find out which processes in this section are already assigned
		for (unsigned i=0; i<contained_s.size(); i++) {
			Process* proc = processnetwork_->getComposite(contained_s[i]);
			if (!proc) proc = processnetwork_->getProcess(contained_s[i]);
			if(proc->getStream() != 0){
				//std::cout<<"Assigned stream = "<<proc->getStream()<<"\n";
				already_assigned.push_back(*proc->getId());
			}
		}
		//std::cout<<"### Already assigned procs:"<<printVector(already_assigned)<<"\n";

		//try to fill up the already assigned stages
		for (unsigned ait=0; ait<already_assigned.size(); ait++) {
			Process* assigned_proc = processnetwork_->getComposite(already_assigned[ait]);
			if (!assigned_proc) assigned_proc = processnetwork_->getProcess(already_assigned[ait]);
			unsigned pos_cs = getPosOf(already_assigned[ait], contained_s);

			//std::cout<<"### AMU ii la "<<pos_cs<<":"<<contained_s[pos_cs]<<"\n";

			if (pos_cs != 0){
				//std::cout<<"### NU E 0 :"<<pos_cs<<" : "<<contained_s[pos_cs]<<"\n";
				Process* proc_to_add = processnetwork_->getComposite(contained_s[pos_cs - 1]);
				if (!proc_to_add) proc_to_add = processnetwork_->getProcess(contained_s[pos_cs - 1]);

				if(proc_to_add->getStream() == 0){
					//the process belongs to the current section, but was not assigned yet
					unsigned assigned_stage = assigned_proc->getStream();
					//get the "new link cost"
					unsigned long long new_sync_cost;
					if (pos_cs - 1 != 0){
						Process* conn_to_proctoadd = processnetwork_->
								getComposite(contained_s[pos_cs - 2]);
						if (!conn_to_proctoadd) conn_to_proctoadd = processnetwork_->
								getProcess(contained_s[pos_cs - 2]);
						new_sync_cost = getSignalCost(conn_to_proctoadd, proc_to_add, true);
					}
					else new_sync_cost = 0;
					unsigned old_sync_cost = getSignalCost(proc_to_add, assigned_proc, true);

					unsigned long long new_stage_cost =
							stage_costs_[assigned_stage] +
							proc_to_add->getCost() * costs_.k_PAR +
							new_sync_cost - old_sync_cost;
					if (new_stage_cost < quantum_cost_ ){
						stage_costs_[assigned_stage] = new_stage_cost;
						proc_to_add->setStream(assigned_stage);
						is_optimized = false;
						logger_.logMessage(Logger::DEBUG, string() + "Added process \""
								+  proc_to_add->getId()->getString()
								+ "\" to stage " + tools::toString(assigned_stage)
								+ ". The new stage cost is " + tools::toString(new_stage_cost));
					}
				}
			}
			if (pos_cs < contained_s.size() - 1){
				//std::cout<<"### NU E LAST :"<<pos_cs<<" : "<<contained_s[pos_cs]<<"\n";
				Process* proc_to_add = processnetwork_->getComposite(contained_s[pos_cs + 1]);
				if (!proc_to_add) proc_to_add = processnetwork_->getProcess(contained_s[pos_cs + 1]);

				if(proc_to_add->getStream() == 0){
					//the process belongs to the current section, but was not assigned yet
					unsigned assigned_stage = assigned_proc->getStream();
					//get the "new link cost"
					unsigned long long new_sync_cost;
					if (pos_cs + 1 < contained_s.size() - 1){
						Process* conn_to_proctoadd = processnetwork_->
								getComposite(contained_s[pos_cs + 2]);
						if (!conn_to_proctoadd) conn_to_proctoadd = processnetwork_->
								getProcess(contained_s[pos_cs + 2]);
						new_sync_cost = getSignalCost(proc_to_add, conn_to_proctoadd, true);
					}
					else new_sync_cost = 0;
					unsigned old_sync_cost = getSignalCost(assigned_proc, proc_to_add, true);

					unsigned long long new_stage_cost =
							stage_costs_[assigned_stage] +
							proc_to_add->getCost() * costs_.k_PAR +
							new_sync_cost - old_sync_cost;
					if (new_stage_cost < quantum_cost_ ){
						stage_costs_[assigned_stage] = new_stage_cost;
						proc_to_add->setStream(assigned_stage);
						is_optimized = false;
						logger_.logMessage(Logger::DEBUG, string() + "Added process \""
								+  proc_to_add->getId()->getString()
								+ "\" to stage " + tools::toString(assigned_stage)
								+ ". The new stage cost is " + tools::toString(new_stage_cost));
					}
				}
			}
		}
	}

	//split all the remaining contained section into free portions
	list<vector<Id> > free_condained;
	vector<Id> dummy_vector;
	bool previous_free = false;
	for (unsigned cit=0; cit<contained_s.size(); cit++) {
		Process* proc = processnetwork_->getComposite(contained_s[cit]);
		if (!proc) proc = processnetwork_->getProcess(contained_s[cit]);
		if(proc->getStream() == 0){
			if (!previous_free) free_condained.push_back(dummy_vector);
			free_condained.back().push_back(*proc->getId());
			previous_free = true;
		}
		else if (previous_free) {
			previous_free = false;
		}
	}
	/*logger_.logMessage(Logger::DEBUG, string() + "The current contained section was split into "
			+  tools::toString(free_condained.size())
			+ " other sections. Proceeding with their analysis...");*/



	//all that is left is to load balance the free contained sections
	for (list<vector<Id> >::iterator fcit = free_condained.begin(); fcit != free_condained.end(); ++fcit){
		vector<Id> remaining;
		remaining.assign(fcit->begin(),fcit->end());
		while (remaining.size() > 0){
			vector<Id> dummy;
			list<pair<vector<Id>, pair<unsigned long long, unsigned long long> > > left_right_costs;
			list<pair<vector<Id>, pair<unsigned long long, unsigned long long> > > right_left_costs;
			left_right_costs.push_back(make_pair(dummy, make_pair(0, 0)));
			right_left_costs.push_back(make_pair(dummy, make_pair(0, 0)));
			//parse section from left to right
			unsigned long long stage_cost = 0;
			unsigned long long sync_cost = 0;
			unsigned long long first_sync_cost = 0;


			//std::cout<<"### Unallocated Contained:"<<printVector(remaining)<<"\n";
			//std::cout<<"### Initial Contained:"<<printVector(contained_s)<<"\n";
			for (unsigned lrit=0; lrit<remaining.size(); lrit++) {
				Process* proc = processnetwork_->getComposite(remaining[lrit]);
				if (!proc) proc = processnetwork_->getProcess(remaining[lrit]);
				//if either first or last in the free contained list, add sync cost.
				unsigned pos_csec = getPosOf(remaining[lrit], contained_s);
				//std::cout<<"IDX LR: "<< lrit<<"; Pos CSEC: "<< pos_csec<<"\n";
				if (lrit == 0){
					//if first in a free contained section, check if there is another connected as contained
					if  (pos_csec > 0){
						Process* connected_proc = processnetwork_->
								getComposite(contained_s[pos_csec - 1]);
						if (!connected_proc) connected_proc = processnetwork_->
								getProcess(contained_s[pos_csec - 1]);
						first_sync_cost = getSignalCost(connected_proc, proc, true);
					}
				}

				if  (pos_csec < contained_s.size() - 1){
					//if this process has another contained after him, calculate the transfer
					Process* connected_proc = processnetwork_->
							getComposite(contained_s[pos_csec + 1]);
					if (!connected_proc) connected_proc = processnetwork_->
							getProcess(contained_s[pos_csec + 1]);
					sync_cost = first_sync_cost + getSignalCost(proc, connected_proc, true);
				}

				//add the computation cost for this process
				stage_cost += proc->getCost() * costs_.k_PAR;
				if (stage_cost + sync_cost <= quantum_cost_){
					left_right_costs.back().first.push_back(remaining[lrit]);
					left_right_costs.back().second.first = sync_cost;
					left_right_costs.back().second.second = stage_cost + sync_cost;
				}
				else{
					if(left_right_costs.back().first.size() == 0){
						//uh oh! found a new critical cost. set it and invalidate the optimization!
						quantum_cost_ = stage_cost + sync_cost;
						logger_.logMessage(Logger::INFO, string() + "Found a new quantum cost, "
								+ tools::toString(quantum_cost_) + ", belonging to \""
								+ proc->getId()->getString()
								+ "\". Invalidating the optimizations and starting over with the"
								+ " load balancing...");
						return false;
					}
					//everything is all right, add a new stage.
					if (lrit < remaining.size() - 1){
						//std::cout<<"### New Stage: "<<printVector(left_right_costs.back().first)<<"\n";
						left_right_costs.push_back(make_pair(dummy, make_pair(0,0)));
						sync_cost = 0;
						first_sync_cost = 0;
						stage_cost = 0;
						//if (lrit == 0) std::cout<<"HOPA!!!!!!!!!!!!!!!!!!!!!!!\n";
						--lrit;
					}
				}
			}
			//std::cout<<"### New Stage: "<<printVector(left_right_costs.back().first)<<"\n";
			//parse section from right to left
			stage_cost = 0;
			sync_cost = 0;
			first_sync_cost = 0;
			for (unsigned rlit = remaining.size() - 1; rlit > 0; rlit--) {
				//std::cout<<"entering "<<rlit<<"\n";
				Process* proc = processnetwork_->getComposite(remaining[rlit]);
				if (!proc) proc = processnetwork_->getProcess(remaining[rlit]);

				//if either first or last in the free contained list, add sync cost.
				unsigned pos_csec = getPosOf(remaining[rlit], contained_s);
				//std::cout<<"IDX RL: "<< rlit<<"; Pos CSEC: "<< pos_csec<<"\n";
				if (rlit == remaining.size() - 1){
					//if first in a free contained section, check if there is another connected as contained
					if  (pos_csec < contained_s.size() - 1){
						Process* connected_proc = processnetwork_->
								getComposite(contained_s[pos_csec + 1]);
						if (!connected_proc) connected_proc = processnetwork_->
								getProcess(contained_s[pos_csec + 1]);
						first_sync_cost = getSignalCost(proc, connected_proc, true);
					}
				}

				if  (pos_csec > 0){
					//if this process has another contained after him, calculate the transfer
					Process* connected_proc = processnetwork_->
							getComposite(contained_s[pos_csec - 1]);
					if (!connected_proc) connected_proc = processnetwork_->
							getProcess(contained_s[pos_csec - 1]);
					sync_cost = first_sync_cost + getSignalCost(connected_proc, proc, true);
				}

				//add the computation cost for this process
				stage_cost += proc->getCost() * costs_.k_PAR;
				if (stage_cost + sync_cost <= quantum_cost_){
					right_left_costs.back().first.insert(right_left_costs.back().first.begin(),
							remaining[rlit]);
					right_left_costs.back().second.first = sync_cost;
					right_left_costs.back().second.second = stage_cost + sync_cost;
				}
				else{
					if(left_right_costs.back().first.size() == 0){
						//uh oh! found a new critical cost. set it and invalidate the optimization!
						quantum_cost_ = stage_cost + sync_cost;
						logger_.logMessage(Logger::INFO, string() + "Found a new quantum cost, "
								+ tools::toString(quantum_cost_) + ", belonging to \""
								+ proc->getId()->getString()
								+ "\". Invalidating the optimizations and starting over with the"
								+ " load balancing...");
						return false;
					}
					//everything is all right, add a new stage.
					if (rlit > 0){
						//std::cout<<"### New Stage: "<<printVector(right_left_costs.back().first)<<"\n";
						right_left_costs.push_back(make_pair(dummy, make_pair(0,0)));
						sync_cost = 0;
						first_sync_cost = 0;
						stage_cost = 0;
						//if (rlit == remaining.size() - 1) std::cout<<"HOPA!!!!!!!!!!!!!!!!!!!!!!!\n";
						++rlit;
					}
				}
				//std::cout<<"exiting "<<rlit<<": "<<printVector(right_left_costs.back().first)<<"\n";
			}

			//std::cout<<"### New Stage: "<<printVector(right_left_costs.back().first)<<"\n";

			bool size_demand = left_right_costs.size() >= right_left_costs.size();
			bool sync_demand = left_right_costs.front().second.first >
					right_left_costs.front().second.first;
			pair<vector<Id>, pair<unsigned long long, unsigned long long> > final_stage_combset =
					(size_demand && sync_demand) ? right_left_costs.front() : left_right_costs.front();

			vector<Id> final_stage = final_stage_combset.first;
			unsigned long long final_stage_cost = final_stage_combset.second.second;
			unsigned new_stage_id = (*stage_costs_.rbegin()).first + 1;
			stage_costs_.insert(make_pair(new_stage_id, final_stage_cost));

			//std::cout<<"### HOP : "<<printVector(final_stage)<<"\n";

			for (unsigned fsit = 0; fsit < final_stage.size() ; fsit++){
				Process* proc = processnetwork_->getComposite(final_stage[fsit]);
				if (!proc) proc = processnetwork_->getProcess(final_stage[fsit]);
				proc->setStream(new_stage_id);
				remaining.erase(remaining.begin());
				logger_.logMessage(Logger::DEBUG, string() + "Added process \""
						+  proc->getId()->getString()
						+ "\" to stage " + tools::toString(proc->getStream()));
			}
			logger_.logMessage(Logger::DEBUG, string() + "Created a new stage ("
					+ tools::toString(new_stage_id) + ") with "
					+ tools::toString(final_stage.size())
					+ " processes, an execution cost of "
					+ tools::toString(final_stage_cost)
					+ " and a sync cost of "
					+ tools::toString(final_stage_combset.second.first));

		}
	}
	return true;
}

void ModelModifierSysC::wrapPipelineStages() throw(
		RuntimeException, InvalidModelException, InvalidProcessException, OutOfMemoryException,
		InvalidArgumentException){

	logger_.logMessage(Logger::INFO, "Specializing parallel composites for pipeline stages...");

	Composite* root = processnetwork_->getComposite(Id("f2cc0"));
	if(!root){
		THROW_EXCEPTION(InvalidModelException, string("Process network ")
						+ "does not have a root process");
	}

	logger_.logMessage(Logger::INFO, "Ordering pipeline stages...");
	map<unsigned, list<Id> > stages = orderStages();

	for (map<unsigned, list<Id> >::iterator sit = stages.begin(); sit != stages.end(); ++sit){
		if(sit->first != 0) groupIntoPipelineComposites(sit->second);
	}
}


void ModelModifierSysC::groupIntoPipelineComposites(std::list<Forsyde::Id> stage) throw(
		RuntimeException, InvalidModelException, InvalidProcessException, OutOfMemoryException,
		    		InvalidArgumentException){

	logger_.logMessage(Logger::INFO, "Grouping all processes within the current stage"
			"into one parallel composite ...");

	Composite* root = processnetwork_->getComposite(Id("f2cc0"));
	if(!root){
		THROW_EXCEPTION(InvalidModelException, string("Process network ")
						+ "does not have a root process");
	}

	ParallelComposite* reference;
	for (list<Id>::iterator idit = stage.begin(); idit != stage.end(); ++idit){
		Composite* pcomp = root->getComposite(*idit);
		if (dynamic_cast<ParallelComposite*>(pcomp)){
			reference = dynamic_cast<ParallelComposite*>(pcomp);
			reference->changeName(Id(string() + "stage_" + tools::toString(reference->getStream())));
			stage.erase(idit);
			logger_.logMessage(Logger::DEBUG, string() + "For this stage, \""
					+ idit->getString()
					+ "\" will be used as reference.");
			break;
		}
	}
	if (!reference){
		Id pcomp_stage_id = processnetwork_->getUniqueCompositeId("f2cc_stage_");
		Hierarchy new_hierarchy = root->getHierarchy();
		reference = new ParallelComposite(pcomp_stage_id, new_hierarchy, pcomp_stage_id, 1);
		root->addComposite(reference);
		logger_.logMessage(Logger::DEBUG, string() + "New reference had to be created: "
				+ pcomp_stage_id.getString());
	}

	unsigned num_procs = reference->getNumProcesses();

	for (list<Id>::iterator idit = stage.begin(); idit != stage.end(); ++idit){
		Composite* pcomp = dynamic_cast<ParallelComposite*>(root->getComposite(*idit));
		Leaf* leaf = root->getProcess(*idit);

		if (leaf){

			moveToNewParent(leaf, root, reference);

			list<Leaf::Port*> inputs  = leaf->getInPorts();
			for (list<Leaf::Port*>::iterator iit = inputs.begin(); iit != inputs.end(); ++iit){
				Process::Interface* connection = (*iit)->getConnectedPort();
				redirectFlow((*iit), connection, reference, false);
			}

			list<Leaf::Port*> outputs  = leaf->getOutPorts();
			for (list<Leaf::Port*>::iterator iit = outputs.begin(); iit != outputs.end(); ++iit){
				Process::Interface* connection = (*iit)->getConnectedPort();
				redirectFlow((*iit), connection, reference, true);
			}
		}
		else if (pcomp){
			moveToNewParent(pcomp, root, reference);
			if ((int)num_procs < pcomp->getNumProcesses()) reference->setNumProcesses(
					pcomp->getNumProcesses());

			list<Composite::IOPort*> inputs  = pcomp->getInIOPorts();
			for (list<Composite::IOPort*>::iterator iit = inputs.begin(); iit != inputs.end(); ++iit){
				Process::Interface* connection = (*iit)->getConnectedPortOutside();
				redirectFlow((*iit), connection, reference, false);
			}

			list<Composite::IOPort*> outputs  = pcomp->getOutIOPorts();
			for (list<Composite::IOPort*>::iterator iit = outputs.begin(); iit != outputs.end(); ++iit){
				Process::Interface* connection = (*iit)->getConnectedPortOutside();
				redirectFlow((*iit), connection, reference, true);
			}

			flattenCompositeProcess(pcomp, reference);

		}
	}
	list<Leaf*> contained_leafs  = reference->getProcesses();
	for (list<Leaf*>::iterator nit = contained_leafs.begin(); nit != contained_leafs.end(); ++nit){
		std::cout<<(*nit)->toString()<<"\n";
	}
	std::cout<<reference->toString()<<"\n";
}

void ModelModifierSysC::redirectFlow(Process::Interface* source, Process::Interface* target,
		ParallelComposite* reference, bool input) throw (
			 InvalidArgumentException, RuntimeException, InvalidProcessException){


	Leaf::Port* src_port = dynamic_cast<Leaf::Port*>(source);
	Composite::IOPort* src_ioport = dynamic_cast<Composite::IOPort*>(source);
	unsigned num_procs = reference->getNumProcesses();
	//unsigned num_procs = 1;
	if (target->getProcess() != reference){
		CDataType outer_type;
		Leaf::Port* conn_port = dynamic_cast<Leaf::Port*>(target);
		if (conn_port) outer_type = conn_port->getDataType();
		else{
			Composite::IOPort* conn_ioport = dynamic_cast<Composite::IOPort*>(target);
			outer_type = conn_ioport->getDataType().first;
		}
		std::cout<<source->toString()<<"\n";
		CDataType inner_type = (src_port) ? src_port->getDataType() : src_ioport->getDataType().first;
		inner_type.setArraySize( (src_port) ? (inner_type.getArraySize() / num_procs) :
				inner_type.getArraySize() / num_procs);

		if (src_port) (src_port)->setConnection(NULL);
		else src_ioport->setConnection(NULL, true);
		//std::cout<<"LALALALALAAAAAAAAAAAAAAAAA\n";
		Id inner_id = Id((source)->toString());
		//std::cout<<"LALALALALAAAAAAAAAAAAAAAAA\n";
		Leaf* trg_leaf = dynamic_cast<Leaf*>(target->getProcess());
		Composite* trg_comp =  dynamic_cast<Composite*>(target->getProcess());
		Composite::IOPort* new_port;
		//std::cout<<"LALALALALAAAAAAAAAAAAAAAAA\n";
		if (trg_leaf){
			if (!input){
				reference->addInIOPort(inner_id, inner_type);
				new_port = reference->getInIOPort(inner_id);
				}
			else {
				reference->addOutIOPort(inner_id, inner_type);
				new_port = reference->getOutIOPort(inner_id);
			}
		}
		else if (trg_comp){
			if (!input){
				reference->addInIOPort(inner_id, inner_type);
				new_port = reference->getInIOPort(inner_id);
				}
			else {
				reference->addOutIOPort(inner_id, inner_type);
				new_port = reference->getOutIOPort(inner_id);
			}
		}
		new_port->setDataType(true, outer_type);
		//std::cout<<"LALALALALAAAAAAAAAAAAAAAAA\n";


		new_port->setConnection(source, false);
		if (src_port) (src_port)->setConnection(new_port);
		else src_ioport->setConnection(new_port, true);
		if (conn_port) conn_port->setConnection(new_port);
		else {
			Composite::IOPort* conn_ioport = dynamic_cast<Composite::IOPort*>(target);
			if (*conn_ioport->getProcess()->getId() == Id("f2cc0")) {
				conn_ioport->setConnection(new_port, false);
			}
			else conn_ioport->setConnection(new_port, true);
		}
		new_port->setConnection(target, true);

		logger_.logMessage(Logger::DEBUG, string() + "Connection between "
				+ source->toString() + " and "
				+ target->toString() + " was made through "
				+ new_port->toString());
	}
	else{
		Composite::IOPort* pcomp_port;
		if (src_port) pcomp_port = dynamic_cast<Composite::IOPort*>(src_port->getConnectedPort());
		else if (src_ioport)  pcomp_port = dynamic_cast<Composite::IOPort*>(
				src_ioport->getConnectedPortOutside());
		Leaf::Port* inside_port = dynamic_cast<Leaf::Port*>(pcomp_port->getConnectedPortInside());

		pcomp_port->setConnection(NULL, true);
		pcomp_port->setConnection(NULL, false);
		//std::cout<<inside_port->toString()<<" : "<<inside_port->getConnectedPort()->toString()<<"\n";
			//	std::cout<<"LULUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU\n";
		//std::cout<<pcomp_port->getId()->getString()<<" : "<<input<<"\n";
		if (input)
				reference->deleteInIOPort(*pcomp_port->getId());
		else
			reference->deleteOutIOPort(*pcomp_port->getId());
		//std::cout<<"LULUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU\n";
		if (src_port) src_port->setConnection(inside_port);
		else if (src_ioport)  src_ioport->setConnection(inside_port, true);
		//std::cout<<"LULUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU\n";
		inside_port->setConnection(source);

		logger_.logMessage(Logger::DEBUG, string() + "Connection between "
				+ source->toString() + " and "
				+ inside_port->toString() + " was made by deleting the intermediary port");
	}

}

std::map<unsigned, std::list<Forsyde::Id> > ModelModifierSysC::orderStages() throw(
		RuntimeException, InvalidModelException, InvalidProcessException, OutOfMemoryException,
		InvalidArgumentException){
	Composite* root = processnetwork_->getComposite(Id("f2cc0"));
	if(!root){
		THROW_EXCEPTION(InvalidModelException, string("Process network ")
						+ "does not have a root process");
	}

	map<unsigned, list<Id> > stages;
	list<Id> dummy;

	list<Composite*> comps = root->getComposites();
	for (list<Composite*>::iterator cit = comps.begin(); cit != comps.end(); ++cit){
		ParallelComposite* pcomp = dynamic_cast<ParallelComposite*>(*cit);
		if(!pcomp){
			THROW_EXCEPTION(InvalidModelException, string("Process network ")
							+ "should not have normal composites at this stage");
		}

		unsigned stage = pcomp->getStream();
		map<unsigned, list<Id> >::iterator found_stage = stages.find(stage);
		if (found_stage != stages.end()) found_stage->second.push_back(*pcomp->getId());
		else {
			stages.insert(make_pair(stage, dummy));
			stages[stage].push_back(*pcomp->getId());
		}
	}

	list<Leaf*> leafs = root->getProcesses();
	for (list<Leaf*>::iterator lit = leafs.begin(); lit != leafs.end(); ++lit){
		Leaf* leaf = *lit;
		unsigned stage = leaf->getStream();
		map<unsigned, list<Id> >::iterator found_stage = stages.find(stage);
		if (found_stage != stages.end()) found_stage->second.push_back(*leaf->getId());
		else {
			stages.insert(make_pair(stage, dummy));
			stages[stage].push_back(*leaf->getId());
		}
	}
	return stages;
}

void ModelModifierSysC::flattenCompositeProcess(Composite* composite, Composite* parent) throw(
		 RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException){
    if (!composite) {
        THROW_EXCEPTION(InvalidArgumentException, "\"composite\" must not be NULL");
    }
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }

	logger_.logMessage(Logger::DEBUG, string() + "Moving leaf processes inside \""
				+ composite->getId()->getString()
				+ "\" to \""
				+ parent->getId()->getString()
				+ "\"...");

    list<Composite*> list_of_composites  = composite->getComposites();
    for (list<Composite*>::iterator it = list_of_composites.begin(); it != list_of_composites.end(); ++it){
		flattenCompositeProcess(*it, composite);
    }

	list<Leaf*> contained_leafs  = composite->getProcesses();
	for (list<Leaf*>::iterator nit = contained_leafs.begin(); nit != contained_leafs.end(); ++nit){
		moveToNewParent(*nit, composite, parent);
	}
	list<Composite::IOPort*> in_ports = composite->getInIOPorts();
	for (list<Composite::IOPort*>::iterator nit = in_ports.begin(); nit != in_ports.end(); ++nit){
		logger_.logMessage(Logger::DEBUG, string() + "Redirecting \""
					+ (*nit)->getConnectedPortOutside()->toString()
					+ "\" to \""
					+ (*nit)->getConnectedPortInside()->toString()+ "\"");
		(*nit)->getConnectedPortOutside()->connect((*nit)->getConnectedPortInside());

	}
	list<Composite::IOPort*> out_ports = composite->getOutIOPorts();
	for (list<Composite::IOPort*>::iterator nit = out_ports.begin(); nit != out_ports.end(); ++nit){
		logger_.logMessage(Logger::DEBUG, string() + "Redirecting \""
					+ (*nit)->getConnectedPortInside()->toString()
					+ "\" to \""
					+ (*nit)->getConnectedPortOutside()->toString()+ "\"");
		(*nit)->getConnectedPortInside()->connect((*nit)->getConnectedPortOutside());
	}


	logger_.logMessage(Logger::DEBUG, string() + "Removing \""
				+ composite->getId()->getString()
				+ "\"...");
	processnetwork_->removeComposite(*composite->getId());
	parent->deleteComposite(*composite->getId());
}



list<list<Leaf*> > ModelModifierSysC::extractEquivalentCombs(Composite* parent)
        throw(InvalidArgumentException, OutOfMemoryException){
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }

    logger_.logMessage(Logger::DEBUG, string() + "Extracting equivalent Comb processes from "
    			+ parent->getId()->getString() + "...");

    list<list<Leaf*> > grouped_equivalent_processes;
    map< string, list<map<Id, SY::Comb*> > > table_of_equivalences;


	list<Leaf*> list_of_leafs = parent->getProcesses();
	list<Leaf*>::iterator it_leafs;

	for (it_leafs = list_of_leafs.begin(); it_leafs != list_of_leafs.end(); ++it_leafs){
		SY::Comb* curr_comb = dynamic_cast<SY::Comb*>(*it_leafs);
		if(curr_comb){
			string function_name = curr_comb->getFunction()->getName();
			map< string, list<map<Id, SY::Comb*> > >::iterator found_func_at =
					table_of_equivalences.find(function_name);
			if (found_func_at != table_of_equivalences.end()){
				logger_.logMessage(Logger::DEBUG, string() + "Verifying dependencies for \""
							+ curr_comb->getId()->getString()
							+ "\"...");

				bool fct_added = false;
				for (list<map<Id, SY::Comb*> >::iterator it = table_of_equivalences[function_name].begin();
						it != table_of_equivalences[function_name].end(); ++it){
					if (!foundDependencyDownstream(curr_comb, *it) &&
							!foundDependencyUpstream(curr_comb, *it)){
						(*it).insert(pair<Id, SY::Comb*>(*curr_comb->getId(), curr_comb));
						fct_added = true;

						break;
					}
				}
				visited_processes_.clear();
				if (!fct_added) {
					map<Id, SY::Comb*> new_map;
					new_map.insert(pair<Id, SY::Comb*>(*curr_comb->getId(), curr_comb));
					table_of_equivalences[function_name].push_back(new_map);
				}
			}
			else {
				list<map<Id, SY::Comb*> > new_list;
				map<Id, SY::Comb*> new_map;
				new_map.insert(pair<Id, SY::Comb*>(*curr_comb->getId(), curr_comb));
				new_list.push_back(new_map);
			    try {
			    	table_of_equivalences.insert(
			    			pair<string, list<map<Id, SY::Comb*> > >(function_name, new_list));
			    }
			    catch(bad_alloc&) {
			        THROW_EXCEPTION(OutOfMemoryException);
			    }
			}
		}
	}


	map< string, list<map<Id, SY::Comb*> > >::iterator itt;
	for (itt = table_of_equivalences.begin(); itt != table_of_equivalences.end(); ++itt){
		list<map<Id, SY::Comb*> >::iterator itl;
		for (itl = itt->second.begin(); itl != itt->second.end(); ++itl){
			if ((*itl).size() > 1){
				list<Leaf*> equivalent_combs;
				logger_.logMessage(Logger::DEBUG, string() + "Found "
							+ tools::toString((*itl).size())
							+ " equivalent Combs having function \""
							+ itt->first + "\". Adding them to the list...");
				map<Id, SY::Comb*>::iterator itm;
				for (itm = (*itl).begin(); itm != (*itl).end(); ++itm){
					try {
						equivalent_combs.push_back(itm->second);
					}
					catch(bad_alloc&) {
						THROW_EXCEPTION(OutOfMemoryException);
					}
				}
				try {
					grouped_equivalent_processes.push_back(equivalent_combs);
				}
				catch(bad_alloc&) {
					THROW_EXCEPTION(OutOfMemoryException);
				}
			}
		}
	}
    return grouped_equivalent_processes;
}


list<list<Leaf*> > ModelModifierSysC::extractEquivalentLeafs(Composite* parent)
        throw(InvalidArgumentException, OutOfMemoryException){
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }

    logger_.logMessage(Logger::INFO, string() + "Extracting equivalent processes from "
    			+ parent->getId()->getString() + "...");

    list<list<Leaf*> > grouped_equivalent_processes;
    map< Id, list<Leaf*> > table_of_equivalences;

	list<Leaf*> list_of_leafs = parent->getProcesses();
	list<Leaf*>::iterator it_leafs;

	for (it_leafs = list_of_leafs.begin(); it_leafs != list_of_leafs.end(); ++it_leafs){
		if (!dynamic_cast<SY::Zipx*>(*it_leafs) && !dynamic_cast<SY::Unzipx*>(*it_leafs)){
			Id tag = Id("");
			bool foundTag = false;
			list<Leaf::Port*> out_ports = (*it_leafs)->getOutPorts();
			for (list<Leaf::Port*>::iterator itp = out_ports.begin(); itp != out_ports.end(); ++itp){
				Process* connected_proc = (*itp)->getConnectedPort()->getProcess();
				if(dynamic_cast<SY::Zipx*>(connected_proc)){
					tag = *connected_proc->getId();
					foundTag = true;
					break;
				}
			}
			if (!foundTag) {
				list<Leaf::Port*> in_ports = (*it_leafs)->getInPorts();
				for (list<Leaf::Port*>::iterator itp = in_ports.begin(); itp != in_ports.end(); ++itp){
					Process* connected_proc = (*itp)->getConnectedPort()->getProcess();
					if(dynamic_cast<SY::Unzipx*>(connected_proc)){
						tag = *connected_proc->getId();
						foundTag = true;
						break;
					}
				}
			}
			if (foundTag){
				map< Id, list<Leaf*> >::iterator found_func_at = table_of_equivalences.find(tag);
				if (found_func_at != table_of_equivalences.end()){
					logger_.logMessage(Logger::DEBUG, string() + "Found potential parallel process: "
								+ (*it_leafs)->getId()->getString() );
					found_func_at->second.push_back(*it_leafs);
				}
				else{
					list<Leaf*> new_list;
					new_list.push_back(*it_leafs);
					table_of_equivalences.insert(pair<Id, list<Leaf*> >(tag, new_list));
				}
			}
		}
	}

	map< Id, list<Leaf*> >::iterator itt;
	for (itt = table_of_equivalences.begin(); itt != table_of_equivalences.end(); ++itt){
		if (itt->second.size() > 1){
			logger_.logMessage(Logger::DEBUG, string() + "Found "
						+ tools::toString(itt->second.size())
						+ " equivalent leafs connected to \""
						+ itt->first.getString() + "\". Adding them to the list...");
			try {
				grouped_equivalent_processes.push_back(itt->second);
			}
			catch(bad_alloc&) {
				THROW_EXCEPTION(OutOfMemoryException);
			}
		}
	}
    return grouped_equivalent_processes;
}

void ModelModifierSysC::removeRedundantZipsUnzips(Forsyde::Composite* parent)
     throw(InvalidArgumentException, OutOfMemoryException){
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }

    logger_.logMessage(Logger::INFO, string() + "Removing redundant Unzipx-Zipx sequences from "
    			+ parent->getId()->getString() + "...");

	list<Leaf*> list_of_leafs = parent->getProcesses();
	list<Leaf*>::iterator it_leafs;
	for (it_leafs = list_of_leafs.begin(); it_leafs != list_of_leafs.end(); ++it_leafs){
		SY::Unzipx* unzip = dynamic_cast<SY::Unzipx*>(*it_leafs);
		SY::Zipx* zip = dynamic_cast<SY::Zipx*>(*it_leafs);
		if (unzip){
			map<Id, Id> equivalence_list;
			list<Leaf::Port*> oports = unzip->getOutPorts();
			for (list<Leaf::Port*>::iterator itp = oports.begin(); itp != oports.end(); ++itp){
				SY::Zipx* pair_zip = dynamic_cast<SY::Zipx*>((*itp)->getConnectedPort()->getProcess());
				if (pair_zip) {
					map<Id, Id>::iterator found_common = equivalence_list.find(*pair_zip->getId());
					if (found_common != equivalence_list.end()){
						Leaf::Port* ref_port = unzip->getOutPort(found_common->second);
						Leaf::Port* con_port = pair_zip->getInPort(*ref_port->getConnectedPort()->getId());
						unsigned new_array_size = ref_port->getDataType().getArraySize() +
								(*itp)->getDataType().getArraySize();
						if (!ref_port->getDataType().isArray()){
							ref_port->getDataType().setIsArray(true);
							ref_port->getDataType().setIsConst(true);
						}
						ref_port->getDataType().setArraySize(new_array_size);
						con_port->getDataType().setArraySize(new_array_size);
						Id here = *(*itp)->getId();
						Id there = *(*itp)->getConnectedPort()->getId();
						pair_zip->deleteInPort(there);
						unzip->deleteOutPort(here);
					}
					else {
						try {
							equivalence_list.insert(pair<Id, Id>(*pair_zip->getId(), *(*itp)->getId()));
						}
						catch(bad_alloc&) {
							THROW_EXCEPTION(OutOfMemoryException);
						}
					}
				}
			}
		}
		else if (zip){
			map<Id, Id> equivalence_list;
			list<Leaf::Port*> iports = zip->getInPorts();
			for (list<Leaf::Port*>::iterator itp = iports.begin(); itp != iports.end(); ++itp){
				SY::Unzipx* pair_unzip = dynamic_cast<SY::Unzipx*>((*itp)->getConnectedPort()->getProcess());
				if (pair_unzip) {
					map<Id, Id>::iterator found_common = equivalence_list.find(*pair_unzip->getId());
					if (found_common != equivalence_list.end()){
						Leaf::Port* ref_port = zip->getInPort(found_common->second);
						Leaf::Port* con_port = pair_unzip->getOutPort(*ref_port->getConnectedPort()->getId());
						unsigned new_array_size = ref_port->getDataType().getArraySize() +
								(*itp)->getDataType().getArraySize();
						if (!ref_port->getDataType().isArray()){
							ref_port->getDataType().setIsArray(true);
							ref_port->getDataType().setIsConst(true);
						}
						ref_port->getDataType().setArraySize(new_array_size);
						con_port->getDataType().setArraySize(new_array_size);
						Id here = *(*itp)->getId();
						Id there = *(*itp)->getConnectedPort()->getId();
						pair_unzip->deleteOutPort(there);
						zip->deleteInPort(here);
					}
					else {
						try {
							equivalence_list.insert(pair<Id, Id>(*pair_unzip->getId(), *(*itp)->getId()));
						}
						catch(bad_alloc&) {
							THROW_EXCEPTION(OutOfMemoryException);
						}
					}
				}
			}
		}
	}

	for (it_leafs = list_of_leafs.begin(); it_leafs != list_of_leafs.end(); ++it_leafs){
		SY::Unzipx* unzip = dynamic_cast<SY::Unzipx*>(*it_leafs);
		SY::Zipx* zip = dynamic_cast<SY::Zipx*>(*it_leafs);
		if (unzip)  {
			if (unzip->getNumOutPorts() <= 1) {
				logger_.logMessage(Logger::DEBUG, string() + "Connecting \""
								+ unzip->getInPorts().front()->getConnectedPort()->toString()
								+ "\" with \""
								+ unzip->getOutPorts().front()->getConnectedPort()->toString()
								+"\"...");

				unzip->getInPorts().front()->getConnectedPort()->connect(
						unzip->getOutPorts().front()->getConnectedPort());

				processnetwork_->removeProcess(*unzip->getId());
				parent->deleteProcess(*unzip->getId());
			}
			else {
				list<Leaf::Port*> oports = unzip->getOutPorts();
				for (list<Leaf::Port*>::iterator itp = oports.begin(); itp != oports.end(); ++itp){
					Process::Interface* connected_interface = (*itp)->getConnectedPort();
					Leaf::Port* connected_port = dynamic_cast<Leaf::Port*>(connected_interface);
					Composite::IOPort* connected_ioport = dynamic_cast<Composite::IOPort*>(connected_interface);
					if (connected_port){
						CDataType type = connected_port->getDataType();
						(*itp)->setDataType(type);
					}
					else if (connected_ioport){
						CDataType type = connected_ioport->getDataType().first;
						(*itp)->setDataType(type);
					}
				}
			}
		}
		else if (zip) {
			if (zip->getNumInPorts() <= 1) {
				logger_.logMessage(Logger::DEBUG, string() + "Connecting \""
								+ zip->getInPorts().front()->getConnectedPort()->toString()
								+ "\" with \""
								+ zip->getOutPorts().front()->getConnectedPort()->toString()
								+"\"...");

				zip->getInPorts().front()->getConnectedPort()->connect(
						zip->getOutPorts().front()->getConnectedPort());

				processnetwork_->removeProcess(*zip->getId());
				parent->deleteProcess(*zip->getId());
			}
			else {
				list<Leaf::Port*> iports = zip->getInPorts();
				for (list<Leaf::Port*>::iterator itp = iports.begin(); itp != iports.end(); ++itp){
					Process::Interface* connected_interface = (*itp)->getConnectedPort();
					Leaf::Port* connected_port = dynamic_cast<Leaf::Port*>(connected_interface);
					Composite::IOPort* connected_ioport = dynamic_cast<Composite::IOPort*>(connected_interface);
					if (connected_port){
						CDataType type = connected_port->getDataType();
						(*itp)->setDataType(type);
					}
					else if (connected_ioport){
						CDataType type = connected_ioport->getDataType().first;
						(*itp)->setDataType(type);
					}
				}
			}
		}
	}
}

std::map<ModelModifierSysC::CostType, unsigned long long> ModelModifierSysC::calculateCostInNetwork(
		Process* process, bool on_device)
	 throw(RuntimeException, InvalidProcessException, InvalidArgumentException){
    if (!process) {
        THROW_EXCEPTION(InvalidArgumentException, "\"process\" must not be NULL");
    }

    map<CostType, unsigned long long> cost_mapset;
    unsigned long long in_costs = 0;
    unsigned long long out_costs = 0;
    unsigned long long process_cost;

    ParallelComposite* pcomp = dynamic_cast<ParallelComposite*>(process);
    Leaf* leaf = dynamic_cast<Leaf*>(process);
    if (leaf) {
    	process_cost = (on_device) ?
    			(leaf->getCost() * costs_.k_PAR) : (leaf->getCost() * costs_.k_SEQ);
        list<Leaf::Port*> in_ports = leaf->getInPorts();
        for (list<Leaf::Port*>::iterator it = in_ports.begin(); it != in_ports.end(); ++it){
        	CDataType datatype = (*it)->getDataType();
        	Process* other_end = (*it)->getConnectedPort()->getProcess();
        	in_costs += datatype.getArraySize() *
        			datatype.getTypeSize() *
        			transferCoefficient(other_end->isMappedToDevice(), on_device,
        					other_end->getStream() == process->getStream() );
        }
        list<Leaf::Port*> out_ports = leaf->getOutPorts();
        for (list<Leaf::Port*>::iterator it = out_ports.begin(); it != out_ports.end(); ++it){
        	CDataType datatype = (*it)->getDataType();
        	Process* other_end = (*it)->getConnectedPort()->getProcess();
        	out_costs += datatype.getArraySize() *
        			datatype.getTypeSize() *
        			transferCoefficient(on_device, other_end->isMappedToDevice(),
        					other_end->getStream() == process->getStream() );
        }
    }
    else if (pcomp) {
    	process_cost = (on_device) ? (pcomp->getCost() * costs_.k_PAR) :
    			(pcomp->getCost() * pcomp->getNumProcesses() * costs_.k_SEQ);
        list<Composite::IOPort*> in_ports = pcomp->getInIOPorts();
        for (list<Composite::IOPort*>::iterator it = in_ports.begin(); it != in_ports.end(); ++it){
        	CDataType datatype = (*it)->getDataType().first;
        	Process* other_end = (*it)->getConnectedPortOutside()->getProcess();
        	in_costs += datatype.getArraySize() *
        			datatype.getTypeSize() *
        			transferCoefficient(other_end->isMappedToDevice(), on_device,
        					other_end->getStream() == process->getStream() );
        }
        list<Composite::IOPort*> out_ports = pcomp->getOutIOPorts();
        for (list<Composite::IOPort*>::iterator it = out_ports.begin(); it != out_ports.end(); ++it){
        	CDataType datatype = (*it)->getDataType().second;
        	Process* other_end = (*it)->getConnectedPortOutside()->getProcess();
        	out_costs += datatype.getArraySize() *
        			datatype.getTypeSize() *
        			transferCoefficient(on_device, other_end->isMappedToDevice(),
        					other_end->getStream() == process->getStream() );
        }
    }

	try {
	    cost_mapset.insert(pair<CostType, unsigned long long>(IN_COST, in_costs));
	    cost_mapset.insert(pair<CostType, unsigned long long>(OUT_COST, out_costs));
	    cost_mapset.insert(pair<CostType, unsigned long long>(PROCESS_COST, process_cost));
	}
	catch(bad_alloc&) {
		THROW_EXCEPTION(OutOfMemoryException);
	}

    return cost_mapset;
}


bool ModelModifierSysC::foundDependencyUpstream(Leaf* current_process,
		std::map<Id, SY::Comb*> to_compare_with)
 	 throw(RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException){

	visited_processes_.insert(pair<Id, bool>(*current_process->getId(), true));

    list<Leaf::Port*> in_ports = current_process->getInPorts();
    for (list<Leaf::Port*>::iterator it = in_ports.begin(); it != in_ports.end(); ++it){
    	if (dynamic_cast<Leaf::Port*>((*it)->getConnectedPort())){
    		Leaf* next_process = dynamic_cast<Leaf*>((*it)->getConnectedPort()->getProcess());
    		if (delay_dependency_ && dynamic_cast<SY::delay*>(next_process)) return false;
    		if (visited_processes_.find(*next_process->getId()) != visited_processes_.end())
    			return false;
    		if (to_compare_with.find(*next_process->getId()) != to_compare_with.end()) return true;
    		else {
    			if (foundDependencyUpstream(next_process, to_compare_with)) return true;
    		}
    	}
    }
    return false;
}

bool ModelModifierSysC::foundDependencyDownstream(Leaf* current_process,
		std::map<Id, SY::Comb*> to_compare_with)
 	 throw(RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException){

	visited_processes_.insert(pair<Id, bool>(*current_process->getId(), true));

    list<Leaf::Port*> out_ports = current_process->getOutPorts();
    for (list<Leaf::Port*>::iterator it = out_ports.begin(); it != out_ports.end(); ++it){
    	if (dynamic_cast<Leaf::Port*>((*it)->getConnectedPort())){
    		Leaf* next_process = dynamic_cast<Leaf*>((*it)->getConnectedPort()->getProcess());
    		if (delay_dependency_ && dynamic_cast<SY::delay*>(next_process)) return false;
    		if (visited_processes_.find(*next_process->getId()) != visited_processes_.end())
				return false;
    		if (to_compare_with.find(*next_process->getId()) != to_compare_with.end()) return true;
    		else {
    			if (foundDependencyDownstream(next_process, to_compare_with)) return true;
    		}
    	}
    }
    return false;
}


void ModelModifierSysC::createParallelComposite(Composite* parent, list<Leaf*> equivalent_processes)
              throw(RuntimeException, InvalidProcessException,InvalidArgumentException, OutOfMemoryException){
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }

    //count the processes
    unsigned number_of_processes = 0;
	list<Leaf*>::iterator it_list;
	for (it_list = equivalent_processes.begin(); it_list != equivalent_processes.end(); ++it_list){
		ParallelComposite* par_comp = dynamic_cast<ParallelComposite*>(*it_list);
		if (par_comp) number_of_processes += par_comp->getNumProcesses();
		else number_of_processes++;
	}

    //create the process
    Hierarchy parent_hierarchy =  parent->getHierarchy();
    ParallelComposite* new_pcomp = new ParallelComposite(processnetwork_->getUniqueCompositeId("pcomp_"),
    		parent_hierarchy,Id(""), number_of_processes);

    processnetwork_->addComposite(new_pcomp);
    parent->addComposite(new_pcomp);

	//Extract the base process
    Process* reference_process = equivalent_processes.front();
    equivalent_processes.pop_front();

	logger_.logMessage(Logger::INFO, string() + "Creating a parallel composite process from \""
				+ reference_process->getId()->getString()
				+ "\" with " + tools::toString(number_of_processes) + " processes...");

    Leaf* leaf_ref = dynamic_cast<Leaf*>(reference_process);
    if (leaf_ref) prepareLeafForParallel(leaf_ref, parent, new_pcomp, number_of_processes);
    else {
		THROW_EXCEPTION(CastException, string("Process \"")
						+ reference_process->getId()->getString()
						+ "\" is not a leaf");
    }
    //redirect the data path through the parallel composite.
    while (equivalent_processes.size() != 0){
        Process* old_process = equivalent_processes.front();
        equivalent_processes.pop_front();
    	redirectFlowThroughParallelComposite(old_process, parent, new_pcomp);
    }
}



void ModelModifierSysC::prepareLeafForParallel(Leaf* reference_leaf, Composite* parent,
		 ParallelComposite* new_pcomp, unsigned number_of_processes)
	 throw(RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException){
    if (!reference_leaf) {
        THROW_EXCEPTION(InvalidArgumentException, "\"reference_leaf\" must not be NULL");
    }
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }
    if (!new_pcomp) {
        THROW_EXCEPTION(InvalidArgumentException, "\"new_pcomp\" must not be NULL");
    }

    //copy the leaf to the ParallelComposite
    moveToParallelComposite(reference_leaf, parent, new_pcomp);

    Hierarchy parent_hierarchy = parent->getHierarchy();
    list<Leaf::Port*> in_ports = reference_leaf->getInPorts();
    for (list<Leaf::Port*>::iterator it = in_ports.begin(); it != in_ports.end(); ++it){
    	Leaf::Port* leaf_port = *it;

    	//add new IOPort to pcomp with array outside and scalar inside
    	new_pcomp->addInIOPort(*leaf_port->getId(), leaf_port->getDataType());
    	Composite::IOPort* new_pcomp_port = new_pcomp->getInIOPort(*(*it)->getId());
    	CDataType type_outside = CDataType(leaf_port->getDataType().getType(),
				true, true, (leaf_port->getDataType().getArraySize() * number_of_processes),
				false, true);
    	new_pcomp_port->setDataType(true,type_outside);

    	//creating a new Zipx and connect it to the new port
    	SY::Zipx* new_zip;
        try {
        	new_zip = new SY::Zipx(Id(processnetwork_->getUniqueProcessId("zipx_")),
        			parent_hierarchy, 0);
        }
        catch (bad_alloc&) {
            THROW_EXCEPTION(OutOfMemoryException);
        }
        parent->addProcess(new_zip);
        processnetwork_->addProcess(new_zip);

		//connect the out Zip port to the new ParallelComposite
        new_zip->addOutPort(Id("oport1"), type_outside);
        Leaf::Port* new_zip_oport = new_zip->getOutPort(Id("oport1"));
        new_pcomp_port->connect(new_zip_oport);
        logger_.logMessage(Logger::DEBUG, string() + "Created  \""
        				+ new_zip->getId()->getString()
        				+ "\" and connected it to \""
        				+ new_pcomp_port->toString() + "\"");

        //redirect the flow into this new Zip
        Process::Interface* connected_port = leaf_port->getConnectedPort();
        Composite::IOPort* connected_ioport = dynamic_cast<Composite::IOPort*>(connected_port);
        ParallelComposite* pcomp_parent = dynamic_cast<ParallelComposite*>(connected_port->getProcess());
        if(connected_ioport){
        	if(pcomp_parent && (connected_ioport->getConnectedPortInside() == leaf_port)){
        		connected_port = connected_ioport->getConnectedPortOutside();
        	}
        }
        Id new_port_id = Id("port_to_" + leaf_port->getProcess()->getId()->getString());
        new_zip->addInPort(new_port_id, leaf_port->getDataType());
		Leaf::Port* new_zip_iport = new_zip->getInPort(new_port_id);
		new_zip_iport->connect(connected_port);
		logger_.logMessage(Logger::DEBUG, string() + "Redirected \""
							+ connected_port->toString()
							+ "\" to \""
							+ new_zip_iport->toString() + "\"");

		//finally connect the inner pcomp ioport to the now-free leaf port
		new_pcomp_port->connect(leaf_port);
		logger_.logMessage(Logger::DEBUG, string() + "Finally, connected \""
							+ leaf_port->toString()
							+ "\" to \""
							+ new_pcomp_port->toString() + "\"");
    }

    list<Leaf::Port*> out_ports = reference_leaf->getOutPorts();
    for (list<Leaf::Port*>::iterator it = out_ports.begin(); it != out_ports.end(); ++it){
    	Leaf::Port* leaf_port = *it;

    	//add new IOPort to pcomp with array outside and scalar inside
    	new_pcomp->addOutIOPort(*leaf_port->getId(), leaf_port->getDataType());
    	Composite::IOPort* new_pcomp_port = new_pcomp->getOutIOPort(*(*it)->getId());
    	CDataType type_outside = CDataType(leaf_port->getDataType().getType(),
				true, true, (leaf_port->getDataType().getArraySize() * number_of_processes),
				false, true);
    	new_pcomp_port->setDataType(true,type_outside);

    	//creating a new Unzipx and connect it to the new port
    	SY::Unzipx* new_unzip;
        try {
        	new_unzip = new SY::Unzipx(Id(processnetwork_->getUniqueProcessId("unzipx_")),
        			parent_hierarchy, 0);
        }
        catch (bad_alloc&) {
            THROW_EXCEPTION(OutOfMemoryException);
        }
        parent->addProcess(new_unzip);
		processnetwork_->addProcess(new_unzip);
		//connect the out Zip port to the new ParallelComposite
        new_unzip->addInPort(Id("iport1"), type_outside);
        Leaf::Port* new_unzip_iport = new_unzip->getInPort(Id("iport1"));
        new_pcomp_port->connect(new_unzip_iport);
        logger_.logMessage(Logger::DEBUG, string() + "Created  \""
        				+ new_unzip->getId()->getString()
        				+ "\" and connected it to \""
        				+ new_pcomp_port->toString() + "\"");

        //redirect the flow into this new Zip
        Process::Interface* connected_port = leaf_port->getConnectedPort();
        Composite::IOPort* connected_ioport = dynamic_cast<Composite::IOPort*>(connected_port);
        ParallelComposite* pcomp_parent = dynamic_cast<ParallelComposite*>(connected_port->getProcess());
        if(connected_ioport){
        	if(pcomp_parent && (connected_ioport->getConnectedPortInside() == leaf_port)){
        		connected_port = connected_ioport->getConnectedPortOutside();
        	}
        }

        Id new_port_id = Id("port_from_" + leaf_port->getProcess()->getId()->getString());
        new_unzip->addOutPort(new_port_id, leaf_port->getDataType());
		Leaf::Port* new_unzip_oport = new_unzip->getOutPort(new_port_id);
		new_unzip_oport->connect(connected_port);
		logger_.logMessage(Logger::DEBUG, string() + "Redirected \""
							+ connected_port->toString()
							+ "\" to \""
							+ new_unzip_oport->toString() + "\"");

		//finally connect the inner pcomp ioport to the now-free leaf port
		new_pcomp_port->connect(leaf_port);
		logger_.logMessage(Logger::DEBUG, string() + "Finally, connected \""
							+ leaf_port->toString()
							+ "\" to \""
							+ new_pcomp_port->toString() + "\"");
    }
}

void ModelModifierSysC::moveToParallelComposite(Process* reference_process, Composite* old_parent,
		 ParallelComposite* new_parent) throw (InvalidProcessException, OutOfMemoryException){
    if (!reference_process) {
        THROW_EXCEPTION(InvalidArgumentException, "\"reference_process\" must not be NULL");
    }
    if (!old_parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"old_parent\" must not be NULL");
    }
    if (!new_parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"new_parent\" must not be NULL");
    }


    Hierarchy new_hierarchy = new_parent->getHierarchy();
    reference_process->setHierarchy(new_hierarchy);
    Leaf* new_leaf = dynamic_cast<Leaf*>(reference_process);
    if (new_leaf) {
    	old_parent->removeProcess(*new_leaf->getId());
    	new_parent->addProcess(new_leaf);

    	SY::Comb* new_comb = dynamic_cast<SY::Comb*>(reference_process);
    	if (new_comb) new_parent->changeName(Id(new_comb->getFunction()->getName()));
    	else new_parent->changeName(Id(string() + "pcomp<" + new_leaf->type()
    			+ ">"));

    }
    else {
    	Composite* new_comp = dynamic_cast<Composite*>(reference_process);
    	if (new_comp){
    		old_parent->removeComposite(*new_comp->getId());
    		new_parent->addComposite(new_comp);
    		new_parent->changeName(new_comp->getName());
    	}
    }

    new_parent->setContainedProcessId(reference_process->getId());
    int new_cost= reference_process->getCost();
    new_parent->setCost(new_cost);

    logger_.logMessage(Logger::DEBUG, string() + "Moved process  \""
				+ reference_process->getId()->getString()
				+ "\" of type " + reference_process->type()
				+ "  to its new parent \""
				+ new_parent->getId()->getString() + "\"");
}


void ModelModifierSysC::moveToNewParent(Process* reference_process, Composite* old_parent,
		Composite* new_parent) throw (InvalidProcessException, OutOfMemoryException){
    if (!reference_process) {
        THROW_EXCEPTION(InvalidArgumentException, "\"reference_process\" must not be NULL");
    }
    if (!old_parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"old_parent\" must not be NULL");
    }
    if (!new_parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"new_parent\" must not be NULL");
    }


    Hierarchy new_hierarchy = new_parent->getHierarchy();
    reference_process->setHierarchy(new_hierarchy);
    Leaf* new_leaf = dynamic_cast<Leaf*>(reference_process);
    if (new_leaf) {
    	old_parent->removeProcess(*new_leaf->getId());
    	new_parent->addProcess(new_leaf);
    }
    else {
    	Composite* new_comp = dynamic_cast<Composite*>(reference_process);
    	if (new_comp){
    		old_parent->removeComposite(*new_comp->getId());
    		new_parent->addComposite(new_comp);
    	}
    }

    logger_.logMessage(Logger::DEBUG, string() + "Moved process  \""
				+ reference_process->getId()->getString()
				+ "\" of type " + reference_process->type()
				+ "  to its new parent \""
				+ new_parent->getId()->getString() + "\"");

}


void ModelModifierSysC::redirectFlowThroughParallelComposite(Process* old_process, Composite* parent,
		ParallelComposite* new_pcomp) throw (
    		 InvalidArgumentException, RuntimeException, InvalidProcessException){
    if (!old_process) {
        THROW_EXCEPTION(InvalidArgumentException, "\"old_process\" must not be NULL");
    }
    if (!new_pcomp) {
        THROW_EXCEPTION(InvalidArgumentException, "\"new_pcomp\" must not be NULL");
    }

    logger_.logMessage(Logger::DEBUG, string() + "Redirecting the data flow from process \""
				+ old_process->getId()->getString()
				+ "\" to process \""
				+ new_pcomp->getId()->getString() + "\"...");

    Composite* comp = dynamic_cast<Composite*>(old_process);
    Leaf* leaf = dynamic_cast<Leaf*>(old_process);

    if (comp) {
    	list<Composite::IOPort*> in_ports = comp->getInIOPorts();
		for (list<Composite::IOPort*>::iterator it = in_ports.begin(); it != in_ports.end(); ++it){
			Process* connected_proc = new_pcomp->getInIOPort(*(*it)->getId())->getConnectedPortOutside(
					)->getProcess();
			SY::Zipx* in_zip = dynamic_cast<SY::Zipx*>(connected_proc);
			if (!in_zip){
				THROW_EXCEPTION(InvalidProcessException,
								string("Process \"")
								+ connected_proc->getId()->getString()
								+ "\" is not Zipx");
			}
			Process::Interface* connected_interface = (*it)->getConnectedPortOutside();
			Leaf::Port* connected_port = dynamic_cast<Leaf::Port*>(connected_interface);
			Composite::IOPort* connected_ioport = dynamic_cast<Composite::IOPort*>(connected_interface);

			//add a new port to the connected zip and redirect the flow through it
			Id new_port_id = Id("port_to_" + (*it)->getProcess()->getId()->getString());
			in_zip->addInPort(new_port_id, (*it)->getDataType().first);
			Leaf::Port* new_port = in_zip->getInPort(new_port_id);
			if(connected_port) connected_port->connect(new_port);
			else if(connected_ioport) connected_ioport->connect(new_port);

		}
    	list<Composite::IOPort*> out_ports = comp->getOutIOPorts();
		for (list<Composite::IOPort*>::iterator it = out_ports.begin(); it != out_ports.end(); ++it){
			Process* connected_proc = new_pcomp->getOutIOPort(*(*it)->getId())->getConnectedPortOutside(
					)->getProcess();
			SY::Unzipx* out_unzip = dynamic_cast<SY::Unzipx*>(connected_proc);
			if (!out_unzip){
				THROW_EXCEPTION(InvalidProcessException,
								string("Process \"")
								+ connected_proc->getId()->getString()
								+ "\" is not Unzipx");
			}
			Process::Interface* connected_interface = (*it)->getConnectedPortOutside();
			Leaf::Port* connected_port = dynamic_cast<Leaf::Port*>(connected_interface);
			Composite::IOPort* connected_ioport = dynamic_cast<Composite::IOPort*>(connected_interface);

			//add a new port to the connected zip and redirect the flow through it
			Id new_port_id = Id("port_from_" + (*it)->getProcess()->getId()->getString());
			out_unzip->addOutPort(new_port_id, (*it)->getDataType().first);
			Leaf::Port* new_port = out_unzip->getOutPort(new_port_id);
			if(connected_port) connected_port->connect(new_port);
			else if(connected_ioport) connected_ioport->connect(new_port);
		}

		processnetwork_->removeComposite(*comp->getId());
		parent->deleteComposite(*comp->getId());
    }
    else if (leaf){
    	list<Leaf::Port*> in_ports = leaf->getInPorts();
		for (list<Leaf::Port*>::iterator it = in_ports.begin(); it != in_ports.end(); ++it){
			Process* connected_proc = new_pcomp->getInIOPort(*(*it)->getId())->getConnectedPortOutside(
					)->getProcess();
			SY::Zipx* in_zip = dynamic_cast<SY::Zipx*>(connected_proc);
			if (!in_zip){
				THROW_EXCEPTION(InvalidProcessException,
								string("Process \"")
								+ connected_proc->getId()->getString()
								+ "\" is not Zipx");
			}
			Process::Interface* connected_interface = (*it)->getConnectedPort();
			Leaf::Port* connected_port = dynamic_cast<Leaf::Port*>(connected_interface);
			Composite::IOPort* connected_ioport = dynamic_cast<Composite::IOPort*>(connected_interface);

			//add a new port to the connected zip and redirect the flow through it
			Id new_port_id = Id("port_to_" + (*it)->getProcess()->getId()->getString());
			in_zip->addInPort(new_port_id, (*it)->getDataType());
			Leaf::Port* new_port = in_zip->getInPort(new_port_id);
			if(connected_port) connected_port->connect(new_port);
			else if(connected_ioport) connected_ioport->connect(new_port);


		}
    	list<Leaf::Port*> out_ports = leaf->getOutPorts();
		for (list<Leaf::Port*>::iterator it = out_ports.begin(); it != out_ports.end(); ++it){
			Process* connected_proc = new_pcomp->getOutIOPort(*(*it)->getId())->getConnectedPortOutside(
					)->getProcess();
			SY::Unzipx* out_unzip = dynamic_cast<SY::Unzipx*>(connected_proc);
			if (!out_unzip){
				THROW_EXCEPTION(InvalidProcessException,
								string("Process \"")
								+ connected_proc->getId()->getString()
								+ "\" is not Unzipx");
			}
			Process::Interface* connected_interface = (*it)->getConnectedPort();
			Leaf::Port* connected_port = dynamic_cast<Leaf::Port*>(connected_interface);
			Composite::IOPort* connected_ioport = dynamic_cast<Composite::IOPort*>(connected_interface);

			//add a new port to the connected zip and redirect the flow through it
			Id new_port_id = Id("port_from_" + (*it)->getProcess()->getId()->getString());
			out_unzip->addOutPort(new_port_id, (*it)->getDataType());
			Leaf::Port* new_port = out_unzip->getOutPort(new_port_id);
			if(connected_port) connected_port->connect(new_port);
			else if(connected_ioport) connected_ioport->connect(new_port);


		}
		processnetwork_->removeProcess(*leaf->getId());
		parent->deleteProcess(*leaf->getId());
    }
    else {
		THROW_EXCEPTION(InvalidProcessException,
						string("Process \"")
						+ old_process->getId()->getString()
						+ "\" is of unknown type");
    }

}

int ModelModifierSysC::transferCoefficient(bool source_on_device, bool target_on_device, bool same_stream)
     	 throw(){
	if (!source_on_device && !target_on_device) return costs_.k_H2H;
	if (!source_on_device && target_on_device) return costs_.k_H2D;
	if (source_on_device && !target_on_device) return costs_.k_D2H;
	if (source_on_device && target_on_device && same_stream) return costs_.k_T2T;
	if (source_on_device && target_on_device && !same_stream) return costs_.k_D2D;
	else return -1;
}

std::list<Id>::iterator  ModelModifierSysC::getIdFromList(Id id, list<Id> list) throw(){
	std::list<Id>::iterator it;
	for (it = list.begin(); it != list.end(); ++it){
		if (*it == id) return it;
	}
	return it;
}

unsigned ModelModifierSysC::getPosOf(Id id, std::vector<Id> vector) throw(RuntimeException){
	unsigned index;
	for (index = 0; index < vector.size() ; index++){
		if (vector[index] == id) return index;
	}
	THROW_EXCEPTION(RuntimeException, string("The Id  \"")
					+ id.getString()
					+ "\" was not found in the list;\n"
					+ printVector(vector));
}

std::list<Id>  ModelModifierSysC::getPortionOfPath(Id start, Id stop, std::list<Id> list) throw(){
	std::list<Id> portion;
	bool is_in_portion = false;
	for (std::list<Id>::iterator it = list.begin(); it != list.end(); ++it){
		if (is_in_portion) portion.push_back(*it);
		if (*it == start) {
			is_in_portion = true;
			portion.push_back(*it);
		}
		if (*it == stop){
			portion.push_back(*it);
			return portion;
		}
	}
	return portion;
}

unsigned long long ModelModifierSysC::calculateLoopCost(Id divergent_proc,
		list<Id> contained) throw(){

		unsigned long long loopcost = 0;
		unsigned num_delays = 0;
		list<Id> loop_path = getPortionOfPath(contained.front(),
				divergent_proc, contained);
		for (list<Id>::iterator iit = loop_path.begin(); iit != loop_path.end(); ++iit){
			Process* current = processnetwork_->getProcess(*iit);
			if (current) if (current->type() == "delay"){
				num_delays++;
				continue;
			}
			if (!current){
				current = processnetwork_->getComposite(*iit);
				ParallelComposite* pcomp = dynamic_cast<ParallelComposite*>(current);
				if (pcomp->getName().getString() == "pcomp<delay>") {
					num_delays++;
					continue;
				}
			}

			std::map<CostType, unsigned long long> proc_costs =
					calculateCostInNetwork(current, true);
			loopcost += proc_costs[PROCESS_COST];

		}
		if (num_delays == 0){
			THROW_EXCEPTION(InvalidModelException, string()
					+ "Found a loop with no delays started by "
					+ divergent_proc.getString());
		}
		loopcost = loopcost / num_delays;

	return loopcost;
}

unsigned long long ModelModifierSysC::getSignalCost(Process* source, Process* target,
		bool sync) throw(){
	Leaf* leaf = dynamic_cast<Leaf*>(source);
	Composite* comp = dynamic_cast<Composite*>(source);

	if(leaf){
		list<Leaf::Port*> oports = leaf->getInPorts();
		for (list<Leaf::Port*>::iterator pit = oports.begin(); pit != oports.end(); ++pit){
			if((*pit)->getConnectedPort()->getProcess() == target)
				return (*pit)->getDataType().getArraySize() * (*pit)->getDataType().getTypeSize() *
						(source->getStream() == target->getStream() ? costs_.k_T2T : costs_.k_D2D);
		}
	}
	if(comp){
		list<Composite::IOPort*> oports = comp->getInIOPorts();
		for (list<Composite::IOPort*>::iterator pit = oports.begin(); pit != oports.end(); ++pit){
			if((*pit)->getConnectedPortOutside()->getProcess() == target)
				return (*pit)->getDataType().first.getArraySize() *
						(*pit)->getDataType().first.getTypeSize() *
						(source->getStream() == target->getStream() ? costs_.k_T2T : costs_.k_D2D);
		}
	}
	return 0;
}

std::string ModelModifierSysC::printVector(std::vector<Id> vector) throw(){
	string str = "";
	for (unsigned index = 0; index < vector.size() ; index++){
		str += vector[index].getString() + ", ";
	}
	return str;
}

ModelModifierSysC::DataPath::DataPath() throw() :
		is_loop_(false), input_process_(Id("")), output_process_(Id("")) {}

ModelModifierSysC::DataPath::~DataPath() throw() {}

bool ModelModifierSysC::DataPath::wasVisited(Id id) throw(){
	for (list<pair<Id, bool> >::iterator it = path_.begin(); it != path_.end(); ++it){
		if (id == it->first) return true;
	}
	return false;
}

std::list<std::list<Id> > ModelModifierSysC::DataPath::getContainedPaths() throw(){

	list<std::list<Id> > contained_paths;
	bool previous_was_contained = false;
	list<Id> current_contained;
	for (list<pair<Id, bool> >::iterator it = path_.begin(); it != path_.end(); ++it){
		if (it->second) {
			current_contained.push_back(it->first);
		}
		else if (previous_was_contained){
			contained_paths.push_back(current_contained);
			current_contained.clear();
		}
		previous_was_contained = it->second;
	}
	if (previous_was_contained) contained_paths.push_back(current_contained);

	return contained_paths	;
}

void ModelModifierSysC::DataPath::operator=(const DataPath& rhs) throw(){
	input_process_ = rhs.input_process_;
	output_process_ = rhs.output_process_;
	path_ = rhs.path_;
}

string ModelModifierSysC::DataPath::printDataPath() throw(){
	string str = "";
	str += "\n";
	str += (is_loop_ ? "* Loop Path = " : "* Data Path = ");
	str += "(" + input_process_.getString() + ")->";
	for (list<pair<Id, bool> >::iterator it = path_.begin(); it != path_.end(); ++it){
		str += it->first.getString() + "->";
	}
	str += "(" + output_process_.getString() + ")\n";
	str += "* Contained sections = { ";
	list<list<Id> > contained = getContainedPaths();
	for (list<list<Id> >::iterator it = contained.begin(); it != contained.end(); ++it){
		str += "[";
		for (list<Id>::iterator itl = (*it).begin(); itl != (*it).end(); ++itl){
			str += itl->getString() + ",";
		}
		str += "] ";
	}
	str += "}";
	return str;
}


