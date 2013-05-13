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

#ifndef F2CC_SOURCE_FORSYDE_COMPOSITE_H_
#define F2CC_SOURCE_FORSYDE_COMPOSITE_H_

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
#include "model.h"
#include "process.h"
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
class Composite : public Model, public Process {
public:
	class IOPort;
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
    Composite(const Forsyde::Id& id, Forsyde::Hierarchy& hierarchy,
    		Forsyde::Id name) throw();

    /**
     * Destroys this composite process. This also destroys all contained processes
     */
    virtual ~Composite() throw();

    /**
     * Gets the name of this composite process, wrapped in an \c Id object.
     *
     * @returns Name of the composite process.
     */
    Forsyde::Id getName() const throw();

    /**
     * Changes the name of this composite process.
     *
     * @param name
     *        New name for the composite process, wrapped in an \c Id object.
     */
    void changeName(Forsyde::Id* name) throw();

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
     bool addInIOPort(const Forsyde::Id& id) throw(OutOfMemoryException);

     /**
      * Deletes and destroys an in IOPort to this composite.
      *
      * @param id
      *        IOPort ID.
      * @returns \b true if such a IOPort was found and successfully deleted.
      */
     bool deleteInIOPort(const Forsyde::Id& id) throw();

     /**
      * Gets the number of in IOPorts of this composite.
      *
      * @returns Number of in IOPorts.
      */
     size_t getNumInIOPorts() const throw();

     /**
      * Gets an in IOPort by ID belonging to this this composite.
      *
      * @param id
      *        IOPort id.
      * @returns IOPort, if found; otherwise \c NULL.
      */
     IOPort* getInIOPort(const Forsyde::Id& id) throw();

     /**
      * Gets a list of in IOPorts belonging to this this leaf.
      *
      * @returns List of in IOPorts.
      */
     std::list<IOPort*> getInIOPorts() throw();

     /**
      * Same as addInIOPort(const Forsyde::Id&) but for out IOPorts.
      *
      * @param id
      *        IOPort ID.
      * @returns \b true if such a IOPort did not already exist and was
      *          successfully added.
      * @throws OutOfMemoryException
      *         When a IOPort cannot be added due to memory shortage.
      */
     bool addOutIOPort(const Forsyde::Id& id) throw(OutOfMemoryException);

     /**
      * Same as deleteOutIOPort(const Forsyde::Id&) but for out IOPorts.
      *
      * @param id
      *        IOPort ID.
      * @returns \b true if such a IOPort was found and successfully deleted.
      */
     bool deleteOutIOPort(const Forsyde::Id& id) throw();

     /**
      * Same as getNumInIOPorts() but for out IOPorts.
      *
      * @returns Number of out IOPorts.
      */
     size_t getNumOutIOPorts() const throw();

     /**
      * Same as getOutIOPort(const Forsyde::Id&) but for out IOPorts.
      *
      * @param id
      *        IOPort ID.
      * @returns IOPort, if found; otherwise \c NULL.
      */
     IOPort* getOutIOPort(const Forsyde::Id& id) throw();

     /**
      * Same as getInIOPorts() but for out IOPorts.
      *
      * @returns List of out IOPorts.
      */
     std::list<IOPort*> getOutIOPorts() throw();

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
     virtual std::string toString() const throw();

     /**
      * Checks whether this composite is equal to another. Two composites are equal
      * if they have the same name (i.e. contain the same processes) and have
      * the same number of in and out IOPorts.
      *
      * @param rhs
      *        Composite to compare with.
      * @returns \b true if both composites are equal.
      */
     virtual bool operator==(const Composite& rhs) const throw();

     /**
      * Same as operator==(const Composite&) but for inequality.
      *
      * @param rhs
      *        Composite to compare with.
      * @returns \b true if both composites are not equal.
      */
     virtual bool operator!=(const Composite& rhs) const throw();

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
      * Attempts to find a IOPort with a given ID from a list of IOPorts. If the list
      * is not empty and such a IOPort is found, an iterator pointing to that IOPort
      * is returned; otherwise the list's \c end() iterator is returned.
      *
      * @param id
      *        Port ID.
      * @param IOPorts
      *        List of IOPorts.
      * @returns Iterator pointing either at the found IOPort, or an iterator equal
      *          to the list's \c end() iterator.
      */
     std::list<IOPort*>::iterator findPort(const Forsyde::Id& id,
                                         std::list<IOPort*>& IOPorts) const throw();

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
     std::string portsToString(const std::list<IOPort*> IOPorts) const throw();

     /**
      * Destroys all IOPorts in a given list.
      *
      * @param IOPorts
      *        List of IOPorts to destroy.
      */
     void destroyAllIOPorts(std::list<IOPort*>& IOPorts) throw();


  protected:
    /**
     * The composite process' name, wrapped inside an \c Id object.
     * It is different than its ID, since its purpose is to describe its content
     * rather than uniquely identify this process. Thus it is possible that many
     * composites have the same name, and this can be exploited.
     */
    Forsyde::Id composite_name_;
    /**
     * List of in IOPorts.
     */
    std::list<IOPort*> in_ports_;

    /**
     * List of out IOPorts.
     */
    std::list<IOPort*> out_ports_;


  public:
    /**
     * @brief Class used for in- and out IOPorts by the \c Composite class.
     *
     * The \c Port class defines a process IOPort. A IOPort is identified by an ID
     * and can be connected to another IOPort.
     */
    class IOPort : public Interface{
      public:

        /**
         * Creates a IOPort belonging to a process. This is the only way to
         * create an IOPort.
         *
         * @param id
         *        Port ID.
         * @param process
         *        Pointer to the process to which this IOPort belongs.
         * @throws InvalidArgumentException
         *         When \c process is \c NULL.
         */
    	IOPort(const Forsyde::Id& id, Composite* process)
            throw(InvalidArgumentException);


        /**
         * Destroys this IOPort. This also breaks the connection, if any.
         */
        virtual ~IOPort() throw();

        /**
		 * Gets the data type of this port.
		 *
		 * @return the associated data type.
		 */
        std::pair<f2cc::CDataType, f2cc::CDataType> getDataType() throw();

        /**
		 * Sets the data type of this port.
		 *
		 * @param datatype
		 *        The new data type that has to be set.
		 */
        void setDataType(bool outside, CDataType datatype) throw();


        /**
         * Checks if this \c IOPort is immediately connected to a \c Process::Interface
         * outside the composite process.
         *
         * @returns \b true if connected.
         */
		bool isConnectedOutside() const throw();

        /**
         * Checks if this \c IOPort is immediately connected to a \c Process::Interface
         * inside the composite process.
         *
         * @returns \b true if connected.
		 */
		bool isConnectedInside() const throw();

        /**
         * Function used for recursive search whether this \c IOPort is connected to a
         * \c Leaf::Port on a chosen side.
         *
         * @param startpoint
         *        The interface where the search starts. The check will be done in the
         *        opposite direction.
         *
         * @returns \b true if connected to a \c Leaf::Port.
         *
		 */
		bool isConnectedToLeaf(const Interface* startpoint) const throw();

        /**
         * Recursively checks whether this \c IOPort ends up in a \c Leaf::Port inside the
         * composite process.
         *
         * @returns \b true if connected.
		 */
		bool isConnectedToLeafInside() const throw();

        /**
         * Recursively checks whether this \c IOPort ends up in a \c Leaf::Port outside the
         * composite process.
         *
         * @returns \b true if connected.
         */
		bool isConnectedToLeafOutside() const throw();

        /**
         * Connects this IOPort to a \c Process::Interface. This also sets the
         * other Interface as connected to this IOPort. If there already is a
         * connection it will automatically be broken.
         *
         * If it connects to an IO IOPort, it checks whether the connection
         * comes from the inside or outside, and acts accordingly.
         *
         * Setting the IOPort parameter to \c NULL is equivalent to breaking the
         * connection. If both ends of a connection is the same IOPort, this
         * method call is effectively ignored.
         *
         * @param interface
         *        Port to connect.
         *
         * @throws IllegalStateException
         *         When the port does not contain a process. A connection without
         *         knowing the hierarchical path is not possible.
         * @throws InvalidArgumentException
         *         When the given interface is out of this \c IOPort's scope of vision.
         * @throws CastException
         *         When the given interface is not \c Leaf::Port or \c Composite::IOPort.
         */
		void connect(Interface* interface) throw(IllegalStateException,
				InvalidArgumentException, CastException);

        /**
         * Breaks the connection between this \c IOPort and the port passed as argument
         *
         * @param interface
         *        Port to unconnect from.
         *
         * @throws InvalidArgumentException
         *         When the given interface is not actually connected to this \c IOPort.
         */
		void unconnect(Interface* interface) throw(InvalidArgumentException);

        /**
         * Breaks the connection that this IOPort may have to another. If there is
         * no connection, nothing happens.
         *
         * @throws IllegalStateException
         *         When the port does not contain a process. A connection without
         *         knowing the hierarchical path is not possible. Also, if by any
         *         means, the outside connection points to an interface outside
         *         the scope of vision for this \c IOPort.
         * @throws CastException
         *         When the given interface is not \c Leaf::Port or \c Composite::IOPort.
         */
		void unconnectOutside() throw(IllegalStateException, CastException);

        /**
         * Breaks the connection that this IOPort may have to another. If there is
         * no connection, nothing happens.
         *
         * @throws IllegalStateException
         *         When the port does not contain a process. A connection without
         *         knowing the hierarchical path is not possible. Also, if by any
         *         means, the outside connection points to an interface outside
         *         the scope of vision for this \c IOPort.
		 *
		 * @return the associated data type.
         * @throws CastException
         *         When the given interface is not \c Leaf::Port or \c Composite::IOPort.
         */
		void unconnectInside() throw(IllegalStateException, CastException);

        /**
         * Recursively unconnects this \c IOPort until it finds the first \c Leaf::Port.
         * The unconnect direction is determined by the start \c Interface.
         *
         * @param previous
         *        The previous interface that was disconnected. the unconnect will be
         *        done in the opposite direction.
         *
         * @returns \b true if it was successfully unconnected.
		 */
		bool unconnectFromLeaf(Interface* previous) throw();

        /**
         * Recursively unconnects this \c IOPort until it finds the first \c Leaf::Port,
         * outside its parent composite process.
         *
         * @returns \b true if the connection was successfully broken.
         */
		bool unconnectFromLeafOutside() throw();

        /**
         * Recursively unconnects this \c IOPort until it finds the first \c Leaf::Port,
         * inside its parent composite process.
         *
         * @returns \b true if the connection was successfully broken.
         */
		bool unconnectFromLeafInside() throw();

        /**
         * Gets the immediate adjacent \c Process::Interface outside
         * this \c IOPort's parent process.
         *
         * @returns Connected interface, if any; otherwise \c NULL.
         */
		Interface* getConnectedPortOutside() const throw();

        /**
         * Gets the immediate adjacent \c Process::Interface inside
         * this \c IOPort's parent process.
         *
         * @returns Connected interface, if any; otherwise \c NULL.
         */
		Interface* getConnectedPortInside() const throw();

        /**
         * Searches recursively through composites and gets
         * the \c Leaf::Port at the other end of the connection, if any.
         *
         * @returns Connected \c Leaf::Port, if any; otherwise \c NULL.
         *
		 * @throws CastException
         *         When the interface pointed outside is not \c Leaf::Port nor
         *         \c Composite::IOPort.
         */
		Leaf::Port* getConnectedLeafPortOutside() const throw(CastException);

		/**
         * Gets the immediate adjacent IOPort at the other end of the connection, if any.
         *
         * @returns Connected IOPort, if any; otherwise \c NULL.
         *
		 * @throws CastException
         *         When the interface pointed inside is not \c Leaf::Port nor
         *         \c Composite::IOPort.
         */
		Leaf::Port* getConnectedLeafPortInside() const throw(CastException);


      private:
        /**
         * Adds the following marker to the caller's string representation
         * is as follows:
         * @code
         *  (I/O)
         * @endcode
         *
         * @returns String representation.
         */
        virtual std::string moretoString() const throw();

        /**
         * Due to how IOPort copying works, the assign operator is hidden and thus
         * not allowed to avoid potential bugs as it is easy to forget this
         * fact.
         */
        void operator=(const Port&) throw();

      private:

        /**
		 * The connected interface inside the composite process
		 */
        Process::Interface* connected_port_inside_;

        /**
		 * The connected interface outside the composite process
		 */
        Process::Interface* connected_port_outside_;

        /**
		 * Port data type.
		 */
		std::pair<CDataType, CDataType> data_types_;

    };
};

}
}

#endif
