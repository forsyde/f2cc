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
namespace Forsyde {

/**
 * @brief A class used for storing and manipulating a process' hierarchy in the
 * process network.
 *
 * The \c Hierarchy class is used to represent and manipulate the hierarchy of a process
 * in the internal representation of ForSyDe process networks. It is basically a list of
 * IDs that determine the path to a process in the network's hierarchy tree.
 *
 * Also, please notice that, unlike in version 0.1, a process does not contain an Id,
 * rather a Hierarchy. The process' unique ID is contained in the Hierarchy.
 */
class Hierarchy {
  public:
    /**
     * Denotes the relationship between leafs in a hierarchical
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
     * Creates an empty hierarchy object.
     */
    Hierarchy() throw();

    /**
     * Creates a Hierarchy object.
     *
     * @param hierarchy
     *        list of IDs, denoting the path to this Process.
     */
    Hierarchy(std::list<Forsyde::Id*> hierarchy) throw();

    /**
     * Destroys the hierarchy object from a list of IDs.
     */
    ~Hierarchy() throw();

    /**
     * Gets the hierarchical path represented as a list of IDs.
     *
     * @return list of IDs, denoting the path to this Process.
     */
    std::list<Forsyde::Id*>  getHierarchy() throw();

    /**
     * Sets the internal hierarchical path represented as a list of IDs.
     *
     * @param hierarchy
     *        list of IDs, denoting the path to this Process.
     */
    void setHierarchy(std::list<Forsyde::Id*> hierarchy) throw();

    /**
     * Introduces an ID at the end of the hierarchical list, thus lowering
     * the level of the process.
     *
     * @param id
     *        new ID.
     */
    void lowerLevel(const Forsyde::Id& id) throw();

    /**
     * Erases the last entrance in the hierarchy list, thus raising the level of
     * the process.
     */
    void raiseLevel() throw();

    /**
     * Gets the ID of this process.
     *
     * @returns The ID of this process.
     */
    const Forsyde::Id* getId() const throw();

    /**
     * Gets the ID of first parent of this process.
     *
     * @returns The first parent's ID.
     */
    const Forsyde::Id* getFirstParent() const throw();

    /**
     * Gets the ID of first child of this process.
     *
     * @returns The first child's ID.
     */
    const Forsyde::Id* getFirstChildAfter(const Forsyde::Id& id) const throw();

    /**
     * Finds the relation between the process having the current hierarchy and another
     * process's hierarchical path.
     *
     * @param compare_hierarchy
     *        The hierarchy of the process that needs to be localized.
     *
     * @returns The position of the given process related to this one.
     */
    Relation findRelation(Hierarchy compare_hierarchy) const throw();


    std::string hierarchyToString() const throw();

  private:
    /**
     * Attempts to find an Id in the contained hierarchy list. If the list
     * is not empty and such a port is found, an iterator pointing to that Id
     * is returned; otherwise the list's \c end() iterator is returned.
     *
     * @param id
     *        ID.
     *
     * @returns Iterator pointing either at the found Id, or an iterator equal
     *          to the list's \c end() iterator.
     */
    std::list<Forsyde::Id*>::const_iterator findId(const Forsyde::Id& id) const throw();

    /**
     * Converts this hierarchy path into a string representation. The resultant string
     * is as follows:
     * @code
     * {
     *  <root>[ <- <parent_id> <- ... <- <first_parent_id>] <- <current_process_id>
     * }
     * @endcode
     *
     * @returns String representation.
     */
    std::string toString(std::list<Forsyde::Id*> ids) const throw();

  private:

    /**
     * The hierarchical path in the process network tree to this process, as a list of IDs
     * It includes the Id of the current process.
     */
    std::list<Forsyde::Id*> hierarchy_;

};

}
}

#endif /* HIERARCHY_H_ */
