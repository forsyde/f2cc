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

#ifndef F2CC_SOURCE_FORSYDE_PROCESS_H_
#define F2CC_SOURCE_FORSYDE_PROCESS_H_

/**
 * @file
 * @author George Ungureanu <ugeorge@kth.se> & Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines the base class for all the processes in the internal
 *        representation of ForSyDe models.
 */

#include "id.h"
#include "hierarchy.h"
#include "../exceptions/outofmemoryexception.h"
#include "../exceptions/notsupportedexception.h"
#include "../exceptions/invalidprocessexception.h"
#include "../exceptions/invalidformatexception.h"
#include "../exceptions/invalidargumentexception.h"
#include "../exceptions/illegalstateexception.h"
#include "../exceptions/illegalcallexception.h"
#include <list>

namespace f2cc {
namespace Forsyde {

/**
 * @brief Base class for all processes in the internal representation of ForSyDe
 * models.
 *
 * The \c Process is a base class for all processes in internal representation
 * of ForSyDe models. It provides functionality common for all processes such as
 * ID and hierarchy management.
 */
class Process{
  public:
    class Port;

  public:
    /**
     * Creates a process node.
     *
     * @param id
     *        Process ID.
     */
    Process(const Forsyde::Id& id) throw();

    /**
     * Creates a process node with a hierarchy.
     *
     * @param id
     *        Process ID.
     * @param hierarchy
     *        Process hierarchy.
     */
    Process(const Forsyde::Id& id, Forsyde::Hierarchy& hierarchy,
    		bool mapped_to_cuda, int cost) throw();

    /**
     * Destroys this process.
     */
    virtual ~Process() throw();

    /**
     * Gets the ID of this process.
     *
     * @returns Process ID.
     */
    const Forsyde::Id* getId() const throw();

    /**
     * Gets the hierarchy of this process.
     *
     * @returns Process hierarchy.
     */
    Forsyde::Hierarchy getHierarchy() const throw();

    /**
     * Finds the status of the process passed as argument relative to this process.
     * @param rhs
     *        Process that needs to be localized.
     * @returns The relation between the two processes.
     */
    Hierarchy::Relation findRelation(const Process* rhs) const throw(RuntimeException);

    /**
	 * Sets the hierarchy for this process.
	 *
	 * @returns Process ID.
	 */
	void setHierarchy(Forsyde::Hierarchy) throw();

    /**
     * Gets the cost for this process.
     *
     * @returns The cost parameter.
     */
    int getCost() const throw();

    /**
     * Sets the cost for this process.
     *
     * @param cost
     *        The cost parameter.
     */
    void setCost(int& cost) throw();

    void mapToDevice(bool mapped_to_cuda) throw();

    bool isMappedToDevice() throw();

    void setStream(unsigned stream_number) throw();

    unsigned getStream() throw();

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
	 * Hierarchy list
	 */
	Forsyde::Hierarchy hierarchy_;

	bool mapped_to_cuda_;

    /**
	 * Process cost parameter.
	 */
	int cost_;

	unsigned stream_number_;


  public:
    /**
     * @brief Base class for all types of interfaces.
     *
     * The \c Interface class defines a process interface.
     *
     * @see \c Leaf::Port
     * @see \c Composite::IOPort
     */
    class Interface {
      public:
        /**
         * Creates a interface belonging to no process.
         *
         * @param id
         *        Port ID.
         */
    	Interface(const Forsyde::Id& id) throw();

        /**
         * Creates a interface belonging to a process.
         *
         * @param id
         *        Port ID.
         * @param process
         *        Pointer to the process to which this interface belongs.
         * @throws InvalidArgumentException
         *         When \c process is \c NULL.
         */
    	Interface(const Forsyde::Id& id, Process* process)
            throw(InvalidArgumentException);


        virtual ~Interface() throw();
        
        /**
         * Gets the process to which this interface belongs.
         *
         * @returns Process, if available; otherwise \c NULL.
         */
        Process* getProcess() const throw();

        /**
         * Gets the ID of this interface.
         *
         * @returns Port ID.
         */
        const Forsyde::Id* getId() const throw();

        virtual void connect(Process::Interface* port) throw(RuntimeException,
        		InvalidArgumentException,IllegalCallException) = 0;

        /**
         * Converts this interface into a string representation. The resultant string
         * is as follows:
         * @code
         *  <process_id>:<interface_id>
         * @endcode
         *
         * @returns String representation.
         */
        std::string toString() const throw();

      protected:
        /**
         * Additional string output to be included when this process is converted to
         * a string representation. By default this returns an empty string.
         *
         * @returns Additional string representation data.
         * @see toString()
         */
        virtual std::string moreToString() const throw();

      private:
        /**
         * Due to how interface copying works, the assign operator is hidden and thus
         * not allowed to avoid potential bugs as it is easy to forget this
         * fact.
         */
        void operator=(const Port&) throw();

      protected:
        /**
         * Port ID.
         */
        const Forsyde::Id id_;

        /**
         * Port name.
         */
        Process* process_;


    };

  public:

};

}
}

#endif
