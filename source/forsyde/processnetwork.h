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
#include "model.h"
#include "../exceptions/outofmemoryexception.h"
#include "../exceptions/illegalstateexception.h"
#include "../exceptions/invalidargumentexception.h"
#include <string>
#include <map>
#include <list>

namespace f2cc {
namespace ForSyDe {

/**
 * @brief Contains the internal representation of a ForSyDe model.
 *
 * The \c ProcessNetwork embodies a complete ForSyDe network of connected \c Process
 * objects. The class also provides inputs and outputs to the network, which
 * actually are inports and outports, respectively, to one or more of the
 * leafs within the network.
 */
class ProcessNetwork: public Model {
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
     * @param interface
     *        Ininterface of a leaf.
     * @returns \c true if the interface did not already exist as input and was
     *          successfully added.
     * @throws InvalidArgumentException
     *         When \c interface is \c NULL.
     * @throws IllegalStateException
     *         When the interface belongs to a leaf not residing in the model.
     * @throws OutOfMemoryException
     *         When a interface cannot be added due to memory shortage.
     */
    bool addInput(Process::Port* interface)
        throw(InvalidArgumentException, IllegalStateException,
              OutOfMemoryException);

    /**
     * Deletes an input interface of this leaf network.
     *
     * @param interface
     *        Port.
     * @returns \c true if such an input interface was found and successfully
     *          deleted.
     * @throws InvalidArgumentException
     *         When \c interface is \c NULL.
     */
    bool deleteInput(Process::Port* interface) throw(InvalidArgumentException);

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
     * @param interface
     *        Outinterface of a leaf.
     * @returns \c true if the interface did not already exist as output and was
     *          successfully added.
     * @throws InvalidArgumentException
     *         When \c interface is \c NULL.
     * @throws IllegalStateException
     *         When the interface belongs to a leaf not residing in the model.
     * @throws OutOfMemoryException
     *         When a interface cannot be added due to memory shortage.
     */
    bool addOutput(Process::Port* interface)
        throw(InvalidArgumentException, IllegalStateException,
              OutOfMemoryException);

    /**
     * Same as deleteInput(Process::Port*) but for outputs.
     *
     * @param interface
     *        Port.
     * @returns \c true if such an output interface was found and successfully
     *          deleted.
     * @throws InvalidArgumentException
     *         When \c interface is \c NULL.
     */
    bool deleteOutput(Process::Port* interface) throw(InvalidArgumentException);

    /**
     * Same as addInput(const Process::Port*) but for outputs.
     * Gets the number of inputs of this model.
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
     * Attempts to find a interface with a given ID from a list of interfaces. If the list
     * is not empty and such a interface is found, an iterator pointing to that interface
     * is returned; otherwise the list's \c end() iterator is returned.
     *
     * @param id
     *        Port ID.
     * @param interfaces
     *        List of interfaces.
     * @returns Iterator pointing either at the found interface, or an iterator equal
     *          to the list's \c end() iterator.
     */
    std::list<Process::Port*>::iterator findPort(
        const Id& id, std::list<Process::Port*>& interfaces) const throw();

    /**
     * Attempts to find a given interface from a list of interfaces. If the list
     * is not empty and such a interface is found, an iterator pointing to that interface
     * is returned; otherwise the list's \c end() iterator is returned.
     *
     * @param interface
     *        Port
     * @param interfaces
     *        List of interfaces.
     * @returns Iterator pointing either at the found interface, or an iterator equal
     *          to the list's \c end() iterator.
     */
    std::list<Process::Port*>::iterator findPort(
        Process::Port* interface, std::list<Process::Port*>& interfaces)  const throw();



    /**
     * Takes a list of interfaces and converts it into a string representation. Each
     * interface is converted into
     * @code
     *  PortID: <interface_id>, belonging to <leaf>,
     *  ...
     * @endcode
     *
     * @param interfaces
     *        Port list.
     * @returns String representation.
     */
    std::string interfacesToString(const std::list<Process::Port*> interfaces) const
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
