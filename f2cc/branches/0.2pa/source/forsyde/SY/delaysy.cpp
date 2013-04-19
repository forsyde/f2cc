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

#include "delaysy.h"
#include "../../tools/tools.h"
#include <typeinfo>

using namespace f2cc::ForSyDe::SY;
using std::string;
using std::bad_cast;

delay::delay(const Id& id, const string& initial_value, const string& moc)
        throw(InvalidArgumentException)
        : Process(id, moc), initial_value_(initial_value) {
    if (initial_value_.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"initial_value\" must not "
                        "be empty string");
    }
}

delay::~delay() throw() {}

string delay::getInitialValue() throw() {
    return initial_value_;
}

bool delay::operator==(const Process& rhs) const throw() {
    if (!Process::operator==(rhs)) return false;

    try {
        const delay& other = dynamic_cast<const delay&>(rhs);
        if (initial_value_ != other.initial_value_) return false;
    }
    catch (bad_cast&) {
        return false;
    }
    return true;
}

string delay::type() const throw() {
    return "delay";
}

void delay::moreChecks() throw(InvalidProcessException) {
    if (getInPorts().size() != 1) {
        THROW_EXCEPTION(InvalidProcessException, string("Process \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\" must have exactly one (1) in port");
    }
    if (getOutPorts().size() != 1) {
        THROW_EXCEPTION(InvalidProcessException, string("Process \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\" must have exactly one (1) out port");
    }
}

string delay::moreToString() const throw() {
    return string("InitialdelayValue: ") + initial_value_;
}
