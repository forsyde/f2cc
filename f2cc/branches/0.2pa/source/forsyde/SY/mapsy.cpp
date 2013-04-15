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

#include "mapsy.h"
#include <typeinfo>
#include <list>

using namespace f2cc;
using namespace f2cc::ForSyDe::SY;
using std::string;
using std::bad_cast;
using std::list;

Map::Map(const Id& id, const Id& parent, const CFunction& function, const string& moc) throw()
        : Process(id, parent, moc), function_(function) {}

Map::~Map() throw() {}

CFunction* Map::getFunction() throw() {
    return &function_;
}

bool Map::operator==(const Process& rhs) const throw() {
    if (!Process::operator==(rhs)) return false;

    try {
        const Map& other = dynamic_cast<const Map&>(rhs);
        if (function_ != other.function_) return false;
    }
    catch (bad_cast&) {
        return false;
    }
    return true;
}

string Map::type() const throw() {
    return "Map";
}

void Map::moreChecks() throw(InvalidProcessException) {
    if (getInPorts().size() < 1) {
        THROW_EXCEPTION(InvalidProcessException, string("Process \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\" must have at least one (1) in port");
    }
    if (getOutPorts().size() != 1) {
        THROW_EXCEPTION(InvalidProcessException, string("Process \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\" must have exactly one (1) out port");
    }
    checkFunction(function_, getNumInPorts());
}

string Map::moreToString() const throw() {
    return string("ProcessFunction: ") + function_.toString();
}

void Map::checkFunction(CFunction& function, size_t num_in_ports) const
    throw(InvalidProcessException) {
    if (function.getInputParameters().size() == num_in_ports) {
        if (function.getReturnDataType()->getFunctionReturnDataTypeString()
            == "void") {
            THROW_EXCEPTION(InvalidProcessException, string("Process \"")
                            + getId()->getString() + "\" of type \""
                            + type() + "\": function arguments with one input "
                            "parameter must return data (i.e. have return "
                            "data type other than \"void\")");
        }
        if (function.getReturnDataType()->isArray()) {
            THROW_EXCEPTION(InvalidProcessException, string("Process \"")
                            + getId()->getString() + "\" of type \""
                            + type() + "\": return type of function arguments "
                            "with one input parameter must not be an array");
        }
    }
    else if (function.getInputParameters().size() == num_in_ports + 1) {
        if (function.getReturnDataType()->getFunctionReturnDataTypeString()
            != "void") {
            THROW_EXCEPTION(InvalidProcessException, string("Process \"")
                            + getId()->getString() + "\" of type \""
                            + type() + "\": function arguments with two input "
                            "parameters must not return data (i.e. have return "
                            "data type \"void\")");
        }
    }
    else {
        THROW_EXCEPTION(InvalidProcessException, string("Process \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\" must have a function argument with "
                        "one or two input parameters");
    }

    size_t i;
    list<CVariable*> input_parameters = function.getInputParameters();
    list<CVariable*>::iterator it;
    for (i = 0, it = input_parameters.begin(); i < num_in_ports; ++i, ++it) {
        CVariable variable = **it;
        CDataType input_data_type = *variable.getDataType();
        if (input_data_type.isArray() && !input_data_type.isConst()) {
            THROW_EXCEPTION(InvalidProcessException, string("Process \"")
                            + getId()->getString() + "\" of type \""
                            + type() + "\": input parameter \""
                            + variable.getReferenceString() + "\"is a "
                            "reference or array but not declared const");
        }
    }
}
