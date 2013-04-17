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
 * of ForSyDe process networks. It inherits both the \c Model class and the \Process class.
 * Hence it behaves like a process that contains other processes.
 */
class Composite : public Model, private Process {
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
    Composite(const ForSyDe::Id& id, const ForSyDe::Id& parent,
    		const std::string name) throw();

    /**
      * Creates a process node.
      *
      * @param id
      *        Process ID.
      */
    Composite(const ForSyDe::Id& id, std::list<const Id> hierarchy,
    		const std::string name) throw();

    /**
     * Destroys this composite process. This also destroys all contained processes
     */
    virtual ~Composite() throw();

    /**
     * Checks whether this port is a composite process.
     *
     * @returns \c true if it a composite process.
     */
    virtual bool isComposite() const throw();

    /**
     * Gets the data type of this port.
     *
     * @returns Port data type.
     */
    const std::string getName() const throw();

    /**
     * Same as Process::operator==(const Process&) const but with the additional
     * check that the composite process' names (thus their structure) is the same.
     *
     * @param rhs
     *        Composite process to compare with.
     * @returns \c true if both processes are equal.
     */
    virtual bool operator==(const Process& rhs) const throw();

    /**
     * @copydoc Process::type()
     */
    virtual std::string type() const throw();

  protected:
    /**
     * Converts this composite process into a string representation. The resultant string
     * is as follows:
     * @code
     * {
     *  List of Processes : {...}
     * @endcode
     *
     * @returns Additional string representation data.
     * @see toString()
     */
    virtual std::string moreToString() const throw();

    /**
     * Checks that this process has at least one in port, one out
     * port and one contained process.
     *
     * @throws InvalidProcessException
     *         When the check fails.
     */
    virtual void moreChecks() throw(InvalidProcessException);

  private:
    /**
     * The composite process' name
     */
    const std::string composite_name_;
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
    class IOPort : private Port{
      public:
        /**
         * Creates a port belonging to no process.
         *
         * @param id
         *        Port ID.
         * @param datatype
         *        Data type contained by port.
         */
    	IOPort(const ForSyDe::Id& id) throw();

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
         * Creates a port belonging to no process with the same ID, data type and
         * connections as another port. The connection at the other port is
         * broken.
         *
         * @param rhs
         *        Port to copy.
         */
        explicit IOPort(IOPort& rhs) throw();

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
        explicit IOPort(IOPort& rhs, Composite* composite)
            throw(InvalidArgumentException);

        /**
         * Destroys this port. This also breaks the connection, if any.
         */
        virtual ~IOPort() throw();

        /**
         * Checks whether this port is an IO port.
         *
         * @returns \c true if it is IO.
         */
		virtual bool isIOPort() const throw();

        /**
         * ATTENTION! This method is kept only for backwards compatibility purpose, and is used by
         * the GraphML model modifier and synthesizer. It is not advisable to use it otherwise.
         *
         * It checks whether the \ inside connection is established with another port.
         *
         * @returns \c true if connected \c inside.
         */
		virtual bool isConnected() const throw();

        /**
         * Checks if this \c IOPort is connected to a port outside the composite process.
         *
         * @returns \c true if connected.
         */
		virtual bool isConnectedOutside() const throw();

        /**
         * Checks if this \c IOPort is connected to a port inside the composite process.
         *
         * @returns \c true if connected.
		 */

		virtual bool isConnectedInside() const throw();

        /**
         * Checks if this \c IOPort is connected to a port outside the composite process.
         *
         * @returns \c true if connected.
         */
		virtual bool isConnectedToLeafOutside() const throw();

        /**
         * Checks if this \c IOPort is connected to a port inside the composite process.
         *
         * @returns \c true if connected.
		 */

		virtual bool isConnectedToLeafInside() const throw();

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
		virtual void connect(Port* port) throw(InvalidArgumentException);

        /**
         * ATTENTION!
         * Breaks the connection that this port may have to another. If there is
         * no connection, nothing happens.
         */
		//virtual void unconnect() throw();

        /**
         * Breaks the connection that this port may have to another. If there is
         * no connection, nothing happens.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
		virtual void unconnectOutside() throw(IllegalCallException);

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
		virtual void unconnectInside() throw(IllegalCallException);

        /**
         * Breaks the connection that this port may have to another. If there is
         * no connection, nothing happens.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
		virtual void unconnectFromLeafOutside() throw(IllegalCallException);

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
		virtual void unconnectFromLeafInside() throw(IllegalCallException);

		/**
         * Searches recursively through composites and gets
         * the port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         */
		virtual Port* getConnectedPort() const throw();
        /**
         * Searches recursively through composites and gets
         * the port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */

		virtual Port* getConnectedPortOutside() const throw(IllegalCallException);
        /**
         * Searches recursively through composites and gets
         * the port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
		virtual Port* getConnectedPortInside() const throw(IllegalCallException);

        /**
         * Gets the immediate adjacent port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
		virtual Port* getConnectedLeafPortOutside() const throw(IllegalCallException);
        /**
         * Gets the immediate adjacent port at the other end of the connection, if any.
         *
         * @returns Connected port, if any; otherwise \c NULL.
         *
		 * @throws IllegalCallException
		 *         When this method was called for a non-IO port
         */
		virtual Port* getConnectedLeafPortInside() const throw(IllegalCallException);


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
         * Parent process.
         */
        //Composite* process_;

        /**
		 * In case it is an IO Port, this will contain a connection inside the
		 * composite process as well
		 */
        Port* connected_port_inside_;

    };
};

}
}

#endif
