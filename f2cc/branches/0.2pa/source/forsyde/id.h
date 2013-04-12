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

#ifndef F2CC_SOURCE_FORSYDE_ID_H_
#define F2CC_SOURCE_FORSYDE_ID_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines a ForSyDe ID class.
 */

#include <string>
#include <iostream>

namespace f2cc {
namespace ForSyDe {

/**
 * @brief A class used for identifying elements in the ForSyDe processnetwork.
 *
 * The \c Id class is used to represent IDs in the internal
 * representation of ForSyDe processnetworks. Although the ID itself is actually just a
 * string, wrapping it inside a class allows the class to be improved in case
 * the performance suffers for very large processnetworks due to working with string
 * comparison (which is not very efficient).
 */
class Id {
  public:
    /**
     * Creates an ID.
     *
     * @param id
     *        ID string.
     */
    Id(const std::string& id) throw();

    /**
     * Destroys this ID.
     */
    ~Id() throw();

    /**
     * Gets the ID as string.
     *
     * @returns ID string.
     */
    std::string getString() const throw();

    /**
     * Checks for equality between this ID and another.
     *
     * @param rhs
     *        ID to compare.
     * @returns \c true if the IDs are identical.
     */
    bool operator==(const Id& rhs) const throw();

    /**
     * Checks for inequality between this ID and another.
     *
     * @param rhs
     *        ID to compare.
     * @returns \c true if the IDs are not identical.
     */
    bool operator!=(const Id& rhs) const throw();

    /**
     * Checks for ordering between this ID and another.
     *
     * @param rhs
     *        ID to compare.
     * @returns \c true if this ID's string is evaluated less than the ID string
     *          of the other.
     */
    bool operator<(const Id& rhs) const throw();

    friend std::ostream& operator<<(std::ostream& stream, const Id& id);

  private:
    /**
     * ID string.
     */
    std::string id_;
};

/**
 * Outputs an ID as a string representation to a stream.
 *
 * @param stream
 *        Output stream.
 * @param id
 *        ID to output.
 * @returns The output stream.
 */
std::ostream& operator<<(std::ostream& stream, const Id& id);

}
}

#endif
