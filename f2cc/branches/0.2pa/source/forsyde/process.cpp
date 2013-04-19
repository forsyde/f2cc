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

#include "process.h"
#include "composite.h"
#include "processnetwork.h"
#include "../tools/tools.h"
#include <new>
#include <vector>

using namespace f2cc::ForSyDe;
using std::string;
using std::list;
using std::bad_alloc;
using std::vector;
using std::pair;


Process::Process(const Id& id, const string moc) throw()
		: id_(id), moc_(moc) {
	hierarchy_.lowerLevel(id_);
}

Process::~Process() throw() {
    destroyAllPorts(in_ports_);
    destroyAllPorts(out_ports_);
}

const Id* Process::getId() const throw() {
    return hierarchy_.getId();
}

Hierarchy Process::getHierarchy() const throw() {
    return hierarchy_;
}

void Process::setHierarchy(Hierarchy hierarchy) throw() {
	hierarchy_.setHierarchy(hierarchy.getHierarchy());
    hierarchy_.lowerLevel(id_);
}

Hierarchy::Relation Process::findRelation(const Process* rhs) const throw(){
	return hierarchy_.findRelation(rhs->hierarchy_);
}

const string Process::getMoc() const throw() {
    return moc_;
}

int Process::getCost() const throw(){
	return cost_;
}

void  Process::setCost(int& cost) throw(){
	cost_ = cost;
}

bool Process::addInPort(const Id& id, const CDataType datatype) throw(OutOfMemoryException) {
    if (findPort(id, in_ports_) != in_ports_.end()) return false;

    try {
        Port* new_port = new Port(id, this, datatype);
        in_ports_.push_back(new_port);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool Process::addInPort(Port& port) throw(OutOfMemoryException) {
    if (findPort(*port.getId(), in_ports_) != in_ports_.end()) {
    	return false;
    }

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

bool Process::addOutPort(const Id& id, const CDataType datatype) throw(OutOfMemoryException) {
    if (findPort(id, out_ports_) != out_ports_.end()) return false;

    try {
        Port* new_port = new Port(id, this, datatype);
        out_ports_.push_back(new_port);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool Process::addOutPort(Port& port) throw(OutOfMemoryException) {
    if (findPort(*port.getId(), out_ports_) != out_ports_.end()) {
    	return false;
    }

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
    str += " MoC: ";
    str += getMoc();
    str += ",\n";
    str += " Parent: ";
	str += hierarchy_.getFirstParent()->getString();
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

Process::Port::Port(const Id& id, CDataType datatype) throw()
        : id_(id), process_(NULL), connected_port_outside_(NULL), data_type_(datatype) {}

Process::Port::Port(const Id& id, Process* process, CDataType datatype)
        throw(InvalidArgumentException)
        : id_(id), process_(process), connected_port_outside_(NULL), data_type_(datatype) {
    if (!process) {
        THROW_EXCEPTION(InvalidArgumentException, "process must not be NULL");
    }
}

Process::Port::Port(Port& rhs) throw(InvalidArgumentException)
        : id_(rhs.id_), process_(NULL), connected_port_outside_(NULL),  data_type_(rhs.data_type_) {

	static const Composite::IOPort* ioport = dynamic_cast<const Composite::IOPort*>(&rhs);

	if (ioport) {
		THROW_EXCEPTION(InvalidArgumentException, "Cannot equate Port and IOPort");
	}
	//manual unconnect, to avoid calling virtual functions
	if (rhs.connected_port_outside_) {
        Port* port = rhs.connected_port_outside_;
        rhs.connected_port_outside_ = NULL;
        if (port != this) {
        	connected_port_outside_ = port;
        	port->connected_port_outside_ = this;
        }
    }
}

Process::Port::Port(Port& rhs, Process* process) throw(InvalidArgumentException)
        : id_(rhs.id_), process_(process), connected_port_outside_(NULL),
         data_type_(rhs.data_type_) {
    if (!process) {
        THROW_EXCEPTION(InvalidArgumentException, "\"process\" must not be "
                        "NULL");
    }
    static const Composite::IOPort* ioport = dynamic_cast<const Composite::IOPort*>(&rhs);
    	if (ioport) {
		THROW_EXCEPTION(InvalidArgumentException, "Cannot equate Port and IOPort");
	}
	//manual unconnect, to avoid calling virtual functions
	if (rhs.connected_port_outside_) {
        Port* port = rhs.connected_port_outside_;
        rhs.connected_port_outside_ = NULL;
        if (port != this) {
        	connected_port_outside_ = port;
        	port->connected_port_outside_ = this;
        }
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

f2cc::CDataType Process::Port::getDataType() throw() {
    return data_type_;
}

bool Process::Port::setDataType(CDataType& datatype) throw() {
	data_type_ = datatype;
	return true;
}


bool Process::Port::isConnected() const throw() {
    return connected_port_outside_;
}

bool Process::Port::isConnectedToLeaf() const throw(){
	if (connected_port_outside_){
		static const Composite::IOPort* ioport = dynamic_cast<const Composite::IOPort*>(connected_port_outside_);
		if(ioport){
			Hierarchy::Relation relation = getProcess()->findRelation(connected_port_outside_->getProcess());
			if (relation == Hierarchy::Sibling)
				return (ioport->isConnectedToLeafInside());
			else
				return (ioport->isConnectedToLeafOutside());
		}
		else return true;
	}
	else return false;
}

void Process::Port::connect(Port* port) throw(InvalidArgumentException) {
    if (port == this) return;
    if (!port) {
        unconnect();
        return;
    }
    Hierarchy::Relation relation = getProcess()->findRelation(port->getProcess());
    if ((relation != Hierarchy::FirstParent) && (relation != Hierarchy::Sibling))
    	THROW_EXCEPTION(InvalidArgumentException, "Connection not possible");

    if (connected_port_outside_) {
        unconnect();
    }
    connected_port_outside_ = port;

    Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(connected_port_outside_);

    if(ioport) ioport->connect(this);
    else port->connected_port_outside_ = this;
}

void Process::Port::connectGlobal(Port* port) throw() {
	//TODO: implement
/*    if (port == this) return;
    if (!port) {
        unconnect();
        return;
    }

    if (connected_port_outside_) {
        unconnect();
    }

    //Treat connections within local scope
    Process::Relation relation = port->getProcess()->findRelation(*process_);
    if ((relation == Sibling) || (relation == FirstParent) || (relation == FirstChild)){
    	connect(port);
    }
    else if (relation == SiblingsChild){ //Treat connections to a sibling's child processes
    	const Id* siblings_id = getFirstChild(getProcess()->id_, port->getProcess()->hierarchy_);
    	if (!siblings_id) {
    		THROW_EXCEPTION(InvalidProcessnetworkException, "A sibling with children must be a composie process");
    	}
		Composite::IOPort* new_port = new (std::nothrow) Composite::IOPort(
						ForSyDe::Id("port_"),
						getProcess()->Composite::getProcess(siblings_id));
		if (!new_port) THROW_EXCEPTION(OutOfMemoryException);

		new_port->connectOutside(this);
    }
    else if (relation == Child){
    	const Id* child_id = getFirstChild(id_, port->getProcess()->hierarchy_);
    	if (!child_id) {
    	    THROW_EXCEPTION(InvalidProcessnetworkException, "A sibling with children must be a composie process");
    	}
		Composite::IOPort* new_port = new (std::nothrow) Composite::IOPort(
						ForSyDe::Id("port_"),
						getProcess()->Composite::getProcess(child_id));
		if (!new_port) THROW_EXCEPTION(OutOfMemoryException);

		new_port->connectOutside(this);
    }

    else if ((relation == Parent) || (relation == Other)){
		Composite::IOPort* new_port = new (std::nothrow) Composite::IOPort(
						ForSyDe::Id("port_"),
						getProcess()->Composite::getProcess(child_id));
		if (!new_port) THROW_EXCEPTION(OutOfMemoryException);
    }
*/
}


void Process::Port::unconnect() throw() {
    if (connected_port_outside_) {
        Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(connected_port_outside_);
    	if (ioport){
    		Hierarchy::Relation relation = getProcess()->findRelation(connected_port_outside_->getProcess());
			if (relation == Hierarchy::Sibling) ioport->unconnectOutside();
			else ioport->unconnectInside();

    	}
    	else {
			connected_port_outside_->connected_port_outside_ = NULL;
			connected_port_outside_ = NULL;
    	}
    }
}

void Process::Port::unconnectFromLeaf() throw() {
    if (connected_port_outside_) {
        Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(connected_port_outside_);
    	if (ioport){
    		ioport->unconnectFromLeafInside();
    		ioport->unconnectFromLeafOutside();
    	}
    	else {
			connected_port_outside_->connected_port_outside_ = NULL;
			connected_port_outside_ = NULL;
    	}
    }
}

Process::Port* Process::Port::getConnectedPort() const throw() {
	return connected_port_outside_;
}
Process::Port* Process::Port::PortGetter() const throw() {
	return connected_port_outside_;
}
void Process::Port::PortSetter(Process::Port* port) throw() {
	connected_port_outside_ = port;
}

Process::Port* Process::Port::getConnectedLeafPort() const throw() {
	Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(connected_port_outside_);
	if (ioport){
		Hierarchy::Relation relation = getProcess()->findRelation(connected_port_outside_->getProcess());
		if (relation == Hierarchy::Sibling){
			return ioport->getConnectedLeafPortInside();
		}
		else return ioport->getConnectedLeafPortOutside();
	}
    else return connected_port_outside_;
}

bool Process::Port::operator==(const Port& rhs) const throw() {
    return (process_ == rhs.process_) && (id_ == rhs.id_) && (data_type_ == rhs.data_type_);
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
    str += " encapsulating: ";
    str += data_type_.toString();
    return str;
}

