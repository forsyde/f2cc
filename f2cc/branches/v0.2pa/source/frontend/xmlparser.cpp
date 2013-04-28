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

#include "xmlparser.h"
#include "../ticpp/ticpp.h"
#include "../ticpp/tinyxml.h"
#include "../tools/tools.h"
//#include "../forsyde/composite.h"
#include "../forsyde/SY/zipxsy.h"
#include "../forsyde/SY/unzipxsy.h"
#include "../forsyde/SY/delaysy.h"
#include "../forsyde/SY/fanoutsy.h"
#include "../forsyde/SY/combsy.h"
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

ProcessNetwork* XmlParser::createProcessNetwork(const string& file)
    throw(InvalidArgumentException, FileNotFoundException, IOException,
          ParseException, InvalidProcessNetworkException, RuntimeException) {
    if (file.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"file\" must not be empty "
                        "string");
    }

    level_ = 0;
    file_ = file;

    logger_.logMessage(Logger::INFO, "Generating internal process network...");
    ProcessNetwork* processnetwork = new (std::nothrow) ProcessNetwork();
    if (!processnetwork) THROW_EXCEPTION(OutOfMemoryException);

    generateComposite(processnetwork, Id("f2cc0"), Hierarchy(), parseXmlFile(file));

    return processnetwork;
}

Element* XmlParser::parseXmlFile(const string& file)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {

    // Read file content
    string xml_data;
    logger_.logMessage(Logger::DEBUG, string("Hierarchy level: " + tools::toString(level_)
                           + "; reading from file: " + file));
    logger_.logMessage(Logger::INFO, string(tools::indent(level_)
                           + "Reading xml data from file: "
                           + file
                           + "..."));
    try {
        tools::readFile(file, xml_data);
    } catch (FileNotFoundException& ex) {
        logger_.logMessage(Logger::ERROR, string("No xml input file \"") + file
                           + "\" could be found");
        throw;
    } catch (IOException& ex) {
        logger_.logMessage(Logger::ERROR, string("Failed to read xml file:\n")
                           + ex.getMessage());
        throw;
    }

    // Parse content
    Document xml_doc;
    try {
        logger_.logMessage(Logger::INFO, string(tools::indent(level_)
                           + file + ": "
                           + "Building xml structure..."));
        xml_doc.Parse(xml_data);
    } catch (ticpp::Exception& ex) {
        // @todo throw more detailed ParseException (with line and column)
        THROW_EXCEPTION(ParseException, file, ex.what());
    }

    logger_.logMessage(Logger::INFO, string(tools::indent(level_)
                           + file
                           + ": Checking xml structure..."));
    checkXmlDocument(&xml_doc);
    logger_.logMessage(Logger::INFO, string(tools::indent(level_)
                           + file
                           + ": All checks passed"));

    std::cout<<"\n\n\n OUTSIDE generateComposite() :" <<
    		getAttributeByTag(findXmlRootElement(&xml_doc, file), "name")<<"\n\n\n";
    return findXmlRootElement(&xml_doc, file);
}

void XmlParser::parseXmlLeafs(Element* xml, ProcessNetwork* processnetwork,
		Composite* parent)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "leaf_process");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logMessage(Logger::DEBUG, string(tools::indent(level_)
                                                 + "Analyzing line "
                                                 + tools::toString((*it)->Row())
                                                 + "..."));
        Leaf* process = generateLeaf(processnetwork, *it, parent);
        try {
            if (!parent->addProcess(process)) {
				THROW_EXCEPTION(ParseException, parent->getName().getString(),
						        (*it)->Row(),
								(*it)->Column(),
								string("Multiple processes with ID \"")
								+ process->getId()->getString() + "\"");
			}
            if (!processnetwork->addProcess(process)) {
                THROW_EXCEPTION(ParseException, parent->getName().getString(),
                		        (*it)->Row(),
                                (*it)->Column(),
                                string("Multiple processes with ID \"")
                                + process->getId()->getString() + "\"");
            }
        } catch (bad_alloc&) {
            THROW_EXCEPTION(OutOfMemoryException);
        }
    }
}

void XmlParser::parseXmlPorts(Element* xml, Composite* parent)
	throw(InvalidArgumentException, ParseException, IOException,
            RuntimeException){
 /*   if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "port");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
          logger_.logMessage(Logger::DEBUG, string(tools::indent(level_)
                                                   + "Analyzing line "
                                                   + tools::toString((*it)->Row())
                                                   + "..."));

          Id port_id = Id(getAttributeByTag((*it), "name"));

          Composite::IOPort* ioport = new Composite::IOPort();
          try {
              if (!processnetwork->addProcess(process)) {
                  THROW_EXCEPTION(ParseException, file_, (*it)->Row(),
                                  (*it)->Column(),
                                  string("Multiple processs with ID \"")
                                  + process->getId()->getString() + "\"");
              }
          } catch (bad_alloc&) {
              THROW_EXCEPTION(OutOfMemoryException);
          }
      }*/
}

void XmlParser::generateComposite(ProcessNetwork* processnetwork, const Id id,
		Hierarchy hierarchy, Element* xml)
    throw(InvalidArgumentException, ParseException, InvalidProcessNetworkException,
          IOException, RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }

    level_++;

    std::cout<<"\n\n\n INSIDE generateComposite() :" <<
    		getAttributeByTag(xml, "name")<<"\n\n\n";
    Composite* curr_composite = new Composite(id, hierarchy, Id(getAttributeByTag(xml,string("name"))));
    if (!processnetwork) THROW_EXCEPTION(OutOfMemoryException);

    logger_.logMessage(Logger::DEBUG, string(tools::indent(level_)
                           + "Parsing \"leaf_process\" elements..."));
    parseXmlLeafs(xml, processnetwork, curr_composite);

    logger_.logMessage(Logger::DEBUG, string(tools::indent(level_)
                           + "Parsing \"composite_process\" elements..."));
    //parseXmlComposites(xml, processnetwork, curr_composite, curr_level_);

    logger_.logMessage(Logger::DEBUG, string(tools::indent(level_)
                           + "Parsing \"port\" elements..."));
    parseXmlPorts(xml, curr_composite);

    logger_.logMessage(Logger::DEBUG, string(tools::indent(level_)
                           + "Parsing \"signal\" elements..."));
    //parseXmlSignals(xml, processnetwork, curr_composite, curr_level_);

    level_--;
}

Leaf* XmlParser::generateLeaf(ProcessNetwork* pn, Element* xml,
		Composite* parent)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    Leaf* leaf_process;

    // Generating Process ID
    string process_id = string(parent->getId()->getString() +
    		getAttributeByTag(xml, "name"));

    Element* constructor_element = getUniqueElement(xml, "process_constructor");

    //Getting type and MoC data
    string process_type = getAttributeByTag(constructor_element, "name");
    tools::toLowerCase(tools::trim(process_type));
    if (string::npos != process_type.find(string("comb"))) process_type = string("comb");
    if (process_type.length() == 0) {
        THROW_EXCEPTION(ParseException, parent->getName().getString(), xml->Row(), "No process type");
    }

    string process_moc = getAttributeByTag(constructor_element, "moc");
    tools::toLowerCase(tools::trim(process_moc));
    if (process_moc.length() == 0) {
        THROW_EXCEPTION(ParseException, parent->getName().getString(), xml->Row(), "No process MoC");
    }

    try {
    	if ((process_type == "unzipx") && (process_moc == "sy")) {
    		leaf_process = new SY::Unzipx(Id(process_id), parent->getHierarchy(), 0);
        }
        else if ((process_type == "zipx") && (process_moc == "sy")) {
        	leaf_process = new SY::Zipx(Id(process_id), parent->getHierarchy(), 0);
        }
        else if ((process_type == "fanout") && (process_moc == "sy"))  {
        	leaf_process = new SY::Fanout(Id(process_id), parent->getHierarchy(), 0);
        }
        else if ((process_type == "delay") && (process_moc == "sy"))  {
        	leaf_process = new SY::delay(Id(process_id), parent->getHierarchy(), 0,
        			getInitialDelayValue(constructor_element, parent));
        }
        else if ((process_type == "comb") && (process_moc == "sy"))  {
        	leaf_process = new SY::Comb(Id(process_id), parent->getHierarchy(), 0,
                                     generateProcessFunction(xml, pn, parent));
        }
        else {
            THROW_EXCEPTION(ParseException, parent->getName().getString(), xml->Row(),
                            string("Unknown process type \"")
                            + process_type
                            + "\"");
        }
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
    if (!leaf_process) THROW_EXCEPTION(OutOfMemoryException);

    logger_.logMessage(Logger::DEBUG, string(tools::indent(level_))
                       + "Generated " + leaf_process->type()
                       + " from \"" + leaf_process->getId()->getString() + "\"");

    // Get ports
    list<Element*> elements = getElementsByName(xml, "port");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logMessage(Logger::DEBUG,
        		             string(tools::indent(level_)
                                  + "Analyzing line "
                                  + tools::toString((*it)->Row()) + "..."));

        Leaf::Port* port = generateLeafPort((*it), leaf_process);
		if (!port) THROW_EXCEPTION(OutOfMemoryException);
		logger_.logMessage(Logger::DEBUG,
				           string(tools::indent(level_)
						   + "Generated port \"") + port->getId()->getString()
						   + "\"");
		string port_direction = getAttributeByTag((*it),"direction");
		bool port_added;
        if (port_direction == "in") port_added = leaf_process->addInPort(*port);
        else if (port_direction == "out") port_added = leaf_process->addOutPort(*port);
        else THROW_EXCEPTION(ParseException, parent->getName().getString(), (*it)->Row(),
                (*it)->Column(), "Invalid port direction");

        if (!(port_added)) {
            THROW_EXCEPTION(ParseException, parent->getName().getString(),
            				(*it)->Row(),
                            (*it)->Column(), string("Multiple ")
                            + ((port_direction == "in") ? "in ports" : "out ports")
                            + " with the same ID \""
                            + port->getId()->getString() + "\"");
        }
        logger_.logMessage(Logger::DEBUG, string()
                           + tools::indent(level_)
                           + ((port_direction == "in") ? "In" : "Out")
                           + " port \"" + port->getId()->getString()
                           + "\" added to process \""
                           + leaf_process->getId()->getString() + "\"");
        delete port;
    }

    return leaf_process;
}

CFunction* XmlParser::generateProcessFunction(Element* xml, ProcessNetwork* pn,
		Composite* parent)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    Element* argument = getUniqueElement(xml, "argument");
    string function_name = getAttributeByTag(argument,"value");
    string name = getAttributeByTag(argument,"name");
    string file_name = getAttributeByTag(argument,"value");

    CFunction* existing_function = pn->getFunction(Id(function_name));

    if (existing_function) {
        logger_.logMessage(Logger::DEBUG,
        		string(tools::indent(level_)
                      + "Function \""
                      + function_name
                      + "\" already exists. It will not be created... "));
        return existing_function;
    }
    else {
        logger_.logMessage(Logger::DEBUG,
        		string(tools::indent(level_)
                      + "Function \""
                      + function_name
                      + "\" is being added to the process network... "));
        CFunction* new_function = new CFunction(function_name, file_name);
        try {
            if (!pn->addFunction(new_function)) {
				THROW_EXCEPTION(ParseException, parent->getName().getString(),
						        xml->Row(),
						        xml->Column(),
								string("Multiple functions with ID \"")
								+ new_function->getName() + "\". Bad check!");
			}
        } catch (bad_alloc&) {
            THROW_EXCEPTION(OutOfMemoryException);
        }
        return new_function;
    }

    // No such element found
    THROW_EXCEPTION(ParseException, parent->getName().getString(), xml->Row(),
                    "No process function argument found");
}

Leaf::Port* XmlParser::generateLeafPort(Element* xml, Leaf* parent)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    string port_name = getAttributeByTag(xml,"name");
    string port_datatype = getAttributeByTag(xml,"type");

    Leaf::Port* port = new (std::nothrow) Leaf::Port(Id(port_name),
    		parent, CDataType());
    // @todo: handle data type conversion
    if (!port) THROW_EXCEPTION(OutOfMemoryException);
    logger_.logMessage(Logger::DEBUG,
                       string("Generated port \"") + port->getId()->getString()
                       + "\"");
    return port;
}

Element* XmlParser::findXmlRootElement(Document* xml, const string& file)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    Node* xml_root_node = xml->FirstChild("process_network", false);
    if (!xml_root_node) {
        THROW_EXCEPTION(ParseException, file,
                        string("Could not find root element \"graphml\""));
    }
    if (xml_root_node->Type() != TiXmlNode::ELEMENT) {
        THROW_EXCEPTION(ParseException, file,
        		        xml_root_node->Row(),
        				xml_root_node->Column(),
                        string("Found \"process_network\" structure is not an "
                               "element"));
    }
    Element* xml_root = dynamic_cast<Element*>(xml_root_node);
    if (!xml_root) THROW_EXCEPTION(CastException);

    return xml_root;
}

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
                    THROW_EXCEPTION(OutOfMemoryException);
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
                logger_.logMessage(Logger::WARNING,
                                   string("Unknown XML data at line ")
                                   + tools::toString(child->Row()) + ", column "
                                   + tools::toString(child->Column()) + ":\n"
                                   + child->Value());
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

Element* XmlParser::getUniqueElement(Node* xml, const string& name)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    if (name.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"name\" must not be empty "
                        "string");
    }

    list<Element*> elements = getElementsByName(xml, "process_constructor");
    if (elements.size() != 1){
    	THROW_EXCEPTION(ParseException, file_,
    			xml->Row(),
    			"Multiple constructors are illegal");
    }
    list<Element*>::iterator it = elements.begin();

    return (*it);
}

void XmlParser::checkXmlDocument(Document* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    // @todo implement
    logger_.logMessage(Logger::WARNING, "XML document check not implemented");
}

string XmlParser::getAttributeByTag(Element* xml, string tag)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    string attr = xml->GetAttribute(tag);
    if (attr.length() == 0) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        string("Element is missing \""
                        		+tag
                        		+"\" attribute"));
    }
    return tools::trim(attr);
}

string XmlParser::getInitialDelayValue(Element* xml, Composite* parent)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "argument");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logMessage(Logger::DEBUG,
        		string(tools::indent(level_)
                      + "Analyzing line "
                      + tools::toString((*it)->Row()) + "..."));
        string value = getAttributeByTag(*it,"value");
        return value;
    }

    // No such element found
    THROW_EXCEPTION(ParseException, parent->getName().getString(), xml->Row(),
                    "No initial delay value found");
}

