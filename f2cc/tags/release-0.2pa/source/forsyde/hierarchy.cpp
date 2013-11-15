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

#include "hierarchy.h"
#include <new>

using namespace f2cc::Forsyde;
using std::string;
using std::list;


Hierarchy::Hierarchy() throw() {
	Id* id = new Id("");
	hierarchy_.push_back(id);
}

Hierarchy::Hierarchy(list<Id*> hierarchy) throw() :
		hierarchy_(hierarchy){}

Hierarchy::~Hierarchy() throw() {}

list<Id*>  Hierarchy::getHierarchy() throw(){
	return hierarchy_;
}

void Hierarchy::setHierarchy(list<Id*> hierarchy) throw(){
	hierarchy_.assign (hierarchy.begin(),hierarchy.end());
}

void Hierarchy::lowerLevel(const Id& id) throw(){
	Id* id_local = new Id(id);
	hierarchy_.push_back(id_local);
}

void Hierarchy::raiseLevel() throw(){
	hierarchy_.erase(--(hierarchy_.end()));
}

const Id* Hierarchy::getId() const throw(){
	return hierarchy_.back();
}

const Id* Hierarchy::getFirstParent() const throw(){
	list<Id*>::const_reverse_iterator it = hierarchy_.rbegin();
	return *(++it);
}

const Id* Hierarchy::getFirstChildAfter(const Id& id) const throw(){
	list<Id*>::const_iterator it = findId(id);
	return *(++it);
}

Hierarchy::Relation Hierarchy::findRelation(Hierarchy compare_hierarchy) const throw(){
	if(compare_hierarchy.getFirstParent() == getFirstParent()) return Sibling;
	if(compare_hierarchy.getFirstParent() == getId()) return FirstChild;
	if(compare_hierarchy.getId() == getFirstParent()) return FirstParent;
	if(compare_hierarchy.findId(*getId()) != hierarchy_.end()) return Child;
	if(compare_hierarchy.findId(*getFirstParent()) != hierarchy_.end())  return SiblingsChild;
	if(findId(*compare_hierarchy.getId()) != hierarchy_.end()) return Parent;
	return Other;
}

string Hierarchy::hierarchyToString() const throw(){
	return toString(hierarchy_);
}

list<Id*>::const_iterator Hierarchy::findId(const Id& id) const throw(){
	list<Id*>::const_iterator it;
	for (it = hierarchy_.begin(); it != hierarchy_.end(); ++it) {
		if (*(*it) == id) {
			return it;
		}
	}
	return it; 	//None was found
}

string Hierarchy::toString(list<Id*> ids) const throw() {
    string str;
    if (ids.size() > 0) {
        bool first = true;
        for (list<Id*>::const_iterator it = ids.begin(); it != ids.end(); ++it) {
            if (!first) {
                str += " <- ";
            }
            else {
                first = false;
            }

            Id* id = *it;
            str += id->getString();
        }
        str += "\n ";
    }
    return str;
}
