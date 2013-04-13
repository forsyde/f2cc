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

#include "process.h"
#include "../tools/tools.h"
#include <new>
#include <vector>

using namespace f2cc::ForSyDe;
using std::string;
using std::list;
using std::bad_alloc;
using std::vector;
using std::pair;

Process::Process(const Id& id, const Id& parent) throw() : id_(id), parent_ (parent) {}

Process::~Process() throw() {
    destroyAllPorts(in_ports_);
    destroyAllPorts(out_ports_);
}

const Id* Process::getId() const throw() {
    return &id_;
}

const Id* Process::getParent() const throw() {
    return &parent_;
}

bool Process::addInPort(const Id& id) throw(OutOfMemoryException) {
    if (findPort(id, in_ports_) != in_ports_.end()) return false;

    try {
        Port* new_port = new Port(id, this);
        in_ports_.push_back(new_port);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool Process::addInPort(Port& port) throw(OutOfMemoryException) {
    if (findPort(*port.getId(), in_ports_) != in_ports_.end()) return false;

    try {
        Port* new_port = new Port(port, this);
        in_ports_.push_back(new_port);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool Process::deleteInPort(const Id& id) throw() {
    list<Port*>::iterator it = findPort(id, in_ports_);
    if (it != in_ports_.end()) {
        Port* removed_port = *it;
        in_ports_.erase(it);
        delete removed_port;
        return true;
    }
    else {
        return false;
    }
}

size_t Process::getNumInPorts() const throw() {
    return in_ports_.size();
}

Process::Port* Process::getInPort(const Id& id) throw() {
    list<Port*>::iterator it = findPort(id, in_ports_);
    if (it != in_ports_.end()) {
        return *it;
    }
    else {
        return NULL;
    }
}

list<Process::Port*> Process::getInPorts() throw() {
    return in_ports_;
}

bool Process::addOutPort(const Id& id) throw(OutOfMemoryException) {
    if (findPort(id, out_ports_) != out_ports_.end()) return false;

    try {
        Port* new_port = new Port(id, this);
        out_ports_.push_back(new_port);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool Process::addOutPort(Port& port) throw(OutOfMemoryException) {
    if (findPort(*port.getId(), out_ports_) != out_ports_.end()) return false;

    try {
        Port* new_port = new Port(port, this);
        out_ports_.push_back(new_port);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool Process::deleteOutPort(const Id& id) throw() {
    list<Port*>::iterator it = findPort(id, out_ports_);
    if (it != out_ports_.end()) {
        Port* removed_port = *it;
        out_ports_.erase(it);
        delete removed_port;
        return true;
    }
    else {
        return false;
    }
}

size_t Process::getNumOutPorts() const throw() {
    return out_ports_.size();
}

Process::Port* Process::getOutPort(const Id& id) throw() {
    list<Port*>::iterator it = findPort(id, out_ports_);
    if (it != out_ports_.end()) {
        return *it;
    }
    else {
        return NULL;
    }
}

list<Process::Port*> Process::getOutPorts() throw() {
    return out_ports_;
}

string Process::toString() const throw() {
    string str;
    str += "{\n";
    str += " ProcessID: ";
    str += getId()->getString();
    str += ",\n";
    str += " ProcessType: ";
    str += type();
    str += ",\n";
    str += " Parent: ";
	str += getParent()->getString();
	str += ",\n";
    str += " NumInPorts: ";
    str += tools::toString(getNumInPorts());
    str += ",\n";
    str += " InPorts = {";
    str += portsToString(in_ports_);
    str += "}";
    str += ",\n";
    str += " NumOutPorts: ";
    str += tools::toString(getNumOutPorts());
    str += ",\n";
    str += " OutPorts = {";
    str += portsToString(out_ports_);
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

string Process::moreToString() const throw() {
    return "";
}

list<Process::Port*>::iterator Process::findPort(
    const Id& id, list<Port*>& ports) const throw() {
    list<Port*>::iterator it;
    for (it = ports.begin(); it != ports.end(); ++it) {
        if (*(*it)->getId() == id) {
            return it;
        }
    }

    // No such port was found
    return it;
}

string Process::portsToString(const list<Port*> ports) const throw() {
    string str;
    if (ports.size() > 0) {
        str += "\n";
        bool first = true;
        for (list<Process::Port*>::const_iterator it = ports.begin();
             it != ports.end(); ++it) {
            if (!first) {
                str += ",\n";
            }
            else {
                first = false;
            }

            Process::Port* port = *it;
            str += "  ID: ";
            str += port->getId()->getString();
            str += ", ";
            if (port->isConnected()) {
                str += "connected to ";
                str += port->getConnectedPort()->getProcess()->getId()
                    ->getString();
                str += ":";
                str += port->getConnectedPort()->getId()->getString();
            }
            else {
                str += "not connected";
            }
        }
        str += "\n ";
    }
    return str;
    //TODO: resolve IO ports
}

void Process::destroyAllPorts(list<Port*>& ports) throw() {
    while (ports.size() > 0) {
        Port* port = ports.front();
        ports.pop_front();
        delete port;
    }
}

void Process::check() throw(InvalidProcessException) {
    moreChecks();
}

bool Process::operator==(const Process& rhs) const throw() {
    if (getNumInPorts() != rhs.getNumInPorts()) return false;
    if (getNumOutPorts() != rhs.getNumOutPorts()) return false;
    return true;
}

bool Process::operator!=(const Process& rhs) const throw() {
    return !operator==(rhs);
}

Process::Port::Port(const Id& id) throw()
        : id_(id), process_(NULL), connected_port_outside_(NULL), connected_port_inside_(NULL) {}

Process::Port::Port(const Id& id, Process* process)
        throw(InvalidArgumentException)
        : id_(id), process_(process), connected_port_outside_(NULL), connected_port_inside_(NULL) {
    if (!process) {
        THROW_EXCEPTION(InvalidArgumentException, "process must not be NULL");
    }
}

Process::Port::Port(Port& rhs) throw()
        : id_(rhs.id_), process_(NULL), connected_port_outside_(NULL), connected_port_inside_(NULL) {
    if (rhs.isConnected()) {
        Port* port = rhs.connected_port_outside_;
        rhs.unconnect();
        connect(port);
    }
    if (rhs.IOisConnectedInside()) {
        Port* port = rhs.connected_port_inside_;
        rhs.IOunconnectInside();
        IOconnectInside(port);
    }
}

Process::Port::Port(Port& rhs, Process* process) throw(InvalidArgumentException)
        : id_(rhs.id_), process_(process), connected_port_outside_(NULL), connected_port_inside_(NULL) {
    if (!process) {
        THROW_EXCEPTION(InvalidArgumentException, "\"process\" must not be "
                        "NULL");
    }

    if (rhs.isConnected()) {
        Port* port = rhs.connected_port_outside_;
        rhs.unconnect();
        connect(port);
    }
    if (rhs.IOisConnectedInside()) {
        Port* port = rhs.connected_port_inside_;
        rhs.IOunconnectInside();
        IOconnectInside(port);
    }
}

Process::Port::~Port() throw() {
    unconnect();
}
        
Process* Process::Port::getProcess() const throw() {
    return process_;
}

const Id* Process::Port::getId() const throw() {
    return &id_;
}

bool Process::Port::isConnected() const throw() {
    return connected_port_outside_;
}

bool Process::Port::IOisConnectedOutside() const throw() {
    return connected_port_outside_;
}

bool Process::Port::IOisConnectedInside() const throw() {
    return connected_port_inside_;
}

bool Process::Port::IOisConnected() const throw() {
    return (connected_port_inside_ && connected_port_outside_);
}

void Process::Port::connect(Port* port) throw() {
    if (port == this) return;
    if (!port) {
        unconnect();
        return;
    }

    if (connected_port_outside_) {
        unconnect();
    }
    connected_port_outside_ = port;
    port->connected_port_outside_ = this;
}

void Process::Port::IOconnectOutside(Port* port) throw() {
	connect(port);
}

void Process::Port::IOconnectInside(Port* port) throw() {
    if (port == this) return;
    if (!port) {
        unconnect();
        return;
    }

    if (connected_port_inside_) {
    	IOunconnectInside();
    }
    connected_port_inside_ = port;
    port->connected_port_inside_ = this;
}

void Process::Port::IOconnect(Port* inside, Port* outside) throw() {
	IOconnectInside(inside);
	IOconnectOutside(outside);
}

void Process::Port::unconnect() throw() {
	IOunconnectOutside();
	IOunconnectInside();
}

void Process::Port::IOunconnect() throw() {
	unconnect();
}

void Process::Port::IOunconnectOutside() throw() {
    if (connected_port_outside_) {
        connected_port_outside_->connected_port_outside_ = NULL;
        connected_port_outside_ = NULL;
    }
}

void Process::Port::IOunconnectInside() throw() {
    if (connected_port_inside_) {
    	connected_port_inside_->connected_port_inside_ = NULL;
    	connected_port_inside_ = NULL;
    }
}

Process::Port* Process::Port::getConnectedPort() const throw() {
	std::string process_type (connected_port_inside_->getProcess()->type());
	std::string composite_type ("composite");
    if (process_type.compare(composite_type) != 0){
    	return connected_port_inside_->getConnectedPort();
    }
    else return connected_port_inside_;
}

Process::Port* Process::Port::IOgetConnectedPortOutside() const throw() {
    return getConnectedPort();
}

Process::Port* Process::Port::IOgetConnectedPortInside() const throw() {
	std::string process_type (connected_port_inside_->getProcess()->type());
	std::string composite_type ("composite");
    if (process_type.compare(composite_type) != 0){
    	return connected_port_inside_->IOgetConnectedPortInside();
    }
    else return connected_port_inside_;
}

pair<Process::Port*,Process::Port*> Process::Port::IOgetConnectedPorts() const throw() {
	pair <Process::Port*,Process::Port*> port_pair;
	port_pair = std::make_pair (IOgetConnectedPortOutside(), IOgetConnectedPortInside());
	return port_pair;
}

Process::Port* Process::Port::getConnectedPortImmediate() const throw() {
    return connected_port_outside_;
}

Process::Port* Process::Port::IOgetConnectedPortOutsideImmediate() const throw() {
    return connected_port_outside_;
}

Process::Port* Process::Port::IOgetConnectedPortInsideImmediate() const throw() {
    return connected_port_inside_;
}

pair<Process::Port*,Process::Port*> Process::Port::IOgetConnectedPortsImmediate() const throw() {
	pair <Process::Port*,Process::Port*> port_pair;
	port_pair = std::make_pair (IOgetConnectedPortOutsideImmediate(), IOgetConnectedPortInsideImmediate());
	return port_pair;
}

bool Process::Port::operator==(const Port& rhs) const throw() {
    return (process_ == rhs.process_) && (id_ == rhs.id_);
}

bool Process::Port::operator!=(const Port& rhs) const throw() {
    return !operator==(rhs);
}

string Process::Port::toString() const throw() {
    string str;
    if (process_) str += process_->getId()->getString();
    else          str += "NULL";
    str += ":";
    str += id_.getString();
    return str;
}
