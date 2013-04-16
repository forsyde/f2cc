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
using std::bad_cast;

Composite::Composite(const Id& id, const Id& parent, const string& name, const string& moc) throw() : Process(id, parent, moc), composite_name_(name){}

Composite::~Composite() throw() {
	destroyAllProcesses();
}

bool Composite::operator==(const Process& rhs) const throw() {
    if (!Process::operator==(rhs)) return false;

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

string Composite::moreToString() const throw() {
    string str;
    str += "\n";

    str += "\n";
	for (std::map<const Id, Process*>::const_iterator it = processes_.begin(); it != processes_.end(); ++it) {
		str+= " Contained process of type: \" ";
		str += it->second->type();
		str+= "  \"; ID = ";
		str += it->second->getId()->getString();
		str += "\n";
	}
    return str;
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

Process::Port::~Port() throw() {
    unconnect();
}

Process* Process::Port::getProcess() const throw() {
    return process_;
}

const Id* Process::Port::getId() const throw() {
    return &id_;
}

f2cc::CDataType* Process::Port::getDataType() throw() {
    return &data_type_;
}

void Process::Port::setDataType(CDataType& datatype) throw() {
	data_type_ = datatype;
}

bool Process::Port::isIOPort() const throw() {
    return false;
}

bool Process::Port::isConnected() const throw() {
    return connected_port_outside_;
}

bool Process::Port::IOisConnectedOutside() const throw(IllegalCallException) {
    if (!isIOport()) {
        THROW_EXCEPTION(IllegalCallException, "\"IOisConnectedOutside\" was called "
                        "from a non-IO port");
    }
    return connected_port_outside_;
}

bool Process::Port::IOisConnectedInside() const throw(IllegalCallException) {
	if(!isIOport()) {
        THROW_EXCEPTION(IllegalCallException, "\"IOisConnectedInside\" was called "
                        "from a non-IO port");
    }
    return connected_port_inside_;
}

bool Process::Port::IOisConnected() const throw(IllegalCallException) {
	if(!isIOport()) {
        THROW_EXCEPTION(IllegalCallException, "\"IOisConnected\" was called "
                        "from a non-IO port");
    }
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
    if (port->isIOport()){
    	if (port->getProcess()->getId() == process_->getParent()){
    		port->connected_port_inside_ = this;
    	}
    	else port->connected_port_outside_ = this;
    }
    else port->connected_port_outside_ = this;
}

void Process::Port::IOconnectOutside(Port* port) throw(IllegalCallException) {
	if(!isIOport()) {
        THROW_EXCEPTION(IllegalCallException, "\"IOconnectOutside\" was called "
                        "from a non-IO port");
    }
	connect(port);
}

void Process::Port::IOconnectInside(Port* port) throw(IllegalCallException) {
	if(!isIOport()) {
        THROW_EXCEPTION(IllegalCallException, "\"IOconnectInside\" was called "
                        "from a non-IO port");
    }
    if (port == this) return;
    if (!port) {
        unconnect();
        return;
    }

    if (connected_port_inside_) {
    	IOunconnectInside();
    }
    connected_port_inside_ = port;
    port->connected_port_outside_ = this;
}

void Process::Port::IOconnect(Port* inside, Port* outside) throw(IllegalCallException) {
	if(!isIOport()) {
        THROW_EXCEPTION(IllegalCallException, "\"IOconnect\" was called "
                        "from a non-IO port");
    }
	IOconnectInside(inside);
	IOconnectOutside(outside);
}

void Process::Port::unconnect() throw() {
	IOunconnectOutside();
	if(isIOport()) IOunconnectInside();
}

void Process::Port::IOunconnect() throw(IllegalCallException) {
	if(!isIOport()) {
        THROW_EXCEPTION(IllegalCallException, "\"IOunconnect\" was called "
                        "from a non-IO port");
    }
	unconnect();
}

void Process::Port::IOunconnectOutside() throw(IllegalCallException) {
	if(!isIOport()) {
        THROW_EXCEPTION(IllegalCallException, "\"IOunconnectOutside\" was called "
                        "from a non-IO port");
    }
    if (connected_port_outside_) {
    	if(connected_port_outside_->isIOport()){
    		if (connected_port_outside_->getProcess()->getId() == process_->getParent()){
    			connected_port_outside_->connected_port_inside_ = NULL;
			}
    		else connected_port_outside_->connected_port_outside_ = NULL;
    	}
    	else connected_port_outside_->connected_port_outside_ = NULL;
        connected_port_outside_ = NULL;
    }
}

void Process::Port::IOunconnectInside() throw(IllegalCallException) {
	if(!isIOport()) {
        THROW_EXCEPTION(IllegalCallException, "\"IOunconnectInside\" was called "
                        "from a non-IO port");
    }
    if (connected_port_inside_) {
    	connected_port_inside_->connected_port_outside_ = NULL;
    	connected_port_inside_ = NULL;
    }
}

Process::Port* Process::Port::getConnectedPort() const throw() {
	std::string process_type (connected_port_outside_->getProcess()->type());
	std::string composite_type ("composite");
    if (process_type.compare(composite_type) == 0){
    	return connected_port_outside_->getConnectedPort();
    }
    else return connected_port_outside_;
}

Process::Port* Process::Port::IOgetConnectedPortOutside() const throw(IllegalCallException) {
	if(!isIOport()) {
        THROW_EXCEPTION(IllegalCallException, "\"IOgetConnectedPortOutside\" was called "
                        "from a non-IO port");
    }
    return getConnectedPort();
}

Process::Port* Process::Port::IOgetConnectedPortInside() const throw(IllegalCallException) {
	if(!isIOport()) {
        THROW_EXCEPTION(IllegalCallException, "\"IOgetConnectedPortInside\" was called "
                        "from a non-IO port");
    }
	std::string process_type (connected_port_inside_->getProcess()->type());
	std::string composite_type ("composite");
    if (process_type.compare(composite_type) == 0){
    	return connected_port_inside_->IOgetConnectedPortInside();
    }
    else return connected_port_inside_;
}

pair<Process::Port*,Process::Port*> Process::Port::IOgetConnectedPorts() const throw(IllegalCallException) {
	if(!isIOport()) {
        THROW_EXCEPTION(IllegalCallException, "\"IOgetConnectedPorts\" was called "
                        "from a non-IO port");
    }
	pair <Process::Port*,Process::Port*> port_pair;
	port_pair = std::make_pair (IOgetConnectedPortOutside(), IOgetConnectedPortInside());
	return port_pair;
}

Process::Port* Process::Port::getConnectedPortImmediate() const throw() {
    return connected_port_outside_;
}

Process::Port* Process::Port::IOgetConnectedPortOutsideImmediate() const throw(IllegalCallException) {
	if(!isIOport()) {
        THROW_EXCEPTION(IllegalCallException, "\"IOgetConnectedPortOutsideImmediate\" was called "
                        "from a non-IO port");
    }
    return connected_port_outside_;
}

Process::Port* Process::Port::IOgetConnectedPortInsideImmediate() const throw(IllegalCallException) {
	if(!isIOport()) {
        THROW_EXCEPTION(IllegalCallException, "\"IOgetConnectedPortInsideImmediate\" was called "
                        "from a non-IO port");
    }
    return connected_port_inside_;
}

pair<Process::Port*,Process::Port*> Process::Port::IOgetConnectedPortsImmediate() const throw(IllegalCallException) {
	if(!isIOport()) {
        THROW_EXCEPTION(IllegalCallException, "\"IOgetConnectedPortsImmediate\" was called "
                        "from a non-IO port");
    }
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

