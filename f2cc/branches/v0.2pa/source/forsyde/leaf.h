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

#ifndef F2CC_SOURCE_FORSYDE_LEAF_H_
#define F2CC_SOURCE_FORSYDE_LEAF_H_

/**
 * @file
 * @author  George Ungureanu <ugeorge@kth.se> & Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.2
 *
 * @brief Defines the base class for leaf nodes in the internal
 *        representation of ForSyDe models.
 */

#include "id.h"
#include "hierarchy.h"
#include "process.h"
#include "../language/cdatatype.h"
#include "../language/cvariable.h"
#include "../exceptions/outofmemoryexception.h"
#include "../exceptions/notsupportedexception.h"
#include "../exceptions/invalidprocessexception.h"
#include "../exceptions/invalidformatexception.h"
#include "../exceptions/invalidargumentexception.h"
#include "../exceptions/illegalcallexception.h"
#include <list>

namespace f2cc {
namespace Forsyde {

/**
 * @brief Base class for leaf process nodes in the internal representation of ForSyDe
 * models.
 *
 * The \c Leaf is a base class for leaf process nodes in internal representation
 * of ForSyDe models. It provides functionality common for all leaf processes such as
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
    Leaf(const Forsyde::Id& id) throw(RuntimeException);

    /**
     * Creates a leaf node, with the information containers initialized.
     *
     * @param id
     *        Leaf ID.
     * @param hierarchy
     *        Leaf hierarchy.
     * @param moc
     *        Leaf MoC.
     * @param cost
     *        Leaf cost parameter.
     */
    Leaf(const Forsyde::Id& id, Forsyde::Hierarchy hierarchy,
    		const std::string moc, int cost) throw(RuntimeException);

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
    const std::string getMoc() const throw(RuntimeException);

    /**
     * Gets the cost for this process.
     *
     * @returns The cost parameter.
     */
    int getCost() const throw(RuntimeException);

    /**
     * Sets the cost for this process.
     *
     * @param cost
     *        The cost parameter.
     */
    void setCost(int& cost) throw(RuntimeException);

    /**
     * Adds an in port to this leaf. Leafs are not allowed to have
     * multiple in ports with the same ID.
     *
     * @param id
     *        Port ID.
     * @returns \b true if such an in port did not already exist and was
     *          successfully added.
     * @throws OutOfMemoryException
     *         When a port cannot be added due to memory shortage.
     */
    bool addInPort(const Forsyde::Id& id) throw(RuntimeException, OutOfMemoryException);

    /**
     * Adds an in port to this leaf, with a CDataType container. Leafs are not
     * allowed to have multiple in ports with the same ID.
     *
     * @param id
     *        Port ID.
     * @param datatype
     *        Data type associated with this port.
     *
     * @returns \b true if such an in port did not already exist and was
     *          successfully added.
     * @throws OutOfMemoryException
     *         When a port cannot be added due to memory shortage.
     */
    bool addInPort(const Forsyde::Id& id, CDataType datatype) throw(RuntimeException, OutOfMemoryException);

    /**
     * Creates a new port with the same ID and connections as another port and
     * adds it as in port to this leaf. The connections at the other port are
     * broken. Leafs are not allowed to have multiple in ports with the same
     * ID.
     *
     * \b WARNING: it is not advisable to use this constructor in the v0.2 execution path
     * since the port connections cannot be maintained. A port can be connected to another
     * only in its scope of vision, fact determined only in the presence of a process that
     * has a hierarchy.
     *
     * @param port
     *        Port.
     * @returns \b true if such an in port did not already exist and was
     *          successfully copied and added.
     * @throws OutOfMemoryException
     *         When a port cannot be added due to memory shortage.
     */
    bool addInPort(Port& port) throw(RuntimeException, OutOfMemoryException);

    /**
     * Deletes and destroys an in port to this leaf.
     *
     * @param id
     *        Port ID.
     * @returns \b true if such a port was found and successfully deleted.
     */
    bool deleteInPort(const Forsyde::Id& id) throw(RuntimeException);

    /**
     * Gets the number of in ports of this leaf.
     *
     * @returns Number of in ports.
     */
    size_t getNumInPorts() const throw(RuntimeException);

    /**
     * Gets an in port by ID belonging to this this leaf.
     *
     * @param id
     *        Port id.
     * @returns Port, if found; otherwise \c NULL.
     */
    Port* getInPort(const Forsyde::Id& id) throw(RuntimeException);

    /**
     * Gets a list of in ports belonging to this this leaf.
     *
     * @returns List of in ports.
     */
    std::list<Port*> getInPorts() throw(RuntimeException);

    /**
     * Same as addIn Port(const Forsyde::Id&) but for out ports.
     *
     * @param id
     *        Port ID.
     * @returns \b true if such a port did not already exist and was
     *          successfully added.
     * @throws OutOfMemoryException
     *         When a port cannot be added due to memory shortage.
     */
    bool addOutPort(const Forsyde::Id& id) throw(RuntimeException, OutOfMemoryException);

    /**
     * Adds an out port to this leaf, with a CDataType container. Leafs are not
     * allowed to have multiple in ports with the same ID.
     *
     * @param id
     *        Port ID.
     * @param datatype
     *        Data type associated with this port.
     *
     * @returns \b true if such an in port did not already exist and was
     *          successfully added.
     * @throws OutOfMemoryException
     *         When a port cannot be added due to memory shortage.
     */
    bool addOutPort(const Forsyde::Id& id, CDataType datatype) throw(RuntimeException, OutOfMemoryException);

    /**
     * Same as addInPort(Port&) but for out ports.
     *
     * @param port
     *        Port.
     * @returns \b true if such an out port did not already exist and was
     *          successfully copied and added.
     * @throws OutOfMemoryException
     *         When a port cannot be added due to memory shortage.
     */
    bool addOutPort(Port& port) throw(RuntimeException, OutOfMemoryException);

    /**
     * Same as deleteOutPort(const Forsyde::Id&) but for out ports.
     *
     * @param id
     *        Port ID.
     * @returns \b true if such a port was found and successfully deleted.
     */
    bool deleteOutPort(const Forsyde::Id& id) throw(RuntimeException);

    /**
     * Same as getNumInPorts() but for out ports.
     *
     * @returns Number of out ports.
     */
    size_t getNumOutPorts() const throw(RuntimeException);

    /**
     * Same as getOutPort(const Forsyde::Id&) but for out ports.
     *
     * @param id
     *        Port ID.
     * @returns Port, if found; otherwise \c NULL.
     */
    Port* getOutPort(const Forsyde::Id& id) throw(RuntimeException);

    /**
     * Same as getInPorts() but for out ports.
     *
     * @returns List of out ports.
     */
    std::list<Port*> getOutPorts() throw(RuntimeException);


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
    virtual std::string toString() const throw(RuntimeException);

    /**
     * Checks whether this leaf is equal to another. Two leafs are equal
     * if they are of the same leaf type and have the same number of in and
     * out ports.
     *
     * @param rhs
     *        Leaf to compare with.
     * @returns \b true if both leafs are equal.
     */
    virtual bool operator==(const Leaf& rhs) const throw(RuntimeException);

    /**
     * Same as operator==(Leaf&) but for inequality.
     *
     * @param rhs
     *        Leaf to compare with.
     * @returns \b true if both leafs are not equal.
     */
    virtual bool operator!=(const Leaf& rhs) const throw(RuntimeException);

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
    virtual std::string moreToString() const throw(RuntimeException);

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
    std::list<Port*>::iterator findPort(const Forsyde::Id& id,
                                        std::list<Port*>& ports) const throw(RuntimeException);

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
    std::string portsToString(const std::list<Port*> ports) const throw(RuntimeException);

    /**
     * Destroys all ports in a given list.
     *
     * @param ports
     *        List of ports to destroy.
     */
    void destroyAllPorts(std::list<Port*>& ports) throw(RuntimeException);

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

    /**
	 * Process cost parameter.
	 */
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
         * \b WARNING: using this constructor is not advised in v0.2 execution path.
         * A newly created port without process cannot be used in a hierarchical process
         * network.
         *
         * @param id
         *        Port ID.
         */
        Port(const Forsyde::Id& id) throw(RuntimeException);

        /**
         * Creates a port belonging to a leaf.
         *
         * @param id
         *        Port ID.
         * @param leaf
         *        Pointer to the leaf which this port belongs to.
         * @throws InvalidArgumentException
         *         When \c leaf is \c NULL.
         */
        Port(const Forsyde::Id& id, Leaf* leaf)
            throw(RuntimeException, InvalidArgumentException);


        /**
         * Creates a port belonging to a leaf, and having an associated data type.
         *
         * @param id
         *        Port ID.
         * @param data_type
         *        Associated data type.
         * @param leaf
         *        Pointer to the leaf to which this port belongs.
         * @throws InvalidArgumentException
         *         When \c leaf is \c NULL.
         */
        Port(const Forsyde::Id& id, Leaf* leaf, CDataType data_type)
            throw(RuntimeException, InvalidArgumentException);

        /**
         * Creates a port belonging to no leaf with the same ID and
         * connections as another port. The connection at the other port is
         * broken.
         *
         * \b WARNING: it is not advisable to use this constructor in the v0.2 execution path
		 * since the port connections cannot be maintained. A port can be connected to another
		 * only in its scope of vision, fact determined only in the presence of a process that
		 * has a hierarchy.
         *
         * @param rhs
         *        Port to copy.
         */
        explicit Port(Port& rhs) throw(RuntimeException);

        /**
         * Creates a port belonging to leaf with the same ID and
         * connections as another port. The connection at the other port is
         * broken.
         *
         *
         * @param rhs
         *        Port to copy.
         * @param leaf
         *        Pointer to the leaf which this port belongs to.
         * @throws InvalidArgumentException
         *         When \c leaf is \c NULL.
         */
        explicit Port(Port& rhs, Leaf* leaf)
            throw(RuntimeException, InvalidArgumentException);

        /**
         * Destroys this port. This also breaks the connection, if any.
         */
        virtual ~Port() throw();
        
        /**
		 * Gets the data type of this port.
		 *
		 * @return the associated data type.
		 */
        CDataType getDataType() throw(RuntimeException);

        /**
		 * Sets the data type of this port.
		 *
		 * @param datatype
		 *        The new data type that has to be set.
		 */
        void setDataType(CDataType datatype) throw(RuntimeException);

        /**
		 * Gets the link to the variable in the \c CFunction which this port
		 * represents.
		 *
		 * @return pointer to associated variable.
		 */
        CVariable* getVariable() throw(RuntimeException);

        /**
		 * Sets the link to the variable in the \c CFunction which this port
		 * represents.
		 *
		 * @param variable
		 *        A valid variable in a \c CFunction.
		 */
        void setVariable(CVariable* variable) throw(RuntimeException);

        /**
         * Checks if this port is connected to any other interface.
         *
         * @returns \b true if connected.
         */
        bool isConnected() const throw(RuntimeException);

        /**
         * Recursively checks whether this ports eventually connects to another
         * \c Leaf::Port.
         *
         * @returns \b true if connected.
         */
        bool isConnectedToLeaf() const throw(RuntimeException, IllegalStateException);

        /**
         * Connects this port to a interface in the process' scope.
         * This also sets the interface port as connected to this port (either
         * inside or outside). If there already is a connection it will
         * automatically be broken. 
         *
         * Setting the port parameter to \c NULL is equivalent to breaking the
         * connection. If both ends of a connection is the same port, this
         * method call is effectively ignored.
         *
         * @param port
         *        Interface to connect.
         *
         * @returns \b true if connection was successful.
         */
        void connect(Process::Interface* port) throw(RuntimeException, InvalidArgumentException,IllegalCallException);

        /**
         * Breaks the connection that this port may have to another. If there is
         * no connection, nothing happens.
         */
        void unconnect() throw(RuntimeException);

        /**
         * Recursively breaks all connections until it reaches a \c Leaf::Port.
         */
        void unconnectFromLeaf() throw(RuntimeException);

        /**
         * Gets the port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         */
        Process::Interface* getConnectedPort() const throw(RuntimeException);

        /**
         * Sets the connected port. Unlike the connect(Process::Interface*) method,
         * this one does not check for the validity of the given port. It is not advisable
         * to use this method from outside f2cc::Forsyde namespace.
         *
         * @param port
         *        Interface to connect.
         */
        void setConnection(Process::Interface* port) throw(RuntimeException);

        //Leaf::Port* getConnectedLeafPort() const throw(RuntimeException);

        /**
         * Checks for equality between this port and another.
         *
         * @param rhs
         *        Port to compare.
         * @returns \b true if both belong to the same leaf and if their IDs
         *          are identical.
         */
        bool operator==(const Leaf::Port& rhs) const throw(RuntimeException);

        /**
         * Checks for inequality between this port and another.
         *
         * @param rhs
         *        Port to compare.
         * @returns \b true if the ports belong to different leafs or if
         *          their IDs are not identical.
         */
        bool operator!=(const Leaf::Port& rhs) const throw(RuntimeException);

      private:

        /**
         * Converts this port into a string representation. The resultant string
         * is as follows:
         * @code
         *  <leaf_id>:<port_id>
         * @endcode
         *
         * @returns String representation.
         */
        virtual std::string moretoString() const throw(RuntimeException);

        /**
         * Due to how port copying works, the assign operator is hidden and thus
         * not allowed to avoid potential bugs as it is easy to forget this
         * fact.
         */
        void operator=(const Port&) throw(RuntimeException);

      private:
        /**
         * Pointer to the other end of a connection.
         */
        Process::Interface* connected_port_;

        /**
		 * Port data type.
		 */
		CDataType data_type_;

        /**
		 * Pointer to a variable inside a \c CFunction.
		 */
		CVariable* variable_;

    };

  public:

};

}
}

#endif
