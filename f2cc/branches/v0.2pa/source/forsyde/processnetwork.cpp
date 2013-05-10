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

#include "processnetwork.h"
#include "../tools/tools.h"
#include <map>
#include <list>
#include <new>

using namespace f2cc::Forsyde;
using namespace f2cc;
using std::string;
using std::map;
using std::list;
using std::pair;
using std::bad_alloc;

ProcessNetwork::ProcessNetwork() throw() :
	Model() {}

ProcessNetwork::~ProcessNetwork() throw() {
	destroyAllFunctions();
}

bool ProcessNetwork::addInput(Process::Interface* port)
    throw(InvalidArgumentException, IllegalStateException,
          OutOfMemoryException) {
    if (!port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"port\" must not be NULL");
    }

    if (!port) return false;
    if (findPort(port, inputs_) != inputs_.end()) return false;

    try {
        inputs_.push_back(port);
        return true;
    }
    catch (bad_alloc& ex) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool ProcessNetwork::deleteInput(Process::Interface* port) throw(InvalidArgumentException) {
    list<Process::Interface*>::iterator it = findPort(port, inputs_);
    if (!port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"port\" must not be NULL");
    }

    if (it != inputs_.end()) {
        inputs_.erase(it);
        return true;
    }
    else {
        return false;
    }
}

int ProcessNetwork::getNumInputs() const throw() {
    return inputs_.size();
}

std::list<Process::Interface*> ProcessNetwork::getInputs() throw() {
    return inputs_;
}

bool ProcessNetwork::addOutput(Process::Interface* port)
    throw(InvalidArgumentException, IllegalStateException,
          OutOfMemoryException) {
    if (!port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"port\" must not be NULL");
    }

    if (!port) return false;
    if (findPort(port, outputs_) != outputs_.end()) return false;

    try {
        outputs_.push_back(port);
        return true;
    }
    catch (bad_alloc& ex) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool ProcessNetwork::deleteOutput(Process::Interface* port) throw(InvalidArgumentException) {
    list<Process::Interface*>::iterator it = findPort(port, outputs_);
    if (!port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"port\" must not be NULL");
    }

    if (it != outputs_.end()) {
        outputs_.erase(it);
        return true;
    }
    else {
        return false;
    }
}

int ProcessNetwork::getNumOutputs() const throw() {
    return outputs_.size();
}

std::list<Process::Interface*> ProcessNetwork::getOutputs() throw() {
    return outputs_;
}

bool ProcessNetwork::addFunction(CFunction* function)
    throw(InvalidArgumentException, OutOfMemoryException) {
    if (!function) {
        THROW_EXCEPTION(InvalidArgumentException, "\"function\" must not be "
                        "NULL");
    }
    try {
        pair<map<const Id, CFunction*>::iterator, bool>
            result = functions_.insert(pair<const Id, CFunction*>(
                    Id(function->getName()), function));
        return result.second;
    }
    catch(bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

void ProcessNetwork::addFunctions(map<const Id, CFunction*> functions)
    throw(OutOfMemoryException) {
    try {
        functions_.insert(functions.begin(), functions.end());
    }
    catch(bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

CFunction* ProcessNetwork::getFunction(const Id& id) throw() {
    map<const Id, CFunction*>::iterator it = findFunction(id);
    return it != functions_.end() ? it->second : NULL;
}

int ProcessNetwork::getNumFunctions() const throw() {
    return functions_.size();
}

list<CFunction*> ProcessNetwork::getFunctions() throw() {
    list<CFunction*> functions;
    map<const Id, CFunction*>::iterator it;
    for (it = functions_.begin(); it != functions_.end(); ++it) {
        functions.push_back(it->second);
    }
    return functions;
}

bool ProcessNetwork::deleteFunction(const Id& id) throw() {
    map<const Id, CFunction*>::iterator it = findFunction(id);
    if (it != functions_.end()) {
    	CFunction* removed_function = it->second;
        functions_.erase(it);
        delete removed_function;
        return true;
    }
    else {
        return false;
    }
}

list<Process::Interface*>::iterator ProcessNetwork::findPort(
    const Id& id, list<Process::Interface*>& ports) const throw() {
    list<Process::Interface*>::iterator it;
    for (it = ports.begin(); it != ports.end(); ++it) {
        if (*(*it)->getId() == id) {
            return it;
        }
    }

    // No such port was found
    return it;
}

list<Process::Interface*>::iterator ProcessNetwork::findPort(
    Process::Interface* port, std::list<Process::Interface*>& ports) const throw() {
    list<Process::Interface*>::iterator it;
    for (it = ports.begin(); it != ports.end(); ++it) {
        if (*it == port) {
            return it;
        }
    }

    // No such port was found
    return it;
}

std::string ProcessNetwork::toString() const throw() {
    string str;
    str += "{\n";
    str += " ProcessNetwork Module\n";
    str += " NumInputs: ";
    str += tools::toString(getNumInputs());
    str += ",\n";
    str += " Inputs = {";
    str += portsToString(inputs_);
    str += "}";
    str += ",\n";
    str += " NumOutputs: ";
    str += tools::toString(getNumOutputs());
    str += ",\n";
    str += " Outputs = {";
    str += portsToString(outputs_);
    str += "}\n";
    str += " NumFunctions: ";
    str += tools::toString(getNumFunctions());
    str += ",\n";
    str += "}";
    return str;
}

string ProcessNetwork::portsToString(const list<Process::Interface*> ports) const throw() {
    string str;
    if (ports.size() > 0) {
        str += "\n";
        bool first = true;
        for (list<Process::Interface*>::const_iterator it = ports.begin();
             it != ports.end(); ++it) {
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

void ProcessNetwork::destroyAllFunctions() throw() {
    map<const Id, CFunction*>::iterator it;
    for (it=functions_.begin(); it != functions_.end(); ++it) {
        delete it->second;
    }
}

map<const Id, CFunction*>::iterator ProcessNetwork::findFunction(const Id& id) throw() {
    return functions_.find(id);
}
