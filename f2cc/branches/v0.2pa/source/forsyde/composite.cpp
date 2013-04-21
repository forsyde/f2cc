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

#include "composite.h"
#include "../tools/tools.h"
#include <new>
#include <map>
#include <vector>

using namespace f2cc::ForSyDe;
using std::string;
using std::list;
using std::bad_alloc;
using std::vector;

Composite::Composite(const Id& id) throw() : id_(id) {}

Composite::~Composite() throw() {
	destroyAllLeafs();
}

const Id* Composite::getId() const throw() {
    return &id_;
}


string Composite::toString() const throw() {
    string str;
    str += "{\n";
    str += "  Composite Leaf: ";
    str += getId()->getString();
    str += ",\n";
    str += " List of Leafs : ";
    str += LeafsToString(leafs_);
    str += ",\n";
    str += "}";

    return str;
}

string Composite::LeafsToString(std::map<const Id, Leaf*> leafs) const throw() {
    string str;
    str += "\n";
    std::map<const Id, Leaf*>::iterator it;
	for (it = leafs.begin(); it != leafs.end(); ++it) {
		str+= "ID = ";
		str += it->second->getId()->getString();
		str += "\n";
	}
    return str;
}


void Composite::check() throw(InvalidLeafException) {
	//TODO: implement checks
}
