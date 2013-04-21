/*
 * Copyright (c) 2011-2012 George Ungureanu <ugeorge@kth.se>
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

#ifndef F2CC_SOURCE_FORSYDE_PROCESSNETWORK_H_
#define F2CC_SOURCE_FORSYDE_PROCESSNETWORK_H_

/**
 * @file
 * @author  George Ungureanu <ugeorge@kth.se>
 * @version 0.2
 *
 * @brief Defines the leaf network, which is the top module.
 */

#include "id.h"
#include "processnetwork.h"
#include "../exceptions/outofmemoryexception.h"
#include "../exceptions/illegalstateexception.h"
#include "../exceptions/invalidargumentexception.h"
#include <string>
#include <map>
#include <list>

namespace f2cc {
namespace ForSyDe {

/**
 * @brief Contains the internal representation of a ForSyDe processnetwork.
 *
 * The \c ProcessNetwork embodies a complete ForSyDe network of connected \c Process
 * objects. The class also provides inputs and outputs to the network, which
 * actually are inports and outports, respectively, to one or more of the
 * leafs within the network.
 */
class ProcessNetwork: public ProcessNetwork {
  public:
    /**
     * Creates a leaf network.
     */
	ProcessNetwork() throw();

    /**
     * Destroys this leaf network. This also destroys all leafs.
     */
    virtual ~ProcessNetwork() throw();

    /**
     * Adds an input to this leaf network. The input must such that it is an inport to
     * a leaf already existing in the leaf network. If the input is \c NULL, nothing
     * happens and \c false is returned.
     *
     * @param port
     *        Inport of a leaf.
     * @returns \c true if the port did not already exist as input and was
     *          successfully added.
     * @throws InvalidArgumentException
     *         When \c port is \c NULL.
     * @throws IllegalStateException
     *         When the port belongs to a leaf not residing in the processnetwork.
     * @throws OutOfMemoryException
     *         When a port cannot be added due to memory shortage.
     */
    bool addInput(Process::Port* port)
        throw(InvalidArgumentException, IllegalStateException,
              OutOfMemoryException);

    /**
     * Deletes an input port of this leaf network.
     *
     * @param port
     *        Port.
     * @returns \c true if such an input port was found and successfully
     *          deleted.
     * @throws InvalidArgumentException
     *         When \c port is \c NULL.
     */
    bool deleteInput(Process::Port* port) throw(InvalidArgumentException);

    /**
     * Gets the number of inputs of this leaf network.
     *
     * @returns Number of inputs.
     */
    int getNumInputs() const throw();

    /**
     * Gets a list of inputs belonging to this leaf network.
     *
     * @returns List of inputs.
     */
    std::list<Process::Port*> getInputs() throw();

    /**
     * Same as addInput(const Process::Port*) but for outputs.
     *
     * @param port
     *        Outport of a leaf.
     * @returns \c true if the port did not already exist as output and was
     *          successfully added.
     * @throws InvalidArgumentException
     *         When \c port is \c NULL.
     * @throws IllegalStateException
     *         When the port belongs to a leaf not residing in the processnetwork.
     * @throws OutOfMemoryException
     *         When a port cannot be added due to memory shortage.
     */
    bool addOutput(Process::Port* port)
        throw(InvalidArgumentException, IllegalStateException,
              OutOfMemoryException);

    /**
     * Same as deleteInput(Process::Port*) but for outputs.
     *
     * @param port
     *        Port.
     * @returns \c true if such an output port was found and successfully
     *          deleted.
     * @throws InvalidArgumentException
     *         When \c port is \c NULL.
     */
    bool deleteOutput(Process::Port* port) throw(InvalidArgumentException);

    /**
     * Same as addInput(const Process::Port*) but for outputs.
     * Gets the number of inputs of this processnetwork.
     *
     * @returns Number of inputs.
     */
    int getNumOutputs() const throw();

    /**
     * Same as getInputs() but for outputs.
     *
     * @returns List of outputs.
     */
    std::list<Process::Port*> getOutputs() throw();

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
    std::list<Process::Port*>::iterator findPort(
        const Id& id, std::list<Process::Port*>& ports) const throw();

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
    std::list<Process::Port*>::iterator findPort(
        Process::Port* port, std::list<Process::Port*>& ports)  const throw();



    /**
     * Takes a list of ports and converts it into a string representation. Each
     * port is converted into
     * @code
     *  PortID: <port_id>, belonging to <leaf>,
     *  ...
     * @endcode
     *
     * @param ports
     *        Port list.
     * @returns String representation.
     */
    std::string portsToString(const std::list<Process::Port*> ports) const
        throw();

    /**
     * Converts this leaf network into a string representation. The resultant string
     * is as follows:
     * @code
     * {
     *  ProcessNetwork,
     *  NumInputs: <num_inputs>,
     *  Inputs = { ... },
     *  NumOututs: <num_outports>,
     *  Outputs = { ... },
     *  NumProcesss: <num_leafs>
     * }
     * @endcode
     *
     * @returns String representation.
     */
    std::string toString() const throw();

  private:

    /**
     * List of inputs.
     */
    std::list<Process::Port*> inputs_;

    /**
     * List of outputs.
     */
    std::list<Process::Port*> outputs_;
};

}
}

#endif
