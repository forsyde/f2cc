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

#ifndef F2CC_SOURCE_FORSYDE_COALESCEDMAP_H_
#define F2CC_SOURCE_FORSYDE_COALESCEDMAP_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Implements a synthesis-related \c coalescedmap leaf.
 */

#include "mapsy.h"
#include "../../language/cfunction.h"
#include "../../exceptions/invalidargumentexception.h"
#include "../../exceptions/outofmemoryexception.h"
#include <string>
#include <list>

namespace f2cc {
namespace ForSyDe {
namespace SY {

/**
 * @brief Implements a synthesis-related \c coalescedmap leaf.
 *
 * This class implements a specialized leaf \c coalescedcomb which is not
 * part of the ForSyDe standard. It is used to replace coalesced \c comb
 * leafs into a single leaf which contains all function arguments of the
 * leafs which it replaces. Thus, executing a single \c CoalescedMap
 * leaf produces the same result as executing a series of \c comb leafs.
 */
class CoalescedMap : public Map {
  public:
    /**
     * Creates a leaf with a single function.
     *
     * @param id
     *        Leaf ID.
     * @param function
     *        Leaf function argument.
     * @throws OutOfMemoryException
     *         When the function list could not be created due to memory
     *         shortage.
     */
    CoalescedMap(const Id& id, const CFunction& function)
        throw(OutOfMemoryException);

    /**
     * Creates a leaf with multiple functions. The list must contain at
     * least one element.
     *
     * @param id
     *        Leaf ID.
     * @param functions
     *        List of functions.
     * @throws InvalidArgumentException
     *         When \c functions is an empty list.
     * @throws OutOfMemoryException
     *         When the leaf could not be created due to memory shortage.
     */
    CoalescedMap(const Id& id, const std::list<CFunction>& functions)
        throw(InvalidArgumentException, OutOfMemoryException);

    /**
     * @copydoc ~Leaf()
     */
    virtual ~CoalescedMap() throw();

    /**
     * Gets the first function argument of this leaf.
     *
     * @returns First function argument.
     */
    virtual CFunction* getFunction() throw();

    /**
     * Gets the list of function arguments of this leaf.
     *
     * @returns List of function arguments.
     */
    std::list<CFunction*> getFunctions() throw();

    /**
     * Inserts a new function as first function of this leaf.
     *
     * @param function
     *        Function to insert.
     * @throws OutOfMemoryException
     *         When the function could not be inserted due to memory shortage.
     */
    void insertFunctionFirst(const CFunction& function)
        throw(OutOfMemoryException);

    /**
     * Inserts a new function as last function of this leaf.
     *
     * @param function
     *        Function to insert.
     * @throws OutOfMemoryException
     *         When the function could not be inserted due to memory shortage.
     */
    void insertFunctionLast(const CFunction& function)
        throw(OutOfMemoryException);

    /**
     * Same as Leaf::operator==(const Leaf&) const but with the additional
     * check that the leafs' function arguments must also be equal.
     *
     * @param rhs
     *        Leaf to compare with.
     * @returns \c true if both leafs are equal.
     */
    virtual bool operator==(const Leaf& rhs) const throw();

    /**
     * @copydoc Leaf::type()
     */
    virtual std::string type() const throw();

  protected:
    /**
     * Checks that this leaf has only one in interface and one out interface. It also
     * checks that all function arguments have one input parameter.
     *
     * @throws InvalidLeafException
     *         When the check fails.
     */
    virtual void moreChecks() throw(InvalidLeafException);

    /**
     * Gets the function argument as string representation in the following
     * format:
     * @code
     * LeafFunction: <function_argument>[,
     * LeafFunction: <function_argument>...]
     * @endcode
     *
     * @returns Additional string representation data.
     * @see toString()
     */
    virtual std::string moreToString() const throw();

  private:
    /**
     * Prevents this from being auto-implemented by the compiler.
     *
     * @param rhs
     */
    CoalescedMap(CoalescedMap& rhs) throw();

    /**
     * @copydoc CoalescedMap(CoalescedMap&)
     *
     * @returns
     */
    CoalescedMap& operator=(CoalescedMap& rhs) throw();

  protected:
    /**
     * List of leaf function arguments.
     */
    std::list<CFunction*> functions_;
};

}
}
}

#endif
