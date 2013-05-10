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

#include "dumper.h"
#include "../ticpp/ticpp.h"
#include "../ticpp/tinyxml.h"
#include "../tools/tools.h"
#include "../forsyde/SY/combsy.h"
#include "../forsyde/SY/mapsy.h"
#include "../forsyde/SY/parallelmapsy.h"
#include "../forsyde/SY/zipxsy.h"
#include "../forsyde/SY/unzipxsy.h"
#include "../forsyde/SY/delaysy.h"
#include "../forsyde/SY/inport.h"
#include "../forsyde/SY/outport.h"
#include "../forsyde/SY/fanoutsy.h"
#include "../forsyde/SY/zipwithnsy.h"
#include "../language/cdatatype.h"
#include "../exceptions/invalidprocessexception.h"
#include "../exceptions/invalidformatexception.h"
#include "../exceptions/castexception.h"
#include <list>

using namespace f2cc;
using namespace f2cc::Forsyde;
using ticpp::Document;
using ticpp::Node;
using ticpp::Element;
using ticpp::Declaration;
using std::string;
using std::list;

XmlDumper::XmlDumper(Logger& logger) throw() : logger_(logger) {}

XmlDumper::~XmlDumper() throw() {}

void XmlDumper::dump(ProcessNetwork* pn, const string& file)
    throw(InvalidArgumentException, IOException,
          InvalidModelException, RuntimeException){

    if (file.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"file\" must not be empty "
                        "string");
    }
    if (!pn) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }

    const char *xml_file = file.c_str();
    Document doc;
	Declaration * decl = new Declaration( "1.0", "", "" );
	doc.LinkEndChild( decl );

    logger_.logMessage(Logger::INFO, string() + "Building XML structure \""
            + " from internal Process Network"
    		+ " for dumping...");

    dumpProcessNetwork(pn, doc);

    logger_.logMessage(Logger::INFO, string() + "Dumping the process network into file \""
            + file
    		+ "\"...");

    doc.SaveFile( xml_file );
}

void XmlDumper::dumpProcessNetwork(ProcessNetwork* pn, Document doc)
    throw(InvalidArgumentException, InvalidModelException, RuntimeException){
    if (!pn) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }

    Element* element = new Element( "process_network" );
    doc.LinkEndChild( element );


    list<Process::Interface*> input_ports = pn->getInputs();
    for (list<Process::Interface*>::iterator it = input_ports.begin();
    		it != input_ports.end(); it++){
    	Element* port_element = new Element( "pointer_to_port" );
        port_element->SetAttribute("direction", "in");
        port_element->SetAttribute("pointed_process",
        		(*it)->getProcess()->getId()->getString().c_str());
        port_element->SetAttribute("pointed_port",
        		(*it)->getId()->getString().c_str());
        element->LinkEndChild( port_element );
    }
    list<Process::Interface*> output_ports = pn->getOutputs();
    for (list<Process::Interface*>::iterator it = output_ports.begin();
    		it != output_ports.end(); it++){
    	Element* port_element = new Element( "pointer_to_port" );
        port_element->SetAttribute("direction", "out");
        port_element->SetAttribute("pointed_process",
        		(*it)->getProcess()->getId()->getString().c_str());
        port_element->SetAttribute("pointed_port",
        		(*it)->getId()->getString().c_str());
        element->LinkEndChild( port_element );
    }

    list<Composite*> composites = pn->getComposites();
    list<Composite*>::iterator it;
    for (it = composites.begin(); it != composites.end(); it++){
    	if (!isVisitedProcess(*it)){
    		dumpComposite(*it, element);
    	}
    }
    list<Leaf*> leafs = pn->getProcesses();
    list<Leaf*>::iterator it1;
    for (it1 = leafs.begin(); it1 != leafs.end(); it1++){
    	if (!isVisitedProcess(*it1)){
    		dumpLeaf(*it1, element);
    	}
    }
}

void XmlDumper::dumpComposite(Composite* composite, Element* parent)
    throw(InvalidArgumentException, InvalidModelException, RuntimeException){

    if (!composite) {
        THROW_EXCEPTION(InvalidArgumentException, "\"composite\" must not be NULL");
    }
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }

    Element* curr_element = new Element( "composite" );
    curr_element->SetAttribute("name",
    		composite->getId()->getString().c_str());
    curr_element->SetAttribute("component_name",
    		composite->getName().getString().c_str());

    parent->LinkEndChild( curr_element );
    visited_processes_.push_back(composite);

    list<Composite::IOPort*> input_ports = composite->getInIOPorts();
    for (list<Composite::IOPort*>::iterator it = input_ports.begin();
    		it != input_ports.end(); it++){
    	dumpPort((*it), curr_element, "in");
    	if (((*it)->isConnectedOutside()) && (!isVisitedPort(*it))){
    		dumpIOSignal((*it), parent);
    	}
    }
    list<Composite::IOPort*> output_ports = composite->getOutIOPorts();
    for (list<Composite::IOPort*>::iterator it = output_ports.begin();
    		it != output_ports.end(); it++){
    	dumpPort((*it), curr_element, "out");
    	if (((*it)->isConnectedOutside()) && (!isVisitedPort(*it))){
    		dumpIOSignal((*it), parent);
    	}
    }

    list<Composite*> contained_composites = composite->getComposites();
    for (list<Composite*>::iterator it = contained_composites.begin();
    		it != contained_composites.end(); it++){
    	if (!isVisitedProcess(*it)){
    		dumpComposite(*it, curr_element);
    	}
    }
    list<Leaf*> contained_leafs = composite->getProcesses();
    for (list<Leaf*>::iterator it = contained_leafs.begin();
    		it != contained_leafs.end(); it++){
    	if (!isVisitedProcess(*it)){
    		dumpLeaf(*it, curr_element);
    	}
    }
}

void XmlDumper::dumpLeaf(Leaf* leaf, Element* parent)
    throw(InvalidArgumentException, InvalidModelException, RuntimeException){
    if (!leaf) {
        THROW_EXCEPTION(InvalidArgumentException, "\"port\" must not be NULL");
    }
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }

    Element* leaf_element = new Element( "leaf_process" );
    leaf_element->SetAttribute("name", leaf->getId()->getString().c_str());
    parent->LinkEndChild( leaf_element );
    visited_processes_.push_back(leaf);

    Element* constructor = new Element( "process_constructor" );
    constructor->SetAttribute("name", leaf->type());
    constructor->SetAttribute("moc", leaf->getMoc());
    leaf_element->LinkEndChild( constructor );

    SY::Comb* comb_leaf = dynamic_cast<SY::Comb*>(leaf);
    if (comb_leaf){
    	Element* argument = new Element( "argument" );
    	argument->SetAttribute("name", "_func");
    	argument->SetAttribute("value", comb_leaf->getFunction()->getName());
    	constructor->LinkEndChild( argument );
    }
    SY::delay* delay_leaf = dynamic_cast<SY::delay*>(leaf);
    if (delay_leaf){
    	Element* argument = new Element( "argument" );
    	argument->SetAttribute("name", "init_val");
    	argument->SetAttribute("value", delay_leaf->getInitialValue());
    	constructor->LinkEndChild( argument );
    }

    list<Leaf::Port*> input_ports = leaf->getInPorts();
    for (list<Leaf::Port*>::iterator it = input_ports.begin();
    		it != input_ports.end(); it++){
    	Element* port_element = new Element( "port" );
    	port_element->SetAttribute("name", (*it)->getId()->getString());
    	port_element->SetAttribute("type",
    			(*it)->getDataType().toString());
    	port_element->SetAttribute("direction", "in");
    	if (comb_leaf) port_element->SetAttribute("associated_variable",
    			(*it)->getVariable()->getReferenceString());
    	leaf_element->LinkEndChild(port_element);
    	if (!isVisitedPort(*it)){
    		dumpSignal((*it), parent);
    	}
    }
    list<Leaf::Port*> output_ports = leaf->getOutPorts();
    for (list<Leaf::Port*>::iterator it = output_ports.begin();
    		it != output_ports.end(); it++){
    	Element* port_element = new Element( "port" );
    	port_element->SetAttribute("name", (*it)->getId()->getString());
    	port_element->SetAttribute("type",
    			(*it)->getDataType().getVariableDataTypeString());
    	port_element->SetAttribute("direction", "out");
    	if (comb_leaf) port_element->SetAttribute("associated_variable",
    	    			(*it)->getVariable()->getReferenceString());
    	leaf_element->LinkEndChild(port_element);
    	if (!isVisitedPort(*it)){
    		dumpSignal((*it), parent);
    	}
    }

}

void XmlDumper::dumpPort(Composite::IOPort* port, Element* composite,
		const char* direction)
        throw(InvalidArgumentException, InvalidModelException, RuntimeException){
    if (!port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"port\" must not be NULL");
    }
    if (!composite) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }

    Element* port_element = new Element( "port" );
    port_element->SetAttribute("name", port->getId()->getString().c_str());
    port_element->SetAttribute("direction", direction);
    port_element->SetAttribute("bound_process",
    		port->getConnectedPortInside()->getProcess()->getId()->getString().c_str());
    port_element->SetAttribute("bound_port",
        		port->getConnectedPortInside()->getId()->getString().c_str());

    composite->LinkEndChild( port_element );
    visited_ports_.push_back(port->getConnectedPortInside());
    //visited_ports_.push_back(port);
}

void XmlDumper::dumpSignal(Leaf::Port* port, Element* composite)
    throw(InvalidArgumentException, InvalidModelException, RuntimeException){
    if (!port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"port\" must not be NULL");
    }
    if (!composite) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }

    Element* signal_element = new Element( "signal" );
    signal_element->SetAttribute("type",
    		port->getDataType().getVariableDataTypeString());
    signal_element->SetAttribute("source", port->getProcess()->getId()->getString());
    signal_element->SetAttribute("source_port", port->getId()->getString());
    signal_element->SetAttribute("target",
    		port->getConnectedPort()->getProcess()->getId()->getString());
    signal_element->SetAttribute("target_port",
    		port->getConnectedPort()->getId()->getString());

    composite->LinkEndChild( signal_element );
    visited_ports_.push_back(port);
    visited_ports_.push_back(port->getConnectedPort());

}

void XmlDumper::dumpIOSignal(Composite::IOPort* port, Element* composite)
    throw(InvalidArgumentException, InvalidModelException, RuntimeException){
    if (!port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"port\" must not be NULL");
    }
    if (!composite) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }

    Element* signal_element = new Element( "signal" );
    signal_element->SetAttribute("source", port->getProcess()->getId()->getString());
    signal_element->SetAttribute("source_port", port->getId()->getString());
    signal_element->SetAttribute("target",
    		port->getConnectedPortOutside()->getProcess()->getId()->getString());
    signal_element->SetAttribute("target_port",
    		port->getConnectedPortOutside()->getId()->getString());

    composite->LinkEndChild( signal_element );
    visited_ports_.push_back(port);
    visited_ports_.push_back(port->getConnectedPortOutside());

}

bool XmlDumper::isVisitedProcess(Process* process) throw(InvalidArgumentException){
    if (!process) {
        THROW_EXCEPTION(InvalidArgumentException, "\"process\" must not be NULL");
    }

    list<Process*>::iterator it;
    for (it = visited_processes_.begin(); it != visited_processes_.end(); it++){
    	if (*it == process) return true;
    }
    return false;
}

bool XmlDumper::isVisitedPort(Process::Interface* port) throw(InvalidArgumentException){
    if (!port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"process\" must not be NULL");
    }

    list<Process::Interface*>::iterator it;
    for (it = visited_ports_.begin(); it != visited_ports_.end(); it++){
    	if (*it == port) return true;
    }
    return false;
}
