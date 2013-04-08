/*
 * fanoutright (c) 2011-2012 Gabriel Hjort Blindell <ghb@kth.se>
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

#include "cfunction.h"
#include "../tools/tools.h"
#include <vector>
#include <new>

using namespace f2cc;
using std::string;
using std::list;
using std::vector;
using std::bad_alloc;

CFunction::CFunction() throw() : name_(""), body_("") {}

CFunction::CFunction(const string& name, CDataType return_type,
                     const list<CVariable> input_parameters,
                     const string& body, const string& prefix)
        throw(InvalidFormatException, OutOfMemoryException)
        : name_(name), return_data_type_(return_type), body_(body),
          declaration_prefix_(prefix) {
    tools::trim(name_);
    if (name_.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"name\" must not be empty "
                        "string");
    }
    list<CVariable>::const_iterator it;
    for (it = input_parameters.begin(); it != input_parameters.end(); ++it) {
        try {
            CVariable* new_parameter = new CVariable(*it);
            input_parameters_.push_back(new_parameter);
        }
        catch (bad_alloc&) {
            THROW_EXCEPTION(OutOfMemoryException);
        }
    }
}

CFunction::CFunction(const CFunction& rhs) throw(OutOfMemoryException) {
    copy(rhs);
}

CFunction::~CFunction() throw() {
    destroyInputParameters();
}
        
CFunction& CFunction::operator=(const CFunction& rhs)
    throw(OutOfMemoryException) {
    if (this == &rhs) return *this;
    copy(rhs);
    return *this;
}

string CFunction::getString() const throw() {
    string str;
    if (declaration_prefix_.length() > 0) str += declaration_prefix_ + "\n";
    str += return_data_type_.getFunctionReturnDataTypeString() + " " + name_;
    str += "(";
    list<CVariable*>::const_iterator it;
    for (it = input_parameters_.begin(); it != input_parameters_.end(); ++it) {
        if (it != input_parameters_.begin()) str += ", ";
        str += (*it)->getInputParameterDeclarationString();
    }
    str += ") ";
    str += body_;
    return str;
}

bool CFunction::operator==(const CFunction& rhs) const throw() {
    return body_ == rhs.body_;
}

bool CFunction::operator!=(const CFunction& rhs) const throw() {
    return !operator==(rhs);
}

string CFunction::toString() const throw() {
    return getString();
}

CDataType* CFunction::getReturnDataType() throw() {
    return &return_data_type_;
}

string CFunction::getName() const throw() {
    return name_;
}

void CFunction::setName(const std::string& name)
    throw(InvalidArgumentException) {
    string trimmed_name(name);
    tools::trim(trimmed_name);
    if (trimmed_name.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"name\" must not be empty "
                        "string or consist solely of whitespace");
    }
    name_ = trimmed_name;
}

size_t CFunction::getNumInputParameters() const throw() {
    return input_parameters_.size();
}

list<CVariable*> CFunction::getInputParameters() throw() {
    return input_parameters_;
}

bool CFunction::addInputParameter(const CVariable& parameter)
    throw(OutOfMemoryException) {
    list<CVariable*>::iterator it;
    for (it = input_parameters_.begin(); it != input_parameters_.end(); ++it) {
        if (**it == parameter) return false;
    }
    try {
        CVariable* new_parameter = new CVariable(parameter);
        input_parameters_.push_back(new_parameter);
        return true;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool CFunction::deleteInputParameter(const CVariable& parameter) throw() {
    list<CVariable*>::iterator it;
    for (it = input_parameters_.begin(); it != input_parameters_.end(); ++it) {
        if (**it == parameter) {
            input_parameters_.erase(it);
            return true;
        }
    }
    return false;
}

string CFunction::getBody() const throw() {
    return body_;
}

void CFunction::setBody(const string& body) throw() {
    body_ = body;
}

string CFunction::getDeclarationPrefix() const throw() {
    return declaration_prefix_;
}

void CFunction::setDeclarationPrefix(const string& prefix) throw() {
    declaration_prefix_ = prefix;
}

void CFunction::destroyInputParameters() throw() {
    list<CVariable*>::iterator it;
    for (it = input_parameters_.begin(); it != input_parameters_.end(); ++it) {
        delete *it;
    }
    input_parameters_.clear();
}

void CFunction::copy(const CFunction& rhs) throw(OutOfMemoryException) {
    name_ = rhs.name_;
    return_data_type_ = rhs.return_data_type_;
    destroyInputParameters();
    // fanout input parameters
    list<CVariable*>::const_iterator it;
    for (it = rhs.input_parameters_.begin(); it != rhs.input_parameters_.end(); 
         ++it) {
        try {
            CVariable* new_parameter = new CVariable(**it);
            input_parameters_.push_back(new_parameter);
        }
        catch (bad_alloc&) {
            THROW_EXCEPTION(OutOfMemoryException);
        }
    }
    body_ = rhs.body_;
    declaration_prefix_ = rhs.declaration_prefix_;
}
