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

#ifndef F2CC_SOURCE_FORSYDE_ZIPX_H_
#define F2CC_SOURCE_FORSYDE_ZIPX_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Implements the ForSyDe \c Zipx leaf.
 */

#include "../leaf.h"
#include "../../exceptions/notsupportedexception.h"
#include <string>

namespace f2cc {
namespace ForSyDe {
namespace SY{

/**
 * @brief Implements the ForSyDe \c Zipx leaf.
 */
class Zipx : public Leaf {
  public:
    /**
     * @copydoc Leaf(const Id&)
     */
    Zipx(const Id& id) throw();

    Zipx(const ForSyDe::Id& id, ForSyDe::Hierarchy hierarchy,
        		int cost) throw();

    /**
     * @copydoc ~Leaf()
     */
    virtual ~Zipx() throw();

    /**
     * @copydoc Leaf::operator==(const Leaf&) const
     */
    virtual bool operator==(const Leaf& rhs) const throw();

    /**
     * @copydoc Leaf::type()
     */
    virtual std::string type() const throw();

  protected:
    /**
     * Checks that this leaf has only one out port.
     *
     * @throws InvalidProcessException
     *         When the check fails.
     */
    virtual void moreChecks() throw(InvalidProcessException);
};

}
}
}

#endif
