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
#include "composite.h"
#include "../language/cfunction.h"
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
 * The \c Processnetwork embodies a complete ForSyDe network of connected \c Process
 * objects. The class also provides inputs and outputs to the network, which
 * actually are inports and outports, respectively, to one or more of the
 * processes within the network.
 */
class Processnetwork: public Composite {
  public:
    /**
     * Creates a process network.
     */
	Processnetwork(std::string name) throw();

    /**
     * Destroys this process network. This also destroys all processes.
     */
    virtual ~Processnetwork() throw();

    /**
     * The process network is identified as "composite" also. This enables it to access
     * the methods dedicated to IO ports.
     */
    virtual std::string type() const throw();

    /**
     * Adds a process function to this model. If the function already exists, it will not be added.
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
    bool addFunction(CFunction* function)
        throw(InvalidArgumentException, OutOfMemoryException);


    /**
     * Gets a process function by name.
     *
     * @param name
     *        Function name.
     * @returns Process, if found; otherwise \c NULL.
     */
    CFunction* getFunction(std::string name) throw();

    /**
     * Gets the number of process functions in this model.
     *
     * @returns Function count.
     */
    size_t getNumFunctions() const throw();

    /**
     * Gets a list of all process functions in this model.
     *
     * @returns Function list.
     */
    std::list<CFunction*> getFunctions() throw();

    /**
     * Removes and destroys a process function by its name.
     *
     * @param name
     *        Function name.
     * @returns \c true if such a function was found and successfully deleted.
     */
    bool deleteFunction(std::string name) throw();


  private:

    /**
     * Attempts to find a function with a given name from a list of functions.
     * If the list is not empty and such a function is found, an iterator
     * pointing to that function is returned; otherwise the list's \c end() iterator is returned.
     *
     * @param id
     *        Port ID.
     * @param ports
     *        List of ports.
     * @returns Iterator pointing either at the found function, or an iterator equal
     *          to the list's \c end() iterator.
     */
    std::list<CFunction*>::iterator findFunction(const std::string name,
                                        std::list<CFunction*>& functions) const throw();

    /**
     * Destroys all functions in a given list.
     *
     * @param functions
     *        List of functions to destroy.
     */
    void destroyAllFunctions() throw();

    /**
     * Converts this process network into a string representation. The resultant string
     * is as follows:
     * @code
     * {
     *  Processnetwork,
     *  NumInputs: <num_inputs>,
     *  Inputs = { ... },
     *  NumOututs: <num_outports>,
     *  Outputs = { ... },
     *  NumProcesses: <num_processes>
     *  NumFunctions: <num_functions>
     * }
     * @endcode
     *
     * @returns String representation.
     */
    std::string toString() const throw();

  protected:
    /**
     * List of process functions.
     */
    std::list<CFunction*> process_functions_;
};

}
}

#endif
