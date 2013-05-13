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
			found_name_at->second.push_back(*it_comp);
		}
		else {
			list<Process*> new_list;
			new_list.push_back(*it_comp);
			list_of_equivalences.insert(pair<string, list<Process*> >(
					current_component_name, new_list));
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
				found_func_at->second.push_back(curr_comb);
			}
			else {
				list<Process*> new_list;
				new_list.push_back(curr_comb);
				list_of_equivalences.insert(pair<string, list<Process*> >(
						pointed_function_name, new_list));
			}
		}
	}
    }

    map<string, list<Process*> >::iterator it_list;
	for (it_list = list_of_equivalences.begin(); it_list != list_of_equivalences.end(); ++it_list){
		if (it_list->second.size() > configuration_.getCosts().min_parallel){
			grouped_equivalent_processes.push_back(it_list->second);
		}
	}
	//TODO: add OutOfMemory guards
    return grouped_equivalent_processes;
}


void ModelModifierSysC::createParallelComposite(Composite* parent, list<Process*> equivalent_processes)
              throw(InvalidProcessException,InvalidArgumentException, OutOfMemoryException){
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }

    //create the process
    Hierarchy parent_hierarchy =  parent->getHierarchy();
    ParallelComposite* new_pcomp = new ParallelComposite(parent->getUniqueCompositeId(),parent_hierarchy,
    		Id(""), 0);

	//Extract the base process



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
