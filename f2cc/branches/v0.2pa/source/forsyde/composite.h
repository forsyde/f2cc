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
#include "../exceptions/invalidformatexception.h"
#include "../exceptions/invalidargumentexception.h"
#include <list>
#include <utility>


namespace f2cc {
namespace ForSyDe {


/**
 * @brief Base class for composite processes in the internal representation of ForSyDe
 * process networks.
 *
 * \c Composite is a base class for composite processes in internal representation
 * of ForSyDe process networks. It inherits both the \c ProcessNetwork class and the \Process class.
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
     *        Process ID.
     * @param parent
     *        its parent ID.
     * @param name
     *        the composite process' name. Initially it is the same as its filename, and it is enough
     *        to identify and compare a composite process' structure.
     */
    Composite(const ForSyDe::Id& id, ForSyDe::Hierarchy hierarchy,
    		ForSyDe::Id& name) throw();

    /**
     * Destroys this composite process. This also destroys all contained processes
     */
    virtual ~Composite() throw();

    /**
     * Gets the data type of this port.
     *
     * @returns Port data type.
     */
    const ForSyDe::Id* getName() const throw();

    void changeName(ForSyDe::Id* name) throw();

    /**
      * Adds an in port to this leaf. Leafs are not allowed to have
      * multiple in ports with the same ID.
      *
      * @param id
      *        IOPort ID.
      * @returns \c true if such an in port did not already exist and was
      *          successfully added.
      * @throws OutOfMemoryException
      *         When a port cannot be added due to memory shortage.
      */
     bool addInIOPort(const ForSyDe::Id& id) throw(OutOfMemoryException);

     /**
      * Deletes and destroys an in port to this leaf.
      *
      * @param id
      *        IOPort ID.
      * @returns \c true if such a port was found and successfully deleted.
      */
     bool deleteInIOPort(const ForSyDe::Id& id) throw();

     /**
      * Gets the number of in ports of this leaf.
      *
      * @returns Number of in ports.
      */
     size_t getNumInIOPorts() const throw();

     /**
      * Gets an in port by ID belonging to this this leaf.
      *
      * @param id
      *        IOPort id.
      * @returns IOPort, if found; otherwise \c NULL.
      */
     IOPort* getInIOPort(const ForSyDe::Id& id) throw();

     /**
      * Gets a list of in ports belonging to this this leaf.
      *
      * @returns List of in ports.
      */
     std::list<IOPort*> getInIOPorts() throw();

     /**
      * Same as addIn IOPort(const ForSyDe::Id&) but for out ports.
      *
      * @param id
      *        IOPort ID.
      * @returns \c true if such a port did not already exist and was
      *          successfully added.
      * @throws OutOfMemoryException
      *         When a port cannot be added due to memory shortage.
      */
     bool addOutIOPort(const ForSyDe::Id& id) throw(OutOfMemoryException);

     /**
      * Same as deleteOutIOPort(const ForSyDe::Id&) but for out ports.
      *
      * @param id
      *        IOPort ID.
      * @returns \c true if such a port was found and successfully deleted.
      */
     bool deleteOutIOPort(const ForSyDe::Id& id) throw();

     /**
      * Same as getNumInIOPorts() but for out ports.
      *
      * @returns Number of out ports.
      */
     size_t getNumOutIOPorts() const throw();

     /**
      * Same as getOutIOPort(const ForSyDe::Id&) but for out ports.
      *
      * @param id
      *        IOPort ID.
      * @returns IOPort, if found; otherwise \c NULL.
      */
     IOPort* getOutIOPort(const ForSyDe::Id& id) throw();

     /**
      * Same as getInIOPorts() but for out ports.
      *
      * @returns List of out ports.
      */
     std::list<IOPort*> getOutIOPorts() throw();

     /**
      * Converts this leaf into a string representation. The resultant string
      * is as follows:
      * @code
      * {
      *  LeafID: <leaf_id>,
      *  LeafType: <leaf_type>
      *  NumInPorts : <num_in_ports>
      *  InPorts = {...}
      *  NumOutPorts : <num_out_ports>
      *  OutPorts = {...}
      *  [aditional data]
      * }
      * @endcode
      * Derived classes may include additional data by overriding the
      * virtual moreToString() method.
      *
      * @returns String representation.
      * @see moreToString()
      */
     virtual std::string toString() const throw();

     /**
      * Checks whether this leaf is equal to another. Two leafs are equal
      * if they are of the same leaf type and have the same number of in and
      * out ports.
      *
      * @param rhs
      *        Leaf to compare with.
      * @returns \c true if both leafs are equal.
      */
     bool operator==(const Composite& rhs) const throw();

     /**
      * Same as operator==(Leaf&) but for inequality.
      *
      * @param rhs
      *        Leaf to compare with.
      * @returns \c true if both leafs are not equal.
      */
     bool operator!=(const Composite& rhs) const throw();

     /**
      * Gets the type of this leaf as a string.
      *
      * @returns Leaf type.
      */
     virtual std::string type() const throw();

   protected:
     /**
      * Performs leaf type-related checks on this leaf. This needs to be
      * derived by all subclasses.
      *
      * @throws InvalidProcessException
      *         When the check fails.
      */
     virtual void moreChecks() throw(InvalidProcessException);

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
     std::list<IOPort*>::iterator findPort(const ForSyDe::Id& id,
                                         std::list<IOPort*>& ports) const throw();

     /**
      * Takes a list of ports and converts it into a string representation. Each
      * port is converted into
      * @code
      *  PortID: <port_id>, not connected / connected to <leaf>:<port>,
      *  ...
      * @endcode
      *
      * @param ports
      *        Port list.
      * @returns String representation.
      */
     std::string portsToString(const std::list<IOPort*> ports) const throw();

     /**
      * Destroys all ports in a given list.
      *
      * @param ports
      *        List of ports to destroy.
      */
     void destroyAllIOPorts(std::list<IOPort*>& ports) throw();


  protected:
    /**
     * The composite process' name
     */
    const ForSyDe::Id composite_name_;
    /**
     * List of in ports.
     */
    std::list<IOPort*> in_ports_;

    /**
     * List of out ports.
     */
    std::list<IOPort*> out_ports_;


  public:
    /**
     * @brief Class used for in- and out ports by the \c Process class.
     *
     * The \c Port class defines a process port. A port is identified by an ID
     * and can be connected to another port.
     */
    class IOPort : public Interface{
      public:

        /**
         * Creates a port belonging to a process.
         *
         * @param id
         *        Port ID.
         * @param process
         *        Pointer to the process to which this port belongs.
         * @param datatype
         *        Data type contained by port.
         * @throws InvalidArgumentException
         *         When \c process is \c NULL.
         */
    	IOPort(const ForSyDe::Id& id, Composite* process)
            throw(InvalidArgumentException);


        /**
         * Destroys this port. This also breaks the connection, if any.
         */
        virtual ~IOPort() throw();


        /**
         * Checks if this \c IOPort is connected to a port outside the composite process.
         *
         * @returns \c true if connected.
         */
		bool isConnectedOutside() const throw();

        /**
         * Checks if this \c IOPort is connected to a port inside the composite process.
         *
         * @returns \c true if connected.
		 */

		bool isConnectedInside() const throw();

        /**
         * Checks if this \c IOPort is connected to a port outside the composite process.
         *
         * @returns \c true if connected.
         */
		bool isConnectedToLeafOutside() const throw();

        /**
         * Checks if this \c IOPort is connected to a port inside the composite process.
         *
         * @returns \c true if connected.
		 */

		bool isConnectedToLeaf(Interface* startpoint) const throw();

        /**
         * Checks if this \c IOPort is connected to a port inside the composite process.
         *
         * @returns \c true if connected.
		 */

		bool isConnectedToLeafInside() const throw();

        /**
         * Connects this port to another. This also sets the other port as
         * connected to this port. If there already is a connection it will
         * automatically be broken.
         *
         * If it connects to an IO port, it checks whether the connection
         * comes from the inside or outside, and acts accordingly.
         *
         * Setting the port parameter to \c NULL is equivalent to breaking the
         * connection. If both ends of a connection is the same port, this
         * method call is effectively ignored.
         *
         * @param port
         *        Port to connect.
         */
		void connect(Interface* port) throw(InvalidArgumentException);


        /**
         *
         * Breaks the connection that this port may have to another. If there is
         * no connection, nothing happens.
         */
		void unconnect(Interface* port) throw(InvalidArgumentException);

        /**
         * Breaks the connection that this port may have to another. If there is
         * no connection, nothing happens.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
		void unconnectOutside() throw(InvalidArgumentException);

        /**
         * Breaks the connection that this port may have to another. If there is
         * no connection, nothing happens.
         *
         * If the other end is an IO port, it checks whether the connection
         * comes from the inside or outside, and acts accordingly.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
		void unconnectInside() throw(InvalidArgumentException);

        /**
         * Breaks the connection that this port may have to another. If there is
         * no connection, nothing happens.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
		bool unconnectFromLeafOutside() throw(InvalidArgumentException);

        /**
         * Breaks the connection that this port may have to another. If there is
         * no connection, nothing happens.
         *
         * If the other end is an IO port, it checks whether the connection
         * comes from the inside or outside, and acts accordingly.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
		bool unconnectFromLeafInside() throw(InvalidArgumentException);

        /**
         * Searches recursively through composites and gets
         * the port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */

		Interface* getConnectedPortOutside() const throw();
        /**
         * Searches recursively through composites and gets
         * the port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
		Interface* getConnectedPortInside() const throw();

        /**
         * Gets the immediate adjacent port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
		Leaf::Port* getConnectedLeafPortOutside() const throw(InvalidArgumentException);
        /**
         * Gets the immediate adjacent port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
		Leaf::Port* getConnectedLeafPortInside() const throw(InvalidArgumentException);


        /**
         * Converts this port into a string representation. The resultant string
         * is as follows:
         * @code
         *  <process_id>:<port_id>
         * @endcode
         *
         * @returns String representation.
         */
        virtual std::string toString() const throw();

      private:
        /**
         * Due to how port copying works, the assign operator is hidden and thus
         * not allowed to avoid potential bugs as it is easy to forget this
         * fact.
         */
        void operator=(const Port&) throw();

      private:

        /**
         * Parent process.
         */
        //Composite* process_;

        /**
		 * In case it is an IO Port, this will contain a connection inside the
		 * composite process as well
		 */
        Process::Interface* connected_port_inside_;

        Process::Interface* connected_port_outside_;

    };
};

}
}

#endif
