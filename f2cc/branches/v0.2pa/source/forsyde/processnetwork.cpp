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

#include "processnetwork.h"
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

ProcessNetwork::ProcessNetwork() throw() {}

ProcessNetwork::~ProcessNetwork() throw() {
    destroyAllLeafs();
}

bool ProcessNetwork::addInput(Leaf::Interface* interface)
    throw(InvalidArgumentException, IllegalStateException,
          OutOfMemoryException) {
    if (!interface) {
        THROW_EXCEPTION(InvalidArgumentException, "\"interface\" must not be NULL");
    }

    if (!interface) return false;
    if (findInterface(interface, inputs_) != inputs_.end()) return false;

    try {
        inputs_.push_back(interface);
        return true;
    }
    catch (bad_alloc& ex) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool ProcessNetwork::deleteInput(Leaf::Interface* interface) throw(InvalidArgumentException) {
    list<Leaf::Interface*>::iterator it = findInterface(interface, inputs_);
    if (!interface) {
        THROW_EXCEPTION(InvalidArgumentException, "\"interface\" must not be NULL");
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

std::list<Leaf::Interface*> ProcessNetwork::getInputs() throw() {
    return inputs_;
}

bool ProcessNetwork::addOutput(Leaf::Interface* interface)
    throw(InvalidArgumentException, IllegalStateException,
          OutOfMemoryException) {
    if (!interface) {
        THROW_EXCEPTION(InvalidArgumentException, "\"interface\" must not be NULL");
    }

    if (!interface) return false;
    if (findInterface(interface, outputs_) != outputs_.end()) return false;

    try {
        outputs_.push_back(interface);
        return true;
    }
    catch (bad_alloc& ex) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool ProcessNetwork::deleteOutput(Leaf::Interface* interface) throw(InvalidArgumentException) {
    list<Leaf::Interface*>::iterator it = findInterface(interface, outputs_);
    if (!interface) {
        THROW_EXCEPTION(InvalidArgumentException, "\"interface\" must not be NULL");
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

std::list<Leaf::Interface*> ProcessNetwork::getOutputs() throw() {
    return outputs_;
}

list<Leaf::Interface*>::iterator ProcessNetwork::findInterface(
    const Id& id, list<Leaf::Interface*>& interfaces) const throw() {
    list<Leaf::Interface*>::iterator it;
    for (it = interfaces.begin(); it != interfaces.end(); ++it) {
        if (*(*it)->getId() == id) {
            return it;
        }
    }

    // No such interface was found
    return it;
}

list<Leaf::Interface*>::iterator ProcessNetwork::findInterface(
    Leaf::Interface* interface, std::list<Leaf::Interface*>& interfaces) const throw() {
    list<Leaf::Interface*>::iterator it;
    for (it = interfaces.begin(); it != interfaces.end(); ++it) {
        if (*it == interface) {
            return it;
        }
    }

    // No such interface was found
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
    str += interfacesToString(inputs_);
    str += "}";
    str += ",\n";
    str += " NumOutputs: ";
    str += tools::toString(getNumOutputs());
    str += ",\n";
    str += " Outputs = {";
    str += interfacesToString(outputs_);
    str += "}\n";
    str += "}";
    return str;
}

string ProcessNetwork::interfacesToString(const list<Leaf::Interface*> interfaces) const throw() {
    string str;
    if (interfaces.size() > 0) {
        str += "\n";
        bool first = true;
        for (list<Leaf::Interface*>::const_iterator it = interfaces.begin();
             it != interfaces.end(); ++it) {
            if (!first) {
                str += ",\n";
            }
            else {
                first = false;
            }

            Leaf::Interface* interface = *it;
            str += "  ID: ";
            str += interface->getId()->getString();
            str += ", ";
            str += "belonging to ";
            str += interface->getLeaf()->getId()->getString();
        }
        str += "\n ";
    }
    return str;
}
