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
 * in and out interface definition and signal management.
 */
class Leaf : public Process{
  public:
    class Interface;

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
     * Destroys this leaf. This also destroys all interfaces and breaks all
     * interface connections.
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
     * Adds an in interface to this leaf. Leafs are not allowed to have
     * multiple in interfaces with the same ID.
     *
     * @param id
     *        Interface ID.
     * @returns \c true if such an in interface did not already exist and was
     *          successfully added.
     * @throws OutOfMemoryException
     *         When a interface cannot be added due to memory shortage.
     */
    bool addInPort(const ForSyDe::Id& id) throw(OutOfMemoryException);

    /**
     * Creates a new interface with the same ID and connections as another interface and
     * adds it as in interface to this leaf. The connections at the other interface are
     * broken. Leafs are not allowed to have multiple in interfaces with the same
     * ID.
     *
     * @param interface
     *        Interface.
     * @returns \c true if such an in interface did not already exist and was
     *          successfully copied and added.
     * @throws OutOfMemoryException
     *         When a interface cannot be added due to memory shortage.
     */
    bool addInPort(Interface& interface) throw(OutOfMemoryException);

    /**
     * Deletes and destroys an in interface to this leaf.
     *
     * @param id
     *        Interface ID.
     * @returns \c true if such a interface was found and successfully deleted.
     */
    bool deleteInPort(const ForSyDe::Id& id) throw();

    /**
     * Gets the number of in interfaces of this leaf.
     *
     * @returns Number of in interfaces.
     */
    size_t getNumInPorts() const throw();

    /**
     * Gets an in interface by ID belonging to this this leaf.
     *
     * @param id
     *        Interface id.
     * @returns Interface, if found; otherwise \c NULL.
     */
    Interface* getInPort(const ForSyDe::Id& id) throw();

    /**
     * Gets a list of in interfaces belonging to this this leaf.
     *
     * @returns List of in interfaces.
     */
    std::list<Interface*> getInPorts() throw();

    /**
     * Same as addIn Interface(const ForSyDe::Id&) but for out interfaces.
     *
     * @param id
     *        Interface ID.
     * @returns \c true if such a interface did not already exist and was
     *          successfully added.
     * @throws OutOfMemoryException
     *         When a interface cannot be added due to memory shortage.
     */
    bool addOutPort(const ForSyDe::Id& id) throw(OutOfMemoryException);

    /**
     * Same as addInPort(Interface&) but for out interfaces.
     *
     * @param interface
     *        Interface.
     * @returns \c true if such an out interface did not already exist and was
     *          successfully copied and added.
     * @throws OutOfMemoryException
     *         When a interface cannot be added due to memory shortage.
     */
    bool addOutPort(Interface& interface) throw(OutOfMemoryException);

    /**
     * Same as deleteOutPort(const ForSyDe::Id&) but for out interfaces.
     *
     * @param id
     *        Interface ID.
     * @returns \c true if such a interface was found and successfully deleted.
     */
    bool deleteOutPort(const ForSyDe::Id& id) throw();

    /**
     * Same as getNumInPorts() but for out interfaces.
     *
     * @returns Number of out interfaces.
     */
    size_t getNumOutPorts() const throw();

    /**
     * Same as getOutPort(const ForSyDe::Id&) but for out interfaces.
     *
     * @param id
     *        Interface ID.
     * @returns Interface, if found; otherwise \c NULL.
     */
    Interface* getOutPort(const ForSyDe::Id& id) throw();

    /**
     * Same as getInPorts() but for out interfaces.
     *
     * @returns List of out interfaces.
     */
    std::list<Interface*> getOutPorts() throw();


    /**
     * Converts this leaf into a string representation. The resultant string
     * is as follows:
     * @code
     * {
     *  LeafID: <leaf_id>,
     *  LeafType: <leaf_type>
     *  NumInPorts : <num_in_interfaces>
     *  InPorts = {...}
     *  NumOutPorts : <num_out_interfaces>
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
     * out interfaces.
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
     * @throws InvalidLeafException
     *         When the check fails.
     */
    virtual void moreChecks() throw(InvalidLeafException) = 0;

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
     * Attempts to find a interface with a given ID from a list of interfaces. If the list
     * is not empty and such a interface is found, an iterator pointing to that interface
     * is returned; otherwise the list's \c end() iterator is returned.
     *
     * @param id
     *        Interface ID.
     * @param interfaces
     *        List of interfaces.
     * @returns Iterator pointing either at the found interface, or an iterator equal
     *          to the list's \c end() iterator.
     */
    std::list<Interface*>::iterator findInterface(const ForSyDe::Id& id,
                                        std::list<Interface*>& interfaces) const throw();

    /**
     * Takes a list of interfaces and converts it into a string representation. Each
     * interface is converted into
     * @code
     *  InterfaceID: <interface_id>, not connected / connected to <leaf>:<interface>,
     *  ...
     * @endcode
     *
     * @param interfaces
     *        Interface list.
     * @returns String representation.
     */
    std::string interfacesToString(const std::list<Interface*> interfaces) const throw();

    /**
     * Destroys all interfaces in a given list.
     *
     * @param interfaces
     *        List of interfaces to destroy.
     */
    void destroyAllInterfaces(std::list<Interface*>& interfaces) throw();

  protected:

    /**
	 * Leaf ID.
	 */
	const ForSyDe::Id id_;
    /**
     * List of in interfaces.
     */
    std::list<Interface*> in_interfaces_;

    /**
     * List of out interfaces.
     */
    std::list<Interface*> out_interfaces_;

    /**
	 * Process MoC.
	 */
	const std::string moc_;

	int cost_;

  public:
    /**
     * @brief Class used for in- and out interfaces by the \c Leaf class.
     *
     * The \c Interface class defines a leaf interface. A interface is identified by an ID
     * and can be connected to another interface.
     */
    class Interface : public Port {
      public:
        /**
         * Creates a interface belonging to no leaf.
         *
         * @param id
         *        Interface ID.
         */
        Interface(const ForSyDe::Id& id) throw();

        /**
         * Creates a interface belonging to a leaf.
         *
         * @param id
         *        Interface ID.
         * @param leaf
         *        Pointer to the leaf to which this interface belongs.
         * @throws InvalidArgumentException
         *         When \c leaf is \c NULL.
         */
        Interface(const ForSyDe::Id& id, Leaf* leaf)
            throw(InvalidArgumentException);

        /**
         * Creates a interface belonging to no leaf with the same ID and
         * connections as another interface. The connection at the other interface is
         * broken.
         *
         * @param rhs
         *        Interface to copy.
         */
        explicit Interface(Interface& rhs) throw();

        /**
         * Creates a interface belonging to leaf with the same ID and
         * connections as another interface. The connection at the other interface is
         * broken.
         *
         * @param rhs
         *        Interface to copy.
         * @param leaf
         *        Pointer to the leaf to which this interface belongs.
         * @throws InvalidArgumentException
         *         When \c leaf is \c NULL.
         */
        explicit Interface(Interface& rhs, Leaf* leaf)
            throw(InvalidArgumentException);

        /**
         * Destroys this interface. This also breaks the connection, if any.
         */
        virtual ~Interface() throw();
        

        /**
         * Checks if this interface is connected to any other interface.
         *
         * @returns \c true if connected.
         */
        bool isConnected() const throw();

        /**
         * Connects this interface to another. This also sets the other interface as
         * connected to this interface. If there already is a connection it will
         * automatically be broken. 
         *
         * Setting the interface parameter to \c NULL is equivalent to breaking the
         * connection. If both ends of a connection is the same interface, this
         * method call is effectively ignored.
         *
         * @param interface
         *        Interface to connect.
         */
        void connect(Process::Port* port) throw();

        /**
         * Breaks the connection that this interface may have to another. If there is
         * no connection, nothing happens.
         */
        void unconnect() throw();

        /**
         * Gets the interface at the other end of the connection, if any.
         *
         * @returns Connected interface, if any; otherwise \c NULL.
         */
        Process::Port* getConnectedPort() const throw();

        /**
         * Checks for equality between this interface and another.
         *
         * @param rhs
         *        Interface to compare.
         * @returns \c true if both belong to the same leaf and if their IDs
         *          are identical.
         */
        bool operator==(const Process::Port& rhs) const throw();

        /**
         * Checks for inequality between this interface and another.
         *
         * @param rhs
         *        Interface to compare.
         * @returns \c true if the interfaces belong to different leafs or if
         *          their IDs are not identical.
         */
        bool operator!=(const Process::Port& rhs) const throw();

        /**
         * Converts this interface into a string representation. The resultant string
         * is as follows:
         * @code
         *  <leaf_id>:<interface_id>
         * @endcode
         *
         * @returns String representation.
         */
        std::string toString() const throw();

      private:
        /**
         * Due to how interface copying works, the assign operator is hidden and thus
         * not allowed to avoid potential bugs as it is easy to forget this
         * fact.
         */
        void operator=(const Interface&) throw();

      private:

        /**
         * Pointer to the other end of a connection.
         */
        Process::Port* connected_port_;
    };

  public:

};

}
}

#endif
