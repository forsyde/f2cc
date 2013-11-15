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

#ifndef F2CC_SOURCE_FORSYDE_MAPSY_H_
#define F2CC_SOURCE_FORSYDE_MAPSY_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Implements the ForSyDe \c mapSY leaf.
 */

#include "../leaf.h"
#include "../../language/cfunction.h"
#include <string>

namespace f2cc {
namespace Forsyde {
namespace SY {

/**
 * @brief Implements the ForSyDe \c mapSY leaf.
 */
class Map : public Leaf {
  public:
    /**
     * Creates a leaf.
     *
     * @param id
     *        Leaf ID.
     * @param function
     *        Leaf function argument.
     */
    Map(const Id& id, const CFunction& function) throw();

    /**
     * @copydoc ~Leaf()
     */
    virtual ~Map() throw();

    /**
     * Gets the function argument of this leaf.
     *
     * @returns Function argument.
     */
    virtual CFunction* getFunction() throw();

    /**
     * Same as Leaf::operator==(const Leaf&) const but with the additional
     * check that the leafs' function arguments must also be equal.
     *
     * @param rhs
     *        Leaf to compare with.
     * @returns \b true if both leafs are equal.
     */
    virtual bool operator==(const Leaf& rhs) const throw();

    /**
     * @copydoc Leaf::type()
     */
    virtual std::string type() const throw();

  protected:
    /**
     * Checks that this leaf has only one in port and one out port. It also
     * checks the function (see checkFunction(const CFunction&)).
     *
     * @throws InvalidProcessException
     *         When the check fails.
     */
    virtual void moreChecks() throw(InvalidProcessException);

    /**
     * Performs a series of checks:
     *    - The function must have either one or two input parameters.
     *    - If the function has one input parameter, then the function must
     *      return data (i.e. have return data type other than \c void) which
     *      also is not an array.
     *    - If the function has two input parameters, then the function must not
     *      return data (i.e. have return data type \c void).
     *    - If the first input parameter is an array or pointer, it must also
     *      be declared \c const.
     *
     * @param function
     *        Function to check.
     * @throws InvalidProcessException
     *         When the check fails.
     */
    virtual void checkFunction(CFunction& function) const
        throw(InvalidProcessException);

    /**
     * Gets the function argument as string representation in the following
     * format:
     * @code
     * LeafFunction: <function_argument>
     * @endcode
     *
     * @returns Additional string representation data.
     * @see toString()
     */
    virtual std::string moreToString() const throw();

  protected:
    /**
     * Leaf function argument.
     */
    CFunction function_;
};

}
}
}

#endif
