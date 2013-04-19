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

#ifndef F2CC_SOURCE_FORSYDE_MODEL_H_
#define F2CC_SOURCE_FORSYDE_MODEL_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se> & George Ungureanu <ugeorge@kth.se>
 * @version 0.2
 *
 * @brief Defines the model abstract class for the internal ForSyDe representation,
 * which will be inherited by \c Model and \c Composite.
 */

#include "hierarchy.h"
#include "processbase.h"
#include "../exceptions/outofmemoryexception.h"
#include "../exceptions/illegalstateexception.h"
#include "../exceptions/invalidargumentexception.h"
#include <string>
#include <map>
#include <list>

namespace f2cc {
namespace ForSyDe {

class Process;
/**
 * @brief Contains the internal representation of a ForSyDe model.
 *
 * The \c Model embodies one or more of the
 * processes within the process network. It provides common methods for both
 * \c Model and \c Composite classes.
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
    bool addProcess(ProcessBase* process, ForSyDe::Hierarchy hierarchy)
        throw(InvalidArgumentException, OutOfMemoryException);

    /**
     * Adds multiple processes to this model at the same time.
     *
     * @param processes
     *        Mapset of processes to add.
     * @throws OutOfMemoryException
     *         When a process cannot be added due to memory shortage.
     */
    void addProcesses(std::map<const Id, ProcessBase*> processes, ForSyDe::Hierarchy hierarchy)
        throw(OutOfMemoryException);

    /**
     * Gets a process by ID.
     *
     * @param id
     *        Process ID.
     * @returns Process, if found; otherwise \c NULL.
     */
    ProcessBase* getProcess(const Id& id) throw();

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
    std::list<ProcessBase*> getProcesses() throw();



    /**
     * Removes and destroys a process by ID.
     *
     * @param id
     *        Process ID.
     * @returns \c true if such a process was found and successfully deleted.
     */
    bool deleteProcess(const Id& id) throw();

    /**
     * Adds a process to this model. Processnetworks are not allowed to have multiple
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

    /**
     * Gets a new process ID which is not currently in use within this model.
     *
     * @returns A unique process ID.
     */
    ForSyDe::Id getUniqueProcessId() const throw();

    /**
     * Same as getUniqueProcessId() but allows an arbitrary string to be
     * prefixed to the ID.
     *
     * @param prefix
     *        ID prefix.
     * @returns A unique process ID.
     */
    ForSyDe::Id getUniqueProcessId(const std::string& prefix) const throw();

  protected:
    /**
     * Attempts to find a process with a given ID. If the Mapset of processes is
     * not empty and such a process is found, an iterator pointing to that port
     * is returned; otherwise the Mapset's \c end() iterator is returned.
     *
     * @param id
     *        Process ID.
     * @returns Iterator pointing either at the found process, or an iterator
     *          equal to Mapset's \c end() iterator.
     */
    std::map<const Id, ProcessBase*>::iterator findProcess(const Id& id) throw();

    /**
     * Destroys all processes in this model.
     */
    void destroyAllProcesses() throw();




  protected:
    /**
     * Mapset of leaf processes.
     */
    std::map<const Id, ProcessBase*> processes_;
};


}
}

#endif
