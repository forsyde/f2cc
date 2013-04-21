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

#ifndef F2CC_SOURCE_FORSYDE_PARALLELMAP_H_
#define F2CC_SOURCE_FORSYDE_PARALLELMAP_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Implements a synthesis-related \c parallelmap leaf.
 */

#include "coalescedmapsy.h"
#include "../../language/cfunction.h"
#include "../../exceptions/invalidargumentexception.h"
#include "../../exceptions/outofmemoryexception.h"
#include <string>
#include <list>

namespace f2cc {
namespace ForSyDe {
namespace SY{

/**
 * @brief Implements a synthesis-related \c parallelmap leaf.
 *
 * This class implements a specialized leaf \c parallelmap which is not
 * part of the ForSyDe standard. Instead, it is used to replace data parallel
 * sections consisting of a \c unzipx leaf, a set of \c comb or \c
 * CoalescedMap leafs, followed by a \c zipx leaf, with a single \c
 * ParallelMap leaf as it entails the same semantic meaning.
 */
class ParallelMap : public CoalescedMap {
  public:
    /**
     * Creates a leaf with a single function argument.
     *
     * @param id
     *        Leaf ID.
     * @param num_leafs
     *        Number of data parallel comb leafs that this leaf will
     *        represent.
     * @param function
     *        Leaf function argument.
     * @throws OutOfMemoryException
     *         When the function list could not be created due to memory
     *         shortage.
     */
    ParallelMap(const Id& id, int num_leafs, const CFunction& function)
        throw(OutOfMemoryException);

    /**
     * Creates a leaf with multiple function arguments.
     *
     * @param id
     *        Leaf ID.
     * @param num_leafs
     *        Number of data parallel comb leafs that this leaf will
     *        represent.
     * @param functions
     *        List of leaf function arguments.
     * @throws InvalidArgumentException
     *         When \c functions is an empty list.
     * @throws OutOfMemoryException
     *         When the function list could not be created due to memory
     *         shortage.
     */
    ParallelMap(const Id& id, int num_leafs,
                  const std::list<CFunction>& functions)
        throw(InvalidArgumentException, OutOfMemoryException);

    /**
     * @copydoc ~Leaf()
     */
    virtual ~ParallelMap() throw();

    /**
     * @copydoc comb::operator==(const Leaf&) const
     */
    virtual bool operator==(const Leaf& rhs) const throw();

    /**
     * Gets the number of data parallel comb or CoalescedMap leafs that
     * this leaf represents.
     *
     * @returns Number of leafs.
     */
    int getNumProcesses() const throw();

    /**
     * @copydoc Leaf::type()
     */
    virtual std::string type() const throw();

  private:
    /**
     * Number of parallel leafs.
     */
    const int num_parallel_leafs_;
};

}
}
}

#endif
