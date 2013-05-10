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

#ifndef F2CC_SOURCE_FORSYDE_DELAY_H_
#define F2CC_SOURCE_FORSYDE_DELAY_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Implements the ForSyDe \c delay leaf.
 */

#include "../leaf.h"
#include "../../exceptions/invalidargumentexception.h"
#include <string>

namespace f2cc {
namespace Forsyde {
namespace SY {

/**
 * @brief Implements the ForSyDe \c delay leaf.
 */
class delay : public Leaf {
  public:
    /**
     * Creates a leaf.
     *
     * @param id
     *        Leaf ID.
     * @param initial_value
     *        Initial delay value.
     * @throws InvalidArgumentException
     *         When the initial delay value is empty string.
     */
    delay(const Id& id, const std::string& initial_value)
        throw(InvalidArgumentException);

    /**
     * Creates a leaf.
     *
     * @param id
     *        Leaf ID.
     * @param initial_value
     *        Initial delay value.
     * @throws InvalidArgumentException
     *         When the initial delay value is empty string.
     */
    delay(const Forsyde::Id& id, Forsyde::Hierarchy hierarchy,
            		int cost, const std::string& initial_value)
    	throw(InvalidArgumentException);

    /**
     * @copydoc ~Leaf()
     */
    virtual ~delay() throw();

    /**
     * Gets the initial value for this leaf.
     *
     * @returns Initial value.
     */
    std::string getInitialValue() throw();

    /**
     * Same as Leaf::operator==(const Leaf&) const but with the additional
     * check that the leafs' initial values must also be equal.
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
     * Checks that this leaf has only one in port and one out port.
     *
     * @throws InvalidProcessException
     *         When the check fails.
     */
    virtual void moreChecks() throw(InvalidProcessException);

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
    std::string initial_value_;
};

}
}
}

#endif
