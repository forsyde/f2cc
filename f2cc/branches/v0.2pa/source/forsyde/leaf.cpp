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

#include "leaf.h"
#include "../tools/tools.h"
#include <new>
#include <vector>

using namespace f2cc::ForSyDe;
using std::string;
using std::list;
using std::bad_alloc;
using std::vector;

Leaf::Leaf(const Id& id) throw() : id_(id) {}

Leaf::~Leaf() throw() {
    destroyAllInterfaces(in_interfaces_);
    destroyAllInterfaces(out_interfaces_);
}

const Id* Leaf::getId() const throw() {
    return &id_;
}

bool Leaf::addInPort(const Id& id) throw(OutOfMemoryException) {
    if (findInterface(id, in_interfaces_) != in_interfaces_.end()) return false;

    try {
        Interface* new_interface = new Interface(id, this);
        in_interfaces_.push_back(new_interface);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool Leaf::addInPort(Interface& interface) throw(OutOfMemoryException) {
    if (findInterface(*interface.getId(), in_interfaces_) != in_interfaces_.end()) return false;

    try {
        Interface* new_interface = new Interface(interface, this);
        in_interfaces_.push_back(new_interface);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool Leaf::deleteInPort(const Id& id) throw() {
    list<Interface*>::iterator it = findInterface(id, in_interfaces_);
    if (it != in_interfaces_.end()) {
        Interface* removed_interface = *it;
        in_interfaces_.erase(it);
        delete removed_interface;
        return true;
    }
    else {
        return false;
    }
}

size_t Leaf::getNumInPorts() const throw() {
    return in_interfaces_.size();
}

Leaf::Interface* Leaf::getInPort(const Id& id) throw() {
    list<Interface*>::iterator it = findInterface(id, in_interfaces_);
    if (it != in_interfaces_.end()) {
        return *it;
    }
    else {
        return NULL;
    }
}

list<Leaf::Interface*> Leaf::getInPorts() throw() {
    return in_interfaces_;
}

bool Leaf::addOutPort(const Id& id) throw(OutOfMemoryException) {
    if (findInterface(id, out_interfaces_) != out_interfaces_.end()) return false;

    try {
        Interface* new_interface = new Interface(id, this);
        out_interfaces_.push_back(new_interface);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool Leaf::addOutPort(Interface& interface) throw(OutOfMemoryException) {
    if (findInterface(*interface.getId(), out_interfaces_) != out_interfaces_.end()) return false;

    try {
        Interface* new_interface = new Interface(interface, this);
        out_interfaces_.push_back(new_interface);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool Leaf::deleteOutPort(const Id& id) throw() {
    list<Interface*>::iterator it = findInterface(id, out_interfaces_);
    if (it != out_interfaces_.end()) {
        Interface* removed_interface = *it;
        out_interfaces_.erase(it);
        delete removed_interface;
        return true;
    }
    else {
        return false;
    }
}

size_t Leaf::getNumOutPorts() const throw() {
    return out_interfaces_.size();
}

Leaf::Interface* Leaf::getOutPort(const Id& id) throw() {
    list<Interface*>::iterator it = findInterface(id, out_interfaces_);
    if (it != out_interfaces_.end()) {
        return *it;
    }
    else {
        return NULL;
    }
}

list<Leaf::Interface*> Leaf::getOutPorts() throw() {
    return out_interfaces_;
}

string Leaf::toString() const throw() {
    string str;
    str += "{\n";
    str += " LeafID: ";
    str += getId()->getString();
    str += ",\n";
    str += " LeafType: ";
    str += type();
    str += ",\n";
    str += " NumInPorts: ";
    str += tools::toString(getNumInPorts());
    str += ",\n";
    str += " InPorts = {";
    str += interfacesToString(in_interfaces_);
    str += "}";
    str += ",\n";
    str += " NumOutPorts: ";
    str += tools::toString(getNumOutPorts());
    str += ",\n";
    str += " OutPorts = {";
    str += interfacesToString(out_interfaces_);
    string additional_data(moreToString());
    if (additional_data.length() > 0) {
        str += "},\n";
        additional_data.insert(0, 1, ' ');
        tools::searchReplace(additional_data, "\n", "\n ");
        str += additional_data;
        str += "\n";
    }
    else {
        str += "}\n";
    }
    str += "}";

    return str;
}

string Leaf::moreToString() const throw() {
    return "";
}

list<Leaf::Interface*>::iterator Leaf::findInterface(
    const Id& id, list<Interface*>& interfaces) const throw() {
    list<Interface*>::iterator it;
    for (it = interfaces.begin(); it != interfaces.end(); ++it) {
        if (*(*it)->getId() == id) {
            return it;
        }
    }

    // No such interface was found
    return it;
}

string Leaf::interfacesToString(const list<Interface*> interfaces) const throw() {
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
            if (interface->isConnected()) {
                str += "connected to ";
                str += interface->getConnectedInterface()->getLeaf()->getId()
                    ->getString();
                str += ":";
                str += interface->getConnectedInterface()->getId()->getString();
            }
            else {
                str += "not connected";
            }
        }
        str += "\n ";
    }
    return str;
}

void Leaf::destroyAllInterfaces(list<Interface*>& interfaces) throw() {
    while (interfaces.size() > 0) {
        Interface* interface = interfaces.front();
        interfaces.pop_front();
        delete interface;
    }
}

void Leaf::check() throw(InvalidLeafException) {
    moreChecks();
}

bool Leaf::operator==(const Leaf& rhs) const throw() {
    if (getNumInPorts() != rhs.getNumInPorts()) return false;
    if (getNumOutPorts() != rhs.getNumOutPorts()) return false;
    return true;
}

bool Leaf::operator!=(const Leaf& rhs) const throw() {
    return !operator==(rhs);
}

Leaf::Interface::Interface(const Id& id) throw()
        : id_(id), leaf_(NULL), connected_interface_(NULL) {}

Leaf::Interface::Interface(const Id& id, Leaf* leaf)
        throw(InvalidArgumentException)
        : id_(id), leaf_(leaf), connected_interface_(NULL) {
    if (!leaf) {
        THROW_EXCEPTION(InvalidArgumentException, "leaf must not be NULL");
    }
}

Leaf::Interface::Interface(Interface& rhs) throw()
        : id_(rhs.id_), leaf_(NULL), connected_interface_(NULL) {
    if (rhs.isConnected()) {
        Interface* interface = rhs.connected_interface_;
        rhs.unconnect();
        connect(interface);
    }
}

Leaf::Interface::Interface(Interface& rhs, Leaf* leaf) throw(InvalidArgumentException)
        : id_(rhs.id_), leaf_(leaf), connected_interface_(NULL) {
    if (!leaf) {
        THROW_EXCEPTION(InvalidArgumentException, "\"leaf\" must not be "
                        "NULL");
    }

    if (rhs.isConnected()) {
        Interface* interface = rhs.connected_interface_;
        rhs.unconnect();
        connect(interface);
    }
}

Leaf::Interface::~Interface() throw() {
    unconnect();
}
        
Leaf* Leaf::Interface::getLeaf() const throw() {
    return leaf_;
}

const Id* Leaf::Interface::getId() const throw() {
    return &id_;
}

bool Leaf::Interface::isConnected() const throw() {
    return connected_interface_;
}

void Leaf::Interface::connect(Interface* interface) throw() {
    if (interface == this) return;
    if (!interface) {
        unconnect();
        return;
    }

    if (connected_interface_) {
        unconnect();
    }
    connected_interface_ = interface;
    interface->connected_interface_ = this;
}

void Leaf::Interface::unconnect() throw() {
    if (connected_interface_) {
        connected_interface_->connected_interface_ = NULL;
        connected_interface_ = NULL;
    }
}

Leaf::Interface* Leaf::Interface::getConnectedInterface() const throw() {
    return connected_interface_;
}

bool Leaf::Interface::operator==(const Interface& rhs) const throw() {
    return (leaf_ == rhs.leaf_) && (id_ == rhs.id_);
}

bool Leaf::Interface::operator!=(const Interface& rhs) const throw() {
    return !operator==(rhs);
}

string Leaf::Interface::toString() const throw() {
    string str;
    if (leaf_) str += leaf_->getId()->getString();
    else          str += "NULL";
    str += ":";
    str += id_.getString();
    return str;
}
