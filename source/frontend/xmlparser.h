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
#include "../language/cvariable.h"
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
 * @brief A class for parsing a XML file into an internal ForSyDe process network
 *        representation.
 *
 * The \c XmlParser is a frontend to the \c f2cc which parses a XML
 * representation of a ForSyDe process network and converts it into an internal
 * equivalent. Any unrecognized elements in the XML file will be ignored.
 *
 * The class uses <a href="http://code.google.com/p/ticpp/">TinyXML++</a> for
 * parsing the XML file.
 */
class XmlParser : public Frontend {
  public:
    /**
     * Creates a XML parser.
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
     * Converts an \c XML element into an internal \c Forsyde::Composite process. The
     * method makes no checks on whether the resultant composite process appears sane or
     * not.  It is the caller's responsibility to destroy the process network when it is
     * no longer used.
     *
     * @param xml
     *        \c XML element containing the \c Composite process.
     * @param processnetwork
     *        \c The \c Forsyde::ProcessNetwork that will contain this Composite.
     * @param id
     *        \c The \c Forsyde::Composite process ID.
     * @param hierarchy
     *        \c The \c Forsyde::Composite process hierarchy path.
     *
     * @returns The Composite process in the internal ForSyDe ProcessNetwork.
     *
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
     * Reads an \c .xml file and returns the \c XML document built after parsing.
     *
     * @param file
     *        File to be parsed.
     * @returns \c XML document.
     *
     * @throws InvalidArgumentException
     *         When \c file is \c NULL.
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
     * Parses the \c leaf XML elements in a \c process_network XML element and passes them
     * to generateLeaf(Forsyde::ProcessNetwork*,ticpp::Element*,Forsyde::Composite* )
     * which converts
     * them into corresponding leafs, which are then added to the \c Forsyde::ProcessNetwork.
     * mapset.
     *
     * @param xml
     *        \c process_network XML element containing the containing \c leaf_process XML
     *        elements.
     * @param processnetwork
     *        \c Forsyde::ProcessNetwork object to add the leaf to.
     * @param parent
     *        The parent \c Forsyde::Composite process object in the \c Forsyde::ProcessNetwork.
     *
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
     * Parses the \c composite_process XML elements in a \c process_network XML
     * element and converts
     * them into corresponding \c Forsyde::Composite processes, which are then
     * added to the \c Forsyde::ProcessNetwork mapset.
     *
     * This method invokes generateComposite(Forsyde::ProcessNetwork*, ticpp::Element*,Forsyde::Composite*)
     * which opens a new \c .xml file for parsing, for each composite process.
     *
     * @param xml
     *        \c process_network XML element containing the \c composite_process XML elements.
     * @param processnetwork
     *        \c Forsyde::ProcessNetwork object to add the composite to.
     * @param parent
     *        \c Forsyde::Composite object that acts as a parent for this one.
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
     * Parses the \c port XML elements in a \c process_network XML element and passes them
     * to generateIOPort(ticpp::Element*,Forsyde::Composite*) which converts
     * them into corresponding \c Forsyde::Composite::IOPort, which are then added to the parent
     * \c Forsyde::Composite port list.
     *
     * @param xml
     *        \c process_network XML element containing the \c port XML elements.
     * @param parent
     *        \c Forsyde::Composite object that contains this IOPort.
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
     * Parses the \c signal XML elements in a \c process_network XML element and passes them
     * to generateSignal(ticpp::Element*,Forsyde::Composite*) which generates connections between two
     * \c Forsyde::Process::Interface objects.
     *
     * @param xml
     *        \c process_network XML element containing the \c signnal XML elements.
     * @param parent
     *        \c Forsyde::Composite object that contains the processes that have to be
     *        connected.
     *
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
     * Generates a \c Forsyde::Leaf object from a \c leaf_process XML element.
     *
     * @param pn
     *        \c Forsyde::ProcessNetwork object to add the leaf to.
     * @param xml
     *        \c leaf_process XML element containing data for creating this object.
     * @param parent
     *        \c Forsyde::Composite object that acts as a parent for this one.
     *
     * @returns Internal leaf process object.
     *
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
     * Generates a \c Forsyde::Leaf object from a \c leaf_process XML element.
     *
     * @param pn
     *        \c Forsyde::ProcessNetwork object to add the leaf to.
     * @param xml
     *        \c leaf_process XML element containing data for creating this object.
     * @param parent
     *        \c Forsyde::Composite object that acts as a parent for this one.
     *
     * @returns Internal composite process object.
     *
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
     * Provides a link for the caller leaf process to a \c CFunction contained by the
     * process network.
     *
     * If this function does not exist, it invokes a \c CParser object and parses
     * the \c ForSyDe-SystemC file to extract the
     * process' C code. The \c Cparser returns a \c CFunction object which is added to the
     * process network and a link from the caller leaf process is provided.
     *
     * @param pn
     *        \c Forsyde::ProcessNetwork object to add the leaf to.
     * @param xml
     *        \c leaf_process XML element containing data for creating this object.
     * @param parent
     *        \c Forsyde::Composite object that acts as a parent for this one.
     *
     * @returns Pointer to a \c CFunction object in the process network.
     *
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
    CFunction* generateLeafFunction(ticpp::Element* xml, Forsyde::ProcessNetwork* pn,
    		Forsyde::Composite* parent)
        throw(InvalidArgumentException, ParseException, IOException,
              RuntimeException);

    /**
     * Generates a \c Forsyde::Leaf::Port object from a \c port XML element, and adds it
     * to the desired leaf process.
     *
     * @param xml
     *        \c port XML element containing data for creating this object.
     * @param parent
     *        \c Forsyde::Leaf object that should contain this port.
     *
     *
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
     * Generates a \c Forsyde::Composite::IOPort object from a \c port XML element,
     * and adds it to the desired leaf process.
     *
     * @param xml
     *        \c port XML element containing data for creating this object.
     * @param parent
     *        \c Forsyde::Composite object that should contain this io-port.
     *
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
     * Gathers the information for generating a connection between two ForSyDe interfaces.
     *
     * @param xml
     *        \c signal XML element containing data for creating the connection.
     * @param parent
     *        \c Forsyde::Parent object that should contain the processes that are to be
     *        connected.
     *
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
     * Generates a connection between two ForSyDe interfaces.
     *
     * \b WARNING: it takes care whether the source process is a fanout and generates
     * a new port accordingly. It does not add a fanout process, though, like in v0.1. It
     * is the designer's responsibility to provide fanouts when a signal is split.
     *
     * @todo: Implement a method that automatically generates fanout processes when needed.
     *
     * @param source_port
     *        Source interface.
     * @param target_port
     *        Target interface.
     *
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
     * Translates a complex \c type string attribute into a ANSI C-compatible \c CDataType
     * object.
     *
     * \b OBS: the type attribute is extracted from the ForSyDe model exactly as the
     * designer defined it. One should take care that the type deffinitions correspond
     * to the ones in the f2cc tool.
     *
     * @param port_datatype
     *        \c complex ForSyDe data type string, as defined in the ForSyDe model.
     * @param port_size
     *        \c Size of the data type, as extracted by ForSyDe instrospection.
     *
     * @returns The \c CDataType object created.
     *
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
     * Links a \c Forsyde::Leaf::Port to a \c CVariable object contained by the leaf
     * process' function. Only \c Fotsyde::SY::Comb processes have functions.
     *
     * @param comb
     *        Comb process that contains the function.
     * @param direction
     *        the direction of the port.
     * @param name
     *        the name of the port.
     *
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
     * Locates the \c process_nertwork XML element in the XML document.
     *
     * @param xml
     *        XML document.
     * @param file
     *        \c .xml file.
     *
     * @returns \c process_network element.
     *
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
     * @returns List of elements with that name.
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
     * Gets a particular element which is an unique immediate child to an XML
     * object.
     *
     * If is more than one element with the same name, an exception is
     * thrown.
     *
     * If none was found, an empty pointer is returned.
     *
     * @param xml
     *        XML object to search.
     * @param name
     *        Name of the elements to search for.
     * @returns Searched element.
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
     * not included in the ForSyDe-XML specification, but also data which actually
     * is part of the specification but not used by the synthesis component.
     *
     * Each pruned data which belongs to the  ForSyDe-XML specification generates an
     * entry of type \c Logger::LogLevel::INFO in the log file. Each pruned data
     * which does not belongs to the ForSyDe-XML specification generates an entry of
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
     * Gets the attribute of an XML element. The attribute is passed as a string
     * argument. Any surrounding whitespace will be trimmed.
     *
     * @param xml
     *        XML element.
     * @param name
     *        Attribute name.
     * @returns Attribute value.
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
     * Gets the initial delay value from a XML \c leaf_process element of type \c delay.
     *
     * @param xml
     *        \c process_constructor element.
     * @param parent
     *        Parent delay process object.
     *
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

    /**
     * Logger indentation level. It describes the level of the parsed XML file.
     */
    int level_;

    /**
     * The ForSyDe-SystemC code file being parsed.
     */
    std::string file_;

  private:

    /**
     * @brief A class for parsing a ForSyDe-SystemC file into a pure C code.
     *
     * The \c CParser is a text parser that identifies ForSyDe-SystemC annotations and
     * complex data types and converts them into C-compatible code.
     */
    class CParser {
      public:
        /**
         * Creates a C parser.
         *
         * @param logger
         *        Reference to the logger.
         * @param indent
         *        Indentation level corresponding to the current XML file parsed.
         */
    	CParser(Logger& logger, int indent) throw();

        /**
         * Destroys this parser. The logger remains open.
         */
        ~CParser() throw();

       /**
    	* Converts a ForSyDe-SystemC function from a \c ".hpp" function file into a
    	* \c CFunction object. This object contains pure C code and it is compatible with
    	* f2cc v0.1.
    	*
    	* @param file
    	*        the function file that has to be parsed.
    	* @param name
    	*        the name of the function.
    	* @returns CFunction object containing the parsed C code.
    	* @throws InvalidArgumentException
    	*         When either \c file or \c name is \c NULL.
    	* @throws ParseException
    	*         When some necessary element is missing.
    	* @throws IOException
    	*         When access to the log file fails.
    	* @throws RuntimeException
    	*         When something unexpected occurs. This is most likely due to a
    	*         bug.
    	*/
        CFunction* parseCFunction(const std::string& file, const std::string& name)
            throw(InvalidArgumentException, IOException, ParseException, RuntimeException);

      private:

       /**
    	* Parses the function declaration. It identifies the declaration section; identifies
    	* individual variables as input or output parameters; identifies and converts complex
    	* STL data types into basic ANSI C data types. Based on this information, it builds
    	* \c CVariable objects and adds them to the function.
    	*
    	* @param function
    	*        the function that will contain the new CVariables.
    	*
    	* @throws InvalidArgumentException
    	*         When \c function is \c NULL.
    	* @throws ParseException
    	*         When some necessary element or attribute is missing.
    	*
    	*/
        void parseDeclaration(CFunction* function)
            throw(InvalidArgumentException, ParseException);

       /**
    	* Extracts the function body from the ForSyDe-SystemC function code between:
    	* @code
    	* #pragma ForSyDe begin <function_name>
    	* ...
    	* #pragma ForSyDe end
    	* @endcode
    	* It is assumed that the code is written in a pure ANSI C format, since no modifications
    	* are made to this part.
    	*
    	* @param function
    	*        the function that will contain the new body.
    	*
    	* @throws InvalidArgumentException
    	*         When \c function is \c NULL.
    	* @throws ParseException
    	*         When some necessary element or attribute is missing.
    	*/
        void extractBody(CFunction* function)
            throw(InvalidArgumentException, IOException);

       /**
    	* Associates function parameters which are wrapped into ForSyDe signal data types, with
    	* the unwrapped C variables inside the ForSyDe pragmas. Based on this associations, a
    	* recursive search is made and the function variables are renamed to match those in
    	* the function body.
    	*
    	* @param function
    	*        the function analyzed.
    	* @throws InvalidArgumentException
    	*         When \c function is \c NULL.
    	* @throws ParseException
    	*         When some necessary element or attribute is missing.
    	*/
        void renameWrappedVariables(CFunction* function)
            throw(InvalidArgumentException, ParseException);

        /**
    	* Creates a CVariable object from an analysis string having the format
    	* @code
    	* <base_data_type>[*&] <variable_name>
    	* @endcode
    	* and adds it to the CFunction.
    	*
    	* @param function
    	*        the function that contains the variables
    	* @param analysis_string
    	*        the string for analysis
    	* @param is_output
    	*        determines whether it is an input or output parameter
    	*
    	* @throws InvalidArgumentException
    	*         When \c function is \c NULL.
    	* @throws ParseException
    	*         When some necessary element or attribute is missing.
    	* @throws OutOfMemoryException
    	*         When the the variable could not be added due to memory shortage.
    	*/
        void createFunctionParameter(CFunction* function, std::string analysis_string,
        		bool is_output) throw(InvalidArgumentException, ParseException,
        				OutOfMemoryException);

        /**
    	* Gets the base data type inside a \c std::array template, and passes it to the
    	* caller.
    	*
    	* @param analysis_string
    	*        the string for analysis.
    	* @param data_type
    	*        pointer to the data type string which has to be filled by this function.
    	*
    	* @returns Position in the string where the template ends.
    	*
    	* @throws InvalidArgumentException
    	*         When \c xml is \c NULL.
    	* @throws ParseException
    	*         When some necessary element or attribute is missing.
    	*/
        unsigned getArrayDataType(const std::string& analysis_string,
        		std::string* data_type) throw(InvalidArgumentException, ParseException);

        /**
    	* Gets the base data type inside a C++ template, and passes it to the
    	* caller.
    	*
    	* @param analysis_string
    	*        the string for analysis.
    	* @param data_type
    	*        pointer to the data type string which has to be filled by this function.
    	*
    	* @returns Position in the string where the template ends.
    	*
    	* @throws InvalidArgumentException
    	*         When \c xml is \c NULL.
    	* @throws ParseException
    	*         When some necessary element or attribute is missing.
    	*/
        unsigned getTemplateBaseDataType(const std::string& analysis_string,
        		std::string* data_type) throw(InvalidArgumentException, ParseException);

      private:

        /**
         * Logger indentation level. It describes the level of the parsed XML file.
         */
        int level_;

        /**
         * The ForSyDe-SystemC code file being parsed.
         */
        std::string file_;

        /**
         * Local storage of the file's text.
         */
        std::string cdata_;

        /**
         * Logger.
         */
        Logger& logger_;

    };

};



}

#endif
