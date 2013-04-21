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

#include "graphmlparser.h"
#include "../ticpp/ticpp.h"
#include "../ticpp/tinyxml.h"
#include "../tools/tools.h"
#include "../forsyde/SY/mapsy.h"
#include "../forsyde/parallelmapsy.h"
#include "../forsyde/SY/zipxsy.h"
#include "../forsyde/SY/unzipxsy.h"
#include "../forsyde/SY/delaysy.h"
#include "../forsyde/SY/inport.h"
#include "../forsyde/outport.h"
#include "../forsyde/SY/fanoutsy.h"
#include "../forsyde/SY/zipwithnsy.h"
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

GraphmlParser::GraphmlParser(Logger& logger) throw() : Frontend(logger) {}

GraphmlParser::~GraphmlParser() throw() {}

Model* GraphmlParser::createModel(const string& file)
    throw(InvalidArgumentException, FileNotFoundException, IOException,
          ParseException, InvalidModelException, RuntimeException) {
    if (file.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"file\" must not be empty "
                        "string");
    }
    file_ = file;

    // Read file content
    string xml_data;
    logger_.logMessage(Logger::INFO, string("Reading xml data from file..."));
    try {
        tools::readFile(file_, xml_data);
    } catch (FileNotFoundException& ex) {
        logger_.logMessage(Logger::ERROR, string("No xml input file \"") + file_
                           + "\" could be found");
        throw;
    } catch (IOException& ex) {
        logger_.logMessage(Logger::ERROR, string("Failed to read xml file:\n")
                           + ex.getMessage());
        throw;
    }

    // Parse content
    Document xml;
    try {
        logger_.logMessage(Logger::INFO, "Building xml structure...");
        xml.Parse(xml_data);
    } catch (ticpp::Exception& ex) {
        // @todo throw more detailed ParseException (with line and column)
        THROW_EXCEPTION(ParseException, file_, ex.what());
    }
            
    logger_.logMessage(Logger::INFO, "Checking xml structure...");
    checkXmlDocument(&xml);
    logger_.logMessage(Logger::INFO, "All checks passed");

    logger_.logMessage(Logger::INFO, "Generating internal model...");
    Model* model = generateModel(findXmlGraphElement(&xml));

    return model;
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

void GraphmlParser::checkXmlDocument(Document* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    // @todo implement
    logger_.logMessage(Logger::WARNING, "XML document check not implemented");
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

Model* GraphmlParser::generateModel(Element* xml)
    throw(InvalidArgumentException, ParseException, InvalidModelException,
          IOException, RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    Model* model = new (std::nothrow) Model();
    if (!model) THROW_EXCEPTION(OutOfMemoryException);

    logger_.logMessage(Logger::DEBUG, "Parsing \"node\" elements...");
    parseXmlNodes(xml, model);

    logger_.logMessage(Logger::DEBUG, "Parsing \"edge\" elements...");
    map<Leaf::Interface*, Leaf*> copy_leafs;
    parseXmlEdges(xml, model, copy_leafs);

    return model;
}

void GraphmlParser::parseXmlNodes(Element* xml, Model* model)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    if (!model) {
        THROW_EXCEPTION(InvalidArgumentException, "\"model\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "node");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logMessage(Logger::DEBUG, string("Analyzing line "
                                                 + tools::toString((*it)->Row())
                                                 + "..."));
        Leaf* leaf = generateLeaf(*it);
        try {
            if (!model->addLeaf(leaf)) {
                THROW_EXCEPTION(ParseException, file_, (*it)->Row(),
                                (*it)->Column(),
                                string("Multiple leafs with ID \"")
                                + leaf->getId()->getString() + "\"");
            }
        } catch (bad_alloc&) {
            THROW_EXCEPTION(OutOfMemoryException);
        }
    }
}

void GraphmlParser::parseXmlEdges(Element* xml, Model* model,
                                  map<Leaf::Interface*, Leaf*>& copy_leafs)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    if (!model) {
        THROW_EXCEPTION(InvalidArgumentException, "\"model\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "edge");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logMessage(Logger::DEBUG, string("Analyzing line "
                                                 + tools::toString((*it)->Row())
                                                 + "..."));
        generateConnection(*it, model, copy_leafs);
    }
}

void GraphmlParser::fixModelInputsOutputs(Model* model)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!model) {
        THROW_EXCEPTION(InvalidArgumentException, "\"model\" must not be NULL");
    }

    Leaf* inport_leaf = NULL;
    Leaf* outport_leaf = NULL;

    // Get InPort and OutPort leafs from the model
    list<Leaf*> leafs = model->getLeafs();
    list<Leaf*>::iterator leaf_it;
    for (leaf_it = leafs.begin(); leaf_it != leafs.end();
         ++leaf_it) {
        Leaf* leaf = *leaf_it;
        if (dynamic_cast<InPort*>(leaf)) {
            inport_leaf = leaf;
        }
        if (dynamic_cast<OutPort*>(leaf)) {
            outport_leaf = leaf;
        }
    }
    if (!inport_leaf) {
        THROW_EXCEPTION(IllegalStateException, "Failed to locate InPort "
                        "leaf");
    }
    if (!outport_leaf) {
        THROW_EXCEPTION(IllegalStateException, "Failed to locate OutPort "
                        "leaf");
    }

    // Set their in and out interfaces as outputs and inputs to the model
    list<Leaf::Interface*> interfaces = inport_leaf->getOutPorts();
    list<Leaf::Interface*>::iterator interface_it;
    for (interface_it = interfaces.begin(); interface_it != interfaces.end(); ++interface_it) {
        model->addInput((*interface_it)->getConnectedInterface());
    }
    interfaces = outport_leaf->getInPorts();
    for (interface_it = interfaces.begin(); interface_it != interfaces.end(); ++interface_it) {
        model->addOutput((*interface_it)->getConnectedInterface());
    }

    // Delete the leafs
    if (!model->deleteLeaf(*inport_leaf->getId())) {
        THROW_EXCEPTION(IllegalStateException, "Failed to delete InPort "
                        "leaf");
    }
    if (!model->deleteLeaf(*outport_leaf->getId())) {
        THROW_EXCEPTION(IllegalStateException, "Failed to delete OutPort "
                        "leaf");
    }
}

Leaf* GraphmlParser::generateLeaf(Element* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    Leaf* leaf;

    // Create leaf of right type
    string leaf_id = getId(xml);
    string leaf_type = getLeafType(xml);
    tools::toLowerCase(tools::trim(leaf_type));
    if (leaf_type.length() == 0) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(), "No leaf type");
    }
    try {
        if (leaf_type == "inport") {
            leaf = new InPort(Id(leaf_id));
        }
        else if (leaf_type == "outport") {
            leaf = new OutPort(Id(leaf_id));
        }
        else if (leaf_type == "mapsy") {
            leaf = new Map(Id(leaf_id), generateLeafFunction(xml));
        }
        else if (leaf_type == "parallelmapsy") {
            leaf = new ParallelMap(Id(leaf_id), getNumLeafs(xml),
                                        generateLeafFunction(xml));
        }
        else if (leaf_type == "unzipxsy") {
            leaf = new unzipx(Id(leaf_id));
        }
        else if (leaf_type == "zipxsy") {
            leaf = new zipx(Id(leaf_id));
        }
        else if (leaf_type == "delaysy") {
            leaf = new delay(Id(leaf_id), getInitialDelayValue(xml));
        }
        else if (leaf_type == "zipwithnsy") {
            leaf = new ZipWithNSY(Id(leaf_id),
                                     generateLeafFunction(xml));
        }
        else {
            THROW_EXCEPTION(ParseException, file_, xml->Row(),
                            string("Unknown leaf type \"")
                            + leaf_type
                            + "\"");
        }
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
    if (!leaf) THROW_EXCEPTION(OutOfMemoryException);
    logger_.logMessage(Logger::DEBUG, string("Generated ") + leaf->type()
                       + " from \"" + leaf->getId()->getString() + "\"");

    // Get interfaces
    list<Element*> elements = getElementsByName(xml, "interface");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logMessage(Logger::DEBUG,
                           string("Analyzing line "
                                  + tools::toString((*it)->Row()) + "..."));
        Leaf::Interface* interface = generateInterface(*it);
        bool is_in_interface = isInPort(interface->getId()->getString());
        bool is_out_interface = isOutPort(interface->getId()->getString());
        if (!is_in_interface && !is_out_interface) {
            THROW_EXCEPTION(ParseException, file_, (*it)->Row(),
                            (*it)->Column(), "Invalid interface ID format");
        }

        bool interface_added;
        if (is_in_interface) interface_added = leaf->addInPort(*interface);
        else            interface_added = leaf->addOutPort(*interface);
        if (!interface_added) {
            THROW_EXCEPTION(ParseException, file_, (*it)->Row(),
                            (*it)->Column(), string("Multiple ")
                            + (is_in_interface ? "in interfaces" : "out interfaces")
                            + " with the same ID \""
                            + interface->getId()->getString() + "\"");
        }
        logger_.logMessage(Logger::DEBUG, string()
                           + (is_in_interface ? "In" : "Out")
                           + " interface \"" + interface->getId()->getString()
                           + "\" added to leaf \""
                           + leaf->getId()->getString() + "\"");
        delete interface;
    }
    
    return leaf;
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

string GraphmlParser::getLeafType(Element* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "data");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logMessage(Logger::DEBUG,
                           string("Analyzing line "
                                  + tools::toString((*it)->Row()) + "..."));
        string attr_name = (*it)->GetAttribute("key");
        if (attr_name == "leaf_type") {
            string type = (*it)->GetText(false);
            return tools::trim(type);
        }
    }

    // No such element found
    THROW_EXCEPTION(ParseException, file_, xml->Row(), "No leaf type found");
}

CFunction GraphmlParser::generateLeafFunction(Element* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "data");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logMessage(Logger::DEBUG,
                           string("Analyzing line "
                                  + tools::toString((*it)->Row()) + "..."));
        string attr_name = (*it)->GetAttribute("key");
        if (attr_name == "procfun_arg") {
            string function_str = (*it)->GetText(false);
            try {
                CFunction function(
                    generateLeafFunctionFromString(function_str));
                findFunctionArraySizes(function, xml);
                return function;
            } catch (InvalidFormatException& ex) {
                THROW_EXCEPTION(ParseException, file_, (*it)->Row(),
                                string("Invalid leaf function argument: ")
                                + ex.getMessage());
            }
        }
    }

    // No such element found
    THROW_EXCEPTION(ParseException, file_, xml->Row(),
                    "No leaf function argument found");
}

CFunction GraphmlParser::generateLeafFunctionFromString(
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
        THROW_EXCEPTION(InvalidFormatException, "References are not supinterfaceed");
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
                        "are not supinterfaceed");
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

int GraphmlParser::getNumLeafs(ticpp::Element* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "data");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logMessage(Logger::DEBUG,
                           string("Analyzing line "
                                  + tools::toString((*it)->Row()) + "..."));
        string attr_name = (*it)->GetAttribute("key");
        if (attr_name == "num_leafs") {
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
    THROW_EXCEPTION(ParseException, file_, xml->Row(), "Number of leafs "
                    "not found");
}

void GraphmlParser::findFunctionArraySizes(CFunction& function,
                                           ticpp::Element* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "interface");

    // If return data type or last input parameter is an array, find the array
    // size by analyzing the out interface XML elements
    CDataType* output_data_type = NULL;
    if (function.getReturnDataType()->isArray()) {
        output_data_type = function.getReturnDataType();

        logger_.logMessage(Logger::DEBUG, "Searching array size for return "
                           "data type...");
    } else if (function.getNumInputParameters() > 1) {
        output_data_type = function.getInputParameters().back()->getDataType();
        // Reset to NULL if the parameter is not what we are looking for
        if (output_data_type->isArray()) {
            logger_.logMessage(Logger::DEBUG, "Searching array size for second "
                               "input parameter data type...");
        }
        else {
            output_data_type = NULL;
        }
    }
    if (output_data_type) {
        list<Element*>::iterator it;
        for (it = elements.begin(); it != elements.end(); ++it) {
            logger_.logMessage(Logger::DEBUG,
                               string("Analyzing line "
                                      + tools::toString((*it)->Row()) + "..."));
            string interface_name = getName(*it);
            if (isOutPort(interface_name)) {
                size_t array_size = findArraySize(*it);
                if (array_size > 0) {
                    logger_.logMessage(Logger::DEBUG,
                                       string("Found array size ")
                                       + tools::toString(array_size));
                    output_data_type->setArraySize(array_size);
                }
                break;
            }
        }
    }

    // Find array sizes for the input parameters which are arrays by analyzing
    // the in interface XML elements
    list<CVariable*> parameters = function.getInputParameters();
    list<CVariable*>::iterator param_it = parameters.begin();
    list<CVariable*>::iterator param_stop_point;
    list<Element*>::iterator xml_it = elements.begin();
    if (function.getNumInputParameters() > 1) {
        param_stop_point = --parameters.end();
    }
    else {
        param_stop_point = parameters.end();
    }
    while (param_it != param_stop_point && xml_it != elements.end()) {
        if (param_it == parameters.begin()) {
            logger_.logMessage(Logger::DEBUG, "Searching array size for "
                               "input parameter data type...");
        }
        logger_.logMessage(Logger::DEBUG,
                           string("Analyzing line "
                                  + tools::toString((*xml_it)->Row())
                                  + "..."));

        if (!isInPort(getName(*xml_it))) {
            logger_.logMessage(Logger::DEBUG, "Not an in interface, moving to next");
            ++xml_it;
            continue;
        }

        if ((*param_it)->getDataType()->isArray()) {
            size_t array_size = findArraySize(*xml_it);
            if (array_size > 0) {
                logger_.logMessage(Logger::DEBUG,
                                   string("Found array size ")
                                   + tools::toString(array_size));
                (*param_it)->getDataType()->setArraySize(array_size);
            }
            else {
                logger_.logMessage(Logger::DEBUG, "No array size key");
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
        logger_.logMessage(Logger::DEBUG,
                           string("Analyzing line "
                                  + tools::toString((*it)->Row()) + "..."));
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

string GraphmlParser::getInitialDelayValue(Element* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "data");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logMessage(Logger::DEBUG,
                           string("Analyzing line "
                                  + tools::toString((*it)->Row()) + "..."));
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

Leaf::Interface* GraphmlParser::generateInterface(Element* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    Leaf::Interface* interface = new (std::nothrow) Leaf::Interface(getName(xml));
    if (!interface) THROW_EXCEPTION(OutOfMemoryException);
    logger_.logMessage(Logger::DEBUG,
                       string("Generated interface \"") + interface->getId()->getString()
                       + "\"");
    return interface;
}

bool GraphmlParser::isInPort(const std::string& id) const throw() {
    return isValidInterfaceId(id, "in");
}

bool GraphmlParser::isOutPort(const std::string& id) const throw() {
    return isValidInterfaceId(id, "out");
}

bool GraphmlParser::isValidInterfaceId(const std::string& id,
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

void GraphmlParser::generateConnection(Element* xml, Model* model,
                                       map<Leaf::Interface*, Leaf*>&
                                       copy_leafs)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    if (!model) {
        THROW_EXCEPTION(InvalidArgumentException, "\"model\" must not be NULL");
    }

    // Get source leaf ID
    string source_leaf_id = xml->GetAttribute("source");
    if (source_leaf_id.length() == 0) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        "\"edge\" element is missing \"source\" attribute");
    }

    // Get source leaf interface ID
    string source_leaf_interface_id = xml->GetAttribute("sourceinterface");
    if (source_leaf_interface_id.length() == 0) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        "\"edge\" element is missing \"sourceinterface\" attribute");
    }

    // Get target leaf ID
    string target_leaf_id = xml->GetAttribute("target");
    if (target_leaf_id.length() == 0) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        "\"edge\" element is missing \"target\" attribute");
    }

    // Get target leaf interface ID
    string target_leaf_interface_id = xml->GetAttribute("targetinterface");
    if (target_leaf_interface_id.length() == 0) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        "\"edge\" element is missing \"targetinterface\" attribute");
    }

    // Get source and target leafs
    Leaf* source_leaf = model->getLeaf(source_leaf_id);
    if (source_leaf == NULL) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        string("No source leaf \"")
                        + source_leaf_id + "\" found");
    }
    Leaf* target_leaf = model->getLeaf(target_leaf_id);
    if (target_leaf == NULL) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        string("No target leaf \"")
                        + target_leaf_id + "\" found");
    }

    // Get source and target interfaces
    Leaf::Interface* source_interface =
        source_leaf->getOutPort(source_leaf_interface_id);
    if (!source_interface) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        string("No source leaf out interface \"")
                        + source_leaf_id + ":"
                        + source_leaf_interface_id + "\" ");
    }
    Leaf::Interface* target_interface =
        target_leaf->getInPort(target_leaf_interface_id);
    if (!target_interface) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        string("No target leaf in interface \"")
                        + target_leaf_id + ":"
                        + target_leaf_interface_id + "\" found");
    }

    // Check that the target interface is not already connected to another interface
    if (target_interface->isConnected()) {
        THROW_EXCEPTION(ParseException, file_, xml->Row(),
                        string("Target interface \"")
                        + target_leaf_id + ":"
                        + target_leaf_interface_id
                        + "\" is already connected to another interface");
    }

    // Make interface connections
    if (!source_interface->isConnected()) {
        source_interface->connect(target_interface);
        logger_.logMessage(Logger::DEBUG,
                           string("Connected interface \"")
                           + source_interface->toString() + "\" with \""
                           + target_interface->toString() + "\"");
    }
    else {
        // Source interface already connected; use intermediate fanout leaf
        logger_.logMessage(Logger::DEBUG,
                           string("Source interface \"")
                           + source_interface->toString() + "\" already connected "
                           + "to \""
                           + source_interface->getConnectedInterface()->toString()
                           + "\". Using intermediate fanout leaf.");

        // Get fanout leaf
        Leaf* copy_leaf;
        map<Leaf::Interface*, Leaf*>::iterator it =
            copy_leafs.find(source_interface);
        if (it != copy_leafs.end()) {
            copy_leaf = it->second;
        }
        else {
            // No such fanout leaf; create a new one
            copy_leaf = new (std::nothrow)
                fanout(model->getUniqueLeafId("_copySY_"));
            if (copy_leaf == NULL) THROW_EXCEPTION(OutOfMemoryException);
            copy_leafs.insert(pair<Leaf::Interface*, Leaf*>(source_interface,
                                                                 copy_leaf));
            logger_.logMessage(Logger::DEBUG, string("New fanout leaf \"")
                               + copy_leaf->getId()->getString()
                               + "\" created");

            // Add to model
            if (!model->addLeaf(copy_leaf)) {
                THROW_EXCEPTION(IllegalStateException, string("Failed to ")
                                + "add new leaf: Leaf with ID \""
                                + copy_leaf->getId()->getString()
                                + "\" already existed");
            }
            logger_.logMessage(Logger::DEBUG, string("New leaf \"")
                               + copy_leaf->getId()->getString()
                               + "\" added to the model");

            // Break the current connection and connect the source and previous
            // target connection through the fanout leaf
            if(!copy_leaf->addInPort(Id("in"))) {
                THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                                + "in interface to leaf \""
                                + copy_leaf->getId()->getString() + "\"");
            }
            Leaf::Interface* old_target_interface = source_interface->getConnectedInterface();
            source_interface->unconnect();
            logger_.logMessage(Logger::DEBUG,
                               string("Broke interface connection \"")
                               + source_interface->toString() + "\"--\""
                               + old_target_interface->toString() + "\"");
            source_interface->connect(copy_leaf->getInPorts().front());
            logger_.logMessage(Logger::DEBUG,
                               string("Connected interface \"")
                               + source_interface->toString()
                               + "\" with \""
                               + copy_leaf->getInPorts().front()->toString()
                               + "\"");
            if(!copy_leaf->addOutPort(Id("out1"))) {
                THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                                + "out interface to leaf \""
                                + copy_leaf->getId()->getString() + "\"");
            }
            old_target_interface->connect(copy_leaf->getOutPorts().front());
            logger_.logMessage(Logger::DEBUG,
                               string("Connected interface \"")
                               + copy_leaf->getOutPorts().front()->toString()
                               + "\" with \""
                               + old_target_interface->toString() + "\"");
        }

        string new_out_interface_id = string("out")
            + tools::toString(copy_leaf->getOutPorts().size() + 1);
        if(!copy_leaf->addOutPort(new_out_interface_id)) {
            THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                            + "out interface to leaf \""
                            + copy_leaf->getId()->getString() + "\"");
        }
        target_interface->connect(copy_leaf->getOutPorts().back());

        logger_.logMessage(Logger::DEBUG,
                           string("Connected interface \"")
                           + copy_leaf->getOutPorts().back()->toString()
                           + "\" with \""
                           + target_interface->toString() + "\"");
    }
}

void GraphmlParser::checkModelMore(Model* model)
    throw(InvalidArgumentException, InvalidModelException, IOException,
          RuntimeException) {
    bool found_in_interface_leaf = false;
    bool found_out_interface_leaf = false;
    list<Leaf*> leafs = model->getLeafs();
    list<Leaf*>::iterator leaf_it;
    for (leaf_it = leafs.begin(); leaf_it != leafs.end();
         ++leaf_it) {
        Leaf* leaf = *leaf_it;
        logger_.logMessage(Logger::DEBUG,
                           string("Checking leaf \"")
                           + leaf->getId()->getString() + "\"");

        // In- and OutPort presence check
        if (dynamic_cast<InPort*>(leaf)) {
            if (!found_in_interface_leaf) {
                found_in_interface_leaf = true;
            }
            else {
                THROW_EXCEPTION(InvalidModelException,
                                "Only one \"InPort\" leaf is allowed");
            }
        }
        if (dynamic_cast<OutPort*>(leaf)) {
            if (!found_out_interface_leaf) {
                found_out_interface_leaf = true;
            }
            else {
                THROW_EXCEPTION(InvalidModelException,
                                "Only one \"OutPort\" leaf is allowed");
            }
        }
    }
    if (!found_out_interface_leaf) {
        THROW_EXCEPTION(InvalidModelException,
                        "No \"OutPort\" leaf found");
    }
}

void GraphmlParser::postCheckFixes(ForSyDe::Model* model)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    fixModelInputsOutputs(model);
}
