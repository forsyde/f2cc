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
#include <new>
#include <stdexcept>

using namespace f2cc;
using namespace f2cc::Forsyde;
using namespace f2cc::Forsyde::SY;
using std::string;
using std::list;
using std::map;
using std::set;
using std::vector;
using std::bad_alloc;
using std::pair;

ModelModifierSysC::ModelModifierSysC(ProcessNetwork* processnetwork,
		Logger& logger, Config& config)
        throw(InvalidArgumentException) : processnetwork_(processnetwork), logger_(logger),
        configuration_(config){
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }
}

ModelModifierSysC::~ModelModifierSysC() throw() {}


void ModelModifierSysC::flattenAndParallelize() throw(
    		RuntimeException, InvalidModelException, InvalidProcessException){

	logger_.logMessage(Logger::INFO, "Flattening the process network while "
			"extracting data parallel processes ...");

	Composite* root = processnetwork_->getComposite(Id("f2cc0"));
	if(!root){
        THROW_EXCEPTION(InvalidModelException, string("Process network ")
                        + "does not have a root process");
	}
	list<list<Process*> > equivalent_processes_in_root = extractEquivalentProcesses(root);
	list<list<Process*> >::iterator equ_it;
	for (equ_it = equivalent_processes_in_root.begin(); equ_it != equivalent_processes_in_root.end(); ++equ_it){
		createParallelComposite(root,*equ_it);
	}

}

list<list<Process*> > ModelModifierSysC::extractEquivalentProcesses(Composite* parent)
        throw(InvalidArgumentException, OutOfMemoryException){
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }

    logger_.logMessage(Logger::DEBUG, string() + "Extracting equivalent processes from "
    			+ parent->getId()->getString() + "...");

    list<list<Process*> > grouped_equivalent_processes;
    map<string, list<Process*> > list_of_equivalences;

    {
	list<Composite*> list_of_composites = parent->getComposites();
	list<Composite*>::iterator it_comp;

	for (it_comp = list_of_composites.begin(); it_comp != list_of_composites.end(); ++it_comp){
		string current_component_name = (*it_comp)->getName().getString();
		map<string, list<Process*> >::iterator found_name_at =
				list_of_equivalences.find(current_component_name);
		if (found_name_at != list_of_equivalences.end()){
		    try {
		    	found_name_at->second.push_back(*it_comp);;
		    }
		    catch(bad_alloc&) {
		        THROW_EXCEPTION(OutOfMemoryException);
		    }
		}
		else {
			list<Process*> new_list;
			new_list.push_back(*it_comp);
		    try {
		    	list_of_equivalences.insert(pair<string, list<Process*> >(current_component_name, new_list));
		    }
		    catch(bad_alloc&) {
		        THROW_EXCEPTION(OutOfMemoryException);
		    }
		}
	}
    }
    {
	list<Leaf*> list_of_leafs = parent->getProcesses();
	list<Leaf*>::iterator it_leafs;

	for (it_leafs = list_of_leafs.begin(); it_leafs != list_of_leafs.end(); ++it_leafs){
		SY::Comb* curr_comb = dynamic_cast<SY::Comb*>(*it_leafs);
		if(curr_comb){
			string pointed_function_name = curr_comb->getFunction()->getName();
			map<string, list<Process*> >::iterator found_func_at =
					list_of_equivalences.find(pointed_function_name);
			if (found_func_at != list_of_equivalences.end()){
			    try {
			    	found_func_at->second.push_back(curr_comb);
			    }
			    catch(bad_alloc&) {
			        THROW_EXCEPTION(OutOfMemoryException);
			    }
			}
			else {
				list<Process*> new_list;
				new_list.push_back(curr_comb);
			    try {
			    	list_of_equivalences.insert(pair<string, list<Process*> >(pointed_function_name,
			    			new_list));
			    }
			    catch(bad_alloc&) {
			        THROW_EXCEPTION(OutOfMemoryException);
			    }
			}
		}
	}
    }

    map<string, list<Process*> >::iterator it_list;
	for (it_list = list_of_equivalences.begin(); it_list != list_of_equivalences.end(); ++it_list){
		if (it_list->second.size() > 1){
			logger_.logMessage(Logger::DEBUG, string() + "Found "
						+ tools::toString(it_list->second.size())
						+ " equivalent processes. Adding them to the list...");
		    try {
		    	grouped_equivalent_processes.push_back(it_list->second);
		    }
		    catch(bad_alloc&) {
		        THROW_EXCEPTION(OutOfMemoryException);
		    }
		}
	}
    return grouped_equivalent_processes;
}


void ModelModifierSysC::createParallelComposite(Composite* parent, list<Process*> equivalent_processes)
              throw(RuntimeException, InvalidProcessException,InvalidArgumentException, OutOfMemoryException){
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }

    //count the processes
    unsigned number_of_processes = 0;
	list<Process*>::iterator it_list;
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

	logger_.logMessage(Logger::DEBUG, string() + "Creating a parallel composite process from \""
				+ reference_process->getId()->getString()
				+ "\" with " + tools::toString(number_of_processes) + " processes...");

    Leaf* leaf_ref = dynamic_cast<Leaf*>(reference_process);
    if (leaf_ref) prepareLeafForParallel(leaf_ref, parent, new_pcomp, number_of_processes);
    else {
    	ParallelComposite* pcomp_ref = dynamic_cast<ParallelComposite*>(reference_process);
    	if (pcomp_ref) prepareParallelCompositeForParallel(pcomp_ref, parent, new_pcomp, number_of_processes);
    	else {
    		Composite* comp_ref = dynamic_cast<Composite*>(reference_process);
    		if (comp_ref) prepareCompositeForParallel(comp_ref, parent, new_pcomp, number_of_processes);
			else {
				THROW_EXCEPTION(CastException, string("Process \"")
								+ reference_process->getId()->getString()
								+ "\" is not leaf, composite or parallel_composite");
			}
    	}
    }

    std::cout<<"ParallelComposite "<<new_pcomp->toString()<< "\nHas now "<<
    		new_pcomp->getComposites().size()<< "composites and "<< new_pcomp->getProcesses().size()<<
    		" leafs: \n"<< new_pcomp->getComposite(*new_pcomp->getContainedProcessId())->toString();

    //redirect the data path through the parallel composite.
    for (it_list = equivalent_processes.begin(); it_list != equivalent_processes.end(); ++it_list){
    	redirectFlow(*it_list, parent, new_pcomp);
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

void ModelModifierSysC::prepareCompositeForParallel(Composite* reference_comp, Composite* parent,
		 ParallelComposite* new_pcomp, unsigned number_of_processes)
	  throw(RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException){
    if (!reference_comp) {
        THROW_EXCEPTION(InvalidArgumentException, "\"reference_comp\" must not be NULL");
    }
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }
    if (!new_pcomp) {
        THROW_EXCEPTION(InvalidArgumentException, "\"new_pcomp\" must not be NULL");
    }

    //copy the leaf to the ParallelComposite
    moveToParallelComposite(reference_comp, parent, new_pcomp);

    Hierarchy parent_hierarchy = parent->getHierarchy();
    list<Composite::IOPort*> in_ports = reference_comp->getInIOPorts();
    for (list<Composite::IOPort*>::iterator it = in_ports.begin(); it != in_ports.end(); ++it){
    	Composite::IOPort* composite_port = *it;

    	//add new IOPort to pcomp with array outside and scalar inside
    	new_pcomp->addInIOPort(*composite_port->getId(), composite_port->getDataType().first);
    	Composite::IOPort* new_pcomp_port = new_pcomp->getInIOPort(*(*it)->getId());
    	CDataType type_outside = CDataType(composite_port->getDataType().first.getType(),
				true, true, (composite_port->getDataType().first.getArraySize() * number_of_processes),
				false, true);
    	new_pcomp_port->setDataType(true,type_outside);

    	//creating a new Zipx and connect it to the new pcomp port
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
        Process::Interface* connected_port = composite_port->getConnectedPortOutside();
        Id new_port_id = Id("port_to_" + composite_port->getProcess()->getId()->getString());
        new_zip->addInPort(new_port_id, composite_port->getDataType().first);
		Leaf::Port* new_zip_iport = new_zip->getInPort(new_port_id);
		new_zip_iport->connect(connected_port);
		logger_.logMessage(Logger::DEBUG, string() + "Redirected \""
							+ connected_port->toString()
							+ "\" to \""
							+ new_zip_iport->toString() + "\"");

		//finally connect the inner pcomp ioport to the now-free leaf port
		new_pcomp_port->connect(composite_port);
		logger_.logMessage(Logger::DEBUG, string() + "Finally, connected \""
							+ composite_port->toString()
							+ "\" to \""
							+ new_pcomp_port->toString() + "\"");
    }

    list<Composite::IOPort*> out_ports = reference_comp->getOutIOPorts();
    for (list<Composite::IOPort*>::iterator it = out_ports.begin(); it != out_ports.end(); ++it){
    	reference_comp->addOutIOPort(*(*it)->getId(), (*it)->getDataType().first);
    	Composite::IOPort* composite_port = *it;

    	//add new IOPort to pcomp with array outside and scalar inside
    	new_pcomp->addOutIOPort(*composite_port->getId(), composite_port->getDataType().first);
    	Composite::IOPort* new_pcomp_port = new_pcomp->getOutIOPort(*(*it)->getId());
    	CDataType type_outside = CDataType(composite_port->getDataType().first.getType(),
				true, true, (composite_port->getDataType().first.getArraySize() * number_of_processes),
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
        Process::Interface* connected_port = composite_port->getConnectedPortOutside();
        Id new_port_id = Id("port_from_" + composite_port->getProcess()->getId()->getString());
        new_unzip->addOutPort(new_port_id, composite_port->getDataType().first);
		Leaf::Port* new_unzip_oport = new_unzip->getOutPort(new_port_id);
		new_unzip_oport->connect(connected_port);
		logger_.logMessage(Logger::DEBUG, string() + "Redirected \""
							+ connected_port->toString()
							+ "\" to \""
							+ new_unzip_oport->toString() + "\"");

		//finally connect the inner pcomp ioport to the now-free leaf port
		new_pcomp_port->connect(composite_port);
		logger_.logMessage(Logger::DEBUG, string() + "Finally, connected \""
							+ composite_port->toString()
							+ "\" to \""
							+ new_pcomp_port->toString() + "\"");
    }
}


void ModelModifierSysC::prepareParallelCompositeForParallel(ParallelComposite* reference_pcomp,
		 Composite* parent, ParallelComposite* new_pcomp, unsigned number_of_processes)
	  throw(RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException){

    if (!reference_pcomp) {
        THROW_EXCEPTION(InvalidArgumentException, "\"reference_pcomp\" must not be NULL");
    }
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }
    if (!new_pcomp) {
        THROW_EXCEPTION(InvalidArgumentException, "\"new_pcomp\" must not be NULL");
    }

	Leaf* contained_leaf = reference_pcomp->getProcess(*reference_pcomp->getContainedProcessId());

	if (contained_leaf) prepareLeafForParallel(contained_leaf, parent, new_pcomp, number_of_processes);
	else {
		Composite* contained_comp = reference_pcomp->getComposite(*reference_pcomp->getContainedProcessId());
		if (contained_comp) prepareCompositeForParallel(contained_comp, parent, new_pcomp,
				number_of_processes);
		else {
			THROW_EXCEPTION(CastException, string("Process \"")
							+ reference_pcomp->getId()->getString()
							+ "\" contains not leaf nor composite");
		}
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
    	else new_parent->changeName(Id(string() + "pcomp_" + new_leaf->getId()->getString()));

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

    logger_.logMessage(Logger::DEBUG, string() + "Moved process  \""
				+ reference_process->getId()->getString()
				+ "\" of type " + reference_process->type()
				+ "  to its new parent \""
				+ new_parent->getId()->getString() + "\"");

}


void ModelModifierSysC::redirectFlow(Process* old_process, Composite* parent,
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
								+ old_process->getId()->getString()
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

		    logger_.logMessage(Logger::DEBUG, string() + "Redirected \""
						+ connected_port->toString()
						+ "\" to \""
						+ new_port->toString() + "\"");
		}
    	list<Composite::IOPort*> out_ports = comp->getOutIOPorts();
		for (list<Composite::IOPort*>::iterator it = out_ports.begin(); it != out_ports.end(); ++it){
			Process* connected_proc = new_pcomp->getOutIOPort(*(*it)->getId())->getConnectedPortOutside(
					)->getProcess();
			SY::Unzipx* out_unzip = dynamic_cast<SY::Unzipx*>(connected_proc);
			if (!out_unzip){
				THROW_EXCEPTION(InvalidProcessException,
								string("Process \"")
								+ old_process->getId()->getString()
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


		    logger_.logMessage(Logger::DEBUG, string() + "Redirected \""
						+ connected_port->toString()
						+ "\" to \""
						+ new_port->toString() + "\"");
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
								+ old_process->getId()->getString()
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

		    logger_.logMessage(Logger::DEBUG, string() + "Redirected \""
						+ connected_port->toString()
						+ "\" to \""
						+ new_port->toString() + "\"");
		}
    	list<Leaf::Port*> out_ports = leaf->getOutPorts();
		for (list<Leaf::Port*>::iterator it = out_ports.begin(); it != out_ports.end(); ++it){
			Process* connected_proc = new_pcomp->getOutIOPort(*(*it)->getId())->getConnectedPortOutside(
					)->getProcess();
			SY::Unzipx* out_unzip = dynamic_cast<SY::Unzipx*>(connected_proc);
			if (!out_unzip){
				THROW_EXCEPTION(InvalidProcessException,
								string("Process \"")
								+ old_process->getId()->getString()
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

		    logger_.logMessage(Logger::DEBUG, string() + "Redirected \""
						+ connected_port->toString()
						+ "\" to \""
						+ new_port->toString() + "\"");
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

///////////////////////////////////////////////////////////////
/*
ModelModifierSysC::ContainedSection::ContainedSection(Process* start, Process* end)
        throw(InvalidArgumentException) : start(start), end(end) {
    if (!start) {
        THROW_EXCEPTION(InvalidArgumentException, "\"start\" must not be NULL");
    }
    if (!end) {
        THROW_EXCEPTION(InvalidArgumentException, "\"end\" must not be NULL");
    }
}

string ModelModifierSysC::ContainedSection::toString() const throw() {
    return string("\"") + start->getId()->getString() + "--" +
        end->getId()->getString() + "\"";
}
*/
