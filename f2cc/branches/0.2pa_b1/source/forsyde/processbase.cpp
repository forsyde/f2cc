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

#include "processbase.h"
#include "../tools/tools.h"
#include <new>
#include <vector>

using namespace f2cc::ForSyDe;
using std::string;
using std::list;
using std::bad_alloc;
using std::vector;
using std::pair;


ProcessBase::ProcessBase(const Id& id) throw()
		: id_(id) {
	hierarchy_.lowerLevel(id_);
}

ProcessBase::~ProcessBase() throw() {
}

const Id* ProcessBase::getId() const throw() {
    return hierarchy_.getId();
}

Hierarchy ProcessBase::getHierarchy() const throw() {
    return hierarchy_;
}

void ProcessBase::setHierarchy(Hierarchy hierarchy) throw() {
	hierarchy_.setHierarchy(hierarchy.getHierarchy());
    hierarchy_.lowerLevel(id_);
}

Hierarchy::Relation ProcessBase::findRelation(const ProcessBase* rhs) const throw(){
	return hierarchy_.findRelation(rhs->hierarchy_);
}

void ProcessBase::check() throw(InvalidProcessException) {
    moreChecks();
}

ProcessBase::PortBase::PortBase(const Id& id) throw()
        : id_(id), process_(NULL) {}

ProcessBase::PortBase::PortBase(const Id& id, ProcessBase* process) throw()
        : id_(id), process_(process) {}

ProcessBase::PortBase::~PortBase() throw() {
}


const Id* ProcessBase::PortBase::getId() const throw() {
    return &id_;
}
