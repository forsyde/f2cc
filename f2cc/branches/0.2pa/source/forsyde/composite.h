/*
 * Copyright (c) 2011-2013 Gabriel Hjort Blindell <ghb@kth.se>
 *                          George Ungureanu <ugeorge@kth.se>
 *
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

#ifndef F2CC_SOURCE_FORSYDE_COMPOSITE_H_
#define F2CC_SOURCE_FORSYDE_COMPOSITE_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines the base class for a composite process in the internal
 *        representation of ForSyDe processnetworks.
 */

#include "id.h"
#include "model.h"
#include "process.h"
#include "../exceptions/outofmemoryexception.h"
#include "../exceptions/notsupportedexception.h"
#include "../exceptions/invalidprocessexception.h"
#include "../exceptions/invalidformatexception.h"
#include "../exceptions/invalidargumentexception.h"
#include <list>
#include <utility>


namespace f2cc {
namespace ForSyDe {


/**
 * @brief Base class for composite processes in the internal representation of ForSyDe
 * processnetworks.
 *
 * \c Composite is a base class for composite processes in internal representation
 * of ForSyDe processnetworks. It inherits the Processnetwork class, hence it encapsulates a list of processes, but has
 * a unique ID
 */
class Composite : public Model, public Process {
  public:
    /**
     * Creates a composite process.
     *
     * @param id
     *        Process ID.
     */
    Composite(const ForSyDe::Id& id, const ForSyDe::Id& parent, std::string name) throw();

    /**
     * Destroys this composite process. This also destroys all contained processes
     */
    virtual ~Composite() throw();

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
     * Converts this composite process into a string representation. The resultant string
     * is as follows:
     * @code
     * {
     *  List of Processes : {...}
     * @endcode
     *
     * @returns Additional string representation data.
     * @see toString()
     */
    virtual std::string moreToString() const throw();

    /**
     * Checks that this process has at least one in port and only one out
     * port. It also checks the function (see checkFunction(const CFunction&)).
     *
     * @throws InvalidProcessException
     *         When the check fails.
     */
    virtual void moreChecks() throw(InvalidProcessException);

  private:
    std::string composite_name_;
};

}
}

#endif
