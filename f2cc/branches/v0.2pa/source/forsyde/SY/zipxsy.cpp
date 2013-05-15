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

#include "zipxsy.h"
#include <typeinfo>

using namespace f2cc::Forsyde::SY;
using std::string;
using std::bad_cast;

Zipx::Zipx(const Id& id) throw()
        : Leaf(id) {}

Zipx::Zipx(const Forsyde::Id& id, Forsyde::Hierarchy hierarchy,
 		int cost) throw()
        : Leaf(id, hierarchy, string("sy"), cost) {}

Zipx::~Zipx() throw() {}

bool Zipx::operator==(const Leaf& rhs) const throw() {
    if (Leaf::operator==(rhs)) return false;

    try {
        dynamic_cast<const Zipx&>(rhs);
    }
    catch (bad_cast&) {
        return false;
    }
    return true;
}

string Zipx::type() const throw() {
    return "zipX";
}

void Zipx::moreChecks() throw(InvalidProcessException) {
    if (getOutPorts().size() != 1) {
        THROW_EXCEPTION(InvalidProcessException, string("Leaf \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\" must have exactly one (1) out port");
        //TODO:: Check port sizes
    }
}
