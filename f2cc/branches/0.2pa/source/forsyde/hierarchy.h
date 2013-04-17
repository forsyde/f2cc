/*
 * Copyright (c) 2011-2013 Gabriel Hjort Blindell <ghb@kth.se>
 *                          George Ungureanu <ugeorge@kth.se>
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

#ifndef F2CC_SOURCE_FORSYDE_HIERARCHY_H_
#define F2CC_SOURCE_FORSYDE_HIERARCHY_H_

/**
 * @file
 * @author  George Ungureanu <ugeorge@kth.se>
 * @version 0.2
 *
 * @brief Defines a ForSyDe Hierarchy class.
 */
#include "id.h"
#include <string>
#include <iostream>
#include <list>

namespace f2cc {
namespace ForSyDe {

/**
 * @brief A class used for storing and manipulating a process' hierarchy in the model.
 *
 * The \c Hierarchy class is used to represent and manipulate the hierarchy of a process
 * in the internal representation of ForSyDe process networks. It is basically a list of
 * IDs.
 */
class Hierarchy {
  public:
    /**
     * Denotes the relationship between processes in a hierarchical
     * process network.
     */
    enum Relation {
        /**
         * A process which resides lower in the hierarchy chain.
         */
        Child,

        /**
		 * A process which is directly contained by the current composite.
		 */
		FirstChild,

        /**
         * A process which resides higher in the hierarchy chain.
         */
        Parent,

        /**
         * The composite which directly includes this process.
         */
        FirstParent,

        /**
         * A process which has the same FirstParent as the current one.
         */
        Sibling,

        /**
         * A child process for one of the current process' siblings (nephew)
         */
        SiblingsChild,

        /**
         * A process which resides in a different hierarchical branch than the
         * current one
         */
        Other
    };

  public:
    /**
     * Creates a Hierarchy object.
     *
     * @param id
     *        ID string.
     */
    Hierarchy(std::list<const ForSyDe::Id> hierarchy) throw();

    ~Hierarchy() throw();

    std::list<const ForSyDe::Id>  getHierarchy() throw();

    void setHierarchy(std::list<const ForSyDe::Id> hierarchy) throw();

    void lowerLevel(const ForSyDe::Id& id) throw();

    void raiseLevel() throw();

    /**
     * Gets the parent of this process.
     *
     * @returns The parent process.
     */
    const ForSyDe::Id* getId() const throw();

    /**
     * Gets the parent of this process.
     *
     * @returns The parent process.
     */
    const ForSyDe::Id* getFirstParent() const throw();

    /**
     * Gets the parent of this process.
     *
     * @returns The parent process.
     */
    const ForSyDe::Id* getFirstChildAfter(const ForSyDe::Id& id) const throw();

    /**
     * Gets the parent of this process.
     *
     * @returns The parent process.
     */
    Relation findRelation(Hierarchy compare_hierarchy) const throw();

  private:
    /**
     * Attempts to find a port with a given ID from a list of ports. If the list
     * is not empty and such a port is found, an iterator pointing to that port
     * is returned; otherwise the list's \c end() iterator is returned.
     *
     * @param id
     *        Port ID.
     * @param ports
     *        List of ports.
     * @returns Iterator pointing either at the found port, or an iterator equal
     *          to the list's \c end() iterator.
     */
    std::list<const ForSyDe::Id>::const_iterator findId(
    		const ForSyDe::Id& id) const throw();

  private:

    std::list<const ForSyDe::Id> hierarchy_;

};

}
}

#endif /* HIERARCHY_H_ */
