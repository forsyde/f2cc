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

#include "composite.h"
#include "../tools/tools.h"
#include <new>
#include <map>
#include <typeinfo>

using namespace f2cc::ForSyDe;
using std::string;
using std::list;
using std::bad_cast;
using std::bad_alloc;

Composite::Composite(const Id& id, const Id& name) throw() : Process(id, string("")), composite_name_(name){}

Composite::~Composite() throw() {
    destroyAllPorts(in_ports_);
    destroyAllPorts(out_ports_);
}

const Id* Composite::getName() const throw() {
    return &composite_name_;
}


bool Composite::addInPort(const Id& id) throw(OutOfMemoryException) {
    if (findPort(id, in_ports_) != in_ports_.end()) return false;

    try {
        IOPort* new_port = new IOPort(id, this);
        in_ports_.push_back(new_port);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool Composite::addInPort(IOPort& port) throw(OutOfMemoryException) {
    if (findPort(*port.getId(), in_ports_) != in_ports_.end()) {
    	return false;
    }

    try {
    	IOPort* new_port = new IOPort(port, this);
        in_ports_.push_back(new_port);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool Composite::deleteInPort(const Id& id) throw() {
    list<IOPort*>::iterator it = findPort(id, in_ports_);
    if (it != in_ports_.end()) {
    	IOPort* removed_port = *it;
        in_ports_.erase(it);
        delete removed_port;
        return true;
    }
    else {
        return false;
    }
}

size_t Composite::getNumInPorts() const throw() {
    return in_ports_.size();
}

Composite::IOPort* Composite::getInPort(const Id& id) throw() {
    list<IOPort*>::iterator it = findPort(id, in_ports_);
    if (it != in_ports_.end()) {
        return *it;
    }
    else {
        return NULL;
    }
}

list<Composite::IOPort*> Composite::getInPorts() throw() {
    return in_ports_;
}

bool Composite::addOutPort(const Id& id) throw(OutOfMemoryException) {
    if (findPort(id, out_ports_) != out_ports_.end()) return false;

    try {
    	IOPort* new_port = new IOPort(id, this);
        out_ports_.push_back(new_port);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool Composite::addOutPort(IOPort& port) throw(OutOfMemoryException) {
    if (findPort(*port.getId(), out_ports_) != out_ports_.end()) {
    	return false;
    }

    try {
    	IOPort* new_port = new IOPort(port, this);
        out_ports_.push_back(new_port);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool Composite::deleteOutPort(const Id& id) throw() {
    list<IOPort*>::iterator it = findPort(id, out_ports_);
    if (it != out_ports_.end()) {
    	IOPort* removed_port = *it;
        out_ports_.erase(it);
        delete removed_port;
        return true;
    }
    else {
        return false;
    }
}

size_t Composite::getNumOutPorts() const throw() {
    return out_ports_.size();
}

Composite::IOPort* Composite::getOutPort(const Id& id) throw() {
    list<IOPort*>::iterator it = findPort(id, out_ports_);
    if (it != out_ports_.end()) {
        return *it;
    }
    else {
        return NULL;
    }
}

list<Composite::IOPort*> Composite::getOutPorts() throw() {
    return out_ports_;
}

string Composite::toString() const throw() {
    string str;
    str += "{\n";
    str += " ProcessID: ";
    str += getId()->getString();
    str += ",\n";
    str += " ProcessType: ";
    str += type();
    str += ",\n";
    str += " Parent: ";
	str += hierarchy_.getFirstParent()->getString();
    str += " Hierarchical address: ";
	str += hierarchy_.hierarchyToString();
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
    for (std::map<const Id, Process*>::const_iterator it = processes_.begin(); it != processes_.end(); ++it) {
		str+= " Contained process of type: \" ";
		str += it->second->type();
		str+= "  \"; ID = ";
		str += it->second->getId()->getString();
		str += "\n";
	}
    str += "}";

    return str;
}


bool Composite::operator==(const Process& rhs) const throw() {
    if (getNumInPorts() != rhs.getNumInPorts()) return false;
    if (getNumOutPorts() != rhs.getNumOutPorts()) return false;

    try {
        const Composite& other = dynamic_cast<const Composite&>(rhs);
        if (composite_name_ != other.composite_name_) return false;
    }
    catch (bad_cast&) {
        return false;
    }
    return true;
}

string Composite::type() const throw() {
    return "composite";
}


void Composite::moreChecks() throw(InvalidProcessException){
    if (getInPorts().size() == 0) {
        THROW_EXCEPTION(InvalidProcessException, string("Process \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\" must have at least one (1) in port");
    }
    if (getOutPorts().size() == 0) {
        THROW_EXCEPTION(InvalidProcessException, string("Process \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\" must have at least one (1) out port");
    }
    if (getProcesses().size() == 0) {
        THROW_EXCEPTION(InvalidProcessException, string("Process \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\" must have at least one (1) process");
    }
}

///////////////

Composite::IOPort::IOPort(const Id& id) throw()
        : PortBase(id), connected_port_inside_(NULL), connected_port_outside_(NULL)  {}

Composite::IOPort::IOPort(const Id& id, Composite* process) throw(InvalidArgumentException)
		:  PortBase(id,process), connected_port_inside_(NULL), connected_port_outside_(NULL) {}

Composite::IOPort::IOPort(ProcessBase::PortBase& rhs) throw()
		: PortBase(rhs.id_), connected_port_inside_(NULL), connected_port_outside_(NULL){

	Process::Port* port_to_copy = dynamic_cast<Process::Port*>(*rhs);
	Composite::IOPort* ioport_to_copy = dynamic_cast<Composite::IOPort*>(*rhs);
    if(port_to_copy){
    	if (port_to_copy->isConnected()) {
        	connect(port_to_copy->connected_port_outside_);
        	port_to_copy->unconnect();
        }
    }
    else if (ioport_to_copy){
    	if (ioport_to_copy->connected_port_outside_) {
        	connect(ioport_to_copy->connected_port_outside_);
        	ioport_to_copy->unconnectOutside();
    	}
    	if (ioport_to_copy->connected_port_inside_) {
        	connect(ioport_to_copy->connected_port_inside_);
        	ioport_to_copy->unconnectInside();
    	}
    }
	else{
		THROW_EXCEPTION(InvalidModelException, "Conflict between Port and IOPort");
	}

}

Composite::IOPort::IOPort(ProcessBase::PortBase& rhs, Composite* process) throw(InvalidArgumentException)
        : PortBase(rhs.id_,process), connected_port_inside_(NULL), connected_port_outside_(NULL) {
    if (!process) {
        THROW_EXCEPTION(InvalidArgumentException, "\"process\" must not be "
                        "NULL");
    }
    Process::Port* port_to_copy = dynamic_cast<Process::Port*>(*rhs);
	Composite::IOPort* ioport_to_copy = dynamic_cast<Composite::IOPort*>(*rhs);
	if(port_to_copy){
		if (port_to_copy->isConnected()) {
			connect(port_to_copy->connected_port_outside_);
			port_to_copy->unconnect();
		}
	}
	else if (ioport_to_copy){
		if (ioport_to_copy->connected_port_outside_) {
			connect(ioport_to_copy->connected_port_outside_);
			ioport_to_copy->unconnectOutside();
		}
		if (ioport_to_copy->connected_port_inside_) {
			connect(ioport_to_copy->connected_port_inside_);
			ioport_to_copy->unconnectInside();
		}
	}
	else{
		THROW_EXCEPTION(InvalidModelException, "Conflict between Port and IOPort");
	}

}

Composite::IOPort::IOPort(IOPort& rhs, Composite* process) throw(InvalidArgumentException)
        : PortBase(rhs.id_,process), connected_port_inside_(NULL), connected_port_outside_(NULL) {
    if (!process) {
        THROW_EXCEPTION(InvalidArgumentException, "\"process\" must not be "
                        "NULL");
    }
	if (rhs.connected_port_outside_) {
    	connect(rhs.connected_port_outside_);
        rhs.unconnect();
	}
	if (rhs.connected_port_inside_) {
    	connect(rhs.connected_port_inside_);
        rhs.unconnect();
	}
}

Composite::IOPort::~IOPort() throw() {
    unconnectOutside();
    unconnectInside();
}

bool Composite::IOPort::isConnected() const throw() {
    return connected_port_inside_;
}

bool Composite::IOPort::isConnectedOutside() const throw() {
    return connected_port_outside_;
}

bool Composite::IOPort::isConnectedInside() const throw() {
    return connected_port_inside_;
}

bool Composite::IOPort::isConnectedToLeafOutside() const throw() {
    if (connected_port_outside_){
    	static const Composite::IOPort* ioport_out = dynamic_cast<const Composite::IOPort*>(connected_port_outside_);
    	if(ioport_out)
    		return ioport_out->isConnectedToLeafOutside();
    	else return true;
    }
    else return false;
}

bool Composite::IOPort::isConnectedToLeafInside() const throw() {
    if (connected_port_inside_){
    	static const Composite::IOPort* ioport_in = dynamic_cast<const Composite::IOPort*>(connected_port_inside_);
    	if(ioport_in)
    		return ioport_in->isConnectedToLeafInside();
    	else return true;
    }
    else return false;
}

void Composite::IOPort::connect(PortBase* port) throw(InvalidArgumentException) {
    if (port == this) return;
    if (!port) {
        unconnect();
        return;
    }
    static Composite::IOPort* ioport_to_connect = dynamic_cast<Composite::IOPort*>(port);
    static Process::Port* port_to_connect = dynamic_cast<Process::Port*>(port);
    if (port_to_connect) {
    	Hierarchy::Relation relation = getProcess()->findRelation(port_to_connect->getProcess());
    	if (relation == Hierarchy::Sibling) {
			connected_port_outside_ = port_to_connect;
			port_to_connect->connected_port_outside_ = this;
		}
    	else if (relation == Hierarchy::FirstChild) {
    		connected_port_inside_ = port_to_connect;
    		port_to_connect->connected_port_outside_ = this;
    	}
    	else THROW_EXCEPTION(InvalidArgumentException, "Connection not possible");
    }
    else if (ioport_to_connect){
    	Hierarchy::Relation relation = getProcess()->findRelation(ioport_to_connect->getProcess());
    	if (relation == Hierarchy::Sibling) {
			connected_port_outside_ = ioport_to_connect;
			ioport_to_connect->connected_port_outside_ = this;
		}
    	else if (relation == Hierarchy::FirstChild) {
    		connected_port_inside_ = ioport_to_connect;
    		ioport_to_connect->connected_port_outside_ = this;
    	}
    	else if (relation == Hierarchy::FirstParent) {
			connected_port_outside_ = ioport_to_connect;
			ioport_to_connect->connected_port_inside_ = this;
		}
    	else THROW_EXCEPTION(InvalidArgumentException, "Connection not possible");
    }
    else{
		THROW_EXCEPTION(InvalidModelException, "Neither Port nor IOPort");
	}

}

void Composite::IOPort::unconnect() throw() {
	unconnectInside();

}

void Composite::IOPort::unconnect(PortBase* port) throw(InvalidArgumentException) {
	if (connected_port_inside_ == port) unconnectInside();
	else if (connected_port_outside_ == port) unconnectOutside();
	else THROW_EXCEPTION(InvalidArgumentException, "Connection should not have been possible");

}

void Composite::IOPort::unconnectOutside() throw(InvalidArgumentException) {
    if (connected_port_outside_) {
        static Composite::IOPort* ioport_to_unconnect = dynamic_cast<Composite::IOPort*>(connected_port_outside_);
        static Process::Port* port_to_unconnect = dynamic_cast<Process::Port*>(connected_port_outside_);
        if (port_to_unconnect) {
        	Hierarchy::Relation relation = getProcess()->findRelation(port_to_unconnect->getProcess());
        	if (relation == Hierarchy::Sibling) {
        		port_to_unconnect->connected_port_outside_ = NULL;
        		connected_port_outside_ = NULL;
        	}
        	else THROW_EXCEPTION(InvalidArgumentException, "Connection not possible");
        }
        else if (ioport_to_unconnect){
        	Hierarchy::Relation relation = getProcess()->findRelation(ioport_to_unconnect->getProcess());
        	if (relation == Hierarchy::Sibling) {
        		ioport_to_unconnect->connected_port_outside_ = NULL;
        		connected_port_outside_ = NULL;
        	}
        	else if (relation == Hierarchy::FirstParent) {
        		ioport_to_unconnect->connected_port_inside_ = NULL;
        		connected_port_outside_ = NULL;
        	}
        	else THROW_EXCEPTION(InvalidArgumentException, "Connection not possible");
        }
        else{
    		THROW_EXCEPTION(InvalidModelException, "Neither Port nor IOPort");
    	}
    }
}

void Composite::IOPort::unconnectInside() throw(InvalidArgumentException) {
    if (connected_port_inside_) {
        static Composite::IOPort* ioport_to_unconnect = dynamic_cast<Composite::IOPort*>(connected_port_inside_);
        static Process::Port* port_to_unconnect = dynamic_cast<Process::Port*>(connected_port_inside_);
        if (port_to_unconnect) {
        	Hierarchy::Relation relation = getProcess()->findRelation(port_to_unconnect->getProcess());
        	if (relation == Hierarchy::FirstChild) {
        		port_to_unconnect->connected_port_outside_ = NULL;
        		connected_port_inside_ = NULL;
        	}
        	else THROW_EXCEPTION(InvalidArgumentException, "Connection not possible");
        }
        else if (ioport_to_unconnect){
        	Hierarchy::Relation relation = getProcess()->findRelation(ioport_to_unconnect->getProcess());
        	if (relation == Hierarchy::FirstChild) {
        		ioport_to_unconnect->connected_port_outside_ = NULL;
        		connected_port_inside_ = NULL;
        	}
        	else THROW_EXCEPTION(InvalidArgumentException, "Connection not possible");
        }
        else{
    		THROW_EXCEPTION(InvalidModelException, "Neither Port nor IOPort");
    	}
    }
}

void Composite::IOPort::unconnectFromLeafOutside() throw(InvalidArgumentException) {
    if (connected_port_outside_) {
        static Composite::IOPort* ioport_to_unconnect = dynamic_cast<Composite::IOPort*>(connected_port_outside_);
        static Process::Port* port_to_unconnect = dynamic_cast<Process::Port*>(connected_port_outside_);
        if (port_to_unconnect) {
        	unconnectOutside();
        }
        else if (ioport_to_unconnect){
        	ioport_to_unconnect->unconnectFromLeafOutside();
        	connected_port_outside_ = NULL;
        }
    	else THROW_EXCEPTION(InvalidModelException, "Neither Port nor IOPort");
    }
}

void Composite::IOPort::unconnectFromLeafInside() throw(InvalidArgumentException) {
    if (connected_port_inside_) {
        static Composite::IOPort* ioport_to_unconnect = dynamic_cast<Composite::IOPort*>(connected_port_inside_);
        static Process::Port* port_to_unconnect = dynamic_cast<Process::Port*>(connected_port_inside_);
        if (port_to_unconnect) {
        	unconnectInside();
        }
        else if (ioport_to_unconnect){
        	ioport_to_unconnect->unconnectFromLeafInside();
        	connected_port_inside_ = NULL;
        }
    	else THROW_EXCEPTION(InvalidModelException, "Neither Port nor IOPort");
    }
}

ProcessBase::PortBase* Composite::IOPort::getConnectedPort() const throw() {
	return connected_port_inside_;
}

ProcessBase::PortBase* Composite::IOPort::getConnectedPortOutside() const throw() {
    return connected_port_outside_;
}

ProcessBase::PortBase* Composite::IOPort::getConnectedPortInside() const throw() {
    return connected_port_inside_;
}

Process::Port* Composite::IOPort::getConnectedLeafPortOutside() const throw(InvalidArgumentException) {
    if (connected_port_outside_) {
        static Composite::IOPort* ioport_to_get = dynamic_cast<Composite::IOPort*>(connected_port_outside_);
        static Process::Port* port_to_get = dynamic_cast<Process::Port*>(connected_port_outside_);
        if (port_to_get) return port_to_get;
        else if (ioport_to_get) return ioport_to_get->getConnectedLeafPortOutside();
    	else THROW_EXCEPTION(InvalidModelException, "Neither Port nor IOPort");
    }
    return connected_port_outside_;
}

Process::Port* Composite::IOPort::getConnectedLeafPortInside() const throw(InvalidArgumentException) {
    if (connected_port_inside_) {
        static Composite::IOPort* ioport_to_get = dynamic_cast<Composite::IOPort*>(connected_port_inside_);
        static Process::Port* port_to_get = dynamic_cast<Process::Port*>(connected_port_inside_);
        if (port_to_get) return port_to_get;
        else if (ioport_to_get) return ioport_to_get->getConnectedLeafPortOutside();
    	else THROW_EXCEPTION(InvalidModelException, "Neither Port nor IOPort");
    }
    return connected_port_inside_;
}


string Composite::IOPort::toString() const throw() {
    string str;
    if (process_) str += process_->getId()->getString();
    else          str += "NULL";
    str += ":";
    str += id_.getString();
    return str;
}



