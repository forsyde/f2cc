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

#include "composite.h"
#include "leaf.h"
#include "../tools/tools.h"
#include <new>
#include <map>
#include <vector>

using namespace f2cc::Forsyde;
using std::string;
using std::list;
using std::pair;
using std::bad_alloc;
using std::vector;

Composite::Composite(const Forsyde::Id& id, Forsyde::Hierarchy& hierarchy,
		Forsyde::Id name) throw(RuntimeException) :
		Model(), Process(id, hierarchy, true, 0), composite_name_(name){}

Composite::~Composite() throw() {
    destroyAllIOPorts(in_ports_);
    destroyAllIOPorts(out_ports_);
}

Id Composite::getName() const throw(RuntimeException) {
    return composite_name_;
}

void Composite::changeName(Forsyde::Id name) throw(RuntimeException) {
	composite_name_ = name;
}


bool Composite::addInIOPort(const Id& id, CDataType datatype) throw(RuntimeException, OutOfMemoryException) {
    if (findPort(id, in_ports_) != in_ports_.end()) return false;

    try {
        IOPort* new_port = new IOPort(id, this, datatype);
        in_ports_.push_back(new_port);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool Composite::deleteInIOPort(const Id& id) throw(RuntimeException) {
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

size_t Composite::getNumInIOPorts() const throw(RuntimeException) {
    return in_ports_.size();
}

Composite::IOPort* Composite::getInIOPort(const Id& id) throw(RuntimeException) {
    list<IOPort*>::iterator it = findPort(id, in_ports_);
    if (it != in_ports_.end()) {
        return *it;
    }
    else {
        return NULL;
    }
}

list<Composite::IOPort*> Composite::getInIOPorts() throw(RuntimeException) {
    return in_ports_;
}

bool Composite::addOutIOPort(const Id& id, CDataType datatype) throw(RuntimeException, OutOfMemoryException) {
    if (findPort(id, out_ports_) != out_ports_.end()) return false;

    try {
        IOPort* new_port = new IOPort(id, this, datatype);
        out_ports_.push_back(new_port);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool Composite::deleteOutIOPort(const Id& id) throw(RuntimeException) {
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

size_t Composite::getNumOutIOPorts() const throw(RuntimeException) {
    return out_ports_.size();
}

Composite::IOPort* Composite::getOutIOPort(const Id& id) throw(RuntimeException) {
    list<IOPort*>::iterator it = findPort(id, out_ports_);
    if (it != out_ports_.end()) {
        return *it;
    }
    else {
        return NULL;
    }
}

list<Composite::IOPort*> Composite::getOutIOPorts() throw(RuntimeException) {
    return out_ports_;
}

string Composite::toString() const throw(RuntimeException) {
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
}

list<Composite::IOPort*>::iterator Composite::findPort(
		const Id& id, list<IOPort*>& ports) const throw(RuntimeException) {
    list<IOPort*>::iterator it;
    for (it = ports.begin(); it != ports.end(); ++it) {
        if (*(*it)->getId() == id) {
            return it;
        }
    }

    // No such port was found
    return it;
}

string Composite::portsToString(const list<IOPort*> ports) const throw(RuntimeException) {
    string str;
    if (ports.size() > 0) {
        str += "\n";
        bool first = true;
        for (list<Composite::IOPort*>::const_iterator it = ports.begin();
             it != ports.end(); ++it) {
            if (!first) {
                str += ",\n";
            }
            else {
                first = false;
            }

            Composite::IOPort* port = *it;
            str += "  ID: ";
            str += port->getId()->getString();
            str += ", ";
            if (port->isConnectedInside()) {
                str += "connected inside to ";
                str += port->getConnectedPortInside()->getProcess()->getId()
                    ->getString();
                str += ":";
                str += port->getConnectedPortInside()->getId()->getString();
            }
            else {
                str += "not connected inside ";
            }
            if (port->isConnectedOutside()) {
                str += "; and connected outside to ";
                str += port->getConnectedPortOutside()->getProcess()->getId()
                    ->getString();
                str += ":";
                str += port->getConnectedPortOutside()->getId()->getString();
            }
            else {
                str += "and not connected outside ";
            }
        }
        str += "\n ";
    }
    return str;
}

void Composite::destroyAllIOPorts(list<IOPort*>& ports) throw(RuntimeException) {
    while (ports.size() > 0) {
    	IOPort* port = ports.front();
        ports.pop_front();
        delete port;
    }
}


bool Composite::operator==(const Composite& rhs) const throw(RuntimeException) {
	if (getNumInIOPorts() != rhs.getNumInIOPorts()) return false;
	if (getNumOutIOPorts() != rhs.getNumOutIOPorts()) return false;
	if (composite_name_ != rhs.composite_name_) return false;

    return true;
}

bool Composite::operator!=(const Composite& rhs) const throw(RuntimeException) {
    return !operator==(rhs);
}

string Composite::type() const throw() {
    return "composite";
}

void Composite::moreChecks() throw(InvalidProcessException){
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
    list<IOPort*> in_ports = getInIOPorts();
    for (list<IOPort*>::iterator it = in_ports.begin(); it != in_ports.end(); ++it){
    	if((*it)->getDataType().first != (*it)->getDataType().second){
    		THROW_EXCEPTION(InvalidProcessException, string("Process \"")
						+ getId()->getString() + "\" of type \""
						+ type() + "\" has port \""
						+ (*it)->getId()->getString()
						+ "\" of mismatching data types: in = "
						+ (*it)->getDataType().first.toString() + "; out = "
						+ (*it)->getDataType().second.toString());
    	}
    }
    list<IOPort*> out_ports = getOutIOPorts();
    for (list<IOPort*>::iterator it = out_ports.begin(); it != out_ports.end(); ++it){
    	if((*it)->getDataType().first != (*it)->getDataType().second){
    		THROW_EXCEPTION(InvalidProcessException, string("Process \"")
						+ getId()->getString() + "\" of type \""
						+ type() + "\" has port \""
						+ (*it)->getId()->getString()
						+ "\" of mismatching data types: in = "
						+ (*it)->getDataType().first.toString() + "; out = "
						+ (*it)->getDataType().second.toString());
    	}
    }
}

///////////////


Composite::IOPort::IOPort(const Id& id, Composite* process, CDataType datatype) throw(RuntimeException, InvalidArgumentException)
		:  Interface(id,process), connected_port_inside_(NULL), connected_port_outside_(NULL),
		   data_types_(pair<CDataType, CDataType>(datatype, datatype)){}

Composite::IOPort::~IOPort() throw() {
    //unconnectOutside();
    //unconnectInside();
}

std::pair<f2cc::CDataType, f2cc::CDataType> Composite::IOPort::getDataType() throw(RuntimeException) {
    return data_types_;
}

void Composite::IOPort::setDataType(bool outside, CDataType datatype) throw(RuntimeException) {
	if (outside) data_types_.first = datatype;
	else data_types_.second = datatype;
}

bool Composite::IOPort::isConnectedOutside() const throw(RuntimeException) {
    return connected_port_outside_;
}

bool Composite::IOPort::isConnectedInside() const throw(RuntimeException) {
    return connected_port_inside_;
}

void Composite::IOPort::setConnection(Process::Interface* port, bool outside) throw() {
	if (outside) connected_port_outside_ = port;
	else connected_port_inside_ = port;
}

void Composite::IOPort::connect(Process::Interface* port)
throw(RuntimeException, IllegalStateException, InvalidArgumentException, CastException) {
    if (port == this) return;
    if (!port) {
        unconnectInside();
        unconnectOutside();
        return;
    }

    //outside guard
	if (!getProcess()) {
		THROW_EXCEPTION(IllegalStateException, string("Error in: ")
					+ toString()
					+ "! Finding relation without hierarchy is not possible");
	}

	Hierarchy::Relation relation = getProcess()->findRelation(port->getProcess());


    // Checking if other end is IOPort
	Composite::IOPort* ioport_to_connect = dynamic_cast<Composite::IOPort*>(port);
	if (ioport_to_connect) {
		if (relation == Hierarchy::Sibling){
			if (connected_port_outside_) unconnect(connected_port_outside_);
			checkType(data_types_.first, ioport_to_connect->getDataType().first);
			connected_port_outside_ = ioport_to_connect;
			ioport_to_connect->connected_port_outside_ = this;
			return;
		}
		else if (relation == Hierarchy::FirstParent){
			if (connected_port_outside_) unconnect(connected_port_outside_);
			checkType(data_types_.first, ioport_to_connect->getDataType().second);
			connected_port_outside_ = ioport_to_connect;
			ioport_to_connect->connected_port_inside_ = this;
			return;
		}
		else if (relation == Hierarchy::FirstChild){
			if (connected_port_inside_) unconnect(connected_port_inside_);
			checkType(data_types_.second, ioport_to_connect->getDataType().first);
			connected_port_inside_ = ioport_to_connect;
			ioport_to_connect->connected_port_outside_ = this;
			return;
		}
		else THROW_EXCEPTION(InvalidArgumentException, string("Connection not possible")
				+ "with port \""
				+ port->toString()
				+ "\". Port is not in scope of vision");
	}


	 // Checking if other end is Port
	Leaf::Port* port_to_connect = dynamic_cast<Leaf::Port*>(port);
	if (port_to_connect) {
		if (relation == Hierarchy::Sibling){
			if (connected_port_outside_) unconnect(connected_port_outside_);
			checkType(data_types_.second, port_to_connect->getDataType());
			connected_port_outside_ = port_to_connect;
			port_to_connect->setConnection(this);
			return;
		}
		else if (relation == Hierarchy::FirstChild){
			if (connected_port_inside_) unconnect(connected_port_inside_);
			checkType(data_types_.first, port_to_connect->getDataType());
			connected_port_inside_ = port_to_connect;
			port_to_connect->setConnection(this);
			return;
		}
		else THROW_EXCEPTION(InvalidArgumentException, string("Connection not possible")
				+ "with port \""
				+ port->toString()
				+ "\". Port is not in scope of vision");
	}

	// It should never be here
	THROW_EXCEPTION(CastException, string("Critical error in ")
					+ toString()
					+ "! Connected port is of unknown type");

}


void Composite::IOPort::unconnect(Process::Interface* port) throw(RuntimeException, InvalidArgumentException) {
	if (connected_port_inside_ == port) unconnectInside();
	else if (connected_port_outside_ == port) unconnectOutside();
	else THROW_EXCEPTION(InvalidArgumentException, "Connection should not have been possible");

}

void Composite::IOPort::unconnectOutside() throw(RuntimeException, IllegalStateException,
		CastException) {
    if (connected_port_outside_) {
        //outside guard
    	if (!getProcess()) {
    		THROW_EXCEPTION(IllegalStateException, string("Error in: ")
    					+ toString()
    					+ "! Finding relation without hierarchy is not possible");
    	}

    	Hierarchy::Relation relation = getProcess()->findRelation(connected_port_outside_->getProcess());


        // Checking if other end is IOPort
    	Composite::IOPort* ioport_to_unconnect = dynamic_cast<Composite::IOPort*>(connected_port_outside_);
    	if (ioport_to_unconnect) {
    		if (relation == Hierarchy::Sibling){
    			ioport_to_unconnect->connected_port_outside_ = NULL;
    			connected_port_outside_ = NULL;
    			return;
    		}
    		else if (relation == Hierarchy::FirstParent){
    			ioport_to_unconnect->connected_port_inside_ = NULL;
				connected_port_outside_ = NULL;
    			return;
    		}
    		else THROW_EXCEPTION(IllegalStateException, "Connection not possible");
    	}


    	 // Checking if other end is Port
    	Leaf::Port* port_to_unconnect = dynamic_cast<Leaf::Port*>(connected_port_outside_);
    	if (port_to_unconnect) {
    		if (relation == Hierarchy::Sibling){
    			port_to_unconnect->setConnection(NULL);
    			connected_port_outside_ = NULL;
    			return;
    		}
    		else THROW_EXCEPTION(IllegalStateException, "Connection not possible");
    	}

    	// It should never be here
    	THROW_EXCEPTION(CastException, string("Critical error in ")
    					+ toString()
    					+ "! Outside connection is of unknown type");
    }
}

void Composite::IOPort::unconnectInside() throw(RuntimeException, IllegalStateException, CastException) {
	if (connected_port_inside_) {
		//outside guard
		if (!getProcess()) {
			THROW_EXCEPTION(IllegalStateException, string("Error in: ")
						+ toString()
						+ "! Finding relation without hierarchy is not possible");
		}

		Hierarchy::Relation relation = getProcess()->findRelation(connected_port_inside_->getProcess());


		// Checking if other end is IOPort
		Composite::IOPort* ioport_to_unconnect = dynamic_cast<Composite::IOPort*>(connected_port_inside_);
		if (ioport_to_unconnect) {
			if (relation == Hierarchy::FirstChild){
				ioport_to_unconnect->connected_port_outside_ = NULL;
				connected_port_inside_ = NULL;
				return;
			}
			else THROW_EXCEPTION(IllegalStateException, "Connection not possible");
		}


		 // Checking if other end is Port
		Leaf::Port* port_to_unconnect = dynamic_cast<Leaf::Port*>(connected_port_inside_);
		if (port_to_unconnect) {
			//if (relation == Hierarchy::FirstChild){
				port_to_unconnect->setConnection(NULL);
				connected_port_inside_ = NULL;
				return;
			//}
			//else THROW_EXCEPTION(IllegalStateException, "Connection not possible");
		}

		// It should never be here
		THROW_EXCEPTION(CastException, string("Critical error in ")
						+ toString()
						+ "! Outside connection is of unknown type");
	}
}

Process::Interface* Composite::IOPort::getConnectedPortOutside() const throw(RuntimeException) {
    return connected_port_outside_;
}

Process::Interface* Composite::IOPort::getConnectedPortInside() const throw(RuntimeException) {
    return connected_port_inside_;
}


bool Composite::IOPort::isConnectedToLeafOutside() const throw(RuntimeException) {
    if (connected_port_outside_) {
    	static const Composite::IOPort* ioport = dynamic_cast<const Composite::IOPort*>(connected_port_outside_);
    	if (ioport) return ioport->isConnectedToLeaf(this);
    	else return true;
    }
    else return false;
}

bool Composite::IOPort::isConnectedToLeafInside() const throw(RuntimeException) {
    if (connected_port_inside_) {
    	static const Composite::IOPort* ioport = dynamic_cast<const Composite::IOPort*>(connected_port_inside_);
    	if (ioport) return ioport->isConnectedToLeaf(this);
    	else return true;
    }
    else return false;
}


bool Composite::IOPort::isConnectedToLeaf(const Process::Interface* startpoit) const throw(RuntimeException) {
    if (startpoit == connected_port_outside_) {
    	static const Composite::IOPort* ioport = dynamic_cast<const Composite::IOPort*>(connected_port_inside_);
    	if (ioport) return ioport->isConnectedToLeaf(this);
    	else return true;
    }
    if (startpoit == connected_port_inside_) {
    	static const Composite::IOPort* ioport = dynamic_cast<const Composite::IOPort*>(connected_port_outside_);
    	if (ioport) return ioport->isConnectedToLeaf(this);
    	else return true;
    }
    return false;
}


bool Composite::IOPort::unconnectFromLeafOutside() throw(RuntimeException) {
	if (isConnectedToLeafOutside()) {
    	static Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(connected_port_outside_);
    	if (ioport) {
    		ioport->unconnectFromLeaf(this);
    		unconnectOutside();
    	}
    	else unconnectOutside();
    	return true;
    }
    return false;
}

bool Composite::IOPort::unconnectFromLeafInside() throw(RuntimeException) {
	if (isConnectedToLeafInside()) {
    	static Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(connected_port_outside_);
    	if (ioport) {
    		ioport->unconnectFromLeaf(this);
    		unconnectInside();
    	}
    	else unconnectInside();
    	return true;
    }
    return false;
}


bool Composite::IOPort::unconnectFromLeaf(Process::Interface* previous) throw(RuntimeException) {
    if (previous == connected_port_outside_) {
    	static Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(connected_port_inside_);
    	if (ioport) {
    		ioport->unconnectFromLeaf(this);
    		unconnectInside();
    		return true;
    	}
    	else{
    		unconnectInside();
    		return true;
    	}
    }
    if (previous == connected_port_inside_) {
    	static Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(connected_port_outside_);
    	if (ioport) {
    		ioport->unconnectFromLeaf(this);
    		unconnectOutside();
    		return true;
    	}
    	else{
    		unconnectOutside();
    		return true;
    	}
    }
    return false;
}

Leaf::Port* Composite::IOPort::getConnectedLeafPortOutside() const throw(RuntimeException, CastException) {
	Hierarchy::Relation relation = getProcess()->findRelation(connected_port_outside_->getProcess());
	static Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(connected_port_outside_);
	Leaf::Port* port = dynamic_cast<Leaf::Port*>(connected_port_outside_);
	if (ioport){
		if (relation == Hierarchy::Sibling) return ioport->getConnectedLeafPortInside();
		else if (relation == Hierarchy::FirstParent) return ioport->getConnectedLeafPortOutside();
		else {
			THROW_EXCEPTION(InvalidArgumentException, "Connection should not have been possible");
			return NULL;
		}
	}
	else if (port) return port;
	else{
	// It should never be here
	THROW_EXCEPTION(CastException, string("Critical error in ")
					+ toString()
					+ "! Outside connection is of unknown type");
	}
}

Leaf::Port* Composite::IOPort::getConnectedLeafPortInside() const throw(RuntimeException, CastException) {
	Hierarchy::Relation relation = getProcess()->findRelation(connected_port_inside_->getProcess());
	static Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(connected_port_inside_);
	Leaf::Port* port = dynamic_cast<Leaf::Port*>(connected_port_outside_);
	if (ioport){
		if (relation == Hierarchy::FirstChild) return ioport->getConnectedLeafPortInside();
		else {
			THROW_EXCEPTION(InvalidArgumentException, "Connection should not have been possible");
			return NULL;
		}
	}
	else if (port) return port;
	else{
	// It should never be here
	THROW_EXCEPTION(CastException, string("Critical error in ")
					+ toString()
					+ "! Outside connection is of unknown type");
	}
}

string Composite::IOPort::moretoString() const throw(RuntimeException) {
    string str;
    str += "(I/O)";
    return str;
}

void Composite::IOPort::checkType(f2cc::CDataType first_type, f2cc::CDataType second_type) throw (IllegalCallException){
/*
	if (first_type != second_type){
		THROW_EXCEPTION(IllegalCallException, string()
				+"Cannot connect "
				+ first_type.toString()
				+ " with " + second_type.toString());
	}
	*/
}

