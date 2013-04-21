/*
 * Copyright (c) 2011-2012 Gabriel Hjort Blindell <ghb@kth.se>
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

using namespace f2cc;
using namespace f2cc::ForSyDe;
using std::string;
using std::bad_cast;

Map::Map(const Id& id, const CFunction& function) throw()
        : Leaf(id), function_(function) {}

Map::~Map() throw() {}

CFunction* Map::getFunction() throw() {
    return &function_;
}

bool Map::operator==(const Leaf& rhs) const throw() {
    if (!Leaf::operator==(rhs)) return false;

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

void Map::moreChecks() throw(InvalidLeafException) {
    if (getInPorts().size() != 1) {
        THROW_EXCEPTION(InvalidLeafException, string("Leaf \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\" must have exactly one (1) in interface");
    }
    if (getOutPorts().size() != 1) {
        THROW_EXCEPTION(InvalidLeafException, string("Leaf \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\" must have exactly one (1) out interface");
    }
    checkFunction(function_);
}

string Map::moreToString() const throw() {
    return string("LeafFunction: ") + function_.toString();
}

void Map::checkFunction(CFunction& function) const
    throw(InvalidLeafException) {
    if (function.getInputParameters().size() == 1) {
        if (function.getReturnDataType()->getFunctionReturnDataTypeString()
            == "void") {
            THROW_EXCEPTION(InvalidLeafException, string("Leaf \"")
                            + getId()->getString() + "\" of type \""
                            + type() + "\": function arguments with one input "
                            "parameter must return data (i.e. have return "
                            "data type other than \"void\")");
        }
        if (function.getReturnDataType()->isArray()) {
            THROW_EXCEPTION(InvalidLeafException, string("Leaf \"")
                            + getId()->getString() + "\" of type \""
                            + type() + "\": return type of function arguments "
                            "with one input parameter must not be an array");
        }
    }
    else if (function.getInputParameters().size() == 2) {
        if (function.getReturnDataType()->getFunctionReturnDataTypeString()
            != "void") {
            THROW_EXCEPTION(InvalidLeafException, string("Leaf \"")
                            + getId()->getString() + "\" of type \""
                            + type() + "\": function arguments with two input "
                            "parameters must not return data (i.e. have return "
                            "data type \"void\")");
        }
    }
    else {
        THROW_EXCEPTION(InvalidLeafException, string("Leaf \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\" must have a function argument with "
                        "one or two input parameters");
    }

    CDataType first_input_data_type =
        *function.getInputParameters().front()->getDataType();
    if (first_input_data_type.isArray() && !first_input_data_type.isConst()) {
        THROW_EXCEPTION(InvalidLeafException, string("Leaf \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\": first input parameter is a "
                        "reference or array but not declared const");
    }
}
