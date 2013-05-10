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

#ifndef F2CC_SOURCE_FRONTEND_XMLPARSER_H_
#define F2CC_SOURCE_FRONTEND_XMLPARSER_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines the Grapml-parsing frontend.
 */

#include "frontend.h"
#include "../logger/logger.h"
#include "../forsyde/id.h"
#include "../forsyde/processnetwork.h"
#include "../forsyde/composite.h"
#include "../forsyde/leaf.h"
#include "../forsyde/SY/combsy.h"
#include "../language/cfunction.h"
#include "../language/cdatatype.h"
#include "../ticpp/ticpp.h"
#include "../exceptions/filenotfoundexception.h"
#include "../exceptions/parseexception.h"
#include "../exceptions/invalidmodelexception.h"
#include "../exceptions/runtimeexception.h"
#include "../exceptions/castexception.h"
#include <string>
#include <map>
#include <list>

namespace f2cc {

/**
 * @brief A class for parsing a GraphML file into an internal ForSyDe processnetwork
 *        representation.
 *
 * The \c GraphmlParser is a frontend to the \c f2cc which parses a GraphML
 * representation of a ForSyDe processnetwork and converts it into an internal
 * equivalent. Any unrecognized elements in the XML file will be ignored.
 *
 * The class uses <a href="http://code.google.com/p/ticpp/">TinyXML++</a> for
 * parsing the XML file.
 */
class XmlParser : public Frontend {
  public:
    /**
     * Creates a GraphML parser.
     *
     * @param logger
     *        Reference to the logger.
     */
    XmlParser(Logger& logger) throw();

    /**
     * Destroys this parser. The logger remains open.
     */
    ~XmlParser() throw();

  private:
    /**
     * @copydoc Frontend::createProcessNetwork(const std::string&)
     */
    virtual Forsyde::ProcessNetwork* createProcessNetwork(const std::string& file)
        throw(InvalidArgumentException, FileNotFoundException, IOException,
              ParseException, InvalidModelException, RuntimeException);

    /**
     * Converts an \c graph XML element into an internal ForSyDe processnetwork. The
     * method makes no checks on whether the resultant processnetwork appears sane or
     * not.  It is the caller's responsibility to destroy the processnetwork when it is
     * no longer used.
     *
     * @param xml
     *        \c graph XML element containing the processnetwork.
     * @returns Internal ForSyDe processnetwork object.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws InvalidModelException
     *         When the processnetwork is invalid (but was successfully parsed).
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     *
     * @todo: reimplement model so that the hierarchy is passed as a reference (faster).
     * @todo: reimplement model so that hierarchy is not needed for Composite constructor.
     */
    Forsyde::Composite* buildComposite(ticpp::Element* xml,
    		Forsyde::ProcessNetwork* processnetwork,const Forsyde::Id id,
    		Forsyde::Hierarchy hierarchy)
    throw(InvalidArgumentException, ParseException, InvalidModelException,
          IOException, RuntimeException);

    /**
     * Gets the ID of an XML element. The ID is specified in an XML attribute
     * named \c id. Any surrounding whitespace will be trimmed.
     *
     * @param xml
     *        XML element.
     * @returns Its ID.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    ticpp::Document parseXmlFile(const std::string& file)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);


    /**
     * Parses the \c node XML elements in a \c graph XML element and converts
     * them into corresponding leafs, which are then added to the processnetwork.
     * mapset.
     *
     * @param xml
     *        \c graph XML element containing the \c node XML elements.
     * @param processnetwork
     *        ProcessNetwork object to add the leaf to.
     * @throws InvalidArgumentException
     *         When \c xml or \c processnetwork is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void parseXmlLeafs(ticpp::Element* xml, Forsyde::ProcessNetwork* processnetwork,
    		Forsyde::Composite* parent)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Parses the \c node XML elements in a \c graph XML element and converts
     * them into corresponding leafs, which are then added to the processnetwork.
     * mapset.
     *
     * @param xml
     *        \c graph XML element containing the \c node XML elements.
     * @param processnetwork
     *        ProcessNetwork object to add the leaf to.
     * @throws InvalidArgumentException
     *         When \c xml or \c processnetwork is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void parseXmlComposites(ticpp::Element* xml, Forsyde::ProcessNetwork* processnetwork,
    		Forsyde::Composite* parent)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Parses the \c node XML elements in a \c graph XML element and converts
     * them into corresponding leafs, which are then added to the processnetwork.
     * mapset.
     *
     * @param xml
     *        \c graph XML element containing the \c node XML elements.
     * @param processnetwork
     *        ProcessNetwork object to add the leaf to.
     * @throws InvalidArgumentException
     *         When \c xml or \c processnetwork is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void parseXmlPorts(ticpp::Element* xml, Forsyde::Composite* parent)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Parses the \c node XML elements in a \c graph XML element and converts
     * them into corresponding leafs, which are then added to the processnetwork.
     * mapset.
     *
     * @param xml
     *        \c graph XML element containing the \c node XML elements.
     * @param processnetwork
     *        ProcessNetwork object to add the leaf to.
     * @throws InvalidArgumentException
     *         When \c xml or \c processnetwork is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void parseXmlSignals(ticpp::Element* xml, Forsyde::Composite* parent)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Converts an XML \c node element into an internal ForSyDe leaf of the
     * same type along with its ports and function argument, if any.
     *
     * @param xml
     *        \c node element containing the leaf.
     * @returns Internal leaf object.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    Forsyde::Leaf* generateLeaf(Forsyde::ProcessNetwork* pn, ticpp::Element* xml,
    		Forsyde::Composite* parent)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Converts an XML \c node element into an internal ForSyDe leaf of the
     * same type along with its ports and function argument, if any.
     *
     * @param xml
     *        \c node element containing the leaf.
     * @returns Internal leaf object.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    Forsyde::Composite* generateComposite(Forsyde::ProcessNetwork* pn, ticpp::Element* xml,
    		Forsyde::Composite* parent)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Gets the leaf function argument from a XML \c node element. The
     * function argument is specified in an XML "data" child element.
     *
     * @param xml
     *        \c node element.
     * @returns Leaf function argument.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When a leaf function argument cannot be found or is invalid.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    CFunction* generateLeafFunction(ticpp::Element* xml, Forsyde::ProcessNetwork* pn,
    		Forsyde::Composite* parent)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Converts an XML \c port element into an internal ForSyDe port.
     *
     * @param xml
     *        \c port element containing the port.
     * @returns Internal leaf port object.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void generateLeafPort(ticpp::Element* xml, Forsyde::Leaf* parent)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException);

    /**
     * Converts an XML \c port element into an internal ForSyDe port.
     *
     * @param xml
     *        \c port element containing the port.
     * @returns Internal leaf port object.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void generateIOPort(ticpp::Element* xml, Forsyde::Composite* parent)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException);

    /**
     * Converts an XML \c port element into an internal ForSyDe port.
     *
     * @param xml
     *        \c port element containing the port.
     * @returns Internal leaf port object.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void generateSignal(ticpp::Element* xml, Forsyde::Composite* parent)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException);

    /**
     * Converts an XML \c port element into an internal ForSyDe port.
     *
     * @param xml
     *        \c port element containing the port.
     * @returns Internal leaf port object.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void generateConnection(
    		Forsyde::Process::Interface* source_port,
    		Forsyde::Process::Interface* target_port)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException, CastException);


    /**
     * Converts an XML \c port element into an internal ForSyDe port.
     *
     * @param xml
     *        \c port element containing the port.
     * @returns Internal leaf port object.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    CDataType getDataType(const std::string& port_datatype,const int &port_size)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException);

    /**
     * Converts an XML \c port element into an internal ForSyDe port.
     *
     * @param xml
     *        \c port element containing the port.
     * @returns Internal leaf port object.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void associatePortWithVariable(Forsyde::SY::Comb* comb, std::string direction,
    		std::string name)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException);


    /**
     * Locates the \c graph XML element in the XML document.
     *
     * @param xml
     *        XML document.
     * @returns graph element.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         If no or multiple graph elements were found.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    ticpp::Node* findXmlRootNode(ticpp::Document* xml,
    		const std::string& file)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Gets a list of elements with a particular name which are immediate
     * children to a XML object. If none are found, an empty list is returned.
     *
     * @param xml
     *        XML object to search.
     * @param name
     *        Name of the elements to search for.
     * @returns List of
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL or when \c name is an empty string.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    std::list<ticpp::Element*> getElementsByName(ticpp::Node* xml,
                                                 const std::string& name)
        throw(InvalidArgumentException, IOException, RuntimeException);

    /**
     * Gets a list of elements with a particular name which are immediate
     * children to a XML object. If none are found, an empty list is returned.
     *
     * @param xml
     *        XML object to search.
     * @param name
     *        Name of the elements to search for.
     * @returns List of
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL or when \c name is an empty string.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    ticpp::Element* getUniqueElement(ticpp::Node* xml,
                                                 const std::string& name)
        throw(InvalidArgumentException, IOException, RuntimeException);

    /**
     * Scans the entire XML structure and checks that all needed elements and
     * attributes are there and also removes all elements and attributes which
     * are not needed for the latter parsing stages. This primarily means data
     * not included in the GraphML specification, but also data which actually
     * is part of the specification but not used by the synthesis component.
     *
     * Each pruned data which belongs to the GraphML specification generates an
     * entry of type \c Logger::LogLevel::INFO in the log file. Each pruned data
     * which does not belongs to the GraphML specification generates an entry of
     * type \c Logger::LogLevel::WARNING in the log file.
     *
     * @param xml
     *        XML structure to sanitize.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     * @todo Implement this method (currently it does nothing).
     */
    void checkXmlDocument(ticpp::Document* xml)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Gets the ID of an XML element. The ID is specified in an XML attribute
     * named \c id. Any surrounding whitespace will be trimmed.
     *
     * @param xml
     *        XML element.
     * @returns Its ID.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    std::string getAttributeByTag(ticpp::Element* xml, std::string name)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Gets the initial delay value from a XML \c node element. The value is
     * specified in an XML "data" child element.
     *
     * @param xml
     *        \c node element.
     * @returns Initial delay value.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When an initial delay value cannot be found or is invalid.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    std::string getInitialDelayValue(ticpp::Element* xml, Forsyde::Composite* parent)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);



  private:

    int level_;

    std::string file_;

};

}

#endif
