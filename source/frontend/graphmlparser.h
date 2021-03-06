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

#ifndef F2CC_SOURCE_FRONTEND_GRAPHMLPARSER_H_
#define F2CC_SOURCE_FRONTEND_GRAPHMLPARSER_H_

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
#include "../forsyde/leaf.h"
#include "../language/cfunction.h"
#include "../language/cdatatype.h"
#include "../ticpp/ticpp.h"
#include "../exceptions/filenotfoundexception.h"
#include "../exceptions/parseexception.h"
#include "../exceptions/invalidmodelexception.h"
#include "../exceptions/runtimeexception.h"
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
class GraphmlParser : public Frontend {
  public:
    /**
     * Creates a GraphML parser.
     *
     * @param logger
     *        Reference to the logger.
     */
    GraphmlParser(Logger& logger) throw();

    /**
     * Destroys this parser. The logger remains open.
     */
    ~GraphmlParser() throw();

  private:
    /**
     * @copydoc Frontend::createProcessNetwork(const std::string&)
     */
    virtual Forsyde::ProcessNetwork* createProcessNetwork(const std::string& file)
        throw(InvalidArgumentException, FileNotFoundException, IOException,
              ParseException, InvalidModelException, RuntimeException);

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
    ticpp::Element* findXmlGraphElement(ticpp::Document* xml)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

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
     */
    Forsyde::ProcessNetwork* generateProcessNetwork(ticpp::Element* xml)
    throw(InvalidArgumentException, ParseException, InvalidModelException,
          IOException, RuntimeException);

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
    void parseXmlNodes(ticpp::Element* xml, Forsyde::ProcessNetwork* processnetwork)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Parses the \c edge XML elements in a \c graph XML element and uses them
     * to connect the ports of the leafs in the processnetwork.
     *
     * @param xml
     *        \c graph XML element containing the \c edge XML elements.
     * @param processnetwork
     *        ProcessNetwork object.
     * @param copy_leafs
     *        Mapset which contains the \c Fanout leafs created during the
     *        parsing leaf.
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
    void parseXmlEdges(ticpp::Element* xml, Forsyde::ProcessNetwork* processnetwork,
                       std::map<Forsyde::Leaf::Port*, Forsyde::Leaf*>&
                       copy_leafs)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Verifies that all in and out ports of all leafs in the processnetwork are
     * connected, and only connected to leafs within the processnetwork.
     *
     * @param processnetwork
     *        ProcessNetwork to verify.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     * @throws ParseException
     *         When a port is not connected or connected to a leaf which does
     *         not belong to the processnetwork.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void verifyLeafConnections(Forsyde::ProcessNetwork* processnetwork)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Sets the out and in ports of the InPort and OutPort leafs,
     * respectively, as inputs and outputs of the processnetwork. The InPort and
     * Outport leafs are then removed.
     *
     * @param processnetwork
     *        ProcessNetwork.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void fixProcessNetworkInputsOutputs(Forsyde::ProcessNetwork* processnetwork)
        throw(InvalidArgumentException, IOException, RuntimeException);

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
    Forsyde::Leaf* generateLeaf(ticpp::Element* xml)
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
    std::string getId(ticpp::Element* xml)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Gets the name of an XML element. The name is specified in an XML
     * attribute named \c name. Any surrounding whitespace will be trimmed.
     * 
     * @param xml
     *        XML element.
     * @returns Its name.
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
    std::string getName(ticpp::Element* xml)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Gets the leaf type from a XML \c node element. The type is specified
     * in an XML \c data child element with attribute \c key="leaf_type".
     * Any surrounding whitespace will be trimmed.
     * 
     * @param xml
     *        \c node element.
     * @returns Its leaf type.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When a leaf type cannot be found.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    std::string getProcessType(ticpp::Element* xml)
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
    CFunction generateLeafFunction(ticpp::Element* xml)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Generates a leaf function argument from a string.
     *
     * @param str
     *        Function string.
     * @returns Leaf function argument.
     * @throws ParseException
     *         When the function string is invalid.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    CFunction generateLeafFunctionFromString(const std::string& str)
        throw(InvalidFormatException);

    /**
     * Gets the data type from a declaration of format "<type> <name>".
     *
     * @param str
     *        Declaration string.
     * @returns Data type.
     * @throws InvalidFormatException
     *         When the string is not of expected format.
     */
    CDataType getDataTypeFromDeclaration(const std::string& str) const
        throw(InvalidFormatException);

    /**
     * Gets the name from a declaration of format "<type> <name>".
     *
     * @param str
     *        Declaration string.
     * @returns Name.
     * @throws InvalidFormatException
     *         When the string is not of expected format.
     */
    std::string getNameFromDeclaration(const std::string& str) const
        throw(InvalidFormatException);

    /**
     * Gets the number of leafs from an XML \c node element. The number is
     * specified in an XML \c data child element with attribute \c
     * key="num_leafs".
     * 
     * @param xml
     *        \c node element.
     * @returns Number of leafs.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When number of leafs cannot be found.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    int getNumProcesses(ticpp::Element* xml)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Finds and sets the array sizes, where required, for a leaf function
     * argument.  The array sizes are extracted from XML "data" elements, with
     * argument "array_size", inside the XML "port" elements, which in turn are
     * children to the XML \c node element where the function was found.
     * 
     * @param function
     *        Leaf function argument.
     * @param xml
     *        \c node element.
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
    void findFunctionArraySizes(CFunction& function, ticpp::Element* xml)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Gets the array size from an XML "port" element. The array size is
     * located in an XML "data" element with argument "array_size".
     * 
     * @param xml
     *        XML element.
     * @returns Array size, if found; otherwise 0.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing, or when
     *         the array size is less than 1.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    size_t findArraySize(ticpp::Element* xml)
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
    std::string getInitialDelayValue(ticpp::Element* xml)
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
    Forsyde::Leaf::Port* generatePort(ticpp::Element* xml)
    throw(InvalidArgumentException, ParseException, IOException,
          RuntimeException);

    /**
     * Converts an XML \c edge element into a port connection.
     * 
     * @param xml
     *        \c edge element containing the connection.
     * @param processnetwork
     *        ProcessNetwork object to add the leaf to.
     * @param copy_leafs
     *        Mapset which contains the \c Fanout leafs created during the
     *        parsing leaf.
     * @throws InvalidArgumentException
     *         When \c xml or \c processnetwork is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void generateConnection(ticpp::Element* xml, Forsyde::ProcessNetwork* processnetwork,
                            std::map<Forsyde::Leaf::Port*,
                                     Forsyde::Leaf*>& copy_leafs)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Checks whether an ID specifies an in port.
     *
     * @param id
     *        Port id.
     * @returns \b true if the ID denotes an in port.
     */
    bool isInPort(const std::string& id) const throw();

    /**
     * Same as isInPort(const std::string&) but for out ports.
     *
     * @param id
     *        Port id.
     * @returns \b true if the ID denotes an out port.
     */
    bool isOutPort(const std::string& id) const throw();

    /**
     * Checks whether a port ID is valid.  All ports are expected to have the
     * following format:
     * @code
     * [<name>_]"in"|"out"[<numberic>]
     * @endcode
     * The last numeric part is optional.
     *
     * @param id
     *        Port id.
     * @param direction
     *        \c "in" or \c "out".
     * @returns \b true if the port ID is valid.
     */
    bool isValidPortId(const std::string& id, const std::string direction) const
        throw();

    /**
     * Checks that there is at most one InPort and exactly one OutPort leaf
     * within the processnetwork.
     * 
     * @param processnetwork
     *        ProcessNetwork to check.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     * @throws InvalidModelException
     *         When any of the checks fails.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void checkProcessNetworkMore(Forsyde::ProcessNetwork* processnetwork)
        throw(InvalidArgumentException, InvalidModelException, IOException,
              RuntimeException);

    /**
     * Sets the out and in ports of the InPort and OutPort leafs,
     * respectively, as inputs and outputs of the processnetwork. The InPort and
     * Outport leafs are then removed.
     * 
     * @param processnetwork
     *        ProcessNetwork to fix.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void postCheckFixes(Forsyde::ProcessNetwork* processnetwork)
        throw(InvalidArgumentException, IOException, RuntimeException);

  private:
    /**
     * File being parsed.
     */
    std::string file_;
};

}

#endif
