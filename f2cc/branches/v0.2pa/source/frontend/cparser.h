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

#ifndef F2CC_SOURCE_FRONTEND_CPARSER_H_
#define F2CC_SOURCE_FRONTEND_CPARSER_H_

/**
 * @file
 * @author  George Ungureanu <ugeorge@kth.se>
 * @version 0.2
 *
 * @brief Defines the parser for ForSyDe-SystemC code.
 */

#include "frontend.h"
#include "../logger/logger.h"
#include "../language/cfunction.h"
#include "../language/cvariable.h"
#include "../language/cdatatype.h"
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

}

#endif //F2CC_SOURCE_FRONTEND_CPARSER_H_
