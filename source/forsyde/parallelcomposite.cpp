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
		Composite(id, hierarchy, name), number_of_processes_(number_of_processes){}

ParallelComposite::~ParallelComposite() throw() {}

int ParallelComposite::getNumberOfProcesses() throw() {
    return number_of_processes_;
}

void ParallelComposite::setNumberOfProcesses(int number_of_processes) throw() {
	number_of_processes_ = number_of_processes;
}


bool ParallelComposite::addInConduit(Process::Interface* conduit) throw(OutOfMemoryException) {
    if (!conduit) {
        THROW_EXCEPTION(InvalidArgumentException, "\"conduit\" must not be NULL");
    }

    if (!conduit) return false;
    if (findConduit(conduit, in_conduits_) != in_conduits_.end()) return false;

    try {
    	in_conduits_.push_back(conduit);
        return true;
    }
    catch (bad_alloc& ex) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool ParallelComposite::deleteInConduit(Process::Interface* port) throw() {
    list<Process::Interface*>::iterator it = findConduit(port, in_conduits_);
    if (!port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"port\" must not be NULL");
    }

    if (it != in_conduits_.end()) {
    	in_conduits_.erase(it);
        return true;
    }
    else {
        return false;
    }
}
size_t ParallelComposite::getNumInConduits() const throw() {
    return in_conduits_.size();
}

Process::Interface* ParallelComposite::getInConduit(Process::Interface* port) throw() {
	list<Process::Interface*>::iterator it = (findConduit(port,in_conduits_));
    if (it != in_conduits_.end()) {
        return *it;
    }
    return NULL;
}


std::list<Process::Interface*> ParallelComposite::getInConduits() throw() {
    return in_conduits_;
}

bool ParallelComposite::addOutConduit(Process::Interface* port)
    throw(InvalidArgumentException, OutOfMemoryException) {
    if (!port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"port\" must not be NULL");
    }

    if (!port) return false;
    if (findConduit(port, out_conduits_) != out_conduits_.end()) return false;

    try {
    	out_conduits_.push_back(port);
        return true;
    }
    catch (bad_alloc& ex) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool ParallelComposite::deleteOutConduit(Process::Interface* port) throw(InvalidArgumentException) {
    list<Process::Interface*>::iterator it = findConduit(port, out_conduits_);
    if (!port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"port\" must not be NULL");
    }

    if (it != out_conduits_.end()) {
    	out_conduits_.erase(it);
        return true;
    }
    else {
        return false;
    }
}

size_t ParallelComposite::getNumOutConduits() const throw() {
    return out_conduits_.size();
}

Process::Interface* ParallelComposite::getOutConduit(Process::Interface* port) throw() {
	list<Process::Interface*>::iterator it = (findConduit(port,out_conduits_));
    if (it != out_conduits_.end()) {
        return *it;
    }
    return NULL;
}

std::list<Process::Interface*> ParallelComposite::getOutConduits() throw() {
    return out_conduits_;
}

/*
string ParallelComposite::toString() const throw() {
    string str;
    str += "{\n";
    str += " Composite Process ID: ";
    str += getId()->getString();
    str += ",\n";
    str += " Name: ";
    str += getName().getString();
    str += ",\n";
    str += " NumInPorts: ";
    str += tools::toString(getNumInIOPorts());
    str += ",\n";
    str += " InPorts = {";
    str += portsToString(in_ports_);
    str += "}";
    str += ",\n";
    str += " NumOutPorts: ";
    str += tools::toString(getNumOutIOPorts());
    str += ",\n";
    str += " OutPorts = {";
    str += portsToString(out_ports_);
    str += "}";

    return str;
}*/

list<Process::Interface*>::iterator ParallelComposite::findConduit(
    const Id& id, list<Process::Interface*>& conduits) const throw() {
    list<Process::Interface*>::iterator it;
    for (it = conduits.begin(); it != conduits.end(); ++it) {
        if (*(*it)->getId() == id) {
            return it;
        }
    }

    // No such port was found
    return it;
}

list<Process::Interface*>::iterator ParallelComposite::findConduit(
    Process::Interface* conduit, std::list<Process::Interface*>& conduits) const throw() {
    list<Process::Interface*>::iterator it;
    for (it = conduits.begin(); it != conduits.end(); ++it) {
        if (*it == conduit) {
            return it;
        }
    }

    // No such port was found
    return it;
}

string ParallelComposite::conduitsToString(const list<Process::Interface*> conduits) const throw() {
    string str;
    if (conduits.size() > 0) {
        str += "\n";
        bool first = true;
        for (list<Process::Interface*>::const_iterator it = conduits.begin();
             it != conduits.end(); ++it) {
            if (!first) {
                str += ",\n";
            }
            else {
                first = false;
            }

            Process::Interface* port = *it;
            str += "  ID: ";
            str += port->getId()->getString();
            str += ": ";
            str += port->toString();
        }
        str += "\n ";
    }
    return str;
}

bool ParallelComposite::operator==(const ParallelComposite& rhs) const throw() {
	if (getName() != rhs.getName()) return false;

    return true;
}

bool ParallelComposite::operator!=(const ParallelComposite& rhs) const throw() {
    return !operator==(rhs);
}

string ParallelComposite::type() const throw() {
    return "parallel composite";
}

void ParallelComposite::moreChecks() throw(InvalidProcessException){
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

