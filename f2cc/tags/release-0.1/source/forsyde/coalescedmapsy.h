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

#ifndef F2CC_SOURCE_FORSYDE_COALESCEDMAPSY_H_
#define F2CC_SOURCE_FORSYDE_COALESCEDMAPSY_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Implements a synthesis-related \c coalescedmapSY process.
 */

#include "mapsy.h"
#include "../language/cfunction.h"
#include "../exceptions/invalidargumentexception.h"
#include "../exceptions/outofmemoryexception.h"
#include <string>
#include <list>

namespace f2cc {
namespace Forsyde {

/**
 * @brief Implements a synthesis-related \c coalescedmapSY process.
 *
 * This class implements a specialized process \c coalescedMapSY which is not
 * part of the ForSyDe standard. It is used to replace coalesced \c MapSY
 * processes into a single process which contains all function arguments of the
 * processes which it replaces. Thus, executing a single \c CoalescedMapSY
 * process produces the same result as executing a series of \c MapSY processes.
 */
class CoalescedMapSY : public MapSY {
  public:
    /**
     * Creates a process with a single function.
     *
     * @param id
     *        Process ID.
     * @param function
     *        Process function argument.
     * @throws OutOfMemoryException
     *         When the function list could not be created due to memory
     *         shortage.
     */
    CoalescedMapSY(const Id& id, const CFunction& function)
        throw(OutOfMemoryException);

    /**
     * Creates a process with multiple functions. The list must contain at
     * least one element.
     *
     * @param id
     *        Process ID.
     * @param functions
     *        List of functions.
     * @throws InvalidArgumentException
     *         When \c functions is an empty list.
     * @throws OutOfMemoryException
     *         When the process could not be created due to memory shortage.
     */
    CoalescedMapSY(const Id& id, const std::list<CFunction>& functions)
        throw(InvalidArgumentException, OutOfMemoryException);

    /**
     * @copydoc ~Process()
     */
    virtual ~CoalescedMapSY() throw();

    /**
     * Gets the first function argument of this process.
     *
     * @returns First function argument.
     */
    virtual CFunction* getFunction() throw();

    /**
     * Gets the list of function arguments of this process.
     *
     * @returns List of function arguments.
     */
    std::list<CFunction*> getFunctions() throw();

    /**
     * Inserts a new function as first function of this process.
     *
     * @param function
     *        Function to insert.
     * @throws OutOfMemoryException
     *         When the function could not be inserted due to memory shortage.
     */
    void insertFunctionFirst(const CFunction& function)
        throw(OutOfMemoryException);

    /**
     * Inserts a new function as last function of this process.
     *
     * @param function
     *        Function to insert.
     * @throws OutOfMemoryException
     *         When the function could not be inserted due to memory shortage.
     */
    void insertFunctionLast(const CFunction& function)
        throw(OutOfMemoryException);

    /**
     * Same as Process::operator==(const Process&) const but with the additional
     * check that the processes' function arguments must also be equal.
     *
     * @param rhs
     *        Process to compare with.
     * @returns \c true if both processes are equal.
     */
    virtual bool operator==(const Process& rhs) const throw();

    /**
     * @copydoc Process::type()
     */
    virtual std::string type() const throw();

  protected:
    /**
     * Checks that this process has only one in port and one out port. It also
     * checks that all function arguments have one input parameter.
     *
     * @throws InvalidProcessException
     *         When the check fails.
     */
    virtual void moreChecks() throw(InvalidProcessException);

    /**
     * Gets the function argument as string representation in the following
     * format:
     * @code
     * ProcessFunction: <function_argument>[,
     * ProcessFunction: <function_argument>...]
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
    CoalescedMapSY(CoalescedMapSY& rhs) throw();

    /**
     * @copydoc CoalescedMapSY(CoalescedMapSY&)
     *
     * @returns
     */
    CoalescedMapSY& operator=(CoalescedMapSY& rhs) throw();

  protected:
    /**
     * List of process function arguments.
     */
    std::list<CFunction*> functions_;
};

}
}

#endif
