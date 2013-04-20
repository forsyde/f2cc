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

#ifndef F2CC_SOURCE_LANGUAGE_CDATATYPE_H_
#define F2CC_SOURCE_LANGUAGE_CDATATYPE_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines class for representing data types in C.
 */

#include "../exceptions/invalidargumentexception.h"

namespace f2cc {

/**
 * @brief Class for representing data types in C.
 *
 * The \c CDataType class represents data types in C. The class is expected to
 * be used in variable and function declarations (more specifically return and
 * input parameter data types).
 */
class CDataType {
  public:
    /**
     * Valid types.
     */
    enum Type {
        CHAR,
        UNSIGNED_CHAR,

        /**
         * Synonym with \c short.
         */
        SHORT_INT,

        /**
         * Synonym with \c unsigned \c short.
         */
        UNSIGNED_SHORT_INT,
        INT,
        UNSIGNED_INT,

        /**
         * Synonym with \c long.
         */
        LONG_INT,

        /**
         * Synonym with \c unsigned \c long.
         */
        UNSIGNED_LONG_INT,
        FLOAT, 
        DOUBLE,
        LONG_DOUBLE,
        VOID
    };

  public:
    /**
     * Creates an empty data type. This is only to allow variable declarations
     * without having to provide an actual data type, and instead postpone it to
     * a later point.
     */
    CDataType() throw();

    /**
     * Creates a data type.
     *
     * @param type
     *        Type.
     * @param is_array
     *        Whether the data type is an array.
     * @param has_array_size
     *        Whether the array size is known.
     * @param array_size
     *        Array size (ignored if \c has_array_size is set to \c false).
     * @param is_pointer
     *        Whether the data type is a pointer. This is different than from
     *        an array since pointers don't need an array size.
     * @param is_const
     *        Whether the data type is an \c const.
     * @throws InvalidArgumentException
     *         When \c type is an empty string or invalid type, when the
     *         array size is less than 1, or when the type is \c VOID and
     *         any of \c is_array, \c is_pointer, or \c is_const is set to
     *         \c true.
     */
    CDataType(Type type, bool is_array, bool has_array_size,
              size_t array_size, bool is_pointer, bool is_const)
        throw(InvalidArgumentException);

    /**
     * Destroys this data type.
     */
    ~CDataType() throw();

    /**
     * Gets the type of this data type.
     *
     * @returns Type.
     */
    Type getType() const throw();

    /**
     * Gets the data type as a string which would be used as part of declaring a
     * C variable. This means that the returned string never contains a pointer
     * indicator, even if the data type is an array. See
     * getInputParameterDataTypeString() and getFunctionReturnDataTypeString()
     * for comparison.
     *
     * @returns Variable declaration data type.
     */
    std::string getVariableDataTypeString() const throw();

    /**
     * Gets the data type as a string which would be used as part of declaring
     * an input parameter to a C function. This means that the returned string
     * may contain a pointer indicator (for instance, if the data type is an
     * array). See getVariableDataTypeString() for comparison.
     *
     * @returns Input parameter declaration data type.
     */
    std::string getInputParameterDataTypeString() const throw();

    /**
     * Gets the data type as a string which would be used as part of declaring
     * the return type of a C function. This means that the returned string
     * may contain a pointer indicator (for instance, if the data type is an
     * array). See getVariableDataTypeString() for comparison.
     *
     * @returns Input parameter declaration data type.
     */
    std::string getFunctionReturnDataTypeString() const throw();

    /**
     * Checks whether this data type is an array.
     *
     * @returns \c true if this data type is an array.
     */
    bool isArray() const throw();

    /**
     * Sets whether this data type is an array or not. If set to an array, it
     * will not have an array size. If the data type is already an array, then
     * this method can be used to make the array size unknown.
     *
     * @param is_array
     *        Arrayness setting.
     */
    void setIsArray(bool is_array) throw();

    /**
     * Checks whether this data type has an array size. If the data type is not
     * an array, this always returns \c true.
     *
     * @returns \c true if it does or is not an array.
     */
    bool hasArraySize() const throw();

    /**
     * Gets the array size of this data type. If the data type is not
     * an array, 1 is returned. If no array size has been set, the
     * returned value is undefined.
     *
     * @returns Array size if the data type is an array and has an
     *          array size; if the data type is not an array, 1 is
     *          returned; otherwise undefined value.
     */
    size_t getArraySize() const throw();

    /**
     * Sets the array size of this data type. Setting the value to 1
     * causes the data type to \em not be an array. If the data type
     * was not previously an array, setting the array size with a value
     * larger than 1 causes the data type to automatically become an
     * array.
     *
     * @param size
     *        Array size.
     * @throws InvalidArgumentException
     *         When \c size is less than 1.
     */
    void setArraySize(size_t size) throw(InvalidArgumentException);
            
    /**
     * Checks whether this data type is a pointer.
     *
     * @returns \c true if this data type is a pointer.
     */
    bool isPointer() const throw();

    /**
     * Sets whether this data type is a pointer or not.
     *
     * @param is_pointer
     *        Pointer setting.
     */
    void setIsPointer(bool is_pointer) throw();

    /**
     * Checks whether this data type is \c const.
     *
     * @returns \c true if this data type is \c const.
     */
    bool isConst() const throw();

    /**
     * Sets whether this data type is \c const or not.
     *
     * @param is_const
     *        Constness setting.
     */
    void setIsConst(bool is_const) throw();

    /**
     * Checks for equality between this data type and another.
     *
     * @param rhs
     *        Data type to compare.
     * @returns \c true if both represent the same data type.
     */
    bool operator==(const CDataType& rhs) const throw();

    /**
     * Checks for inequality between this data type and another.
     *
     * @param rhs
     *        Data type to compare.
     * @returns \c true if they do not represent the same data type.
     */
    bool operator!=(const CDataType& rhs) const throw();

    /**
     * Converts this data type into a string representation.
     *
     * @returns String representation.
     */
    std::string toString() const throw();

    /**
     * Converts a string into the corresponding type. Valid strings are:
     *    - \c "char",
     *    - \c "unsigned char",
     *    - \c "short int" (or just \c "short"),
     *    - \c "unsigned short int" (or just \c "unsigned short"),
     *    - \c "int",
     *    - \c "unsigned int",
     *    - \c "long int" (or just \c "long"),
     *    - \c "unsigned long int" (or just \c "unsigned long"),
     *    - \c "float", 
     *    - \c "double",
     *    - \c "long double", and
     *    - \c "void".
     *
     * @param str
     *        String to convert.
     * @returns Type.
     * @throws InvalidArgumentException
     *         If the string is not valid.
     */
    static Type stringToType(const std::string& str)
        throw(InvalidArgumentException);

    /**
     * Converts a type into the corresponding string representation.
     *
     * @param type
     *        Type to convert.
     * @returns String representation.
     */
    static std::string typeToString(Type type) throw();

  private:
    /**
     * Checks that the array size is valid.
     *
     * @param size
     *        Array size to check.
     * @throws InvalidArgumentException
     *         When \c size is less than 1.
     */
    void checkArraySize(size_t size) const
        throw(InvalidArgumentException);

  private:
    /**
     * Type (e.g. int, long, float, etc.). Pointer indicator is never
     * part of this string.
     */
    Type type_;

    /**
     * Whether the data type is an array or not.
     */
    bool is_array_;

    /**
     * Whether the data type (if it is an array) has a size or not.
     */
    bool has_array_size_;

    /**
     * Size of the array, if the data type is an array.
     */
    size_t array_size_;

    /**
     * Whether the data type is a pointer or not.
     */
    bool is_pointer_;

    /**
     * Whether this data type is a \c const.
     */
    bool is_const_;
};

}

#endif
