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

#ifndef F2CC_SOURCE_FRONTEND_XMLPARSER_H_
#define F2CC_SOURCE_FRONTEND_XMLPARSER_H_

/**
 * @file
 * @author  George Ungureanu <ugeorge@kth.se>
 * @version 0.2
 *
 * @brief Defines the XML-parsing frontend.
 */

#include "frontend.h"
#include "../logger/logger.h"
#include "../forsyde/id.h"
#include "../forsyde/processnetwork.h"
#include "../forsyde/process.h"
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
 * @brief A class for parsing a XML file into an internal ForSyDe process network
 *        representation.
 *
 * The \c XmlParser is a frontend to the \c f2cc which parses an XML
 * representation of a ForSyDe processnetwork and converts it into an internal
 * equivalent. Any unrecognized elements in the XML file will be ignored.
 *
 * The class uses <a href="http://code.google.com/p/ticpp/">TinyXML++</a> for
 * parsing the XML file.
 */
class XmlParser : public Frontend {
  public:
    /**
     * Creates an XML parser.
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
     * @copydoc Frontend::createProcessnetwork(const std::string&)
     */
    virtual ForSyDe::Processnetwork* createProcessnetwork(const std::string& file)
        throw(InvalidArgumentException, FileNotFoundException, IOException,
              ParseException, InvalidProcessnetworkException, RuntimeException);

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
     * Converts an \c graph XML element into an internal ForSyDe processnetwork. The
     * method makes no checks on whether the resultant processnetwork appears sane or
     * not.  It is the caller's responsibility to destroy the process network when it is
     * no longer used.
     *
     * @param xml
     *        \c graph XML element containing the process network.
     * @returns Internal ForSyDe processnetwork object.
     * @throws InvalidArgumentException
     *         When \c xml is \c NULL.
     * @throws ParseException
     *         When some necessary element or attribute is missing.
     * @throws InvalidProcessnetworkException
     *         When the process network is invalid (but was successfully parsed).
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    ForSyDe::Processnetwork* generateProcessnetwork(ticpp::Element* xml)
    throw(InvalidArgumentException, ParseException, InvalidProcessnetworkException,
          IOException, RuntimeException);


    /**
     * Parses the \c leaf_process XML elements in a \c process_network XML element and converts
     * them into corresponding processes, which are then added both to the process network
     * mapset and to their parent composite process mapset.
     *
     * @param xml
     *        \c graph XML element containing the \c node XML elements.
     * @param processnetwork
     *        Process network object to add the process to.
     * @param parent
     *        Composite object that includes this process
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
    void parseXmlLeafs(ticpp::Element* xml, ForSyDe::Processnetwork* processnetwork,
    		ForSyDe::Composite* parent)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Converts an XML \c node element into an internal ForSyDe leaf process of the
     * same type along with its ports and function argument, if any.
     *
     * @param xml
     *        \c node element containing the process.
     * @returns Internal process object.
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
    ForSyDe::Process* generateLeafProcess(ticpp::Element* xml)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

  private:

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
     * Generates a tabbed header to increase readability of the generated log file.
     *
     * @returns sting of tabs, depending on the parsing level.
     */
    std::string generateLogHeader() throw();

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
    ticpp::Element* findRootElement(ticpp::Document* xml)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Gets the attribute value for an XML element. Any surrounding whitespace will be trimmed.
     *
     * @param xml
     *        XML element.
     * @param attribute
     *        Attribute name.
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
    std::string getAttributeValue(ticpp::Element* xml, std::string attribute)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Checks whether the process constructor passed as string is of type "comb".
     *
     * @param constructor_name
     *        Process constructor name.
     *
     * @returns \c true if process constructor is "comb".
     */
    bool isComb(std::string constructor_name) throw();

  private:
    /**
     * File being parsed.
     */
    std::string file_;
    /**
     * Current depth of the parsed model.
     */
    int current_level_;
};

}

#endif
