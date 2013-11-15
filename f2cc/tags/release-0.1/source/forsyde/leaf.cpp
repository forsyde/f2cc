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

#include "leaf.h"
#include "composite.h"
#include "../tools/tools.h"
#include <new>
#include <vector>

using namespace f2cc::Forsyde;
using std::string;
using std::list;
using std::bad_alloc;
using std::vector;

Leaf::Leaf(const Id& id) throw() : Process(id) {}

Leaf::Leaf(const Forsyde::Id& id, Forsyde::Hierarchy hierarchy,
 		const std::string moc, int cost) throw() :
		Process(id, hierarchy), moc_(moc), cost_(cost){}

Leaf::~Leaf() throw() {
    destroyAllPorts(in_ports_);
    destroyAllPorts(out_ports_);
}

const string Leaf::getMoc() const throw() {
    return moc_;
}


int Leaf::getCost() const throw(){
	return cost_;
}

void  Leaf::setCost(int& cost) throw(){
	cost_ = cost;
}

bool Leaf::addInPort(const Id& id) throw(OutOfMemoryException) {
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

bool Leaf::addInPort(const Id& id, CDataType datatype) throw(OutOfMemoryException) {
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


bool Leaf::addInPort(Port& port) throw(OutOfMemoryException) {
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

bool Leaf::deleteInPort(const Id& id) throw() {
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

size_t Leaf::getNumInPorts() const throw() {
    return in_ports_.size();
}

Leaf::Port* Leaf::getInPort(const Id& id) throw() {
    list<Port*>::iterator it = findPort(id, in_ports_);
    if (it != in_ports_.end()) {
        return *it;
    }
    else {
        return NULL;
    }
}

list<Leaf::Port*> Leaf::getInPorts() throw() {
    return in_ports_;
}

bool Leaf::addOutPort(const Id& id) throw(OutOfMemoryException) {
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

bool Leaf::addOutPort(const Id& id, CDataType datatype) throw(OutOfMemoryException) {
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


bool Leaf::addOutPort(Port& port) throw(OutOfMemoryException) {
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

bool Leaf::deleteOutPort(const Id& id) throw() {
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

size_t Leaf::getNumOutPorts() const throw() {
    return out_ports_.size();
}

Leaf::Port* Leaf::getOutPort(const Id& id) throw() {
    list<Port*>::iterator it = findPort(id, out_ports_);
    if (it != out_ports_.end()) {
        return *it;
    }
    else {
        return NULL;
    }
}

list<Leaf::Port*> Leaf::getOutPorts() throw() {
    return out_ports_;
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

string Leaf::moreToString() const throw() {
    return "";
}

list<Leaf::Port*>::iterator Leaf::findPort(
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

string Leaf::portsToString(const list<Port*> ports) const throw() {
    string str;
    if (ports.size() > 0) {
        str += "\n";
        bool first = true;
        for (list<Leaf::Port*>::const_iterator it = ports.begin();
             it != ports.end(); ++it) {
            if (!first) {
                str += ",\n";
            }
            else {
                first = false;
            }

            Leaf::Port* port = *it;
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

void Leaf::destroyAllPorts(list<Port*>& ports) throw() {
    while (ports.size() > 0) {
        Port* port = ports.front();
        ports.pop_front();
        delete port;
    }
}

bool Leaf::operator==(const Leaf& rhs) const throw() {
    if (getNumInPorts() != rhs.getNumInPorts()) return false;
    if (getNumOutPorts() != rhs.getNumOutPorts()) return false;
    return true;
}

bool Leaf::operator!=(const Leaf& rhs) const throw() {
    return !operator==(rhs);
}

Leaf::Port::Port(const Id& id) throw()
        : Interface(id), connected_port_(NULL), data_type_(CDataType()),
          variable_(NULL){}

Leaf::Port::Port(const Id& id, Leaf* leaf) throw(InvalidArgumentException)
        : Interface(id, leaf), connected_port_(NULL), data_type_(CDataType()),
          variable_(NULL){
    if (!leaf) {
        THROW_EXCEPTION(InvalidArgumentException, "leaf must not be NULL");
    }
}

Leaf::Port::Port(const Id& id, Leaf* leaf, CDataType data_type) throw(InvalidArgumentException)
        : Interface(id, leaf), connected_port_(NULL), data_type_(data_type),
          variable_(NULL){
    if (!leaf) {
        THROW_EXCEPTION(InvalidArgumentException, "leaf must not be NULL");
    }
}

Leaf::Port::Port(Port& rhs) throw()
        : Interface(rhs.id_), connected_port_(NULL), data_type_(CDataType()),
          variable_(NULL){
    if (rhs.isConnected()) {
    	Process::Interface* port = rhs.connected_port_;
        rhs.unconnect();
        connect(port);
    }
}

Leaf::Port::Port(Port& rhs, Leaf* leaf) throw(InvalidArgumentException)
        : Interface(rhs.id_, leaf), connected_port_(NULL), data_type_(CDataType()),
          variable_(NULL){
    if (!leaf) {
        THROW_EXCEPTION(InvalidArgumentException, "\"leaf\" must not be "
                        "NULL");
    }

    if (rhs.isConnected()) {
    	Process::Interface* port = rhs.connected_port_;
        rhs.unconnect();
        connect(port);
    }
}

Leaf::Port::~Port() throw() {
    unconnect();
}

f2cc::CDataType Leaf::Port::getDataType() throw() {
    return data_type_;
}

void Leaf::Port::setDataType(CDataType datatype) throw() {
	data_type_ = datatype;
}

f2cc::CVariable* Leaf::Port::getVariable() throw() {
    return variable_;
}

void Leaf::Port::setVariable(CVariable* variable) throw() {
	variable_ = variable;
}


bool Leaf::Port::isConnected() const throw() {
    return connected_port_;
}

bool Leaf::Port::isConnectedToLeaf() const throw(IllegalStateException) {
	if (connected_port_){
		static const Composite::IOPort* ioport =
				dynamic_cast<const Composite::IOPort*>(connected_port_);

		if(ioport) return (ioport->isConnectedToLeaf(this));
		else return true;
	}
	else return false;
}

void Leaf::Port::connect(Process::Interface* port) throw(InvalidArgumentException) {
    if (!port) {
        unconnect();
        return;
    }
    // Checking if other end is IOPort
	Composite::IOPort* ioport_to_connect = dynamic_cast<Composite::IOPort*>(port);
	if (ioport_to_connect) {
		ioport_to_connect->connect(this);
		return;
	}
	 // Checking if other end is Port
	Leaf::Port* port_to_connect = dynamic_cast<Leaf::Port*>(port);
	if (port_to_connect) {
		if (port_to_connect == this) return;
		if (connected_port_) {
			unconnect();
		}
		connected_port_ = port_to_connect;
		port_to_connect->connected_port_ = this;
		return;
	}
	// It should never be here
	THROW_EXCEPTION(InvalidArgumentException, string("Critical error in ")
					+ toString()
					+ "! Connected port is of unknown type");
}

void Leaf::Port::unconnect() throw() {
    if (connected_port_) {
    	// Checking if other end is IOPort
		Composite::IOPort* ioport_to_unconnect = dynamic_cast<Composite::IOPort*>(connected_port_);
		if (ioport_to_unconnect) {
			ioport_to_unconnect->unconnect(this);
			return;
		}
		 // Checking if other end is Port
		Leaf::Port* port_to_unconnect = dynamic_cast<Leaf::Port*>(connected_port_);
		if (port_to_unconnect) {
			port_to_unconnect->connected_port_ = NULL;
			connected_port_ = NULL;
			return;
		}
		// It should never be here
		THROW_EXCEPTION(InvalidArgumentException, string("Critical error in ")
						+ toString()
						+ "! Connected port is of unknown type");
    }
}

void Leaf::Port::unconnectFromLeaf() throw() {
    if (connected_port_) {
    	// Checking if other end is IOPort
		Composite::IOPort* ioport_to_unconnect = dynamic_cast<Composite::IOPort*>(connected_port_);
		if (ioport_to_unconnect) {
			ioport_to_unconnect->unconnectFromLeafOutside();
			ioport_to_unconnect->unconnectFromLeafInside();
		}
		 // Checking if other end is Port
		Leaf::Port* port_to_unconnect = dynamic_cast<Leaf::Port*>(connected_port_);
		if (port_to_unconnect) {
			port_to_unconnect->connected_port_ = NULL;
			connected_port_ = NULL;
		}
		// It should never be here
		THROW_EXCEPTION(InvalidArgumentException, string("Critical error in ")
						+ toString()
						+ "! Connected port is of unknown type");
    }
}

Process::Interface* Leaf::Port::getConnectedPort() const throw() {
    return connected_port_;
}

void Leaf::Port::setConnection(Process::Interface* port) throw() {
    connected_port_ = port;
}

/*
Leaf::Port* Leaf::Port::getConnectedLeafPort() const throw() {
	if (!getProcess()) {
		THROW_EXCEPTION(IllegalStateException, string("Error in: ")
					+ toString()
					+ "! Finding relation without hierarchy is not possible");
	}
    if (connected_port_) {
    	// Checking if other end is IOPort
		Composite::IOPort* ioport_to_get = dynamic_cast<Composite::IOPort*>(connected_port_);
		if (ioport_to_get) {
			Hierarchy::Relation relation = getProcess()->findRelation(ioport_to_get->getProcess());
			if (relation == Hierarchy::Sibling){
				return ioport_to_get->getConnectedLeafPortInside();
			}
			else return ioport_to_get->getConnectedLeafPortOutside();
		}
		 // Checking if other end is Port
		Leaf::Port* port_to_get = dynamic_cast<Composite::IOPort*>(port);
		if (port_to_get) {
			return connected_port_;
		}
		// It should never be here
		THROW_EXCEPTION(InvalidArgumentException, string("Critical error in ")
						+ toString()
						+ "! Connected port is of unknown type");
    }
    else return NULL;
}
*/

bool Leaf::Port::operator==(const Port& rhs) const throw() {
    return (process_ == rhs.process_) && (id_ == rhs.id_) && (data_type_ == rhs.data_type_);
}

bool Leaf::Port::operator!=(const Port& rhs) const throw() {
    return !operator==(rhs);
}

string Leaf::Port::moretoString() const throw() {
    string str;
    str += "(";
    str += data_type_.toString();
    str += ")";
    return str;
}
