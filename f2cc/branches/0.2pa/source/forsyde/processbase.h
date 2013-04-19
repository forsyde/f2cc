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

#ifndef F2CC_SOURCE_FORSYDE_PROCESSBASE_H_
#define F2CC_SOURCE_FORSYDE_PROCESSBASE_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se> & George Ungureanu <ugeorge@kth.se>
 * @version 0.2
 *
 * @brief Defines the base class for process nodes in the internal
 *        representation of ForSyDe processnetworks.
 */

#include "id.h"
#include "hierarchy.h"
#include "../exceptions/outofmemoryexception.h"
#include "../exceptions/notsupportedexception.h"
#include "../exceptions/invalidprocessexception.h"
#include "../exceptions/invalidmodelexception.h"
#include "../exceptions/invalidformatexception.h"
#include "../exceptions/invalidargumentexception.h"
#include "../exceptions/illegalcallexception.h"
#include <list>
#include <utility>

namespace f2cc {
namespace ForSyDe {


class ProcessBase{
  public:
	class PortBase;
  public:
   /**
	 * Creates a process node.
	 *
	 * @param id
	 *        Process ID.
	 */
	ProcessBase(const Id& id) throw();

	/**
	 * Destroys this process. This also destroys all ports and breaks all
	 * port connections.
	 */
	virtual ~ProcessBase() throw();

    /**
     * Gets the ID of this process.
     *
     * @returns Process ID.
     */
    const ForSyDe::Id* getId() const throw();

    /**
     * Gets the ID of this process.
     *
     * @returns Process ID.
     */
    ForSyDe::Hierarchy getHierarchy() const throw();

    /**
	 * Gets the ID of this process.
	 *
	 * @returns Process ID.
	 */
	void setHierarchy(ForSyDe::Hierarchy) throw();

    /**
     * Gets the parent of this process.
     *
     * @returns The parent process.
     */
    Hierarchy::Relation findRelation(const ProcessBase* rhs) const throw();

    /**
     * Checks that this process is valid. This does nothing except invoke the
     * purely virtual method moreChecks() for process type-related checks.
     *
     * @throws InvalidProcessException
     *         When the check fails.
     */
    void check() throw(InvalidProcessException);

    /**
     * Gets the type of this process as a string.
     *
     * @returns Process type.
     */
    virtual std::string type() const throw() = 0;

  protected:
    /**
     * Performs process type-related checks on this process. This needs to be
     * derived by all subclasses.
     *
     * @throws InvalidProcessException
     *         When the check fails.
     */
    virtual void moreChecks() throw(InvalidProcessException) = 0;

  protected:
    /**
	 * Process ID
	 */
	const ForSyDe::Id id_;
    /**
	 * Hierarchy list
	 */
	ForSyDe::Hierarchy hierarchy_;

  public:
    /**
     * @brief Class used for in- and out ports by the \c Process class.
     *
     * The \c Port class defines a process port. A port is identified by an ID
     * and can be connected to another port.
     */
    class PortBase {
      public:

        /**
         * Creates a port belonging to no process.
         *
         * @param id
         *        Port ID.
         * @param datatype
         *        Data type contained by port.
         */
    	PortBase(const ForSyDe::Id& id) throw();

        /**
         * Creates a port belonging to no process.
         *
         * @param id
         *        Port ID.
         * @param datatype
         *        Data type contained by port.
         */
    	PortBase(const ForSyDe::Id& id, ProcessBase* process) throw();

        /**
         * Destroys this port. This also breaks the connection, if any.
         */
        virtual ~PortBase() throw();

        /**
         * Gets the ID of this port.
         *
         * @returns Port ID.
         */
        const ForSyDe::Id* getId() const throw();

        /**
		 * Sets the data type of this port.
		 *
		 * @param datatype
		 *        The new data type that has to be set.
		 */
      protected:
        /**
         * Port ID.
         */
        const ForSyDe::Id id_;

        /**
         * Parent process.
         */
        ProcessBase* process_;
    };

};

}
}

#endif
