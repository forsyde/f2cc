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

#include "parallelmapsy.h"
#include <typeinfo>

using namespace f2cc::Forsyde;
using std::string;
using std::list;
using std::bad_cast;

ParallelMapSY::ParallelMapSY(const Id& id, int num_processes,
                             const CFunction& function)
        throw(OutOfMemoryException) : CoalescedMapSY(id, function),
                                      num_parallel_processes_(num_processes) {}

ParallelMapSY::ParallelMapSY(const Id& id, int num_processes,
                             const list<CFunction>& functions)
        throw(InvalidArgumentException, OutOfMemoryException)
        : CoalescedMapSY(id, functions),
          num_parallel_processes_(num_processes) {}

ParallelMapSY::~ParallelMapSY() throw() {}

bool ParallelMapSY::operator==(const Process& rhs) const throw() {
    if (CoalescedMapSY::operator==(rhs)) return false;

    try {
        const ParallelMapSY& other = dynamic_cast<const ParallelMapSY&>(rhs);
        if (num_parallel_processes_ != other.num_parallel_processes_) {
            return false;
        }
    }
    catch (bad_cast&) {
        return false;
    }
    return true;
}

int ParallelMapSY::getNumProcesses() const throw() {
    return num_parallel_processes_;
}

string ParallelMapSY::type() const throw() {
    return "ParallelMapSY";
}
