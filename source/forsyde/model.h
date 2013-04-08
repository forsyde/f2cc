/*
 * fanoutright (c) 2011-2012 Gabriel Hjort Blindell <ghb@kth.se>
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

#ifndef F2CC_SOURCE_FORDE_MODEL_H_
#define F2CC_SOURCE_FORDE_MODEL_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines the model part of the internal ForSyDe representation.
 */

#include "id.h"
#include "process.h"
#include "../exceptions/outofmemoryexception.h"
#include "../exceptions/illegalstateexception.h"
#include "../exceptions/invalidargumentexception.h"
#include <string>
#include <map>
#include <list>

namespace f2cc {
namespace ForSyDe {
namespace SY{

/**
 * @brief Contains the internal representation of a ForSyDe model.
 *
 * The \c Model embodies a complete ForSyDe network of connected \c Process
 * objects. The class also provides inputs and outputs to the network, which
 * actually are inports and outports, respectively, to one or more of the
 * processes within the network.
 */
class Model {
  public:
    /**
     * Creates a model.
     */
    Model() throw();

    /**
     * Destroys this model. This also destroys all processes.
     */
    virtual ~Model() throw();

    /**
     * Adds a process to this model. Models are not allowed to have multiple
     * processes with the same ID.
     *
     * @param process
     *        Process to add.
     * @returns \c true if such a process did not already exist and was 
     *          successfully added.
     * @throws InvalidArgumentException
     *         When \c process is \c NULL.
     * @throws OutOfMemoryException
     *         When a process cannot be added due to memory shortage.
     */
    bool addProcess(Process* process)
        throw(InvalidArgumentException, OutOfMemoryException);

    /**
     * Adds multiple processes to this model at the same time.
     *
     * @param processes
     *        combset of processes to add.
     * @throws OutOfMemoryException
     *         When a process cannot be added due to memory shortage.
     */
    void addProcesses(std::map<const Id, Process*> processes)
        throw(OutOfMemoryException);

    /**
     * Gets a process by ID.
     *
     * @param id
     *        Process ID.
     * @returns Process, if found; otherwise \c NULL.
     */
    Process* getProcess(const Id& id) throw();

    /**
     * Gets the number of processes in this model.
     *
     * @returns Process count.
     */
    int getNumProcesses() const throw();

    /**
     * Gets a list of all processes in this model.
     *
     * @returns Process list.
     */
    std::list<Process*> getProcesses() throw();

    /**
     * Removes and destroys a process by ID.
     *
     * @param id
     *        Process ID.
     * @returns \c true if such a process was found and successfully deleted.
     */
    bool deleteProcess(const Id& id) throw();

    /**
     * Adds an input to this model. The input must such that it is an inport to
     * a process already existing in the model. If the input is \c NULL, nothing
     * happens and \c false is returned.
     *
     * @param port
     *        Inport of a process.
     * @returns \c true if the port did not already exist as input and was
     *          successfully added.
     * @throws InvalidArgumentException
     *         When \c port is \c NULL.
     * @throws IllegalStateException
     *         When the port belongs to a process not residing in the model.
     * @throws OutOfMemoryException
     *         When a port cannot be added due to memory shortage.
     */
    bool addInput(Process::Port* port)
        throw(InvalidArgumentException, IllegalStateException,
              OutOfMemoryException);

    /**
     * Deletes an input port of this model.
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
     * Gets the number of inputs of this model.
     *
     * @returns Number of inputs.
     */
    int getNumInputs() const throw();

    /**
     * Gets a list of inputs belonging to this model.
     *
     * @returns List of inputs.
     */
    std::list<Process::Port*> getInputs() throw();

    /**
     * Same as addInput(const Process::Port*) but for outputs.
     *
     * @param port
     *        Outport of a process.
     * @returns \c true if the port did not already exist as output and was
     *          successfully added.
     * @throws InvalidArgumentException
     *         When \c port is \c NULL.
     * @throws IllegalStateException
     *         When the port belongs to a process not residing in the model.
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

    /**
     * Gets a new process ID which is not currently in use within this model.
     *
     * @returns A unique process ID.
     */
    ForSyDe::SY::Id getUniqueProcessId() const throw();

    /**
     * Same as getUniqueProcessId() but allows an arbitrary string to be
     * prefixed to the ID.
     *
     * @param prefix
     *        ID prefix.
     * @returns A unique process ID.
     */
    ForSyDe::SY::Id getUniqueProcessId(const std::string& prefix) const throw();

    /**
     * Converts this model into a string representation. The resultant string
     * is as follows:
     * @code
     * {
     *  Model,
     *  NumInputs: <num_inputs>,
     *  Inputs = { ... },
     *  NumOututs: <num_outports>,
     *  Outputs = { ... },
     *  NumProcesses: <num_processes>
     * }
     * @endcode
     *
     * @returns String representation.
     */
    std::string toString() const throw();

  private:
    /**
     * Attempts to find a process with a given ID. If the mapset of processes is
     * not empty and such a process is found, an iterator pointing to that port
     * is returned; otherwise the mapset's \c end() iterator is returned.
     *
     * @param id
     *        Process ID.
     * @returns Iterator pointing either at the found process, or an iterator
     *          equal to mapset's \c end() iterator.
     */
    std::map<const Id, Process*>::iterator findProcess(const Id& id) throw();

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
     * Checks if a port exists in a given list of ports. If the list is not
     * empty and such a port is found, an iterator pointing to that port is
     * returned; otherwise the list's \c end() iterator is returned.
     *
     * @param port
     *        Port.
     * @param ports
     *        List of ports.
     * @returns Iterator pointing either at the found port, or an iterator equal
     *          to the list's \c end() iterator.
     */
    std::list<Process::Port*>::iterator findPort(
        Process::Port* port, std::list<Process::Port*>& ports) const throw();

    /**
     * Destroys all processes in this model.
     */
    void destroyAllProcesses() throw();

    /**
     * Takes a list of ports and converts it into a string representation. Each
     * port is converted into
     * @code
     *  PortID: <port_id>, belonging to <process>,
     *  ...
     * @endcode
     *
     * @param ports
     *        Port list.
     * @returns String representation.
     */
    std::string portsToString(const std::list<Process::Port*> ports) const
        throw();

  private:
    /**
     * combset of processes.
     */
    std::map<const Id, Process*> processes_;

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
}

#endif
