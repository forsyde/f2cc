/*
 * Copyright (c) 2011-2013
 *     Gabriel Hjort Blindell <ghb@kth.se>
 *     George Ungureanu <ugeorge@kth.se>
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

#ifndef F2CC_SOURCE_FORSYDE_PARALLELCOMPOSITE_H_
#define F2CC_SOURCE_FORSYDE_PARALLELCOMPOSITE_H_

/**
 * @file
 * @author  George Ungureanu <ugeorge@kth.se>
 * @version 0.2
 *
 * @brief Defines the base class for a composite process in the internal
 *        representation of ForSyDe process networks.
 */

#include "id.h"
#include "hierarchy.h"
#include "composite.h"
#include "../exceptions/outofmemoryexception.h"
#include "../exceptions/notsupportedexception.h"
#include "../exceptions/invalidprocessexception.h"
#include "../exceptions/castexception.h"
#include "../exceptions/illegalstateexception.h"
#include "../exceptions/invalidformatexception.h"
#include "../exceptions/invalidargumentexception.h"
#include <list>
#include <utility>


namespace f2cc {
namespace Forsyde {


/**
 * @brief Base class for composite processes in the internal representation of ForSyDe
 * process networks.
 *
 * \c Composite is a base class for composite processes in internal representation
 * of ForSyDe process networks. It inherits both the \c Model class and the \c Process class.
 * Hence it behaves like a process that contains other processes.
 */
class ParallelComposite : public Composite {
  public:
    /**
     * Creates a composite process.
     *
     * @param id
     *        The composite process' ID. It uniquely identifies it among all
     *        processes in a process network.
     * @param hierarchy
     *        A \c Hierarchy object, describing its path in the model's structure.
     * @param name
     *        the composite process' name. Initially it is the same as its filename, and it is enough
     *        to identify and compare a composite process' structure.
     */
	ParallelComposite(const Forsyde::Id& id, Forsyde::Hierarchy& hierarchy,
    		Forsyde::Id name, int number_of_processes) throw();

    /**
     * Destroys this composite process. This also destroys all contained processes
     */
    virtual ~ParallelComposite() throw();

    /**
     * Gets the name of this composite process, wrapped in an \c Id object.
     *
     * @returns Name of the composite process.
     */
    virtual int getNumProcesses() throw();

    /**
     * Changes the name of this composite process.
     *
     * @param name
     *        New name for the composite process, wrapped in an \c Id object.
     */
    void setNumProcesses(int number_of_processes) throw();

    /**
     * Gets the name of this composite process, wrapped in an \c Id object.
     *
     * @returns Name of the composite process.
     */
    const Id* getContainedProcessId() const throw();

    /**
     * Changes the name of this composite process.
     *
     * @param name
     *        New name for the composite process, wrapped in an \c Id object.
     */
    void setContainedProcessId(const Id* id) throw();

     /**
	  * @copydoc Process::type()
	  */
     virtual std::string type() const throw();

  private:
     /**
      * Checks that this composite has both in and out IOPorts, and
      * at least one process.
      *
      * @throws InvalidProcessException
      *         When the check fails.
      */
     virtual void moreChecks() throw(InvalidProcessException);



  protected:

    int number_of_processes_;

    const Id* contained_process_id_;

};

}
}

#endif
