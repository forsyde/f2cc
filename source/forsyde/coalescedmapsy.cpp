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

#include "coalescedmapsy.h"
#include <typeinfo>
#include <new>

using namespace f2cc;
using namespace f2cc::Forsyde;
using std::string;
using std::list;
using std::bad_alloc;
using std::bad_cast;

CoalescedMapSY::CoalescedMapSY(const Id& id, const CFunction& function) 
        throw(OutOfMemoryException) : MapSY(id, function) {
    try {
        CFunction* new_function = new CFunction(function);
        functions_.push_back(new_function);
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

CoalescedMapSY::CoalescedMapSY(const Id& id,
                               const list<CFunction>& functions)
        throw(InvalidArgumentException, OutOfMemoryException)
        // MapSY requires a function, but can not be certain at this point that
        // functions is not empty, so we provide MapSY with a dummy function
        // (we will never access it anyway)
        : MapSY(id, CFunction()) {
    if (functions.size() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"functions\" must not be "
                        "an empty list");
    }
    try {
        list<CFunction>::const_iterator it;
        for (it = functions.begin(); it != functions.end(); ++it) {
            CFunction* new_function = new CFunction(*it);
            functions_.push_back(new_function);
        }
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

CoalescedMapSY::~CoalescedMapSY() throw() {
    list<CFunction*>::iterator it;
    for (it = functions_.begin(); it != functions_.end(); ++it) {
        delete *it;
    }
}

CFunction* CoalescedMapSY::getFunction() throw() {
    return functions_.front();
}

list<CFunction*> CoalescedMapSY::getFunctions() throw() {
    return functions_;
}

void CoalescedMapSY::insertFunctionFirst(const CFunction& function)
    throw(OutOfMemoryException) {
    try {
        CFunction* new_function = new CFunction(function);
        functions_.push_front(new_function);
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

void CoalescedMapSY::insertFunctionLast(const CFunction& function)
    throw(OutOfMemoryException) {
    try {
        CFunction* new_function = new CFunction(function);
        functions_.push_back(new_function);
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

bool CoalescedMapSY::operator==(const Process& rhs) const throw() {
    if (!Process::operator==(rhs)) return false;

    try {
        const CoalescedMapSY& other = dynamic_cast<const CoalescedMapSY&>(rhs);
        if (functions_.size() != other.functions_.size()) return false;
        list<CFunction*>::const_iterator it1;
        list<CFunction*>::const_iterator it2;
        for (it1 = functions_.begin(), it2 = other.functions_.begin();
             it1 != functions_.end(); ++it1, ++it2) {
            if (**it1 != **it2) return false;
        }
    }
    catch (bad_cast&) {
        return false;
    }
    return true;
}

string CoalescedMapSY::type() const throw() {
    return "CoalescedMapSY";
}

void CoalescedMapSY::moreChecks() throw(InvalidProcessException) {
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
    list<CFunction*>::const_iterator it;
    for (it = functions_.begin(); it != functions_.end(); ++it) {
        checkFunction(**it);
    }
}

string CoalescedMapSY::moreToString() const throw() {
    string str;
    list<CFunction*>::const_iterator it;
    for (it = functions_.begin(); it != functions_.end(); ++it) {
        if (it != functions_.begin()) str += ",\n";
        str += string("ProcessFunction: ") + (*it)->toString();
    }
    return str;
}
