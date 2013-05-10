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
using namespace f2cc::Forsyde;
using ticpp::Document;
using ticpp::Node;
using ticpp::Element;
using std::string;
using std::stringstream;
using std::map;
using std::pair;
using std::list;
using std::vector;
using std::bad_alloc;

XmlParser::XmlParser(Logger& logger) throw() : Frontend(logger) {}

XmlParser::~XmlParser() throw() {}

ProcessNetwork* XmlParser::createProcessNetwork(const string& file)
    throw(InvalidArgumentException, FileNotFoundException, IOException,
          ParseException, InvalidModelException, RuntimeException) {
    if (file.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"file\" must not be empty "
                        "string");
    }

    level_ = 0;
    file_ = file;

    logger_.logMessage(Logger::INFO, "Generating internal process network...");
    ProcessNetwork* processnetwork = new (std::nothrow) ProcessNetwork();
    if (!processnetwork) THROW_EXCEPTION(OutOfMemoryException);

    Document xml_doc = (parseXmlFile(file));
    Element* xml_root = dynamic_cast<Element*>(findXmlRootNode(&xml_doc, file));
    if (!xml_root) THROW_EXCEPTION(CastException);

    Composite* root_comp = buildComposite(xml_root, processnetwork,
    		Id("f2cc0"), Hierarchy());

    processnetwork->addComposite(root_comp);

    list<Composite::IOPort*> input_ports = root_comp->getInIOPorts();
    for (list<Composite::IOPort*>::iterator it = input_ports.begin();
    		it != input_ports.end(); it++){
    	processnetwork->addInput(*it);
    }

    list<Composite::IOPort*> output_ports = root_comp->getOutIOPorts();
    for (list<Composite::IOPort*>::iterator it = output_ports.begin();
    		it != output_ports.end(); it++){
    	processnetwork->addOutput(*it);
    }

    return processnetwork;
}

Composite* XmlParser::buildComposite(Element* xml, ProcessNetwork* processnetwork,
		const Id id, Hierarchy hierarchy)
    throw(InvalidArgumentException, ParseException, InvalidModelException,
          IOException, RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }

    Composite* curr_composite = new Composite(id, hierarchy, Id(getAttributeByTag(xml,string("name"))));
    if (!curr_composite) THROW_EXCEPTION(OutOfMemoryException);

    logger_.logMessage(Logger::DEBUG, string(tools::indent(level_)
                           + "Parsing \"leaf_process\" elements..."));
    parseXmlLeafs(xml, processnetwork, curr_composite);

    logger_.logMessage(Logger::DEBUG, string(tools::indent(level_)
                           + "Parsing \"composite_process\" elements..."));
    parseXmlComposites(xml, processnetwork, curr_composite);

    logger_.logMessage(Logger::DEBUG, string(tools::indent(level_)
                           + "Parsing \"port\" elements..."));
    parseXmlPorts(xml, curr_composite);

    logger_.logMessage(Logger::DEBUG, string(tools::indent(level_)
                           + "Parsing \"signal\" elements..."));
    parseXmlSignals(xml, curr_composite);

    return curr_composite;
}

Document XmlParser::parseXmlFile(const string& file)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {

    // Read file content
    string xml_data;
    logger_.logMessage(Logger::INFO, string(tools::indent(level_)
                           + "Level " + tools::toString(level_)
                           + ". Reading xml data from file: "
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

    return xml_doc;
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
                                                 + "Analyzing leaf line "
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

void XmlParser::parseXmlComposites(Element* xml, ProcessNetwork* processnetwork,
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

    list<Element*> elements = getElementsByName(xml, "composite_process");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        logger_.logMessage(Logger::DEBUG, string(tools::indent(level_)
                                                 + "Analyzing composite line "
                                                 + tools::toString((*it)->Row())
                                                 + "..."));
        Composite* process = generateComposite(processnetwork, *it, parent);
        try {
            if (!parent->addComposite(process)) {
				THROW_EXCEPTION(ParseException, parent->getName().getString(),
						        (*it)->Row(),
								(*it)->Column(),
								string("Multiple processes with ID \"")
								+ process->getId()->getString() + "\"");
			}
            if (!processnetwork->addComposite(process)) {
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
    if (!xml) {
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

          generateIOPort(*it, parent);
      }
}

void XmlParser::parseXmlSignals(Element* xml, Composite* parent)
	throw(InvalidArgumentException, ParseException, IOException,
            RuntimeException){
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }

    list<Element*> elements = getElementsByName(xml, "signal");
    list<Element*>::iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
          logger_.logMessage(Logger::DEBUG, string(tools::indent(level_)
                                                   + "Analyzing line "
                                                   + tools::toString((*it)->Row())
                                                   + "..."));
          generateSignal(*it, parent);
      }
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
    string process_id = string(parent->getId()->getString() + "_" +
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
        			generateLeafFunction(constructor_element, pn, parent));
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

        generateLeafPort((*it), leaf_process);
    }

    return leaf_process;
}

Composite* XmlParser::generateComposite(ProcessNetwork* pn, Element* xml,
		Composite* parent)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    // Getting composite component name and file
    string composite_name = getAttributeByTag(xml, "component_name");
    if (composite_name.length() == 0) {
        THROW_EXCEPTION(ParseException, parent->getName().getString(), xml->Row(),
        		"No composite component name");
    }
    string composite_filename = string(composite_name + ".xml");

    // Generating Composite ID
    Id composite_id = Id(string(parent->getId()->getString()
    		+ "_" + getAttributeByTag(xml, "name")));

    //building XML data from its file
    level_++;
    string previous_file = file_;
    file_ = composite_filename;
    Document xml_doc = (parseXmlFile(composite_filename));
    Element* xml_root = dynamic_cast<Element*>(findXmlRootNode(&xml_doc, composite_filename));
    if (!xml_root) THROW_EXCEPTION(CastException);

    Composite* composite_process = buildComposite(xml_root, pn, composite_id,
    		parent->getHierarchy());
    if (!composite_process) THROW_EXCEPTION(OutOfMemoryException);
    level_--;
    file_ = previous_file;
    logger_.logMessage(Logger::DEBUG, string(tools::indent(level_))
                       + "Generated " + composite_process->type()
                       + " with ID: " + composite_process->getId()->getString()
                       + " from \"" + composite_process->getName().getString() + "\"");

    return composite_process;
}

CFunction* XmlParser::generateLeafFunction(Element* xml, ProcessNetwork* pn,
		Composite* parent)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }

    Element* argument = getUniqueElement(xml, "argument");
    string file_name = getAttributeByTag(argument,"value");
    string name = getAttributeByTag(argument,"name");
    string function_name = file_name;
    tools::searchReplace(function_name, name, "");
    file_name += ".hpp";

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
        CParser* code_parser = new CParser(logger_, level_);
        CFunction* new_function = code_parser->parseCFunction(file_name, function_name);
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
        delete code_parser;
        return new_function;
    }

    // No such element found
    THROW_EXCEPTION(ParseException, parent->getName().getString(), xml->Row(),
                    "No process function argument found");
}

void XmlParser::generateLeafPort(Element* xml, Leaf* parent)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    string port_name = getAttributeByTag(xml,"name");
    string port_datatype = getAttributeByTag(xml,"type");
    int port_size = tools::toInt(getAttributeByTag(xml, "size"));
	string port_direction = getAttributeByTag(xml, "direction");

	//datatype conversion
	CDataType data_type = getDataType(port_datatype, port_size);

	bool port_added;
    if (port_direction == "in") port_added = parent->addInPort(Id(port_name), data_type);
    else if (port_direction == "out") port_added = parent->addOutPort(Id(port_name), data_type);
    else THROW_EXCEPTION(ParseException, file_, xml->Row(),
    		xml->Column(), "Invalid port direction");

    if (!(port_added)) {
        THROW_EXCEPTION(ParseException, file_,
        				xml->Row(),
        				xml->Column(), string("Multiple ")
                        + ((port_direction == "in") ? "in ports" : "out ports")
                        + " with the same ID \""
                        + port_name + "\"");
    }
    logger_.logMessage(Logger::DEBUG, string()
                       + tools::indent(level_)
                       + ((port_direction == "in") ? "In" : "Out")
                       + " port \"" + port_name
                       + "\" added to leaf process \""
                       + parent->getId()->getString() + "\"");

    SY::Comb* comb = dynamic_cast<SY::Comb*>(parent);
    if (comb){
    	associatePortWithVariable(comb, port_direction, port_name);
    }
}

void XmlParser::generateIOPort(Element* xml, Composite* parent)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }
    string port_name = getAttributeByTag(xml,"name");
    string port_direction = getAttributeByTag(xml,"direction");
    string bound_process = getAttributeByTag(xml,"bound_process");
    string bound_port = getAttributeByTag(xml,"bound_port");

    /* @todo: check whether the xml port data in the opened file corresponds to
     * the xml port data in the caller file
     */

    Composite::IOPort* this_ioport;
	bool port_added;
	if (port_direction == "in") {
		port_added = parent->addInIOPort(Id(port_name));
		this_ioport = parent->getInIOPort(Id(port_name));
	}
	else if (port_direction == "out"){
		port_added = parent->addOutIOPort(Id(port_name));
		this_ioport = parent->getOutIOPort(Id(port_name));
	}
	else THROW_EXCEPTION(ParseException, parent->getName().getString(), xml->Row(),
			xml->Column(), "Invalid port direction");

	if (!(port_added)) {
		THROW_EXCEPTION(ParseException, parent->getName().getString(),
						xml->Row(),
						xml->Column(), string("Multiple ")
						+ ((port_direction == "in") ? "in ports" : "out ports")
						+ " with the same ID \""
						+ port_name + "\"");
	}
	logger_.logMessage(Logger::DEBUG, string()
					   + tools::indent(level_)
					   + ((port_direction == "in") ? "In" : "Out")
					   + " port \"" + port_name
					   + "\" added to composite process \""
					   + parent->getId()->getString() + "\"");

	Leaf* bound_leaf = parent->getProcess(Id(string(
			parent->getId()->getString() + "_" + bound_process)));
	if (bound_leaf){
		if (port_direction == "in"){
			Leaf::Port* bound_ioport = bound_leaf->getInPort(Id(bound_port));
			generateConnection(this_ioport, bound_ioport);
		}
		else{
			Leaf::Port* bound_ioport = bound_leaf->getOutPort(Id(bound_port));
			generateConnection( bound_ioport, this_ioport);
		}
	}
	else {
		Composite* bound_composite = parent->getComposite(Id(string(
				parent->getId()->getString() + "_" + bound_process)));
		if (!bound_composite){
			THROW_EXCEPTION(ParseException, file_,
							xml->Row(),
							xml->Column(), string("Cannot find \"")
							+ bound_process
							+ "\" inside composite process\""
							+ parent->getId()->getString() + "\"");
		}
		if (port_direction == "in"){
			Composite::IOPort* bound_ioport = bound_composite->getInIOPort(Id(bound_port));
			generateConnection(this_ioport, bound_ioport);
		}
		else{
			Composite::IOPort* bound_ioport = bound_composite->getOutIOPort(Id(bound_port));
			generateConnection(bound_ioport, this_ioport);
		}
	}
}

void XmlParser::generateSignal(Element* xml, Composite* parent)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException) {
    if (!xml) {
        THROW_EXCEPTION(InvalidArgumentException, "\"xml\" must not be NULL");
    }
    if (!parent) {
        THROW_EXCEPTION(InvalidArgumentException, "\"parent\" must not be NULL");
    }
    string source = getAttributeByTag(xml,"source");
    string source_port = getAttributeByTag(xml,"source_port");
    string target = getAttributeByTag(xml,"target");
    string target_port = getAttributeByTag(xml,"target_port");

    /* @todo: check whether the xml port data in the opened file corresponds to
     * the xml port data in the caller file
     */

    Process::Interface* source_interface;
	Leaf* source_leaf = parent->getProcess(Id(string(
			parent->getId()->getString() + "_" + source)));
	if (source_leaf){
		source_interface = source_leaf->getOutPort(Id(source_port));
	}
	else {
		Composite* source_composite = parent->getComposite(Id(string(
				parent->getId()->getString() + "_" + source)));
		if (source_composite){
			source_interface = source_composite->getOutIOPort(Id(source_port));
		}
		else {
			THROW_EXCEPTION(ParseException, file_,
							xml->Row(),
							xml->Column(), string("Cannot find \"")
							+ source
							+ "\" inside composite process\""
							+ parent->getId()->getString() + "\"");
		}
	}

    Process::Interface* target_interface;
	Leaf* target_leaf = parent->getProcess(Id(string(
			parent->getId()->getString() + "_" + target)));
	if (target_leaf){
		target_interface = target_leaf->getInPort(Id(target_port));
	}
	else {
		Composite* target_composite = parent->getComposite(Id(string(
				parent->getId()->getString() + "_" + target)));
		if (target_composite){
			target_interface = target_composite->getInIOPort(Id(target_port));
		}
		else {
			THROW_EXCEPTION(ParseException, file_,
							xml->Row(),
							xml->Column(), string("Cannot find \"")
							+ target
							+ "\" inside composite process\""
							+ parent->getId()->getString() + "\"");
		}
	}

	generateConnection(source_interface, target_interface);

}

CDataType XmlParser::getDataType(const string& port_datatype,
		const int &port_size)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException){

	CDataType* data_type;

	if (string::npos != port_datatype.find("array")){
		unsigned type_begin = port_datatype.find_last_of("<") + 1;
		unsigned type_end = port_datatype.find_first_of(">");
		string base_datatype = port_datatype.substr(type_begin, type_end - type_begin);
		tools::trim(base_datatype);

		int size = tools::noElements(port_size, base_datatype);
		if (size == -1){
			THROW_EXCEPTION(InvalidArgumentException, "\"port_datatype\" carries the wrong type");
		}

		data_type = new CDataType(CDataType::stringToType(base_datatype),
					true, true, size, false, false);

		return *data_type;
	}
	else{
		string base_datatype = port_datatype;
		tools::trim(base_datatype);
		data_type = new CDataType(CDataType::stringToType(base_datatype),
					false, false, 0, false, false);
		return *data_type;
	}

}

void XmlParser::associatePortWithVariable(SY::Comb* comb, string direction,
		string port_name)
throw(InvalidArgumentException, ParseException, IOException,
      RuntimeException){
    if (!comb) {
        THROW_EXCEPTION(InvalidArgumentException, "\"comb\" must not be NULL");
    }

    CVariable* assoc_param;
    Leaf::Port* assoc_port;
    CDataType* param_type;
    CDataType port_type ;
    if (direction == "in"){
    	list<CVariable*> inputs = comb->getFunction()->getInputParameters();

    	size_t last_index = port_name.find_last_not_of("0123456789");
    	string numeral = port_name.substr(last_index + 1);
		unsigned int number = tools::toInt(numeral);
		if (number < 1){
			THROW_EXCEPTION(InvalidArgumentException, string("\"port_name\" ")
					+"does not have a valid order: "
					+ port_name);
		}
		if (inputs.size() > number - 1){
			list<CVariable*>::iterator it = inputs.begin();
			std::advance(it, number - 1);
			assoc_param = *it;
		}
		else{
			THROW_EXCEPTION(InvalidArgumentException, string("\"port_name\" ")
					+"(" + port_name + ") "
					+"has higher numeral (" + tools::toString(number - 1)
					+ ") than the number of ports available: "
					+ tools::toString(inputs.size()));
		}
		assoc_port = comb->getInPort(Id(port_name));
    }
    else{
    	assoc_param = comb->getFunction()->getOutputParameter();
    	assoc_port = comb->getOutPort(Id(port_name));
    }
    param_type = assoc_param->getDataType();
    port_type = assoc_port->getDataType();

	//double check
	bool match_is_array = param_type->isArray() == port_type.isArray();
	bool match_data_type = param_type->getType() == port_type.getType();
	if ((!match_is_array) || (!match_data_type)){
		THROW_EXCEPTION(RuntimeException, "Function and port data types do not match.");
	}

	//set the array size in variable declaration
	if (port_type.hasArraySize() && (!param_type->hasArraySize())){
		param_type->setArraySize(port_type.getArraySize());
		logger_.logMessage(Logger::DEBUG, string()
							   + tools::indent(level_)
							   + "Added array size to input parameter \""
							   + assoc_param->getReferenceString()
							   + "\" : "
							   + tools::toString(param_type->getArraySize()) );
	}

	//associate port with variable
	assoc_port->setVariable(assoc_param);

	logger_.logMessage(Logger::DEBUG, string()
					   + tools::indent(level_)
					   + "Associated port \"" + assoc_port->getId()->getString()
					   + "\" with variable \"" + assoc_param->getReferenceString()
					   + "\" in function \"" + comb->getFunction()->getName()
					   + "\"");

}

void XmlParser::generateConnection(Process::Interface* source_port,
		Process::Interface* target_port)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException, CastException) {
    if (!source_port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"source_port\" must not be NULL");
    }
    if (!target_port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"target_port\" must not be NULL");
    }

	logger_.logMessage(Logger::DEBUG, string()
					   + tools::indent(level_)
					   + "Generating connection between \""
					   + source_port->toString()
					   + "\" and \""
					   + target_port->toString() + "\"...");



	Leaf::Port* source = dynamic_cast<Leaf::Port*>(source_port);
	if(source){
		if(!source->isConnected()){
			source->connect(target_port);
			logger_.logMessage(Logger::DEBUG, string()
							   + tools::indent(level_)
							   + "Generated connection for \""
							   + source->toString()
							   + "\"");
		}
		else {
			SY::Fanout* fanout = dynamic_cast<SY::Fanout*>(source->getProcess());
			if (fanout){
				logger_.logMessage(Logger::DEBUG, string()
								   + tools::indent(level_)
								   + "Parent process for \""
								   + source->toString()
								   + "\" is a fanout. Generating a new port.");
				Id new_id = Id(string(fanout->getOutPorts().back()->getId()->getString()
						+ "_"));
				fanout->addOutPort(new_id, source->getDataType());
				fanout->getOutPort(new_id)->connect(target_port);
				logger_.logMessage(Logger::DEBUG, string()
								   + tools::indent(level_)
								   + "Added new port \""
								   + new_id.getString()
								   + "\" to \""
								   + source->getProcess()->getId()->getString()
								   + "\" and generated connection");

			}
			else {
				THROW_EXCEPTION(ParseException, file_,
								string("The port ")
								+ source->getId()->getString()
								+ " has multiple connections. Automatic handling"
								+ " is not yet available. Please make sure that all"
								+ " multiple connections pass through a fanout.");
			}
		}
	}
	else{
		Composite::IOPort* source_io = dynamic_cast<Composite::IOPort*>(source_port);
		if (!source_io) THROW_EXCEPTION(CastException);
		else {
			logger_.logMessage(Logger::WARNING, string()
					+ tools::indent(level_)
					+ "Multiple connections are not treated for IO ports.");

			source_io->connect(target_port);
			logger_.logMessage(Logger::DEBUG, string()
							   + tools::indent(level_)
							   + "Generated connection for \""
							   + source_io->toString()
							   + "\"");
		}
	}
}

Node* XmlParser::findXmlRootNode(Document* xml, const string& file)
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

    return xml_root_node;
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

    list<Element*> elements = getElementsByName(xml, name);
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
    logger_.logMessage(Logger::WARNING,string()
			   + tools::indent(level_)
    		   + "XML document check not implemented");
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


////////////////////////////////////////////////////////////////////

XmlParser::CParser::CParser(Logger& logger, int indent) throw() :
		level_(indent), file_(NULL), cdata_(NULL), logger_(logger){}

XmlParser::CParser::~CParser() throw() {}

CFunction* XmlParser::CParser::parseCFunction(const string& file, const string& name)
    throw(InvalidArgumentException, IOException, ParseException, RuntimeException) {
    if (file.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"file\" must not be empty "
                        "string");
    }
    if (file.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"file\" must not be empty "
                        "string");
    }
    if (!tools::existsFile(file)) {
        THROW_EXCEPTION(IOException, string("File \"")
        		        + file + "\" does not exist");
    }

    file_ = file;
    tools::readFile(file,cdata_);
    if (cdata_.length() == 0) {
        THROW_EXCEPTION(IOException, file_, "file contains no data ");
    }

    CFunction* function = new CFunction(name, file);

	logger_.logMessage(Logger::DEBUG,
	        		string(tools::indent(level_)
	                      + "Parsing the function declaration..."));
    parseDeclaration(function);

	logger_.logMessage(Logger::DEBUG,
	        		string(tools::indent(level_)
	                      + "Extracting the function body..."));
    extractBody(function);

	logger_.logMessage(Logger::DEBUG,
	        		string(tools::indent(level_)
	                      + "Renaming wrapped variables..."));
	renameWrappedVariables(function);

    return function;
}

void XmlParser::CParser::parseDeclaration(CFunction* function)
    throw(InvalidArgumentException, ParseException) {
    if (!function) {
        THROW_EXCEPTION(InvalidArgumentException, "\"function\" must not be empty "
                        "string");
    }

    std::string declaration = "";
    string name = function->getName();

    //getting the declaration stream
    stringstream tempcode(cdata_);
    std::string line;
    unsigned current_line = 0;
    bool is_declaration = false;
    while (std::getline(tempcode, line)) {
    	if((string::npos != line.find(name)) &&
    			(string::npos != line.find("void"))){
    		is_declaration = true;
    		declaration += line;
    	}
    	else if((is_declaration) && (string::npos != line.find("{"))){
			declaration += line;
			cdata_.erase(cdata_.find(line), line.size());
			break;
		}
    	else if (is_declaration){
    		declaration += line;
    	}
    	cdata_.erase(cdata_.find(line), line.size() + 1);
    	current_line++;
    }
    if (!is_declaration){
		THROW_EXCEPTION(ParseException, file_,
				        current_line,
						string("Function \"")
						+ name + "\" does not have a declaration.");
	}
    tools::searchReplace(declaration, "\t", "");
    tools::searchReplace(declaration, "void", "");
    tools::searchReplace(declaration, name, "");
    tools::trim(declaration);

    bool is_output = true;
    unsigned previous = 0;
    unsigned found = declaration.find_first_of(",<{");
    while (found!=std::string::npos){
    	string part_before = declaration.substr(previous, found - previous);
    	if (declaration[found] == ','){
    		createFunctionParameter(function, part_before, is_output);
    		is_output = false;
    	}
    	if (declaration[found] == '<'){
    		string * data_type = new string();
    		string part_after = declaration.substr(found + 1,
    				declaration.size() - found - 1);
    		if(string::npos != part_before.find("array")){
    			found = found + getArrayDataType(part_after, data_type) + 1;
    			unsigned var_pos = declaration.find_first_of(",)", found);
    			string var_name = declaration.substr(found, var_pos - found);
    			createFunctionParameter(function, *data_type + var_name, is_output);
    			is_output = false;
    			found = var_pos + 1;
    		}
    		else{
    			found = found + getTemplateBaseDataType(part_after, data_type) + 1;
    			unsigned var_pos = declaration.find_first_of(",)", found);
    			string var_name = declaration.substr(found, var_pos - found);
    			createFunctionParameter(function, *data_type + var_name, is_output);
    			is_output = false;
    			found = var_pos + 1;
    		}
    		delete data_type;
    	}
    	if (declaration[found] == '{') break;
    	previous = found;
    	found = declaration.find_first_of(",<{", found+1);
    }
}

void XmlParser::CParser::extractBody(CFunction* function)
    throw(InvalidArgumentException, IOException){
    if (!function) {
        THROW_EXCEPTION(InvalidArgumentException, "\"function\" must not be empty "
                        "string");
    }
    std::string body = "";

    //getting the declaration stream
    stringstream tempcode(cdata_);
    std::string line;
    bool is_body = false;
    while (std::getline(tempcode, line)) {
    	if(string::npos != line.find("#pragma ForSyDe begin")){
    		is_body = true;
    	}
    	else if((is_body) && (string::npos != line.find("#pragma ForSyDe end"))){
			break;
		}
    	else if (is_body){
    		body += line;
    		cdata_.erase(cdata_.find(line), line.size() + 1);
    	}
    }
    if (!is_body){
		THROW_EXCEPTION(IOException,
						string("The function in file \"")
						+ file_ + "\" has no body.");
	}
}

void XmlParser::CParser::renameWrappedVariables(CFunction* function)
        throw(InvalidArgumentException, ParseException){
	if (!function) {
		THROW_EXCEPTION(InvalidArgumentException, "\"function\" must not be empty "
						"string");
	}
    stringstream tempcode(cdata_);
    std::string line;

    list<CVariable*> var_list =function->getInputParameters();
    var_list.push_back(function->getOutputParameter());

    list<pair<string, string> > name_dict;

    while (std::getline(tempcode, line)) {
    	if(string::npos != line.find("=")){

			unsigned equal_pos = line.find_first_of("=");

			//get lhs;
			string lhs = line.substr(0, equal_pos);
			tools::trim(lhs);
			if(string::npos != lhs.find(" ")){
				unsigned decl_end_pos = lhs.find_last_of(" ");
				lhs = lhs.substr(decl_end_pos, lhs.size() - decl_end_pos);
				tools::trim(lhs);
			}
			//get rhs
			string rhs = line.substr(equal_pos, line.size() - equal_pos);
			unsigned first_par_pos = rhs.find_last_of("(");
			rhs = rhs.substr(first_par_pos + 1, rhs.size() - first_par_pos - 1);
			rhs = rhs.substr(0, rhs.find_first_of("),"));

			if ((lhs.length() == 0) || (rhs.length() == 0)){
				THROW_EXCEPTION(ParseException, file_,
								string("Could not find rhs or lhs in:\n")
								+ line);
			}
			name_dict.push_back(std::make_pair(lhs,rhs));
    	}
    }

    list<CVariable*>::iterator var_it;
    list<pair<string, string> >::iterator dict_it;
    for (var_it = var_list.begin(); var_it != var_list.end(); ++var_it){
    	string var_name = (*var_it)->getReferenceString();
    	for (dict_it = name_dict.begin(); dict_it != name_dict.end(); ++dict_it){
    		if (var_name == (*dict_it).first) {
    			(*var_it)->changeReferenceString((*dict_it).second);
    			logger_.logMessage(Logger::DEBUG,
    			        		string(tools::indent(level_)
    			                      + "Renamed variable \""
    			                      + var_name
    			                      +"\" to \""
    			                      + (*dict_it).second
    			                      + "\" to function \""
    			                      + file_
    			                      +  "\"..."));
    			name_dict.erase(dict_it);
    			var_it = var_list.begin();
    			break;
    		}
    		if (var_name == (*dict_it).second) {
				(*var_it)->changeReferenceString((*dict_it).first);
				logger_.logMessage(Logger::DEBUG,
								string(tools::indent(level_)
									  + "Renamed variable \""
									  + var_name
									  +"\" to \""
									  + (*dict_it).first
									  + "\" to function \""
									  + file_
									  +  "\"..."));
				name_dict.erase(dict_it);
				var_it = var_list.begin();
				break;
			}
    	}
    }

    if (name_dict.size() != 0){
    	THROW_EXCEPTION(ParseException, file_,
						string("Parameter renaming is incomplete.")
						+ " Remaining are: \n"
						+ (*name_dict.begin()).first + " : "
						+ (*name_dict.begin()).second);
    }

}

void XmlParser::CParser::createFunctionParameter(CFunction* function, string analysis_string,
		bool is_output)
     throw(InvalidArgumentException, ParseException, OutOfMemoryException){
    if (!function) {
        THROW_EXCEPTION(InvalidArgumentException, "\"function\" must not be empty "
                        "string");
    }
	int separator = analysis_string.find_last_of(" ");

	CDataType* c_data_type;
	bool is_array = false;
	string data_type_string = analysis_string.substr(0, separator);
	if (string::npos != data_type_string.find("*")){
		is_array=true;
		tools::searchReplace(data_type_string, "*", "");
	}
	tools::searchReplace(data_type_string, "&", "");
	tools::trim(data_type_string);
	c_data_type = new CDataType(CDataType::stringToType(data_type_string),
			is_array, false, 0, false, false);

	CVariable* c_variable;
	string name_string = analysis_string.substr(separator + 1,
			analysis_string.length() - separator - 1);
	tools::searchReplace(name_string, "&", "");
	tools::trim(name_string);
	c_variable = new CVariable(name_string, *c_data_type);

	bool is_added;
	try {
		if (is_output) is_added = function->setOutputParameter(*c_variable);
		else is_added = function->addInputParameter(*c_variable);
		if (is_added) {
			logger_.logMessage(Logger::DEBUG,
			        		string(tools::indent(level_)
			                      + "Added variable \""
			                      + c_variable->getDataType()->toString()
			                      +" "
			                      + c_variable->getReferenceString()
			                      + "\" to function \""
			                      + file_
			                      +  "\"..."));
		}
	} catch (bad_alloc&) {
		THROW_EXCEPTION(OutOfMemoryException);
	}
	delete c_data_type;
	delete c_variable;
}

unsigned XmlParser::CParser::getArrayDataType(const string& analysis_string, string* data_type)
     throw(InvalidArgumentException, ParseException){
    if (analysis_string.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"analysis_string\" must not be empty "
                        "string");
    }

    unsigned found_data_type = analysis_string.find_first_of(",");
    *data_type = analysis_string.substr(0, found_data_type);
    tools::trim(*data_type);
	*data_type+="*";

	unsigned found_template_end = analysis_string.find_first_of(">");

    return found_template_end + 1;

}

unsigned XmlParser::CParser::getTemplateBaseDataType(const string& analysis_string, string* data_type)
     throw(InvalidArgumentException, ParseException){
    if (analysis_string.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"analysis_string\" must not be empty "
                        "string");
    }

    unsigned template_end = analysis_string.find_first_of(">");
    unsigned template_start = analysis_string.find_first_of("<",1);
    unsigned out_pos;

    bool found_nested_template = (template_end!=string::npos) &&
    		(template_start!=string::npos) && (template_start < template_end);

	if (found_nested_template){
		string part_before = analysis_string.substr(0, template_start);
		string part_after = analysis_string.substr(template_start + 1,
				analysis_string.size() - template_start - 1);
		if(string::npos != part_before.find("array")){
			out_pos = getArrayDataType(part_after, data_type);
			out_pos = out_pos +template_start + 1;
			template_end = analysis_string.find_first_of(">",out_pos);
			template_start = analysis_string.find_first_of("<",out_pos);
			if((template_end!=string::npos) && (template_start!=string::npos) &&
					(template_start < template_end)){
				THROW_EXCEPTION(ParseException, file_,
								string("Declaration has complex templates \n")
								+ analysis_string
								+"\n in file \""
								+ file_ + "\"");
			}
		}
		else{
			out_pos = getTemplateBaseDataType(part_after, data_type);
			out_pos = out_pos +template_start + 1;
			template_end = analysis_string.find_first_of(">",out_pos);
			template_start = analysis_string.find_first_of("<",out_pos);
			if((template_end!=string::npos) && (template_start!=string::npos) &&
					(template_start < template_end)){
				THROW_EXCEPTION(ParseException, file_,
								string("Declaration has complex templates \n")
								+ analysis_string
								+"\n in file \""
								+ file_ + "\"");
			}
		}
	}
	else if (template_end!=string::npos){
	    *data_type = analysis_string.substr(0, template_end);
	    tools::searchReplace(*data_type, "<", "");
	    tools::searchReplace(*data_type, ">", "");
	    tools::trim(*data_type);

	    out_pos = template_end;
	}
	else {
		THROW_EXCEPTION(ParseException, file_,
						string("Declaration template is not closed \n")
						+ analysis_string
						+"\n in file \""
						+ file_ + "\"");
	}

	return out_pos + 1;
}

