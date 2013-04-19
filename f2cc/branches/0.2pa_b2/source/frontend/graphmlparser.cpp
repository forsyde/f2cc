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

#include "graphmlparser.h"
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

GraphmlParser::GraphmlParser(Logger& logger) throw() : Frontend(logger) {}

GraphmlParser::~GraphmlParser() throw() {}

Processnetwork* GraphmlParser::createProcessnetwork(const string& file)
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
    Processnetwork* processnetwork = generateProcessnetwork(findXmlGraphElement(&xml));

    return processnetwork;
}

list<Element*> GraphmlParser::getElementsByName(Node* xml, const string& name)
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

void GraphmlParser::checkXmlDocument(Document* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    // @todo implement
    logger_.logWarningMessage("XML document check not implemented");
}

Element* GraphmlParser::findXmlGraphElement(Document* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    Node* xml_graphml_node = xml->FirstChild("graphml", false);
    if (!xml_graphml_node) {
        THROW_EXCEPTION(ParseException, file_,
                        string("Could not find root element \"graphml\""));
    }
    if (xml_graphml_node->Type() != TiXmlNode::ELEMENT) {
        THROW_EXCEPTION(ParseException, file_, xml_graphml_node->Row(),
                        xml_graphml_node->Column(),
                        string("Found \"graphml\" structure is not an "
                               "element"));
    }
    Element* xml_graphml = dynamic_cast<Element*>(
        xml_graphml_node);
    if (!xml_graphml) THROW_EXCEPTION(CastException);
    Node* xml_graph_node = xml_graphml->FirstChild("graph", false);
    if (!xml_graph_node) {
        THROW_EXCEPTION(ParseException, file_,
                        string("Could not find element \"graph\""));
    }
    if (xml_graph_node->Type() != TiXmlNode::ELEMENT) {
        THROW_EXCEPTION(ParseException, file_, xml_graphml_node->Row(),
                        xml_graphml_node->Column(),
                        string("Found \"graph\" structure is not an element"));
    }
    Element* xml_graph = dynamic_cast<Element*>(xml_graph_node);
    if (!xml_graph) THROW_EXCEPTION(CastException);

    return xml_graph;
}

Processnetwork* GraphmlParser::generateProcessnetwork(Element* xml)
    throw(InvalidArgumentException, ParseException, InvalidProcessnetworkException,
          IOException, RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    Processnetwork* processnetwork = new (std::nothrow) Processnetwork(Id("GraphML_Network"));
    if (!processnetwork) THROW_EXCEPTION(OutOfMemoryException);

    logger_.logDebugMessage("Parsing \"node\" elements...");
    parseXmlNodes(xml, processnetwork);

    logger_.logDebugMessage("Parsing \"edge\" elements...");
    map<Process::Port*, Process*> copy_processes;
    parseXmlEdges(xml, processnetwork, copy_processes);

    return processnetwork;
}

void GraphmlParser::parseXmlNodes(Element* xml, Processnetwork* processnetwork)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "node");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logDebugMessage(string("Analyzing line "
                                       + tools::toString((*it)->Row())
                                       + "..."));
        Process* process = generateProcess(*it);
        try {
            if (!processnetwork->addProcess(process, processnetwork->getHierarchy())) {
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

void GraphmlParser::parseXmlEdges(Element* xml, Processnetwork* processnetwork,
                                  map<Process::Port*, Process*>& copy_processes)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "edge");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logDebugMessage(string("Analyzing line ")
                                + tools::toString((*it)->Row()) + "...");
        generateConnection(*it, processnetwork, copy_processes);
    }
}

void GraphmlParser::fixProcessnetworkInputsOutputs(Processnetwork* processnetwork)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }

    logger_.logInfoMessage("Running post-check fixes - removing InPort and "
                            "OutPort processes from the process network...");

    list<Process*> inport_processes;
    list<Process*> outport_processes;

    // Get InPort and OutPort processes from the process network
    logger_.logDebugMessage("Searching for InPort and OutPort processes...");
    list<Process*> processes = processnetwork->getProcesses();
    list<Process*>::iterator process_it;
    for (process_it = processes.begin(); process_it != processes.end();
         ++process_it) {
        Process* process = *process_it;

        logger_.logDebugMessage(string("Analyzing process \"")
                                + process->getId()->getString() + "\"...");

        if (dynamic_cast<InPort*>(process)) {
            logger_.logDebugMessage("Is an InPort");
            inport_processes.push_back(process);
        }
        if (dynamic_cast<OutPort*>(process)) {
            logger_.logDebugMessage("Is an OutPort");
            outport_processes.push_back(process);
        }
    }
    if (inport_processes.empty()) {
        THROW_EXCEPTION(IllegalStateException, "Failed to locate InPort "
                        "processes");
    }
    if (outport_processes.empty()) {
        THROW_EXCEPTION(IllegalStateException, "Failed to locate OutPort "
                        "processes");
    }

    // Redirect and remove the InPort processes
    for (process_it = inport_processes.begin();
         process_it != inport_processes.end();
         ++process_it) {
        Process* process = *process_it;

        logger_.logDebugMessage(string("Redirecting out ports of InPort ")
                                + "process \"" + process->getId()->getString()
                                + " to processnetwork inputs...");
        list<Process::Port*> ports = process->getOutPorts();
        list<Process::Port*>::iterator port_it;
        for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
            processnetwork->addInPort(*((*port_it)->getConnectedPort()));
        }

        Id id = *process->getId();
        if (!processnetwork->deleteProcess(id)) {
            THROW_EXCEPTION(IllegalStateException,
                            string("Failed to delete InPort process \"")
                            + id.getString() + "\"");
        }
    }

    // Redirect and remove the OutPort processes
    for (process_it = outport_processes.begin();
         process_it != outport_processes.end();
         ++process_it) {
        Process* process = *process_it;

        logger_.logDebugMessage(string("Redirecting in ports of OutPort ")
                                + "process \"" + process->getId()->getString()
                                + " to processnetwork outputs...");
        list<Process::Port*> ports = process->getInPorts();
        list<Process::Port*>::iterator port_it;
        for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
            processnetwork->addOutPort(*((*port_it)->getConnectedPort()));
        }

        Id id = *process->getId();
        if (!processnetwork->deleteProcess(id)) {
            THROW_EXCEPTION(IllegalStateException,
                            string("Failed to delete OutPort process \"")
                            + id.getString() + "\"");
        }
    }

    logger_.logInfoMessage("Post-check fixes complete");
}

Process* GraphmlParser::generateProcess(Element* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    Process* process;

    // Create process of right type
    string process_id = getId(xml);
    string process_type = getProcessType(xml);
    tools::toLowerCase(tools::trim(process_type));
    if (process_type.length() == 0) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(), "No process type");
    }
    try {
        if (process_type == "inport") {
            process = new InPort(Id(process_id), string("sy"));
        }
        else if (process_type == "outport") {
            process = new OutPort(Id(process_id), string("sy"));
        }
        else if (process_type == "mapsy") {
            process = new Map(Id(process_id), generateProcessFunction(xml), string("sy"));
        }
        else if (process_type == "parallelmapsy") {
            process = new ParallelMap(Id(process_id), getNumProcesses(xml),
                                        generateProcessFunction(xml), string("sy"));
        }
        else if (process_type == "unzipxsy") {
            process = new unzipx(Id(process_id), string("sy"));
        }
        else if (process_type == "zipxsy") {
            process = new zipx(Id(process_id), string("sy"));
        }
        else if (process_type == "delaysy") {
            process = new delay(Id(process_id), getInitialdelayValue(xml), string("sy"));
        }
        else if (process_type == "zipwithnsy") {
            process = new Map(Id(process_id),
                                     generateProcessFunction(xml), string("sy"));
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
        bool port_added;
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

string GraphmlParser::getId(Element* xml)
throw(InvalidArgumentException, ParseException, IOException,
      RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    string id = xml->GetAttribute("id");
    if (id.length() == 0) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        string("Element is missing \"id\" attribute"));
    }
    return tools::trim(id);
}

string GraphmlParser::getName(Element* xml)
throw(InvalidArgumentException, ParseException, IOException,
      RuntimeException) {
    string name = xml->GetAttribute("name");
    if (name.length() == 0) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        string("Element is missing \"name\" attribute"));
    }
    return tools::trim(name);
}

string GraphmlParser::getProcessType(Element* xml)
throw(InvalidArgumentException, ParseException, IOException,
      RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "data");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logDebugMessage(string("Analyzing line ")
                                + tools::toString((*it)->Row()) + "...");
        string attr_name = (*it)->GetAttribute("key");
        if (attr_name == "process_type") {
            string type = (*it)->GetText(false);
            return tools::trim(type);
        }
    }

    // No such element found
    THROW_EXCEPTION(ParseException, file_, xml->Row(), "No process type found");
}

CFunction GraphmlParser::generateProcessFunction(Element* xml)
throw(InvalidArgumentException, ParseException, IOException,
      RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "data");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logDebugMessage(string("Analyzing line ")
                                + tools::toString((*it)->Row()) + "...");
        string attr_name = (*it)->GetAttribute("key");
        if (attr_name == "procfun_arg") {
            string function_str = (*it)->GetText(false);
            try {
                CFunction function(
                    generateProcessFunctionFromString(function_str));
                findFunctionArraySizes(function, xml);
                return function;
            } catch (InvalidFormatException& ex) {
                THROW_EXCEPTION(ParseException, file_, (*it)->Row(),
                                string("Invalid process function argument: ")
                                + ex.getMessage());
            }
        }
    }

    // No such element found
    THROW_EXCEPTION(ParseException, file_, xml->Row(),
                    "No process function argument found");
}

CFunction GraphmlParser::generateProcessFunctionFromString(
    const std::string& str) throw(InvalidFormatException) {
    // Find function prototype and body
    size_t pos = str.find("{");
    if (pos == string::npos) {
        pos = str.find("(");
        if (pos == string::npos) {
            THROW_EXCEPTION(InvalidFormatException, "No '{' " "of ')' found");
        }
        ++pos; // Include the ')' as we need it later
    }
    string prototype = str.substr(0, pos);
    string function_body = str.substr(pos);

    // Separate input parameters and function head
    pos = prototype.find("(");
    size_t pos2 = prototype.find(")");
    if (pos == string::npos) {
        THROW_EXCEPTION(InvalidFormatException, "No '(' found in the "
                        "prototype");
    }
    if (pos2 == string::npos) {
        THROW_EXCEPTION(InvalidFormatException, "No ')' found in the "
                        "prototype");
    }
    string input_params_str = prototype.substr(pos + 1, pos2 - pos - 1);
    string function_head = prototype.substr(0, pos);
    
    try {
        string function_name = getNameFromDeclaration(function_head);
        CDataType function_return_data_type =
            getDataTypeFromDeclaration(function_head);

        // Find input parameters
        list<CVariable> input_parameters;
        vector<string> declarations = tools::split(input_params_str, ',');
        for (vector<string>::iterator it = declarations.begin();
             it != declarations.end(); ++it) {
            CVariable parameter = CVariable(getNameFromDeclaration(*it),
                                            getDataTypeFromDeclaration(*it));
            input_parameters.push_back(parameter);
        }
        
        return CFunction(function_name, function_return_data_type,
                         input_parameters, function_body);
    }
    catch (InvalidArgumentException& ex) {
        THROW_EXCEPTION(InvalidFormatException, ex.getMessage());
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

CDataType GraphmlParser::getDataTypeFromDeclaration(const string& str) const
throw(InvalidFormatException) {
    size_t pos = str.find_last_of(" ");
    if (pos == string::npos) {
        THROW_EXCEPTION(InvalidFormatException, "No ' ' "
                        "found in the variable declaration");
    }
    string data_type_str = str.substr(0, pos);
    tools::trim(data_type_str);

    if (data_type_str.length() == 0) {
        THROW_EXCEPTION(InvalidFormatException, "No data type in declaration");
    }

    bool is_const = false;
    if (data_type_str.length() >= 6 && data_type_str.substr(0, 6) == "const ") {
        is_const = true;
        data_type_str.erase(0, 6);
    }
    tools::trim(data_type_str);

    if (data_type_str.length() == 0) {
        THROW_EXCEPTION(InvalidFormatException, "No data type in declaration");
    }

    if (data_type_str.find("&") != string::npos) {
        THROW_EXCEPTION(InvalidFormatException, "References are not supported");
    }

    if (data_type_str.length() == 0) {
        THROW_EXCEPTION(InvalidFormatException, "No data type in declaration");
    }

    bool is_array = false;
    if (data_type_str[data_type_str.length() - 1] == '*') {
        is_array = true;
        data_type_str.erase(data_type_str.length() - 1, 1);
    }
    tools::trim(data_type_str);
    if (data_type_str.find("*") != string::npos) {
        THROW_EXCEPTION(InvalidFormatException, "Pointer-to-pointer data types "
                        "are not supported");
    }

    if (data_type_str.length() == 0) {
        THROW_EXCEPTION(InvalidFormatException, "No data type in declaration");
    }

    try {
        CDataType::Type type = CDataType::stringToType(data_type_str);
        return CDataType(type, is_array, false, 0, false, is_const);
    }
    catch (InvalidArgumentException& ex) {
        THROW_EXCEPTION(InvalidFormatException, ex.getMessage());
    }
}

string GraphmlParser::getNameFromDeclaration(const string& str) const
throw(InvalidFormatException) {
    size_t pos = str.find_last_of(" ");
    if (pos == string::npos) {
        THROW_EXCEPTION(InvalidFormatException, "No ' ' "
                        "found in the variable declaration");
    }
    string name = str.substr(pos + 1);
    tools::trim(name);
    return name;
}

int GraphmlParser::getNumProcesses(ticpp::Element* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "data");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logDebugMessage(string("Analyzing line ")
                                + tools::toString((*it)->Row()) + "...");
        string attr_name = (*it)->GetAttribute("key");
        if (attr_name == "num_processes") {
            string str = (*it)->GetText(false);
            tools::trim(str);
            try {
                return tools::toInt(str);
            }
            catch (InvalidArgumentException&) {
                THROW_EXCEPTION(ParseException, file_, xml->Row(), "Not a "
                                "number");
            }
        }
    }

    // No such element found
    THROW_EXCEPTION(ParseException, file_, xml->Row(), "Number of processes "
                    "not found");
}

void GraphmlParser::findFunctionArraySizes(CFunction& function,
                                           ticpp::Element* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "port");

    // If return data type or last input parameter is an array, find the array
    // size by analyzing the out port XML elements
    CDataType* output_data_type = NULL;
    if (function.getReturnDataType()->isArray()) {
        output_data_type = function.getReturnDataType();

        logger_.logDebugMessage("Searching array size for return "
                                "data type...");
    } else if (function.getNumInputParameters() > 1) {
        output_data_type = function.getInputParameters().back()->getDataType();
        // Reset to NULL if the parameter is not what we are looking for
        if (output_data_type->isArray()) {
            logger_.logDebugMessage("Searching array size for second "
                                    "input parameter data type...");
        }
        else {
            output_data_type = NULL;
        }
    }
    if (output_data_type) {
        list<Element*>::iterator it;
        for (it = elements.begin(); it != elements.end(); ++it) {
            logger_.logDebugMessage(string("Analyzing line ")
                                    + tools::toString((*it)->Row()) + "...");
            string port_name = getName(*it);
            if (isOutPort(port_name)) {
                size_t array_size = findArraySize(*it);
                if (array_size > 0) {
                    logger_.logDebugMessage(string("Found array size ")
                                            + tools::toString(array_size));
                    output_data_type->setArraySize(array_size);
                }
                break;
            }
        }
    }

    // Find array sizes for the input parameters which are arrays by analyzing
    // the in port XML elements
    list<CVariable*> parameters = function.getInputParameters();
    list<CVariable*>::iterator param_it = parameters.begin();
    list<CVariable*>::iterator param_sprocessnetwork_point;
    list<Element*>::iterator xml_it = elements.begin();
    if (function.getNumInputParameters() > 1) {
        param_sprocessnetwork_point = --parameters.end();
    }
    else {
        param_sprocessnetwork_point = parameters.end();
    }
    while (param_it != param_sprocessnetwork_point && xml_it != elements.end()) {
        if (param_it == parameters.begin()) {
            logger_.logDebugMessage("Searching array size for "
                                    "input parameter data type...");
        }
        logger_.logDebugMessage(string("Analyzing line ")
                                + tools::toString((*xml_it)->Row()) + "...");

        if (!isInPort(getName(*xml_it))) {
            logger_.logDebugMessage("Not an in port, moving to next");
            ++xml_it;
            continue;
        }

        if ((*param_it)->getDataType()->isArray()) {
            size_t array_size = findArraySize(*xml_it);
            if (array_size > 0) {
                logger_.logDebugMessage(string("Found array size ")
                                        + tools::toString(array_size));
                (*param_it)->getDataType()->setArraySize(array_size);
            }
            else {
                logger_.logDebugMessage("No array size key");
            }
        }
        ++param_it;
        ++xml_it;
    }
}

size_t GraphmlParser::findArraySize(ticpp::Element* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "data");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logDebugMessage(string("Analyzing line ")
                                + tools::toString((*it)->Row()) + "...");
        string attr_name = (*it)->GetAttribute("key");
        if (attr_name == "array_size") {
            string array_size_str = (*it)->GetText(false);
            if (!tools::isNumeric(array_size_str)) {
                THROW_EXCEPTION(ParseException, file_, xml->Row(),
                                "Array size must be numeric");
            }
            int array_size = tools::toInt(array_size_str);
            if (array_size < 1) {
                THROW_EXCEPTION(ParseException, file_, xml->Row(),
                                "Array size must not be less than 1");
            }
            return (size_t) array_size;
        }
    }

    // No such element found
    return 0;
}

string GraphmlParser::getInitialdelayValue(Element* xml)
throw(InvalidArgumentException, ParseException, IOException,
      RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "data");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logDebugMessage(string("Analyzing line ")
                                + tools::toString((*it)->Row()) + "...");
        string attr_name = (*it)->GetAttribute("key");
        if (attr_name == "initial_value") {
            string value = (*it)->GetText(false);
            tools::trim(value);
            if (value.length() == 0) {
                THROW_EXCEPTION(ParseException, file_, xml->Row(),
                                "No initial delay value found");
            }
            return value;
        }
    }

    // No such element found
    THROW_EXCEPTION(ParseException, file_, xml->Row(),
                    "No initial delay value found");
}

Process::Port* GraphmlParser::generatePort(Element* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    Process::Port* port = new (std::nothrow) Process::Port(getName(xml), CDataType());
    if (!port) THROW_EXCEPTION(OutOfMemoryException);
    logger_.logDebugMessage(string("Generated port \"")
                            + port->getId()->getString() + "\"");
    return port;
}

bool GraphmlParser::isInPort(const std::string& id) const throw() {
    return isValidPortId(id, "in");
}

bool GraphmlParser::isOutPort(const std::string& id) const throw() {
    return isValidPortId(id, "out");
}

bool GraphmlParser::isValidPortId(const std::string& id,
                                  const std::string direction) const throw() {
    try {
        size_t separator_pos = id.find_last_of("_");
        size_t direction_pos =
            separator_pos == string::npos ? 0 : separator_pos + 1;

        // Check direction
        if (id.substr(direction_pos, direction.length()) != direction) {
            return false;
        }

        // Check that the part after the direction is numeric, if any
        if (direction_pos + direction.length() < id.length()) {
            string remaining(id.substr(direction_pos + direction.length()));
            if (!tools::isNumeric(remaining)) return false;

            // All tests passed
            return true;
        }
        else {
            // No trailing numeric part
            return true;
        }
    } catch (std::out_of_range&) {
        // Do nothing, but a check will fail because of this
    }
    
    return false;
}

void GraphmlParser::generateConnection(Element* xml, Processnetwork* processnetwork,
                                       map<Process::Port*, Process*>&
                                       copy_processes)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }
    // Get source process ID
    string source_process_id = xml->GetAttribute("source");
    if (source_process_id.length() == 0) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        "\"edge\" element is missing \"source\" attribute");
    }
    // Get source process port ID
    string source_process_port_id = xml->GetAttribute("sourceport");
    if (source_process_port_id.length() == 0) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        "\"edge\" element is missing \"sourceport\" attribute");
    }
    // Get target process ID
    string target_process_id = xml->GetAttribute("target");
    if (target_process_id.length() == 0) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        "\"edge\" element is missing \"target\" attribute");
    }
    // Get target process port ID
    string target_process_port_id = xml->GetAttribute("targetport");
    if (target_process_port_id.length() == 0) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        "\"edge\" element is missing \"targetport\" attribute");
    }
    // Get source and target processes
    Process* source_process = processnetwork->getProcess(source_process_id);
    if (source_process == NULL) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        string("No source process \"")
                        + source_process_id + "\" found");
    }
    Process* target_process = processnetwork->getProcess(target_process_id);
    if (target_process == NULL) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        string("No target process \"")
                        + target_process_id + "\" found");
    }
    // Get source and target ports
    Process::Port* source_port =
        source_process->getOutPort(source_process_port_id);
    if (!source_port) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        string("No source process out port \"")
                        + source_process_id + ":"
                        + source_process_port_id + "\" ");
    }
    Process::Port* target_port =
        target_process->getInPort(target_process_port_id);
    if (!target_port) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        string("No target process in port \"")
                        + target_process_id + ":"
                        + target_process_port_id + "\" found");
    }
    // Check that the target port is not already connected to another port
    if (target_port->isConnected()) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        string("Target port \"")
                        + target_process_id + ":"
                        + target_process_port_id
                        + "\" is already connected to another port");
    }
    // Make port connections
    if (!source_port->isConnected()) {
        source_port->connect(target_port);
        logger_.logDebugMessage(string("Connected port \"")
                                + source_port->toString() + "\" with \""
                                + target_port->toString() + "\"");
    }
    else {
        // Source port already connected; use intermediate fanout process
        logger_.logDebugMessage(string("Source port \"")
                                + source_port->toString()
                                + "\" already connected to \""
                                + source_port->getConnectedPort()->toString()
                                + "\". Using intermediate fanout process.");

        // Get fanout process
        Process* copy_process;
        map<Process::Port*, Process*>::iterator it =
            copy_processes.find(source_port);
        if (it != copy_processes.end()) {
            copy_process = it->second;
        }
        else {
            // No such fanout process; create a new one
            copy_process = new (std::nothrow)
                fanout(processnetwork->getUniqueProcessId("_copy_"), string("sy"));
            if (copy_process == NULL) THROW_EXCEPTION(OutOfMemoryException);
            copy_processes.insert(pair<Process::Port*, Process*>(source_port,
                                                                 copy_process));
            logger_.logDebugMessage(string("New fanout process \"")
                                    + copy_process->getId()->getString()
                                    + "\" created");

            // Add to processnetwork
            if (!processnetwork->addProcess(copy_process, processnetwork->getHierarchy())) {
                THROW_EXCEPTION(IllegalStateException, string("Failed to ")
                                + "add new process: Process with ID \""
                                + copy_process->getId()->getString()
                                + "\" already existed");
            }
            logger_.logDebugMessage(string("New process \"")
                                    + copy_process->getId()->getString()
                                    + "\" added to the process network");

            // Break the current connection and connect the source and previous
            // target connection through the fanout process
            if(!copy_process->addInPort(Id("in"), CDataType())) {
                THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                                + "in port to process \""
                                + copy_process->getId()->getString() + "\"");
            }
            Process::Port* old_target_port = source_port->getConnectedPort();
            source_port->unconnect();
            logger_.logDebugMessage( string("Broke port connection \"")
                                     + source_port->toString() + "\"--\""
                                     + old_target_port->toString() + "\"");
            source_port->connect(copy_process->getInPorts().front());
            logger_.logDebugMessage(string("Connected port \"")
                                    + source_port->toString()
                                    + "\" with \""
                                    + copy_process->getInPorts().front()
                                    ->toString() + "\"");
            if(!copy_process->addOutPort(Id("out1"), CDataType())) {
                THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                                + "out port to process \""
                                + copy_process->getId()->getString() + "\"");
            }
            old_target_port->connect(copy_process->getOutPorts().front());
            logger_.logDebugMessage(string("Connected port \"")
                                    + copy_process->getOutPorts().front()
                                    ->toString()
                                    + "\" with \""
                                    + old_target_port->toString() + "\"");
        }

        string new_out_port_id = string("out")
            + tools::toString(copy_process->getOutPorts().size() + 1);
        if(!copy_process->addOutPort(new_out_port_id, CDataType())) {
            THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                            + "out port to process \""
                            + copy_process->getId()->getString() + "\"");
        }
        target_port->connect(copy_process->getOutPorts().back());

        logger_.logDebugMessage(string("Connected port \"")
                                + copy_process->getOutPorts().back()->toString()
                                + "\" with \""
                                + target_port->toString() + "\"");
    }
}

void GraphmlParser::checkProcessnetworkMore(Processnetwork* processnetwork)
    throw(InvalidArgumentException, InvalidProcessnetworkException, IOException,
          RuntimeException) {
    logger_.logInfoMessage("Checking that the process network contains at least one "
                            "InPort and OutPort process...");

    bool found_in_port_process = false;
    bool found_out_port_process = false;
    list<Process*> processes = processnetwork->getProcesses();
    list<Process*>::iterator process_it;
    for (process_it = processes.begin(); process_it != processes.end();
         ++process_it) {
        Process* process = *process_it;
        logger_.logDebugMessage(string("Checking process \"")
                                + process->getId()->getString() + "\"");

        // In- and OutPort presence check
        if (dynamic_cast<InPort*>(process)) {
            logger_.logDebugMessage("InPort found");
            found_in_port_process = true;
        }
        if (dynamic_cast<OutPort*>(process)) {
            logger_.logDebugMessage("OutPort found");
            found_out_port_process = true;
        }
    }
    if (!found_in_port_process) {
        THROW_EXCEPTION(InvalidProcessnetworkException, "No InPort process found");
    }
    if (!found_out_port_process) {
        THROW_EXCEPTION(InvalidProcessnetworkException, "No OutPort process found");
    }
}

void GraphmlParser::postCheckFixes(ForSyDe::Processnetwork* processnetwork)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    fixProcessnetworkInputsOutputs(processnetwork);
}
