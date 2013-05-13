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

#include "parallelcomposite.h"
#include "leaf.h"
#include "../tools/tools.h"
#include <new>
#include <map>
#include <vector>

using namespace f2cc::Forsyde;
using std::string;
using std::list;
using std::bad_alloc;
using std::vector;

ParallelComposite::ParallelComposite(const Forsyde::Id& id,
		Forsyde::Hierarchy& hierarchy, Forsyde::Id name, int number_of_processes) throw():
		Composite(id, hierarchy, name), number_of_processes_(number_of_processes),
		contained_process_id_(NULL){}

ParallelComposite::~ParallelComposite() throw() {}

int ParallelComposite::getNumberOfProcesses() throw() {
    return number_of_processes_;
}

void ParallelComposite::setNumberOfProcesses(int number_of_processes) throw() {
	number_of_processes_ = number_of_processes;
}

Id* ParallelComposite::getContainedProcessId() throw() {
    return contained_process_id_;
}

void ParallelComposite::setContainedProcessId(Id* id) throw() {
	contained_process_id_ = id;
}

string ParallelComposite::type() const throw() {
    return "parallel composite";
}

void ParallelComposite::moreChecks() throw(InvalidProcessException){

	//TODO: check inner and outer data type.
    if (getInIOPorts().size() == 0) {
        THROW_EXCEPTION(InvalidProcessException, string("Process \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\" must have at least one (1) in port");
    }
    if (getOutIOPorts().size() == 0) {
        THROW_EXCEPTION(InvalidProcessException, string("Process \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\" must have at least one (1) out port");
    }
    if ((getProcesses().size() == 0) && (getComposites().size() == 0)) {
        THROW_EXCEPTION(InvalidProcessException, string("Process \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\" must have at least one (1) process");
    }
}

