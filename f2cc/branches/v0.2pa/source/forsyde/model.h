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

#ifndef F2CC_SOURCE_FORSYDE_MODEL_H_
#define F2CC_SOURCE_FORSYDE_MODEL_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se> & George Ungureanu <ugeorge@kth.se>
 * @version 0.2
 *
 * @brief Defines the model abstract class for the internal ForSyDe representation,
 * which will be inherited by \c Processnetwork and \c Composite.
 */

#include "id.h"
#include "leaf.h"
#include "../exceptions/outofmemoryexception.h"
#include "../exceptions/illegalstateexception.h"
#include "../exceptions/invalidargumentexception.h"
#include <string>
#include <map>
#include <list>

namespace f2cc {
namespace Forsyde {

class Composite;

/**
 * @brief Contains the internal representation of a ForSyDe model.
 *
 * The \c Model embodies one or more of the
 * processes within the process network. It provides common methods for both
 * \c Processnetwork and \c Composite classes.
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
     * @returns \b true if such a process did not already exist and was
     *          successfully added.
     * @throws InvalidArgumentException
     *         When \c process is \c NULL.
     * @throws OutOfMemoryException
     *         When a process cannot be added due to memory shortage.
     */
    bool addProcess(Leaf* process)
        throw(InvalidArgumentException, OutOfMemoryException);

    /**
     * Adds multiple processes to this model at the same time.
     *
     * @param processes
     *        Combset of processes to add.
     * @throws OutOfMemoryException
     *         When a process cannot be added due to memory shortage.
     */
    void addProcesses(std::map<const Id, Leaf*> processes)
        throw(OutOfMemoryException);

    /**
     * Gets a process by ID.
     *
     * @param id
     *        Process ID.
     * @returns Process, if found; otherwise \c NULL.
     */
    Leaf* getProcess(const Id& id) throw();

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
    std::list<Leaf*> getProcesses() throw();

    /**
     * Removes and destroys a process by ID.
     *
     * @param id
     *        Process ID.
     * @returns \b true if such a process was found and successfully deleted.
     */
    bool deleteProcess(const Id& id) throw();

    /**
     * Gets a new process ID which is not currently in use within this model.
     *
     * @returns A unique process ID.
     */
    Forsyde::Id getUniqueProcessId() const throw();

    /**
     * Same as getUniqueCompositeId() but allows an arbitrary string to be
     * prefixed to the ID.
     *
     * @param prefix
     *        ID prefix.
     * @returns A unique process ID.
     */
    Forsyde::Id getUniqueProcessId(const std::string& prefix) const throw();

    /**
     * Adds a composite to this model. Models are not allowed to have multiple
     * composites with the same ID.
     *
     * @param composite
     *        Compoiste to add.
     * @returns \b true if such a process did not already exist and was
     *          successfully added.
     * @throws InvalidArgumentException
     *         When \c composite is \c NULL.
     * @throws OutOfMemoryException
     *         When a composite cannot be added due to memory shortage.
     */
    bool addComposite(Composite* composite)
        throw(InvalidArgumentException, OutOfMemoryException);

    /**
     * Adds multiple composites to this model at the same time.
     *
     * @param composite
     *        Combset of composites to add.
     * @throws OutOfMemoryException
     *         When a composite cannot be added due to memory shortage.
     */
    void addComposites(std::map<const Id, Composite*> composites)
        throw(OutOfMemoryException);

    /**
     * Gets a composite by ID.
     *
     * @param id
     *        Composite ID.
     * @returns Composite, if found; otherwise \c NULL.
     */
    Composite* getComposite(const Id& id) throw();

    /**
     * Gets the number of composites in this model.
     *
     * @returns Composite count.
     */
    int getNumComposites() const throw();

    /**
     * Gets a list of all composites in this model.
     *
     * @returns Composite list.
     */
    std::list<Composite*> getComposites() throw();

    /**
     * Removes and destroys a composite by ID.
     *
     * @param id
     *        Composite ID.
     * @returns \b true if such a composite was found and successfully deleted.
     */
    bool deleteComposite(const Id& id) throw();

    /**
     * Gets a new composite ID which is not currently in use within this model.
     *
     * @returns A unique composite ID.
     */
    Forsyde::Id getUniqueCompositeId() const throw();

    /**
     * Same as getUniqueCompositeId() but allows an arbitrary string to be
     * prefixed to the ID.
     *
     * @param prefix
     *        ID prefix.
     * @returns A unique composite ID.
     */
    Forsyde::Id getUniqueCompositeId(const std::string& prefix) const throw();

  protected:
    /**
     * Attempts to find a process with a given ID. If the mapset of processes is
     * not empty and such a process is found, an iterator pointing to that port
     * is returned; otherwise the mapset's \c end() iterator is returned.
     *
     * @param id
     *        Composite ID.
     * @returns Iterator pointing either at the found process, or an iterator
     *          equal to mapset's \c end() iterator.
     */
    std::map<const Id, Leaf*>::iterator findProcess(const Id& id) throw();

    /**
     * Destroys all processes in this model.
     */
    void destroyAllProcesses() throw();

    /**
     * Attempts to find a process with a given ID. If the mapset of processes is
     * not empty and such a process is found, an iterator pointing to that port
     * is returned; otherwise the mapset's \c end() iterator is returned.
     *
     * @param id
     *        Composite ID.
     * @returns Iterator pointing either at the found process, or an iterator
     *          equal to mapset's \c end() iterator.
     */
    std::map<const Id, Composite*>::iterator findComposite(const Id& id) throw();

    /**
     * Destroys all processes in this model.
     */
    void destroyAllComposites() throw();

  protected:
    /**
     * Combset of processes.
     */
    std::map<const Id, Leaf*> leafs_;

    /**
     * Combset of composites.
     */
    std::map<const Id, Composite*> composites_;
};

}
}

#endif
