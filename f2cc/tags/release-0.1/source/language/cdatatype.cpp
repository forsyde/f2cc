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

#include "cdatatype.h"
#include "../tools/tools.h"

using namespace f2cc;
using std::string;

CDataType::CDataType() throw()
        : type_(VOID), is_array_(false), has_array_size_(false), array_size_(0),
          is_pointer_(false), is_const_(false) {}

CDataType::CDataType(CDataType::Type type, bool is_array,
                     bool has_array_size, size_t array_size, bool is_pointer,
                     bool is_const)
        throw(InvalidArgumentException)
        : type_(type), is_array_(is_array), has_array_size_(has_array_size),
          is_pointer_(false), is_const_(is_const) {
    if (is_array_ && has_array_size_) {
        checkArraySize(array_size);
        array_size_ = array_size;
    }

    if (type == VOID && is_array) {
        THROW_EXCEPTION(InvalidArgumentException, "void types cannot be an "
                        "array");
    }
    if (type == VOID && is_pointer) {
        THROW_EXCEPTION(InvalidArgumentException, "void types cannot be a "
                        "pointer");
    }
    if (type == VOID && is_const) {
        THROW_EXCEPTION(InvalidArgumentException, "void types cannot be const");
    }
}

CDataType::~CDataType() throw() {}

CDataType::Type CDataType::getType() const throw() {
    return type_;
}

bool CDataType::isArray() const throw() {
    return is_array_;
}

void CDataType::setIsArray(bool is_array) throw() {
    is_array_ = is_array;
    has_array_size_ = false;
}

bool CDataType::hasArraySize() const throw() {
    if (!is_array_) return true;
    return has_array_size_;
}

size_t CDataType::getArraySize() const throw() {
    if (is_array_) return array_size_;
    else           return 1;
}

void CDataType::setArraySize(size_t size)
    throw(InvalidArgumentException) {
    checkArraySize(size);
    if (size == 1) is_array_ = false;
    else           is_array_ = true;
    has_array_size_ = true;
    array_size_ = size;
}
            
bool CDataType::isPointer() const throw() {
    return is_pointer_;
}

void CDataType::setIsPointer(bool is_pointer) throw() {
    is_pointer_ = is_pointer;
}

bool CDataType::isConst() const throw() {
    return is_const_;
}

void CDataType::setIsConst(bool is_const) throw() {
    is_const_ = is_const;
}

string CDataType::getVariableDataTypeString() const throw() {
    string str;
    if (is_const_) str += "const ";
    str += typeToString(type_);
    if (is_pointer_) str += "*";
    return str;
}

string CDataType::getInputParameterDataTypeString() const throw() {
    string str;
    if (is_const_) str += "const ";
    str += typeToString(type_);
    if (is_array_) str += "*";
    if (is_pointer_) str += "*";
    return str;
}

string CDataType::getFunctionReturnDataTypeString() const throw() {
    return getInputParameterDataTypeString();
}

bool CDataType::operator==(const CDataType& rhs) const throw() {
    if (type_ != rhs.type_) return false;
    if (is_array_ != rhs.is_array_) return false;
    if (has_array_size_ != rhs.has_array_size_) return false;
    if (array_size_ != rhs.array_size_) return false;
    if (is_pointer_ != rhs.is_pointer_) return false;
    if (is_const_ != rhs.is_const_) return false;
    return true;
}

bool CDataType::operator!=(const CDataType& rhs) const throw() {
    return !operator==(rhs);
}

string CDataType::toString() const throw() {
    string str;
    if (is_const_) str += "const ";
    str += typeToString(type_);
    if (is_array_) {
        str += "[";
        if (has_array_size_) {
            str += tools::toString(array_size_);
        }
        else {
            str += "?";
        }
        str += "]";
    }
    if (is_pointer_) str += "*";
    return str;
}

CDataType::Type CDataType::stringToType(const string& str)
    throw(InvalidArgumentException) {
    if (str.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"str\" must not be an "
                        "empty string");
    }

    if (str == "char") return CHAR;
    else if (str == "unsigned char") return UNSIGNED_CHAR;
    else if (str == "short int" || str == "short") return SHORT_INT;
    else if (str == "unsigned short int" || str == "unsigned short") {
        return UNSIGNED_SHORT_INT;
    }
    else if (str == "int") return INT;
    else if (str == "unsigned int") return UNSIGNED_INT;
    else if (str == "long int" || str == "long") return LONG_INT;
    else if (str == "unsigned long int" || str == "unsigned long") {
        return UNSIGNED_LONG_INT;
    }
    else if (str == "float") return FLOAT;
    else if (str == "double") return DOUBLE;
    else if (str == "long double") return LONG_DOUBLE;
    else if (str == "void") return VOID;
    else {
        THROW_EXCEPTION(InvalidArgumentException, string("\"") + str + "\" is "
                        + "not a valid type");
    }
}

string CDataType::typeToString(CDataType::Type type) throw() {
    switch (type) {
        case CHAR: return "char";
        case UNSIGNED_CHAR: return "unsigned char";
        case SHORT_INT: return "short int";
        case UNSIGNED_SHORT_INT: return "unsigned short int";
        case INT: return "int";
        case UNSIGNED_INT: return "unsigned int";
        case LONG_INT: return "long int";
        case UNSIGNED_LONG_INT: return "unsigned long int";
        case FLOAT: return "float"; 
        case DOUBLE: return "double";
        case LONG_DOUBLE: return "long double";
        case VOID: return "void";
        default: return "?"; // Should never happen
    }
}

void CDataType::checkArraySize(size_t size) const
    throw(InvalidArgumentException) {
    if (size < 1) {
        THROW_EXCEPTION(InvalidArgumentException, "Array size must not be less "
                        "than 1");
    }
}
