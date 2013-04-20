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

Composite::Composite(const Id& id, const Id& name) throw() : Process(id, string("")), composite_name_(name){}

Composite::~Composite() throw() {
	destroyAllProcesses();
}

const Id* Composite::getName() const throw() {
    return &composite_name_;
}

const string Composite::getMoc() const throw(){return string("Irrelevant");}

int Composite::getCost() const throw() {return 0;}

void Composite::setCost(int cost) const throw(){}

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

///////////////
/*
Composite::IOPort::IOPort(const Id& id) throw()
        : Port(id,CDataType()), connected_port_inside_(NULL)  {}
*/
Composite::IOPort::IOPort(const Id& id, Composite* process) throw(InvalidArgumentException)
		:  Port(id,process,CDataType()), connected_port_inside_(NULL) {}

Composite::IOPort::IOPort(Port& rhs) throw()
		: Port(*rhs.getId(), CDataType()),
		  connected_port_inside_(NULL){

	static const Composite::IOPort* ioport = dynamic_cast<const Composite::IOPort*>(&rhs);
	if (ioport) {
		THROW_EXCEPTION(InvalidArgumentException, string("Right hand equivalence between IOPorts without a hierarchy")
						+" is not allowed!");
	}
	Port* port = rhs.PortGetter();
	Composite::IOPort* connected_ioport = dynamic_cast<Composite::IOPort*>(port);
	if (connected_ioport){
		THROW_EXCEPTION(InvalidArgumentException, string("Loopbacks within IOPorts")
						+" is not allowed!");
	}
	rhs.PortSetter(NULL);
	port->PortSetter(this);
	connected_port_inside_ = port;
}
/*
Composite::IOPort::IOPort(IOPort& rhs) throw()
		: Port(*rhs.getId(), rhs.getProcess(), CDataType()),
		  connected_port_inside_(NULL){

	if (rhs.connected_port_outside_) {
		Composite::IOPort* connected_ioport = dynamic_cast<Composite::IOPort*>(rhs.connected_port_outside_);
		if (connected_ioport) {
			Port* port = new Port(rhs);
			connected_ioport->unconnect(port);
			connected_ioport->connectNonVirtual(this);
		}
		else {
			Port* port = rhs.PortGetter();
			rhs.PortSetter(NULL);
			if (port != this) {
				connectNonVirtual(port);
				port->PortSetter(this);
			}
		}
	}
	if (rhs.connected_port_inside_) {
		Composite::IOPort* connected_ioport = dynamic_cast<Composite::IOPort*>(rhs.connected_port_inside_);
		if (connected_ioport) {
			Port* port = new Port(rhs);
			connected_ioport->unconnect(port);
			connected_ioport->connectNonVirtual(this);
		}
		else {
			Port* port = rhs.PortGetter();
			rhs.PortSetter(NULL);
			if (port != this) {
				connectNonVirtual(port);
				port->PortSetter(this);
			}
		}
	}
}*/
/*
Composite::IOPort::IOPort(Port& rhs, Composite* process) throw(InvalidArgumentException)
        : Port(*rhs.getId(),process,CDataType()), connected_port_inside_(NULL) {
    if (!process) {
        THROW_EXCEPTION(InvalidArgumentException, "\"process\" must not be "
                        "NULL");
    }
	if (rhs.PortGetter()) {
		Composite::IOPort* connected_ioport = dynamic_cast<Composite::IOPort*>(rhs.PortGetter());
		if (connected_ioport) {
			Port* port = new Port(rhs);
			connected_ioport->unconnect(port);
			connected_ioport->connectNonVirtual(this);
		}
		else {
			Port* port = rhs.PortGetter();
			rhs.PortSetter(NULL);
			if (port != this) {
				connectNonVirtual(port);
				port->PortSetter(this);
			}
		}
	}
}
*/
Composite::IOPort::IOPort(IOPort& rhs, Composite* process) throw(InvalidArgumentException)
        : Port(*rhs.getId(),process,CDataType()), connected_port_inside_(NULL) {
    if (!process) {
        THROW_EXCEPTION(InvalidArgumentException, "\"process\" must not be "
                        "NULL");
    }
	if (rhs.connected_port_outside_) {
		IOPort* connected_ioport = dynamic_cast<IOPort*>(rhs.connected_port_outside_);
		if (connected_ioport) {
			IOPort* ioport = connected_ioport;
			rhs.unconnect(ioport);
			connectNonVirtual(ioport);
		}
		else {
			Port* port = rhs.connected_port_outside_;
			rhs.unconnect(port);
			connectNonVirtual(port);
		}
	}
	if (rhs.connected_port_inside_) {
		Composite::IOPort* connected_ioport = dynamic_cast<Composite::IOPort*>(rhs.connected_port_inside_);
		if (connected_ioport) {
			IOPort* ioport = connected_ioport;
			rhs.unconnect(ioport);
			connectNonVirtual(connected_ioport);
		}
		else {
			Port* port = rhs.connected_port_inside_;
			rhs.unconnect(port);
			connectNonVirtual(port);

		}
	}
}

Composite::IOPort::~IOPort() throw() {
    unconnectOutside();
    unconnectInside();
}

f2cc::CDataType Composite::IOPort::getDataType() throw() {return CDataType();}
bool Composite::IOPort::setDataType(CDataType& datatype) throw(){return false;}

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

void Composite::IOPort::connect(Port* port) throw(InvalidArgumentException) {
    if (port == this) return;
    if (!port) {
        unconnect();
        return;
    }
	Hierarchy::Relation relation = getProcess()->findRelation(port->getProcess());
	static Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(port);
	if (relation == Hierarchy::Sibling) {
		connected_port_outside_ = port;
		port->PortSetter(this);
	}
	else if (relation == Hierarchy::FirstParent) {
		connected_port_outside_ = port;
		if (!ioport) THROW_EXCEPTION(InvalidArgumentException, "Something");
		ioport->connected_port_inside_ = this;
	}
	else if (relation == Hierarchy::FirstChild) {
		connected_port_inside_ = port;
		port->PortSetter(this);
	}
	else THROW_EXCEPTION(InvalidArgumentException, "Connection not possible");
}

void Composite::IOPort::unconnect() throw() {
	unconnectInside();

}

void Composite::IOPort::unconnect(Port* port) throw(InvalidArgumentException) {
	if (connected_port_inside_ == port) unconnectInside();
	else if (connected_port_outside_ == port) unconnectOutside();
	else THROW_EXCEPTION(InvalidArgumentException, "Connection should not have been possible");

}

void Composite::IOPort::unconnectOutside() throw(InvalidArgumentException) {
    if (connected_port_outside_) {
    	static Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(connected_port_outside_);
    	Hierarchy::Relation relation = getProcess()->findRelation(connected_port_outside_->getProcess());
    	if (relation == Hierarchy::Sibling) {
    		connected_port_outside_->PortSetter(NULL);
    		connected_port_outside_ = NULL;
    	}
    	else if (relation == Hierarchy::FirstParent) {
    		ioport->connected_port_inside_ = NULL;
    		connected_port_outside_ = NULL;
    	}
    	else THROW_EXCEPTION(InvalidArgumentException, "Connection should not have been possible");
    }
}

void Composite::IOPort::unconnectInside() throw(InvalidArgumentException) {
    if (connected_port_inside_) {
        Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(connected_port_inside_);
        if (ioport){
			Hierarchy::Relation relation = getProcess()->findRelation(connected_port_inside_->getProcess());
			std::cout<<getProcess()->toString();
			if (relation == Hierarchy::FirstChild) {
				ioport->connected_port_outside_ = NULL;
				ioport = NULL;
			}
			else THROW_EXCEPTION(InvalidArgumentException, "Connection should not have been possible");
        }
        else {
        	connected_port_inside_->PortSetter(NULL);
        	connected_port_inside_ = NULL;
        }
    }
}

void Composite::IOPort::unconnectFromLeafOutside() throw(InvalidArgumentException) {
    if (connected_port_outside_) {
    	Hierarchy::Relation relation = getProcess()->findRelation(connected_port_outside_->getProcess());
    	static Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(connected_port_outside_);
    	if (relation == Hierarchy::Sibling) {
    		if (ioport) ioport->unconnectFromLeafInside();
    		else connected_port_outside_->PortSetter(NULL);
    		connected_port_outside_ = NULL;
    	}
    	else if (relation == Hierarchy::FirstParent) {
    		if (ioport) ioport->unconnectFromLeafOutside();
    		else connected_port_outside_->PortSetter(NULL);
    		connected_port_outside_ = NULL;
    	}
    	else THROW_EXCEPTION(InvalidArgumentException, "Connection should not have been possible");
    }
}

void Composite::IOPort::unconnectFromLeafInside() throw(InvalidArgumentException) {
    if (connected_port_inside_) {
    	Hierarchy::Relation relation = getProcess()->findRelation(connected_port_outside_->getProcess());
    	static Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(connected_port_outside_);
    	if (relation == Hierarchy::FirstChild) {
    		if (ioport) ioport->unconnectFromLeafInside();
    		else connected_port_inside_->PortSetter(NULL);
    		connected_port_inside_ = NULL;
    	}
    	else THROW_EXCEPTION(InvalidArgumentException, "Connection should not have been possible");
    }
}

Process::Port* Composite::IOPort::getConnectedPort() const throw() {
	return connected_port_inside_;
}

Process::Port* Composite::IOPort::getConnectedPortOutside() const throw() {
    return connected_port_outside_;
}

Process::Port* Composite::IOPort::getConnectedPortInside() const throw() {
    return connected_port_inside_;
}

Process::Port* Composite::IOPort::getConnectedLeafPortOutside() const throw(InvalidArgumentException) {
	Hierarchy::Relation relation = getProcess()->findRelation(connected_port_outside_->getProcess());
	static Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(connected_port_outside_);
	if (ioport){
		if (relation == Hierarchy::Sibling) return ioport->getConnectedLeafPortInside();
		else if (relation == Hierarchy::FirstParent) return ioport->getConnectedLeafPortOutside();
		else {
			THROW_EXCEPTION(InvalidArgumentException, "Connection should not have been possible");
			return NULL;
		}
	}
	else return connected_port_outside_;
}

Process::Port* Composite::IOPort::getConnectedLeafPortInside() const throw(InvalidArgumentException) {
	Hierarchy::Relation relation = getProcess()->findRelation(connected_port_outside_->getProcess());
	static Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(connected_port_outside_);
	if (ioport){
		if (relation == Hierarchy::FirstChild) return ioport->getConnectedLeafPortInside();
		else {
			THROW_EXCEPTION(InvalidArgumentException, "Connection should not have been possible");
			return NULL;
		}
	}
	else return connected_port_inside_;
}


string Composite::IOPort::toString() const throw() {
    string str;
    if (process_) str += process_->getId()->getString();
    else          str += "NULL";
    str += ":";
    str += id_.getString();
    return str;
}


void Composite::IOPort::connectPrvIn(Port* port) throw(InvalidArgumentException) {
    if (port == this) return;
    if (!port) {
    	connected_port_inside_ = NULL;
    	connected_port_inside_->PortSetter(NULL);
        return;
    }
	connected_port_inside_ = port;
	port->PortSetter(this);
}

void Composite::IOPort::connectPrvOut(Port* port) throw(InvalidArgumentException) {
    if (port == this) return;
    if (!port) {
    	connected_port_outside_ = NULL;
    	connected_port_outside_->PortSetter(NULL);
        return;
    }
    connected_port_outside_ = port;
	port->PortSetter(this);

}

void Composite::IOPort::connectNonVirtual(Port* port) throw(InvalidArgumentException) {
    if (port == this) return;
    if (!port) {
        unconnect();
        return;
    }
	Hierarchy::Relation relation = getProcess()->findRelation(port->getProcess());
	static Composite::IOPort* ioport = dynamic_cast<Composite::IOPort*>(port);
	if (relation == Hierarchy::Sibling) {
		connected_port_outside_ = port;
		port->PortSetter(this);
	}
	else if (relation == Hierarchy::FirstParent) {
		connected_port_outside_ = port;
		if (!ioport) THROW_EXCEPTION(InvalidArgumentException, "Something");
		ioport->connected_port_inside_ = this;
	}
	else if (relation == Hierarchy::FirstChild) {
		connected_port_inside_ = port;
		port->PortSetter(this);
	}
	else THROW_EXCEPTION(InvalidArgumentException, "Connection not possible");
}
