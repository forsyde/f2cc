/*
 * Copyright (c) 2011-2013 Gabriel Hjort Blindell <ghb@kth.se>
 *                          George Ungureanu <ugeorge@kth.se>
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

#include "processnetwork.h"
#include "../tools/tools.h"
#include <map>
#include <list>
#include <new>

using namespace f2cc;
using namespace f2cc::ForSyDe;
using std::string;
using std::map;
using std::list;
using std::pair;
using std::bad_alloc;

Processnetwork::Processnetwork(const Id& name) throw() :
		Composite(Id("f2cc0"), name) {}

Processnetwork::~Processnetwork() throw() {
	destroyAllFunctions();
}


string Processnetwork::type() const throw() {
    return "composite";
}

bool Processnetwork::addFunction(CFunction* function)
    throw(InvalidArgumentException, OutOfMemoryException) {
    if (!function) {
        THROW_EXCEPTION(InvalidArgumentException, "\"function\" must not be "
                        "NULL");
    }
    if (findFunction(function->getName(), process_functions_) !=
    		process_functions_.end()) return false;

    try {
    	process_functions_.push_back(function);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

CFunction* Processnetwork::getFunction(std::string name) throw() {
    list<CFunction*>::iterator it = findFunction(name, process_functions_);
    if (it != process_functions_.end()) {
        return *it;
    }
    else {
        return NULL;
    }
}

size_t Processnetwork::getNumFunctions() const throw() {
    return process_functions_.size();
}

list<CFunction*> Processnetwork::getFunctions() throw() {
    return process_functions_;
}

bool Processnetwork::deleteFunction(std::string name) throw() {
    list<CFunction*>::iterator it = findFunction(name, process_functions_);
    if (it != process_functions_.end()) {
    	CFunction* removed_function = *it;
    	process_functions_.erase(it);
        delete removed_function;
        return true;
    }
    else {
        return false;
    }
}

/*
std::string Processnetwork::toString() const throw() {
    string str;
    str += "{\n";
    str += " Process Network\n";
    str += " NumInputs: ";
    str += tools::toString(getNumInPorts());
    str += ",\n";
 //   str += " Inputs = {";
 //   str += portsToString(in_ports_);
 //   str += "}";
    str += ",\n";
    str += " NumOutputs: ";
    str += tools::toString(getNumOutPorts());
    str += ",\n";
 //   str += " Outputs = {";
 //   str += portsToString(out_ports_);
 //   str += "}\n";
    str += " NumProcesses: ";
    str += tools::toString(getNumProcesses());
    str += ",\n";
    str += " NumFunctions: ";
    str += tools::toString(getNumFunctions());
    str += ",\n";
    str += "}";
    return str;
}
*/

/*

Process::Port* Processnetwork::getInPort(const Id& id) throw() {
    list<Composite::IOPort*>::iterator it = findPort(id, in_ports_);
    if (it != in_ports_.end()) {
        return (*it)->getConnectedPortInside();
    }
    else {
        return NULL;
    }
}

list<Process::Port*> Processnetwork::getInPorts() throw() {
	list<Process::Port*> connection_list;
	list<Composite::IOPort*>::iterator it;
	for (it = in_ports_.begin(); it != in_ports_.end(); ++it) {
		connection_list.push_back((*it)->getConnectedPort());
	}
    return connection_list;
}

Process::Port* Processnetwork::getOutPort(const Id& id) throw() {
    list<Composite::IOPort*>::iterator it = findPort(id, out_ports_);
    if (it != out_ports_.end()) {
        return (*it)->getConnectedPortInside();
    }
    else {
        return NULL;
    }
}

list<Process::Port*> Processnetwork::getOutPorts() throw() {
	list<Process::Port*> connection_list;
	list<Composite::IOPort*>::iterator it;
	for (it = out_ports_.begin(); it != out_ports_.end(); ++it) {
		connection_list.push_back((*it)->getConnectedPort());
	}
    return connection_list;
}

list<Composite::IOPort*>::iterator Processnetwork::findPort(
    const Id& id, list<Composite::IOPort*>& ports) const throw() {
    list<Composite::IOPort*>::iterator it;
    for (it = ports.begin(); it != ports.end(); ++it) {
        if (*(*it)->getId() == id) {
            return it;
        }
    }

    // No such port was found
    return it;
}

*/

list<CFunction*>::iterator Processnetwork::findFunction(const string name,
                                        list<CFunction*>& functions) const throw(){
    list<CFunction*>::iterator it;
    for (it = functions.begin(); it != functions.end(); ++it) {
        if ((*it)->getName() == name) {
            return it;
        }
    }

    // No such port was found
    return it;
}


void Processnetwork::destroyAllFunctions() throw() {
    while (process_functions_.size() > 0) {
    	CFunction* function = process_functions_.front();
        process_functions_.pop_front();
        delete function;
    }
}


