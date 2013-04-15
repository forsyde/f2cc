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

#ifndef F2CC_SOURCE_FORSYDE_PROCESS_H_
#define F2CC_SOURCE_FORSYDE_PROCESS_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se> & George Ungureanu <ugeorge@kth.se>
 * @version 0.2
 *
 * @brief Defines the base class for process nodes in the internal
 *        representation of ForSyDe processnetworks.
 */

#include "id.h"
#include "../language/cdatatype.h"
#include "../exceptions/outofmemoryexception.h"
#include "../exceptions/notsupportedexception.h"
#include "../exceptions/invalidprocessexception.h"
#include "../exceptions/invalidformatexception.h"
#include "../exceptions/invalidargumentexception.h"
#include "../exceptions/illegalcallexception.h"
#include <list>
#include <utility>

namespace f2cc {
namespace ForSyDe {

/**
 * @brief Base class for process nodes in the internal representation of ForSyDe
 * models.
 *
 * The \c Process is a base class for process nodes in internal representation
 * of ForSyDe models. It provides functionality common for all processes such as
 * in and out port definition and signal management.
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
    Process(const Id& id, const Id& parent, const std::string moc) throw();

    /**
     * Destroys this process. This also destroys all ports and breaks all
     * port connections.
     */
    virtual ~Process() throw();

    /**
     * Gets the ID of this process.
     *
     * @returns Process ID.
     */
    const ForSyDe::Id* getId() const throw();

    /**
     * Gets the parent of this process.
     *
     * @returns The parent process.
     */
    const ForSyDe::Id* getParent() const throw();

    /**
     * Gets the MoC of this process.
     *
     * @returns The MoC.
     */
    const std::string getMoc() const throw();

    /**
     * Checks whether this port is a composite process.
     *
     * @returns \c true if it a composite process.
     */
    bool isComposite() const throw();

    /**
     * Adds an in port to this process. Processes are not allowed to have
     * multiple in ports with the same ID.
     *
     * @param id
     *        Port ID.
     * @returns \c true if such an in port did not already exist and was
     *          successfully added.
     * @throws OutOfMemoryException
     *         When a port cannot be added due to memory shortage.
     */
    bool addInPort(const ForSyDe::Id& id, const CDataType datatype) throw(OutOfMemoryException);

    /**
     * Creates a new port with the same ID and connections as another port and
     * adds it as in port to this process. The connections at the other port are
     * broken. Processes are not allowed to have multiple in ports with the same
     * ID.
     *
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
     * Deletes and destroys an in port to this process.
     *
     * @param id
     *        Port ID.
     * @returns \c true if such a port was found and successfully deleted.
     */
    bool deleteInPort(const ForSyDe::Id& id) throw();

    /**
     * Gets the number of in ports of this process.
     *
     * @returns Number of in ports.
     */
    size_t getNumInPorts() const throw();

    /**
     * Gets an in port by ID belonging to this this process.
     *
     * @param id
     *        Port id.
     * @returns Port, if found; otherwise \c NULL.
     */
    Port* getInPort(const ForSyDe::Id& id) throw();

    /**
     * Gets a list of in ports belonging to this this process.
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
    bool addOutPort(const ForSyDe::Id& id, const CDataType datatype) throw(OutOfMemoryException);

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
     * Checks that this process is valid. This does nothing except invoke the
     * purely virtual method moreChecks() for process type-related checks.
     *
     * @throws InvalidProcessException
     *         When the check fails.
     */
    void check() throw(InvalidProcessException);


    /**
     * Converts this process into a string representation. The resultant string
     * is as follows:
     * @code
     * {
     *  ProcessID: <process_id>,
     *  ProcessType: <process_type>
     *  MoC: <moc>
     *  Parent: <parent_process>
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
     * Checks whether this process is equal to another. Two processes are equal
     * if they are of the same process type and have the same number of in and
     * out ports.
     *
     * @param rhs
     *        Process to compare with.
     * @returns \c true if both processes are equal.
     */
    virtual bool operator==(const Process& rhs) const throw();

    /**
     * Same as operator==(Process&) but for inequality.
     *
     * @param rhs
     *        Process to compare with.
     * @returns \c true if both processes are not equal.
     */
    virtual bool operator!=(const Process& rhs) const throw();

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

    /**
     * Additional string output to be included when this process is converted to
     * a string representation. By default this returns an empty string.
     *
     * @returns Additional string representation data.
     * @see toString()
     */
    virtual std::string moreToString() const throw();

    /**
     * Takes a list of ports and converts it into a string representation. Each
     * port is converted into
     * @code
     *  PortID: <port_id>, not connected / connected to <process>:<port>,
     *  ...
     * @endcode
     *
     * @param ports
     *        Port list.
     * @returns String representation.
     */
    std::string portsToString(const std::list<Port*> ports) const throw();

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
     * Destroys all ports in a given list.
     *
     * @param ports
     *        List of ports to destroy.
     */
    void destroyAllPorts(std::list<Port*>& ports) throw();

  protected:
    /**
	 * Process ID.
	 */
	const ForSyDe::Id id_;
    /**
	 * Parent ID.
	 */
	const ForSyDe::Id parent_;
    /**
	 * Process MoC.
	 */
	const std::string moc_;
    /**
     * List of in ports.
     */
    std::list<Port*> in_ports_;

    /**
     * List of out ports.
     */
    std::list<Port*> out_ports_;

  public:
    /**
     * @brief Class used for in- and out ports by the \c Process class.
     *
     * The \c Port class defines a process port. A port is identified by an ID
     * and can be connected to another port.
     */
    class Port {
      public:
        /**
         * Creates a port belonging to no process.
         *
         * @param id
         *        Port ID.
         * @param datatype
         *        Data type contained by port.
         */
        Port(const ForSyDe::Id& id, CDataType datatype) throw();

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
        Port(const ForSyDe::Id& id, Process* process, CDataType datatype)
            throw(InvalidArgumentException);

        /**
         * Creates a port belonging to no process with the same ID, data type and
         * connections as another port. The connection at the other port is
         * broken.
         *
         * @param rhs
         *        Port to copy.
         */
        explicit Port(Port& rhs) throw();

        /**
         * Creates a port belonging to process with the same ID, data type and
         * connections as another port. The connection at the other port is
         * broken.
         *
         * @param rhs
         *        Port to copy.
         * @param process
         *        Pointer to the process to which this port belongs.
         * @throws InvalidArgumentException
         *         When \c process is \c NULL.
         */
        explicit Port(Port& rhs, Process* process)
            throw(InvalidArgumentException);

        /**
         * Destroys this port. This also breaks the connection, if any.
         */
        virtual ~Port() throw();
        
        /**
         * Gets the process to which this port belongs.
         *
         * @returns Process, if available; otherwise \c NULL.
         */
        Process* getProcess() const throw();

        /**
         * Gets the ID of this port.
         *
         * @returns Port ID.
         */
        const ForSyDe::Id* getId() const throw();

        /**
         * Gets the data type of this port.
         *
         * @returns Port data type.
         */
        CDataType* getDataType() throw();

        /**
		 * Sets the data type of this port.
		 *
		 * @param datatype
		 *        The new data type that has to be set.
		 */
		void setDataType(CDataType& datatype) throw();

        /**
         * Checks whether this port is an IO port.
         *
         * @returns \c true if it is IO.
         */
        bool isIOport() const throw();

        /**
         * In case of a normal port, it checks if it is connected to another port.
         * In case of an IO port, it is the same as IOisConnectedOutside().
         *
         * @returns \c true if connected.
         */
        bool isConnected() const throw();

        /**
         * Checks if this IO port is connected to a port outside the composite process.
         *
         * @returns \c true if connected.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
        bool IOisConnectedOutside() const throw(IllegalCallException);

        /**
         * Checks if this IO port is connected to a port inside the composite process.
         *
         * @returns \c true if connected.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
		 */

        bool IOisConnectedInside() const throw(IllegalCallException);

        /**
         * Checks if this IO port is fully connected to both an upstream port and a downstream port.
         *
         * @returns \c true if connected.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
        bool IOisConnected() const throw(IllegalCallException);

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
        void connect(Port* port) throw();

        /**
         * Connects this IO port to another, outside the composite process.
         * This also sets the other port as
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
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
        void IOconnectOutside(Port* port) throw(IllegalCallException);

        /**
         * Connects this IO port to another, inside the composite process.
         * This also sets the other port as
         * connected to this port. If there already is a connection it will
         * automatically be broken.
         *
         * Setting the port parameter to \c NULL is equivalent to breaking the
         * connection. If both ends of a connection is the same port, this
         * method call is effectively ignored.
         *
         * @param port
         *        Port to connect.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
        void IOconnectInside(Port* port) throw(IllegalCallException);

        /**
         * Connects this IO port to another, inside the composite process.
         * This also sets the other port as
         * connected to this port. If there already is a connection it will
         * automatically be broken.
         *
         * Setting the port parameter to \c NULL is equivalent to breaking the
         * connection. If both ends of a connection is the same port, this
         * method call is effectively ignored.
         *
         * @param port
         *        Port to connect.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
        void IOconnect(Port* inside, Port* outside) throw(IllegalCallException);

        /**
         * Breaks the connection that this port may have to another. If there is
         * no connection, nothing happens.
         */
        void unconnect() throw();

        /**
         * Breaks the connection that this port may have to another. If there is
         * no connection, nothing happens.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
        void IOunconnectOutside() throw(IllegalCallException);

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
        void IOunconnectInside() throw(IllegalCallException);

        /**
         * Exactly the same as unconnect(), but with another name. Its purpose is
         * purely for naming coherence purposes.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
        void IOunconnect() throw(IllegalCallException);

        /**
         * Searches recursively through composites and gets
         * the port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         */
        Port* getConnectedPort() const throw();
        /**
         * Searches recursively through composites and gets
         * the port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
        Port* IOgetConnectedPortOutside() const throw(IllegalCallException);
        /**
         * Searches recursively through composites and gets
         * the port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
        Port* IOgetConnectedPortInside() const throw(IllegalCallException);
        /**
         * Searches recursively through composites and gets
         * the port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
        std::pair<Port*,Port*> IOgetConnectedPorts() const throw(IllegalCallException);

        /**
         * Gets the immediate adjacent port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
        Port* getConnectedPortImmediate() const throw();
        /**
         * Gets the immediate adjacent port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
        Port* IOgetConnectedPortOutsideImmediate() const throw(IllegalCallException);
        /**
         * Gets the immediate adjacent port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
        Port* IOgetConnectedPortInsideImmediate() const throw(IllegalCallException);

        /**
		 * Gets the immediate adjacent ports at the other ends of the connection, if any,
		 * for an IO port
		 *
		 * @returns Connected port, if any; otherwise \c NULL.
		 *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
		 */
        std::pair<Port*,Port*> IOgetConnectedPortsImmediate() const throw(IllegalCallException);

        /**
         * Checks for equality between this port and another.
         *
         * @param rhs
         *        Port to compare.
         * @returns \c true if both belong to the same process and if their IDs
         *          are identical.
         */
        virtual bool operator==(const Port& rhs) const throw();

        /**
         * Checks for inequality between this port and another.
         *
         * @param rhs
         *        Port to compare.
         * @returns \c true if the ports belong to different processes or if
         *          their IDs are not identical.
         */
        virtual bool operator!=(const Port& rhs) const throw();

        /**
         * Converts this port into a string representation. The resultant string
         * is as follows:
         * @code
         *  <process_id>:<port_id>
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
         * Port ID.
         */
        const ForSyDe::Id id_;

        /**
         * Parent process.
         */
        Process* process_;

        /**
         * Pointer to the other end of a connection.
         */
        Port* connected_port_outside_;

        /**
		 * In case it is an IO Port, this will contain a connection inside the
		 * composite process as well
		 */
        Port* connected_port_inside_;

        /**
         * Port data type.
         */
        CDataType data_type_;
    };

};

}
}

#endif
