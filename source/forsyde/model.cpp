/*
 * Copyright (c) 2011-2012 Gabriel Hjort Blindell <ghb@kth.se>
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

#include "model.h"
#include "../tools/tools.h"
#include <map>
#include <list>
#include <new>

using namespace f2cc::ForSyDe;
using std::string;
using std::map;
using std::list;
using std::pair;
using std::bad_alloc;

Model::Model() throw() {}

Model::~Model() throw() {
    destroyAllProcesses();
}

bool Model::addProcess(Process* process)
    throw(InvalidArgumentException, OutOfMemoryException) {
    if (!process) {
        THROW_EXCEPTION(InvalidArgumentException, "\"process\" must not be "
                        "NULL");
    }

    try {
        pair<map<const Id, Process*>::iterator, bool>
            result = processes_.insert(
                pair<const Id, Process*>(
                    *process->getId(), process));
        return result.second;
    }
    catch(bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

void Model::addProcesses(map<const Id, Process*> processes)
    throw(OutOfMemoryException) {
    try {
        processes_.insert(processes.begin(), processes.end());
    }
    catch(bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

Process* Model::getProcess(const Id& id) throw() {
    map<const Id, Process*>::iterator it = findProcess(id);
    return it != processes_.end() ? it->second : NULL;
}

int Model::getNumProcesses() const throw() {
    return processes_.size();
}

list<Process*> Model::getProcesses() throw() {
    list<Process*> processes;
    map<const Id, Process*>::iterator it;
    for (it = processes_.begin(); it != processes_.end(); ++it) {
        processes.push_back(it->second);
    }
    return processes;
}

bool Model::deleteProcess(const Id& id) throw() {
    map<const Id, Process*>::iterator it = findProcess(id);
    if (it != processes_.end()) {
        Process* removed_process = it->second;
        processes_.erase(it);
        delete removed_process;
        return true;
    }
    else {
        return false;
    }
}


Id Model::getUniqueProcessId() const throw() {
    return getUniqueProcessId("");
}

Id Model::getUniqueProcessId(const string& prefix) const throw() {
    for (int i = 1; ; ++i) {
        Id new_id = Id(prefix + tools::toString(i));
        if (processes_.find(new_id) == processes_.end()) return new_id;
    }
}

void Model::destroyAllProcesses() throw() {
    map<const Id, Process*>::iterator it;
    for (it=processes_.begin(); it != processes_.end(); ++it) {
        delete it->second;
    }
}

map<const Id, Process*>::iterator Model::findProcess(const Id& id) throw() {
    return processes_.find(id);
}


