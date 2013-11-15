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

#ifndef F2CC_SOURCE_LANGUAGE_CFUNCTION_H_
#define F2CC_SOURCE_LANGUAGE_CFUNCTION_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines class for representing functions in C.
 */

#include "cvariable.h"
#include "cdatatype.h"
#include "../exceptions/invalidargumentexception.h"
#include "../exceptions/invalidformatexception.h"
#include "../exceptions/outofmemoryexception.h"
#include <list>

namespace f2cc {

/**
 * @brief Class for representing data types in C.
 *
 * The \c CFunction class declares a function in C. It separates the and manages
 * the function into distinct parts - return data type, function name,
 * input parameters, and body.
 */
class CFunction {
  public:
    /**
     * Creates an unnamed function, with no return type, input parameters nor
     * body.
     */
    CFunction() throw();

    /**
     * Creates an unnamed function, with no return type, input parameters nor
     * body.
     *
     * @param name
     *        Function name.
     * @param file
     *        \c .hpp file that initially contained the ForSyDe-SystemC function code.
     */
    CFunction(const std::string& name, const std::string& file) throw();

    /**
     * Creates a function.
     *
     * @param name
     *        Function name.
     * @param return_type
     *        Return data type.
     * @param input_parameters
     *        List of input parameters.
     * @param body
     *        Function body.
     * @param prefix
     *        Prefix to add before the declaration of the entire function (e.g.,
     *        in CUDA C, \c __global__ needs to be prefixed before the
     *        declaration of CUDA kernel functions).
     * @throws InvalidFormatException
     *         When \c name is an empty string.
     * @throws OutOfMemoryException
     *         When the function could not be created due to memory shortage.
     */
    CFunction(const std::string& name, CDataType return_type,
              const std::list<CVariable> input_parameters,
              const std::string& body, const std::string& prefix
              = std::string(""))
        throw(InvalidFormatException, OutOfMemoryException);

    /**
     * Creates a function.
     *
     * @param name
     *        Function name.
     * @param output_parameters
     *        List of output parameters.
     * @param input_parameters
     *        List of input parameters.
     * @param body
     *        Function body.
     * @param prefix
     *        Prefix to add before the declaration of the entire function (e.g.,
     *        in CUDA C, \c __global__ needs to be prefixed before the
     *        declaration of CUDA kernel functions).
     * @throws InvalidFormatException
     *         When \c name is an empty string.
     * @throws OutOfMemoryException
     *         When the function could not be created due to memory shortage.
     */
    CFunction(const std::string& name, const std::list<CVariable> output_parameters,
              const std::list<CVariable> input_parameters,
              const std::string& body, const std::string& prefix
              = std::string(""))
        throw(InvalidFormatException, OutOfMemoryException);

    /**
     * Creates a copy of another function.
     * 
     * @param rhs
     *        Function to copy.
     * @throws OutOfMemoryException
     *         When the function could not be created due to memory shortage.
     */
    CFunction(const CFunction& rhs) throw(OutOfMemoryException);

    /**
     * Destroys this function.
     */
    virtual ~CFunction() throw();

    /**
     * Copies another function.
     * 
     * @param rhs
     *        Function to copy.
     * @returns This function.
     * @throws OutOfMemoryException
     *         When the function could not be created due to memory shortage.
     */
    CFunction& operator=(const CFunction& rhs) throw(OutOfMemoryException);
        
    /**
     * Gets the function as a string.
     *
     * @returns Function as a string.
     */
    std::string getString() const throw();

    /**
     * Gets the function as a string.
     *
     * @returns Function as a string.
     */
    std::string getStringNew() const throw();

    /**
     * Gets the function as a string.
     *
     * @returns Function as a string.
     */
    std::string getStringNewRoot() const throw();

    /**
     * Gets the name of this function.
     *
     * @returns Function name.
     */
    std::string getName() const throw();

    /**
     * Sets a new name to this function.
     *
     * @param name
     *        New function name.
     * @throws InvalidArgumentException
     *         When \c name is an empty string or consists solely of 
     *         whitespace.
     */
    void setName(const std::string& name) throw(InvalidArgumentException);

    /**
     * Gets the return data type for this function.
     *
     * @returns Data type.
     */
    CDataType* getReturnDataType() throw();

    /**
     * Gets the number of input parameters to this function.
     *
     * @returns Number of input parameters.
     */
    size_t getNumInputParameters() const throw();

    /**
     * Adds an input parameter to this function. The new parameter will be added
     * as the last parameter to the function. If the parameter already exists
     * then it will not be added and \b false is returned.
     *
     * @param parameter
     *        Variable to add as input parameter.
     * @returns \b true if the parameter was successfully added.
     * @throws OutOfMemoryException
     *         When the parameter fails to be added due to memory shortage.
     */
    bool addInputParameter(const CVariable& parameter)
        throw(OutOfMemoryException);

    /**
     * Adds an input parameter to this function. The new parameter will be added
     * as the last parameter to the function. If the parameter already exists
     * then it will not be added and \b false is returned.
     *
     * @param parameter
     *        Variable to add as input parameter.
     * @returns \b true if the parameter was successfully added.
     * @throws OutOfMemoryException
     *         When the parameter fails to be added due to memory shortage.
     */
    bool addOutputParameter(const CVariable& parameter)
        throw(OutOfMemoryException);

    /**
     * Deletes an input parameter from this function.
     *
     * @param parameter
     *        Variable to delete from the input parameters.
     * @returns \b true if such a parameter was found and successfully deleted.
     */
    bool deleteInputParameter(const CVariable& parameter) throw();

    /**
     * Gets a list of the input parameters to this function. The list will be
     * in the order as they were added to the function.
     *
     * @returns List of input parameters.
     */
    std::list<CVariable*> getInputParameters() throw();


    /**
     * Sets the output parameters to this function. This replaces the existing ones.
     *
     * @param parameters
     *        List of variables to add as output parameter.
     */
    void setOutputParameters(std::list<CVariable*> parameters) throw();


    /**
     * Gets a list of the input parameters to this function. The list will be
     * in the order as they were added to the function.
     *
     * @returns List of input parameters.
     */
    std::list<CVariable*> getOutputParameters() throw();


    /**
     * Adds an input parameter to this function. The new parameter will be added
     * as the last parameter to the function. If the parameter already exists
     * then it will not be added and \b false is returned.
     *
     * @param parameter
     *        Variable to add as input parameter.
     * @returns \b true if the parameter was successfully added.
     * @throws OutOfMemoryException
     *         When the parameter fails to be added due to memory shortage.
     */
    bool setOutputParameter(const CVariable& parameter) throw();


    /**
     * Gets a list of the output parameters to this function. The list will be
     * in the order as they were added to the function.
     *
     * @returns List of output parameters.
     */
    CVariable* getOutputParameter() throw();

    /**
     * Gets the body of this function. Initial block declaration is
     * included if it was specified in the function string.
     *
     * @returns C code.
     */
    std::string getBody() const throw();

    /**
     * Sets the body of this function.
     *
     * @param body
     *        C code.
     */
    void setBody(const std::string& body) throw();

    /**
     * Gets the declaration prefix of this function. E.g., in CUDA C,
     * \c __global__ needs to be prefixed before the declaration of CUDA kernel
     * functions).
     *
     * @returns Function declaration prefix string.
     */
    std::string getDeclarationPrefix() const throw();

    /**
     * Sets the declaration prefix of this function.
     *
     * @param prefix
     *        Function declaration prefix.
     * @see getDeclarationPrefix()
     */
    void setDeclarationPrefix(const std::string& prefix) throw();

    /**
     * Checks for equality between this function and another.
     *
     * @param rhs
     *        Function to compare.
     * @returns \b true if the bodies of both funtions are exactly identical,
     *          character by character.
     */
    bool operator==(const CFunction& rhs) const throw();

    /**
     * Checks for inequality between this function and another.
     *
     * @param rhs
     *        Function to compare.
     * @returns \b true if the string representation of both are not
     *          identical.
     */
    bool operator!=(const CFunction& rhs) const throw();

    /**
     * Converts this function into a string representation. This is the same as
     * calling getString().
     *
     * @returns String representation.
     */
    std::string toString() const throw();

  private:
    /**
     * Clears and destroys all input parameters to this function.
     */
    void destroyInputParameters() throw();

    /**
     * Copies another function.
     * 
     * @param rhs
     *        Function to copy.
     * @throws OutOfMemoryException
     *         When the function could not be copied due to memory shortage.
     */
    void copy(const CFunction& rhs) throw(OutOfMemoryException);

  private:
    /**
     * Function name.
     */
    std::string name_;

    /**
     * Function's filename.
     */
    std::string file_;

    /**
     * Return value data type.
     */
    CDataType return_data_type_;

    /**
     * Input parameters.
     */
    std::list<CVariable*> input_parameters_;

    /**
     * Input parameters.
     */
    std::list<CVariable*> output_parameters_;

    /**
     * In case the parent process is a comb, this holds a pointer to its output.
     */
    CVariable* comb_output_;

    /**
     * Function body.
     */
    std::string body_;

    /**
     * Prefix to add before the declaration of the entire function.
     */
    std::string declaration_prefix_;
};

}

#endif
