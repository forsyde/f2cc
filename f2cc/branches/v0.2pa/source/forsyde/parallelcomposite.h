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
    int getNumberOfProcesses() throw();

    /**
     * Changes the name of this composite process.
     *
     * @param name
     *        New name for the composite process, wrapped in an \c Id object.
     */
    void setNumberOfProcesses(int number_of_processes) throw();

    /**
      * Adds an in IOPort to this composite. Composites are not allowed to have
      * multiple in IOPorts with the same ID.
      *
      * @param id
      *        IOPort ID.
      * @returns \b true if such an in IOPort did not already exist and was
      *          successfully added.
      * @throws OutOfMemoryException
      *         When a IOPort cannot be added due to memory shortage.
      */
     bool addInConduit(Process::Interface* conduit) throw(OutOfMemoryException);

     /**
      * Deletes and destroys an in IOPort to this composite.
      *
      * @param id
      *        IOPort ID.
      * @returns \b true if such a IOPort was found and successfully deleted.
      */
     bool deleteInConduit(Process::Interface* port) throw();

     /**
      * Gets the number of in IOPorts of this composite.
      *
      * @returns Number of in IOPorts.
      */
     size_t getNumInConduits() const throw();

     /**
	   * Gets a list of in IOPorts belonging to this this leaf.
	   *
	   * @returns List of in IOPorts.
	   */
	  Process::Interface* getInConduit(Process::Interface* port) throw();

     /**
      * Gets a list of in IOPorts belonging to this this leaf.
      *
      * @returns List of in IOPorts.
      */
     std::list<Process::Interface*> getInConduits() throw();

     /**
       * Adds an in IOPort to this composite. Composites are not allowed to have
       * multiple in IOPorts with the same ID.
       *
       * @param id
       *        IOPort ID.
       * @returns \b true if such an in IOPort did not already exist and was
       *          successfully added.
       * @throws OutOfMemoryException
       *         When a IOPort cannot be added due to memory shortage.
       */
      bool addOutConduit(Process::Interface* conduit) throw(InvalidArgumentException,
    		  OutOfMemoryException);

      /**
       * Deletes and destroys an in IOPort to this composite.
       *
       * @param id
       *        IOPort ID.
       * @returns \b true if such a IOPort was found and successfully deleted.
       */
      bool deleteOutConduit(Process::Interface* conduit) throw(InvalidArgumentException);

      /**
       * Gets the number of in IOPorts of this composite.
       *
       * @returns Number of in IOPorts.
       */
      size_t getNumOutConduits() const throw();

      /**
 	   * Gets a list of in IOPorts belonging to this this leaf.
 	   *
 	   * @returns List of in IOPorts.
 	   */
 	  Process::Interface* getOutConduit(Process::Interface* port) throw();

      /**
       * Gets a list of in IOPorts belonging to this this leaf.
       *
       * @returns List of in IOPorts.
       */
      std::list<Process::Interface*> getOutConduits() throw();

     /**
      * Converts this leaf into a string representation. The resultant string
      * is as follows:
      * @code
      * {
      *  Composite Process ID: <composite_id>,
      *  Name: <composite_name>,
      *  NumInPorts : <num_in_ports>,
      *  InPorts = {...},
      *  NumOutPorts : <num_out_ports>,
      *  OutPorts = {...},
      * }
      * @endcode
      *
      * @returns String representation.
      */
     //virtual std::string toString() const throw();

     /**
      * Checks whether this composite is equal to another. Two composites are equal
      * if they have the same name (i.e. contain the same processes) and have
      * the same number of in and out IOPorts.
      *
      * @param rhs
      *        Composite to compare with.
      * @returns \b true if both composites are equal.
      */
     virtual bool operator==(const ParallelComposite& rhs) const throw();

     /**
      * Same as operator==(const Composite&) but for inequality.
      *
      * @param rhs
      *        Composite to compare with.
      * @returns \b true if both composites are not equal.
      */
     virtual bool operator!=(const ParallelComposite& rhs) const throw();

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
     std::list<Process::Interface*>::iterator findConduit(
         const Id& id, std::list<Process::Interface*>& conduits) const throw();

     /**
      * Attempts to find a given port from a list of ports. If the list
      * is not empty and such a port is found, an iterator pointing to that port
      * is returned; otherwise the list's \c end() iterator is returned.
      *
      * @param port
      *        Port
      * @param ports
      *        List of ports.
      * @returns Iterator pointing either at the found port, or an iterator equal
      *          to the list's \c end() iterator.
      */
     std::list<Process::Interface*>::iterator findConduit(
         Process::Interface* conduit, std::list<Process::Interface*>& conduits)  const throw();
     /**
      * Takes a list of IOPorts and converts it into a string representation. Each
      * IOPort is converted into
      * @code
      *  PortID: <port_id>, not connected inside / connected  inside to <Process>:<Interface>;
      *  and not connected outside / connected  outside to <Process>:<Interface>
      *  ...
      * @endcode
      *
      * @param IOPorts
      *        Port list.
      * @returns String representation.
      */
     std::string conduitsToString(const std::list<Process::Interface*> conduits) const throw();



  protected:

    int number_of_processes_;
    /**
     * List of in IOPorts.
     */
    std::list<Interface*> in_conduits_;

    /**
     * List of out IOPorts.
     */
    std::list<Interface*> out_conduits_;

};

}
}

#endif
