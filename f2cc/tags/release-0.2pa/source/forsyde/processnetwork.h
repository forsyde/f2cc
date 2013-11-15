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

#ifndef F2CC_SOURCE_FORSYDE_PROCESSNETWORK_H_
#define F2CC_SOURCE_FORSYDE_PROCESSNETWORK_H_

/**
 * @file
 * @author  George Ungureanu <ugeorge@kth.se>
 * @version 0.2
 *
 * @brief Defines the process network, which is the top module.
 */

#include "id.h"
#include "model.h"
#include "process.h"
#include "../language/cfunction.h"
#include "../exceptions/outofmemoryexception.h"
#include "../exceptions/illegalstateexception.h"
#include "../exceptions/invalidargumentexception.h"
#include <string>
#include <map>
#include <list>

namespace f2cc {
namespace Forsyde {

/**
 * @brief Contains the internal representation of a ForSyDe process network.
 *
 * The \c ProcessNetwork embodies a complete ForSyDe network of connected \c Process
 * objects. The class also provides inputs and outputs to the network, which
 * actually are inports and outports, respectively, to one or more of the
 * processes within the network.
 */
class ProcessNetwork: public Model {
  public:
    /**
     * Creates a process network.
     */
	ProcessNetwork() throw();

    /**
     * Destroys this process network. This also destroys all processs.
     */
    virtual ~ProcessNetwork() throw();

    /**
     * Adds an input to this process network. The input must such that it is an inport to
     * a process already existing in the process network. If the input is \c NULL, nothing
     * happens and \b false is returned.
     *
     * @param port
     *        Inport of a process.
     * @returns \b true if the port did not already exist as input and was
     *          successfully added.
     * @throws InvalidArgumentException
     *         When \c port is \c NULL.
     * @throws IllegalStateException
     *         When the port belongs to a process not residing in the processnetwork.
     * @throws OutOfMemoryException
     *         When a port cannot be added due to memory shortage.
     */
    bool addInput(Process::Interface* port)
        throw(InvalidArgumentException, IllegalStateException,
              OutOfMemoryException);

    /**
     * Deletes an input port of this process network.
     *
     * @param port
     *        Interface.
     * @returns \b true if such an input port was found and successfully
     *          deleted.
     * @throws InvalidArgumentException
     *         When \c port is \c NULL.
     */
    bool deleteInput(Process::Interface* port) throw(InvalidArgumentException);

    /**
     * Gets the number of inputs of this process network.
     *
     * @returns Number of inputs.
     */
    int getNumInputs() const throw();

    /**
     * Gets a list of inputs belonging to this process network.
     *
     * @returns List of inputs.
     */
    std::list<Process::Interface*> getInputs() throw();

    /**
     * Same as addInput(const Process::Interface*) but for outputs.
     *
     * @param port
     *        Outport of a process.
     * @returns \b true if the port did not already exist as output and was
     *          successfully added.
     * @throws InvalidArgumentException
     *         When \c port is \c NULL.
     * @throws IllegalStateException
     *         When the port belongs to a process not residing in the processnetwork.
     * @throws OutOfMemoryException
     *         When a port cannot be added due to memory shortage.
     */
    bool addOutput(Process::Interface* port)
        throw(InvalidArgumentException, IllegalStateException,
              OutOfMemoryException);

    /**
     * Same as deleteInput(Process::Interface*) but for outputs.
     *
     * @param port
     *        Port.
     * @returns \b true if such an output port was found and successfully
     *          deleted.
     * @throws InvalidArgumentException
     *         When \c port is \c NULL.
     */
    bool deleteOutput(Process::Interface* port) throw(InvalidArgumentException);

    /**
     * Same as addInput(const Process::Interface*) but for outputs.
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
    std::list<Process::Interface*> getOutputs() throw();

    /**
     * Adds a function to this model. Models are not allowed to have multiple
     * functions with the same name.
     *
     * @param function
     *        Process to add.
     * @returns \b true if such a function did not already exist and was
     *          successfully added.
     * @throws InvalidArgumentException
     *         When \c function is \c NULL.
     * @throws OutOfMemoryException
     *         When a function cannot be added due to memory shortage.
     */
    bool addFunction(CFunction* function)
        throw(InvalidArgumentException, OutOfMemoryException);

    /**
     * Adds multiple functions to this model at the same time.
     *
     * @param functions
     *        Combset of functions to add.
     * @throws OutOfMemoryException
     *         When a function cannot be added due to memory shortage.
     */
    void addFunctions(std::map<const Id, CFunction*> functions)
        throw(OutOfMemoryException);

    /**
     * Gets a function by name.
     *
     * @param id
     *        function name.
     * @returns function, if found; otherwise \c NULL.
     */
    CFunction* getFunction(const Id& id) throw();

    /**
     * Gets the number of functions in this model.
     *
     * @returns function count.
     */
    int getNumFunctions() const throw();

    /**
     * Gets a list of all functions in this model.
     *
     * @returns function list.
     */
    std::list<CFunction*> getFunctions() throw();

    /**
     * Removes and destroys a function by ID.
     *
     * @param id
     *        function name.
     * @returns \b true if such a function was found and successfully deleted.
     */
    bool deleteFunction(const Id& id) throw();


    /**
     * Converts this process network into a string representation. The resultant string
     * is as follows:
     * @code
     * {
     *  ProcessNetwork Module,
     *  NumInputs: <num_inputs>,
     *  Inputs = { ... },
     *  NumOututs: <num_outports>,
     *  Outputs = { ... },
     *  NumFunctions: <num_functions>
     * }
     * @endcode
     *
     * @returns String representation.
     */
    virtual std::string toString() const throw();

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
    std::list<Process::Interface*>::iterator findPort(
        const Id& id, std::list<Process::Interface*>& ports) const throw();

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
    std::list<Process::Interface*>::iterator findPort(
        Process::Interface* port, std::list<Process::Interface*>& ports)  const throw();



    /**
     * Takes a list of ports and converts it into a string representation. Each
     * port is converted into
     * @code
     *  PortID: <port_id>: [additional information],
     *  ...
     * @endcode
     *
     * @param ports
     *        Port list.
     * @returns String representation.
     */
    std::string portsToString(const std::list<Process::Interface*> ports) const
        throw();


    /**
     * Attempts to find a function with a given ID. If the mapset of functions is
     * not empty and such a function is found, an iterator pointing to that port
     * is returned; otherwise the mapset's \c end() iterator is returned.
     *
     * @param id
     *        function name.
     * @returns Iterator pointing either at the found function, or an iterator
     *          equal to mapset's \c end() iterator.
     */
    std::map<const Id, CFunction*>::iterator findFunction(const Id& id) throw();

    /**
     * Destroys all functions in this model.
     */
    void destroyAllFunctions() throw();

  private:

    /**
     * List of inputs.
     */
    std::list<Process::Interface*> inputs_;

    /**
     * List of outputs.
     */
    std::list<Process::Interface*> outputs_;

    /**
     * Combset of functions.
     */
    std::map<const Id, CFunction*> functions_;
};

}
}

#endif
