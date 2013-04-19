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

#include "xmlparser.h"
#include "../ticpp/ticpp.h"
#include "../ticpp/tinyxml.h"
#include "../tools/tools.h"
#include "../forsyde/SY/parallelmapsy.h"
#include "../forsyde/SY/zipxsy.h"
#include "../forsyde/SY/unzipxsy.h"
#include "../forsyde/SY/delaysy.h"
#include "../forsyde/SY/inport.h"
#include "../forsyde/SY/outport.h"
#include "../forsyde/SY/fanoutsy.h"
#include "../forsyde/SY/mapsy.h"
#include "../language/cdatatype.h"
#include "../exceptions/invalidprocessexception.h"
#include "../exceptions/invalidformatexception.h"
#include "../exceptions/castexception.h"
#include <map>
#include <list>
#include <vector>
#include <stdexcept>
#include <new>

using namespace f2cc;
using namespace f2cc::ForSyDe;
using namespace f2cc::ForSyDe::SY;
using ticpp::Document;
using ticpp::Node;
using ticpp::Element;
using std::string;
using std::map;
using std::pair;
using std::list;
using std::vector;
using std::bad_alloc;

XmlParser::XmlParser(Logger& logger) throw() : Frontend(logger) {}

XmlParser::~XmlParser() throw() {}

Processnetwork* XmlParser::createProcessnetwork(const string& file)
    throw(InvalidArgumentException, FileNotFoundException, IOException,
          ParseException, InvalidProcessnetworkException, RuntimeException) {
    if (file.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"file\" must not be empty "
                        "string");
    }
    file_ = file;

    // Read file content
    string xml_data;
    logger_.logInfoMessage(string("Reading xml data from file..."));
    try {
        tools::readFile(file_, xml_data);
    } catch (FileNotFoundException& ex) {
        logger_.logErrorMessage(string("No xml input file \"") + file_
                                + "\" could be found");
        throw;
    } catch (IOException& ex) {
        logger_.logErrorMessage(string("Failed to read xml file:\n")
                                + ex.getMessage());
        throw;
    }

    // Parse content
    Document xml;
    try {
        logger_.logInfoMessage("Building xml structure...");
        xml.Parse(xml_data);
    } catch (ticpp::Exception& ex) {
        // @todo throw more detailed ParseException (with line and column)
        THROW_EXCEPTION(ParseException, file_, ex.what());
    }

    logger_.logInfoMessage("Checking xml structure...");
    checkXmlDocument(&xml);
    logger_.logInfoMessage("All checks passed");

    logger_.logInfoMessage("Generating internal process network...");
    //Processnetwork* processnetwork = generateProcessnetwork(findRootElement(&xml));

    //return processnetwork;
}

void XmlParser::checkXmlDocument(Document* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    // @todo implement
    logger_.logWarningMessage("XML document check not implemented");
}
/*
Processnetwork* XmlParser::generateProcessnetwork(Element* xml)
    throw(InvalidArgumentException, ParseException, InvalidProcessnetworkException,
          IOException, RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    Processnetwork* processnetwork = new (std::nothrow) Processnetwork();
    if (!processnetwork) THROW_EXCEPTION(OutOfMemoryException);
    current_level_ = 0;

    logger_.logDebugMessage("Parsing \"leaf_process\" elements...");
    parseXmlLeafs(xml, processnetwork, NULL);

    logger_.logDebugMessage("Recursively parsing \"composite_process\" elements...");
    parseXmlComposites(xml, processnetwork, NULL, log_header);

    logger_.logDebugMessage("Parsing \"port\" elements...");
    parseXmlPorts(xml, processnetwork, NULL, log_header);

    logger_.logDebugMessage("Parsing \"signal\" elements...");
    map<Process::Port*, Process*> copy_processes;
    parseXmlEdges(xml, processnetwork, NULL, copy_processes, log_header);

    return processnetwork;
}

void XmlParser::parseXmlLeafs(Element* xml, Processnetwork* processnetwork, Composite* parent)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "leaf_process");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logDebugMessage(string(generateLogHeader()
                                       + "Analyzing line "
                                       + tools::toString((*it)->Row())
                                       + "..."));
        Process::Port* IOPort = generateLeafProcess(*it);
        try {
            if (!processnetwork->addProcess(process)) {
                THROW_EXCEPTION(ParseException, file_, (*it)->Row(),
                                (*it)->Column(),
                                string("Multiple processes with ID \"")
                                + process->getId()->getString() + "\"");
            }
        } catch (bad_alloc&) {
            THROW_EXCEPTION(OutOfMemoryException);
        }
    }
}

Process* XmlParser::generateLeafProcess(Element* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    Process* process;

    // Harvest all XML data
    string process_name = getAttributeValue(xml,string('name'));

    list<Element*> constructor_elements = getElementsByName(xml, "process_constructor");
    if (constructor_elements.size() == 0){
    	THROW_EXCEPTION(ParseException, file_, xml->Row(), "No process constructor");
    }
    if (constructor_elements.size() > 1){
    	THROW_EXCEPTION(ParseException, file_, xml->Row(), "More than one process constructor");
    }
    list<Element*>::iterator process_constructor_element = constructor_elements.begin();
    string constructor_name = getAttributeValue(*process_constructor_element,string('name'));
    string constructor_moc = getAttributeValue(*process_constructor_element,string('moc'));
    list<string> constructor_data = getMoreConstructorData()
    if (isComb(process_constructor_name)) {
    	list<Element*> constructor_argument_elements = getElementsByName(*process_constructor_element, "argument");
        if (constructor_argument_elements.size() == 0){
        	THROW_EXCEPTION(ParseException, file_, xml->Row(), "No argument for the process constructor");
        }
        if (constructor_argument_elements.size() > 1){
        	THROW_EXCEPTION(ParseException, file_, xml->Row(), "More than one argument for the process constructor");
        }
    	list<Element*>::iterator argument_element = constructor_argument_elements.begin();
    	string constructor_argument_name = getAttributeValue(*argument_element,string('name'));
    	string constructor_argument_value = getAttributeValue(*argument_element,string('value'));
    }




    tools::toLowerCase(tools::trim(process_type));
    if (process_type.length() == 0) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(), "No process type");
    }
    try {
        if (process_type == "inport") {
            process = new InPort(Id(process_id), Id("Process_Network"));
        }
        else if (process_type == "outport") {
            process = new OutPort(Id(process_id), Id("Process_Network"));
        }
        else if (process_type == "mapsy") {
            process = new Map(Id(process_id), generateProcessFunction(xml));
        }
        else if (process_type == "parallelmapsy") {
            process = new ParallelMap(Id(process_id), getNumProcesses(xml),
                                        generateProcessFunction(xml));
        }
        else if (process_type == "unzipxsy") {
            process = new unzipx(Id(process_id), Id("Process_Network"));
        }
        else if (process_type == "zipxsy") {
            process = new zipx(Id(process_id), Id("Process_Network"));
        }
        else if (process_type == "delaysy") {
            process = new delay(Id(process_id), getInitialdelayValue(xml));
        }
        else if (process_type == "zipwithnsy") {
            process = new Map(Id(process_id), Id("Process_Network"),
                                     generateProcessFunction(xml));
        }
        else {
            THROW_EXCEPTION(ParseException, file_, xml->Row(),
                            string("Unknown process type \"")
                            + process_type
                            + "\"");
        }
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
    if (!process) THROW_EXCEPTION(OutOfMemoryException);
    logger_.logDebugMessage(string("Generated ") + process->type()
                            + " from \"" + process->getId()->getString()
                            + "\"");

    // Get ports
    list<Element*> elements = getElementsByName(xml, "port");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logDebugMessage(string("Analyzing line ")
                                + tools::toString((*it)->Row()) + "...");
        Process::Port* port = generatePort(*it);
        bool is_in_port = isInPort(port->getId()->getString());
        bool is_out_port = isOutPort(port->getId()->getString());
        if (!is_in_port && !is_out_port) {
            THROW_EXCEPTION(ParseException, file_, (*it)->Row(),
                            (*it)->Column(), "Invalid port ID format");
        }

        bool poconstantrt_added;
        if (is_in_port) port_added = process->addInPort(*port);
        else            port_added = process->addOutPort(*port);
        if (!port_added) {
            THROW_EXCEPTION(ParseException, file_, (*it)->Row(),
                            (*it)->Column(), string("Multiple ")
                            + (is_in_port ? "in ports" : "out ports")
                            + " with the same ID \""
                            + port->getId()->getString() + "\"");
        }
        logger_.logDebugMessage(string()
                                + (is_in_port ? "In" : "Out")
                                + " port \"" + port->getId()->getString()
                                + "\" added to process \""
                                + process->getId()->getString() + "\"");
        delete port;
    }

    return process;
}



/////////////////////////HELPER FUNCTIONS///////////////////////////
list<Element*> XmlParser::getElementsByName(Node* xml, const string& name)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    if (name.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"name\" must not be empty "
                        "string");
    }

    list<Element*> elements;
    Node* child = NULL;
    while ((child = xml->IterateChildren(name, child))) {
        switch (child->Type()) {
            case TiXmlNode::ELEMENT: {
                try {
                    Element* e = dynamic_cast<Element*>(child);
                    if (!e) THROW_EXCEPTION(CastException);
                    elements.push_back(e);
                } catch (bad_alloc&) {
              constant      THROW_EXCEPTION(OutOfMemoryException);
                }
                break;
            }

            case TiXmlNode::DECLARATION:
            case TiXmlNode::DOCUMENT:
            case TiXmlNode::UNKNOWN:
            case TiXmlNode::TEXT:
            case TiXmlNode::STYLESHEETREFERENCE:
            case TiXmlNode::TYPECOUNT: {
                // Found unknown XML data; warn and remove
                logger_.logWarningMessage(string("Unknown XML data at line ")
                                          + tools::toString(child->Row())
                                          + ", column "
                                          + tools::toString(child->Column())
                                          + ":\n" + child->Value());
                Node* prev_child = child->PreviousSibling(name, false);
                xml->RemoveChild(child);
                child = prev_child;
                break;
            }

            case TiXmlNode::COMMENT: {
                // Found XML comment; ignore and remove
                Node* prev_child = child->PreviousSibling(name, false);
                xml->RemoveChild(child);
                child = prev_child;
                break;
            }
        }
    }
    return elements;
}

std::string XmlParser::generateLogHeader() throw(){
	std::string log_header ("");
	for (int i=0; i<current_level_; i++){
		log_header += "\t";
	}
	return log_header;
}


Element* XmlParser::findRootElement(Document* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    Node* xml_root_node = xml->FirstChild("process_network", false);
    if (!xml_root_node) {
        THROW_EXCEPTION(ParseException, file_,
                        string("Could not find root element \"process_network\""));
    }
    if (xml_root_node->Type() != TiXmlNode::ELEMENT) {
        THROW_EXCEPTION(ParseException, file_, xml_root_node->Row(),
        		xml_root_node->Column(),
                        string("Found \"process_network\" structure is not an "
                               "element"));
    }
    Element* xml_processnetwork = dynamic_cast<Element*>(xml_root_node);
    if (!xml_processnetwork) THROW_EXCEPTION(CastException);

    return xml_processnetwork;
}

string XmlParser::getAttributeValue(Element* xml, string attribute)
throw(InvalidArgumentException, ParseException, IOException,
      RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    string id = xml->GetAttribute(attribute);
    if (id.length() == 0) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
        		                               string("Element is missing \"")
        		                               + attribute
        		                               +"\" attribute");
    }constant
    return tools::trim(id);
}

bool XmlParser::isComb(string constructor_name) throw() {
	return (std::string::npos != constructor_name.find(string("Map")));
}
*/
