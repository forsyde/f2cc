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

#ifndef F2CC_SOURCE_FORSYDE_LEAF_H_
#define F2CC_SOURCE_FORSYDE_LEAF_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines the base class for leaf nodes in the internal
 *        representation of ForSyDe models.
 */

#include "id.h"
#include "hierarchy.h"
#include "process.h"
#include "../language/cdatatype.h"
#include "../exceptions/outofmemoryexception.h"
#include "../exceptions/notsupportedexception.h"
#include "../exceptions/invalidprocessexception.h"
#include "../exceptions/invalidformatexception.h"
#include "../exceptions/invalidargumentexception.h"
#include <list>

namespace f2cc {
namespace ForSyDe {

/**
 * @brief Base class for leaf nodes in the internal representation of ForSyDe
 * models.
 *
 * The \c Leaf is a base class for leaf nodes in internal representation
 * of ForSyDe models. It provides functionality common for all leafs such as
 * in and out port definition and signal management.
 */
class Leaf : public Process{
  public:
    class Port;

  public:
    /**
     * Creates a leaf node.
     *
     * @param id
     *        Leaf ID.
     */
    Leaf(const ForSyDe::Id& id) throw();

    Leaf(const ForSyDe::Id& id, ForSyDe::Hierarchy hierarchy,
    		const std::string moc, int cost) throw();

    /**
     * Destroys this leaf. This also destroys all ports and breaks all
     * port connections.
     */
    virtual ~Leaf() throw();

    /**
     * Gets the MoC of this process.
     *
     * @returns The MoC.
     */
    const std::string getMoc() const throw();

    /**
     * Gets the MoC of this process.
     *
     * @returns The MoC.
     */
    int getCost() const throw();

    void setCost(int& cost) throw();

    /**
     * Adds an in port to this leaf. Leafs are not allowed to have
     * multiple in ports with the same ID.
     *
     * @param id
     *        Port ID.
     * @returns \c true if such an in port did not already exist and was
     *          successfully added.
     * @throws OutOfMemoryException
     *         When a port cannot be added due to memory shortage.
     */
    bool addInPort(const ForSyDe::Id& id) throw(OutOfMemoryException);

    /**
     * Creates a new port with the same ID and connections as another port and
     * adds it as in port to this leaf. The connections at the other port are
     * broken. Leafs are not allowed to have multiple in ports with the same
     * ID.
     *
     * @param port
     *        Port.
     * @returns \c true if such an in port did not already exist and was
     *          successfully copied and added.
     * @throws OutOfMemoryException
     *         When a port cannot be added due to memory shortage.
     */
    bool addInPort(Port& port) throw(OutOfMemoryException);

    /**
     * Deletes and destroys an in port to this leaf.
     *
     * @param id
     *        Port ID.
     * @returns \c true if such a port was found and successfully deleted.
     */
    bool deleteInPort(const ForSyDe::Id& id) throw();

    /**
     * Gets the number of in ports of this leaf.
     *
     * @returns Number of in ports.
     */
    size_t getNumInPorts() const throw();

    /**
     * Gets an in port by ID belonging to this this leaf.
     *
     * @param id
     *        Port id.
     * @returns Port, if found; otherwise \c NULL.
     */
    Port* getInPort(const ForSyDe::Id& id) throw();

    /**
     * Gets a list of in ports belonging to this this leaf.
     *
     * @returns List of in ports.
     */
    std::list<Port*> getInPorts() throw();

    /**
     * Same as addIn Port(const ForSyDe::Id&) but for out ports.
     *
     * @param id
     *        Port ID.
     * @returns \c true if such a port did not already exist and was
     *          successfully added.
     * @throws OutOfMemoryException
     *         When a port cannot be added due to memory shortage.
     */
    bool addOutPort(const ForSyDe::Id& id) throw(OutOfMemoryException);

    /**
     * Same as addInPort(Port&) but for out ports.
     *
     * @param port
     *        Port.
     * @returns \c true if such an out port did not already exist and was
     *          successfully copied and added.
     * @throws OutOfMemoryException
     *         When a port cannot be added due to memory shortage.
     */
    bool addOutPort(Port& port) throw(OutOfMemoryException);

    /**
     * Same as deleteOutPort(const ForSyDe::Id&) but for out ports.
     *
     * @param id
     *        Port ID.
     * @returns \c true if such a port was found and successfully deleted.
     */
    bool deleteOutPort(const ForSyDe::Id& id) throw();

    /**
     * Same as getNumInPorts() but for out ports.
     *
     * @returns Number of out ports.
     */
    size_t getNumOutPorts() const throw();

    /**
     * Same as getOutPort(const ForSyDe::Id&) but for out ports.
     *
     * @param id
     *        Port ID.
     * @returns Port, if found; otherwise \c NULL.
     */
    Port* getOutPort(const ForSyDe::Id& id) throw();

    /**
     * Same as getInPorts() but for out ports.
     *
     * @returns List of out ports.
     */
    std::list<Port*> getOutPorts() throw();


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
    virtual bool operator==(const Leaf& rhs) const throw();

    /**
     * Same as operator==(Leaf&) but for inequality.
     *
     * @param rhs
     *        Leaf to compare with.
     * @returns \c true if both leafs are not equal.
     */
    virtual bool operator!=(const Leaf& rhs) const throw();

    /**
     * Gets the type of this leaf as a string.
     *
     * @returns Leaf type.
     */
    virtual std::string type() const throw() = 0;

  protected:
    /**
     * Performs leaf type-related checks on this leaf. This needs to be
     * derived by all subclasses.
     *
     * @throws InvalidProcessException
     *         When the check fails.
     */
    virtual void moreChecks() throw(InvalidProcessException) = 0;

    /**
     * Additional string output to be included when this leaf is converted to
     * a string representation. By default this returns an empty string.
     *
     * @returns Additional string representation data.
     * @see toString()
     */
    virtual std::string moreToString() const throw();

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
    std::list<Port*>::iterator findPort(const ForSyDe::Id& id,
                                        std::list<Port*>& ports) const throw();

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
    std::string portsToString(const std::list<Port*> ports) const throw();

    /**
     * Destroys all ports in a given list.
     *
     * @param ports
     *        List of ports to destroy.
     */
    void destroyAllPorts(std::list<Port*>& ports) throw();

  protected:
    /**
     * List of in ports.
     */
    std::list<Port*> in_ports_;

    /**
     * List of out ports.
     */
    std::list<Port*> out_ports_;

    /**
	 * Process MoC.
	 */
	const std::string moc_;

	int cost_;

  public:
    /**
     * @brief Class used for in- and out ports by the \c Leaf class.
     *
     * The \c Port class defines a leaf port. A port is identified by an ID
     * and can be connected to another port.
     */
    class Port : public Interface {
      public:
        /**
         * Creates a port belonging to no leaf.
         *
         * @param id
         *        Port ID.
         */
        Port(const ForSyDe::Id& id) throw();

        /**
         * Creates a port belonging to a leaf.
         *
         * @param id
         *        Port ID.
         * @param leaf
         *        Pointer to the leaf to which this port belongs.
         * @throws InvalidArgumentException
         *         When \c leaf is \c NULL.
         */
        Port(const ForSyDe::Id& id, Leaf* leaf)
            throw(InvalidArgumentException);


        /**
         * Creates a port belonging to a leaf.
         *
         * @param id
         *        Port ID.
         * @param leaf
         *        Pointer to the leaf to which this port belongs.
         * @throws InvalidArgumentException
         *         When \c leaf is \c NULL.
         */
        Port(const ForSyDe::Id& id, Leaf* leaf, CDataType data_type)
            throw(InvalidArgumentException);

        /**
         * Creates a port belonging to no leaf with the same ID and
         * connections as another port. The connection at the other port is
         * broken.
         *
         * @param rhs
         *        Port to copy.
         */
        explicit Port(Port& rhs) throw();

        /**
         * Creates a port belonging to leaf with the same ID and
         * connections as another port. The connection at the other port is
         * broken.
         *
         * @param rhs
         *        Port to copy.
         * @param leaf
         *        Pointer to the leaf to which this port belongs.
         * @throws InvalidArgumentException
         *         When \c leaf is \c NULL.
         */
        explicit Port(Port& rhs, Leaf* leaf)
            throw(InvalidArgumentException);

        /**
         * Destroys this port. This also breaks the connection, if any.
         */
        virtual ~Port() throw();
        
        /**
		 * Sets the data type of this port.
		 *
		 * @param datatype
		 *        The new data type that has to be set.
		 */
        CDataType getDataType() throw();

        /**
		 * Sets the data type of this port.
		 *
		 * @param datatype
		 *        The new data type that has to be set.
		 */
        void setDataType(CDataType datatype) throw();

        /**
         * Checks if this port is connected to any other port.
         *
         * @returns \c true if connected.
         */
        bool isConnected() const throw();

        /**
         * Checks if this port is connected to any other port.
         *
         * @returns \c true if connected.
         */
        bool isConnectedToLeaf() const throw(IllegalStateException);

        /**
         * Connects this port to another. This also sets the other port as
         * connected to this port. If there already is a connection it will
         * automatically be broken. 
         *
         * Setting the port parameter to \c NULL is equivalent to breaking the
         * connection. If both ends of a connection is the same port, this
         * method call is effectively ignored.
         *
         * @param port
         *        Port to connect.
         */
        void connect(Process::Interface* port) throw(InvalidArgumentException);

        /**
         * Breaks the connection that this port may have to another. If there is
         * no connection, nothing happens.
         */
        void unconnect() throw();

        void unconnectFromLeaf() throw();

        /**
         * Gets the port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         */
        Process::Interface* getConnectedPort() const throw();

        void setConnection(Process::Interface* port) const throw();

        Leaf::Port* getConnectedLeafPort() const throw();

        /**
         * Checks for equality between this port and another.
         *
         * @param rhs
         *        Port to compare.
         * @returns \c true if both belong to the same leaf and if their IDs
         *          are identical.
         */
        bool operator==(const Leaf::Port& rhs) const throw();

        /**
         * Checks for inequality between this port and another.
         *
         * @param rhs
         *        Port to compare.
         * @returns \c true if the ports belong to different leafs or if
         *          their IDs are not identical.
         */
        bool operator!=(const Leaf::Port& rhs) const throw();

        /**
         * Converts this port into a string representation. The resultant string
         * is as follows:
         * @code
         *  <leaf_id>:<port_id>
         * @endcode
         *
         * @returns String representation.
         */
        std::string toString() const throw();

      private:
        /**
         * Due to how port copying works, the assign operator is hidden and thus
         * not allowed to avoid potential bugs as it is easy to forget this
         * fact.
         */
        void operator=(const Port&) throw();

      private:
        /**
         * Pointer to the other end of a connection.
         */
        Process::Interface* connected_port_;
        /**
		 * Port data type.
		 */
		CDataType data_type_;

    };

  public:

};

}
}

#endif
