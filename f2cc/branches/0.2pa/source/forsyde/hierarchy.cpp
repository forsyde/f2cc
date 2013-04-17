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

#include "hierarchy.h"
#include <new>

using namespace f2cc::ForSyDe;
using std::string;
using std::list;

Hierarchy::Hierarchy(list<const Id> hierarchy) throw() :
		hierarchy_(hierarchy){}

Hierarchy::~Hierarchy() throw() {}

list<const Id>  Hierarchy::getHierarchy() throw(){
	return hierarchy_;
}

void Hierarchy::setHierarchy(list<const Id> hierarchy) throw(){
	hierarchy_.assign (hierarchy.begin(),hierarchy.end());
}

void Hierarchy::lowerLevel(const Id& id) throw(){
	hierarchy_.push_back(id);
}

void Hierarchy::raiseLevel() throw(){
	hierarchy_.erase(hierarchy_.end());
}

const Id* Hierarchy::getId() const throw(){
	return hierarchy_.back();
}

const Id* Hierarchy::getFirstParent() const throw(){
	list<const Id>::const_iterator it = hierarchy_.end();
	return *(it-1);
}

const Id* Hierarchy::getFirstChildAfter(const Id& id) const throw(){
	list<const Id>::const_iterator it = findId(id);
	return *(it+1);
}

Hierarchy::Relation Hierarchy::findRelation(Hierarchy compare_hierarchy) const throw(){
	if(compare_hierarchy.getFirstParent() == getFirstParent()) return Sibling;
	if(compare_hierarchy.getFirstParent() == getId()) return FirstChild;
	if(compare_hierarchy.getId() == getFirstParent()) return FirstParent;
	if(compare_hierarchy.findId(*getId())) return Child;
	if(compare_hierarchy.findId(*getFirstParent())) return SiblingsChild;
	if(findId(*compare_hierarchy.getId())) return Parent;
	return Other;
}

list<const Id>::const_iterator Hierarchy::findId(const Id& id) const throw(){
	list<const Id>::const_iterator it;
	for (it = hierarchy_.begin(); it != hierarchy_.end(); ++it) {
		if (*(*it) == id) {
			return it;
		}
	}
	return it; 	//None was found
}
