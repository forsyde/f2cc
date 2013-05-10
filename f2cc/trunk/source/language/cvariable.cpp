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

#include "cvariable.h"
#include "../tools/tools.h"

using namespace f2cc;
using std::string;

CVariable::CVariable() throw() {}

CVariable::CVariable(const std::string& name, const CDataType& type)
        throw(InvalidArgumentException) : name_(name), type_(type) {}

CVariable::~CVariable() throw() {}

void CVariable::changeReferenceString(std::string name) throw(){
	name_ = name;
}

CDataType* CVariable::getDataType() throw() {
    return &type_;
}

string CVariable::getLocalVariableDeclarationString() const
    throw(UnknownArraySizeException) {
    string str;
    str += type_.getVariableDataTypeString() + " " + name_;
    if (type_.isArray()) {
        if (!type_.hasArraySize()) {
            THROW_EXCEPTION(UnknownArraySizeException,
                            string("Size not known for array variable \"")
                            + name_ + "\"");
        }
        str += string("[") + tools::toString(type_.getArraySize()) + "]";
    }
    return str;
}

string CVariable::getDynamicVariableDeclarationString() const
    throw(UnknownArraySizeException) {
    string str;
    CDataType type_no_const = type_;
    type_no_const.setIsConst(false);
    str += type_.getVariableDataTypeString() + "* " + name_ + " = new "
        + type_no_const.getVariableDataTypeString();
    if (type_.isArray()) {
        if (!type_.hasArraySize()) {
            THROW_EXCEPTION(UnknownArraySizeException,
                            string("Size not known for array variable \"")
                            + name_ + "\"");
        }
        str += string("[") + tools::toString(type_.getArraySize()) + "]";
    }
    return str;
}

string CVariable::getInputParameterDeclarationString() const throw() {
    return type_.getInputParameterDataTypeString() + " " + name_;
}

string CVariable::getPointerDeclarationString() const throw() {
    return type_.getVariableDataTypeString() + "* " + name_;
}

string CVariable::getReferenceString() const throw() {
    return name_;
}

bool CVariable::operator==(const CVariable& rhs) const throw() {
    if (name_ != rhs.name_) return false;
    if (type_ != rhs.type_) return false;
    return true;
}

bool CVariable::operator!=(const CVariable& rhs) const throw() {
    return !operator==(rhs);
}
