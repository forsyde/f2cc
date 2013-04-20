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

#ifndef F2CC_SOURCE_LANGUAGE_CVARIABLE_H_
#define F2CC_SOURCE_LANGUAGE_CVARIABLE_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines class for representing data types in C.
 */

#include "cdatatype.h"
#include "../exceptions/invalidargumentexception.h"
#include "../exceptions/unknownarraysizeexception.h"

namespace f2cc {

/**
 * @brief Class for representing variables in C.
 *
 * The \c CVariable class represents variables in C. A variable consists of a
 * name and a data type. The reason for having this class is to simply handling
 * of arrays, in particular as declaration of arrays differ greatly in C
 * depending on whether they are defined as a stand-alone variable or as an
 * input parameter to a function.
 */
class CVariable {
  public:
    /**
     * Creates an empty variable.
     */
    CVariable() throw();

    /**
     * Creates a variable.
     *
     * @param name
     *        Variable name.
     * @param type
     *        Data type.
     * @throws InvalidArgumentException
     *         When \c name is an empty string.
     */
    CVariable(const std::string& name, const CDataType& type)
        throw(InvalidArgumentException);

    /**
     * Destroys this variable.
     */
    ~CVariable() throw();

    /**
     * Gets the data type of this variable.
     *
     * @returns Data type.
     */
    CDataType* getDataType() throw();

    /**
     * Gets a string which uses this variable's name and data type to declare a
     * local C variable. If the variable is an array, the code will include the
     * array size in the declaration (i.e., if declared in a function, the
     * entire array will be allocated on the stack). Note that the string does
     * not end with any special character like a semi-colon (';') or a comma
     * (',').
     *
     * @returns Variable declaration string.
     * @throws UnknownArraySizeException
     *         When trying to declare a variable for an array where its size is
     *         unknown.
     */
    std::string getLocalVariableDeclarationString() const
        throw(UnknownArraySizeException);

    /**
     * Gets a string which uses this variable's name and data type to declare a
     * dynamic C variable. If the variable is an array, the code will include
     * the array size in the declaration. For array variables the "constness",
     * if any, is removed in the assignment declaration, but present on the
     * variable declaration side. Note also that the string does not end with
     * any special character like a semi-colon (';') or a comma (',').
     *
     * @returns Variable declaration string.
     * @throws UnknownArraySizeException
     *         When trying to declare a variable for an array where its size is
     *         unknown.
     */
    std::string getDynamicVariableDeclarationString() const
        throw(UnknownArraySizeException);

    /**
     * Gets a string which uses this variable's name and data type to declare
     * an input parameter to a C function. Note that the string does not end
     * with any special character like a semi-colon (';') or a comma (',').
     *
     * @returns Input parameter declaration string.
     */
    std::string getInputParameterDeclarationString() const throw();

    /**
     * Gets a string which declares this variable as a pointer. Memory
     * allocation has to be done manually (i.e. this does not initialize the
     * pointer). Note that the string does not end with any special character
     * like a semi-colon (';') or a comma (',').
     *
     * @returns Dynamic variable declaration string.
     */
    std::string getPointerDeclarationString() const throw();

    /**
     * Gets a string which uses this variable's name as a usage reference.
     * The reference string is expected to be used in assignments and as
     * function input value.
     *
     * @returns Reference string.
     */
    std::string getReferenceString() const throw();

    /**
     * Checks for equality between this variable and another.
     *
     * @param rhs
     *        Variable to compare.
     * @returns \c true if the data types and names are equal.
     */
    bool operator==(const CVariable& rhs) const throw();

    /**
     * Checks for inequality between this variable and another.
     *
     * @param rhs
     *        Variable to compare.
     * @returns \c true if either the data types or names are not equal.
     */
    bool operator!=(const CVariable& rhs) const throw();

  private:
    /**
     * Variable name.
     */
    std::string name_;

    /**
     * Variable data type.
     */
    CDataType type_;

    /**
     * Specifies that this variable should always be declared locally. This
     * means that if this variable is an array, the entire array will be
     * allocated on the stack.
     */
    bool use_local_declaration_;
};

}

#endif
