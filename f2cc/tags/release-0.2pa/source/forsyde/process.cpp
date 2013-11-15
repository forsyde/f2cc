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

#include "process.h"
#include "../tools/tools.h"
#include <new>
#include <vector>

using namespace f2cc::Forsyde;
using std::string;
using std::list;
using std::bad_alloc;
using std::vector;

Process::Process(const Id& id) throw()
		: hierarchy_(Hierarchy()) {
	hierarchy_.lowerLevel(id);
}

Process::Process(const Id& id, Hierarchy& hierarchy, bool mapped_to_cuda, int cost) throw() :
		hierarchy_(hierarchy), mapped_to_cuda_(mapped_to_cuda), cost_(cost), stream_number_(0) {
	hierarchy_.lowerLevel(id);
}

Process::~Process() throw() {
}

const Id* Process::getId() const throw() {
    return hierarchy_.getId();
}

Hierarchy Process::getHierarchy() const throw() {
    return hierarchy_;
}

void Process::setHierarchy(Hierarchy hierarchy) throw() {
	Id id_copy = Id(getId()->getString());
	hierarchy_.setHierarchy(hierarchy.getHierarchy());
    hierarchy_.lowerLevel(id_copy);
}

void Process::mapToDevice(bool mapped_to_cuda) throw(){
	mapped_to_cuda_=mapped_to_cuda;
}

bool Process::isMappedToDevice() throw(){
	return mapped_to_cuda_;
}


int Process::getCost() const throw(){
	return cost_;
}

void  Process::setCost(int& cost) throw(){
	cost_ = cost;
}

unsigned Process::getStream() throw(){
	return stream_number_;
}

void  Process::setStream(unsigned stream_number) throw(){
	stream_number_ = stream_number;
}


Hierarchy::Relation Process::findRelation(const Process* rhs) const throw(RuntimeException){
	if (rhs) return hierarchy_.findRelation(rhs->hierarchy_);
	else {
		THROW_EXCEPTION(IllegalStateException, string("Error in : ")
					+ getId()->getString()
		            + "! Finding relation without hierarchy is not possible");
	}
}

void Process::check() throw(InvalidProcessException) {
    moreChecks();
}

Process::Interface::Interface(const Id& id) throw()
        : id_(id), process_(NULL) {}

Process::Interface::Interface(const Id& id, Process* process) throw(InvalidArgumentException)
        : id_(id), process_(process) {
    if (!process) {
        THROW_EXCEPTION(InvalidArgumentException, "process must not be NULL");
    }
}

Process::Interface::~Interface() throw() {
}
        
Process* Process::Interface::getProcess() const throw() {
    return process_;
}

const Id* Process::Interface::getId() const throw() {
    return &id_;
}

string Process::Interface::toPrint() const throw() {
    string str;
    if (process_) str += process_->getId()->getString();
    else          str += "NULL";
    str += ":";
    str += id_.getString();
    string additional_data(moreToString());
    if (additional_data.length() > 0) {
        str += additional_data;
        str += "\n";
    }
    return str;
}

string Process::Interface::toString() const throw() {
    string str;
    if (process_) str += process_->getId()->getString();
    else          str += "NULL";
    str += "_";
    str += id_.getString();
    return str;
}

string Process::Interface::moreToString() const throw() {
    return "";
}
