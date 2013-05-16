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

#ifndef F2CC_SOURCE_FORSYDE_MODELMODIFIERSYSC_H_
#define F2CC_SOURCE_FORSYDE_MODELMODIFIERSYSC_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines a class for performing \c ProcessNetwork modifications.
 */

#include "../id.h"
#include "../hierarchy.h"
#include "../processnetwork.h"
#include "../leaf.h"
#include "../parallelcomposite.h"
#include "../SY/combsy.h"
#include "../SY/unzipxsy.h"
#include "../SY/zipxsy.h"
#include "../../logger/logger.h"
#include "../../config/config.h"
#include "../../exceptions/ioexception.h"
#include "../../exceptions/invalidargumentexception.h"
#include "../../exceptions/outofmemoryexception.h"
#include "../../exceptions/invalidmodelexception.h"
#include <list>
#include <set>
#include <vector>

namespace f2cc {
namespace Forsyde {

/**
 * @brief Performs semantic-preserving modifications on a \c ProcessNetwork
 *        object.
 *
 * The \c ModelModifier is a class which provides a set of processnetwork modification
 * methods. The modifications are such that they preserve the semantics of the
 * processnetwork, and are used to simplify the latter synthesis leaf or affect the
 * structure of the generated code (i.e. whether to generate sequential C code
 * or parallel CUDA C code).
 */
class ModelModifierSysC {
 // private:
  //  class ContainedSection;

  public:

    /**
     * Creates a processnetwork modifier.
     *
     * @param processnetwork
     *        ForSyDe processnetwork.
     * @param logger
     *        Reference to the logger.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     */
    ModelModifierSysC(Forsyde::ProcessNetwork* processnetwork,
    		Logger& logger, Config& config)
        throw(InvalidArgumentException);

    /**
     * Destroys this processnetwork modifier. The logger remains open.
     */
    ~ModelModifierSysC() throw();

    void flattenAndParallelize() throw(
    		RuntimeException, InvalidModelException, InvalidProcessException, OutOfMemoryException);

    void firstCostAnalysis() throw(
    		RuntimeException, InvalidModelException, InvalidProcessException, OutOfMemoryException);

  private:


    void flattenCompositeProcess(Composite* composite, Composite* parent) throw(
   		 RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException);


    /**
      * Coalesces data parallel leafs across different segments into a
      * single data parallel leaf.
      *
      * @throws IOException
      *         When access to the log file failed.
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      */
     std::list<std::list<Leaf*> > extractEquivalentCombs(Forsyde::Composite* parent)
         throw(InvalidArgumentException, OutOfMemoryException);

     std::list<std::list<Leaf*> > extractEquivalentLeafs(Forsyde::Composite* parent)
         throw(InvalidArgumentException, OutOfMemoryException);

     void removeRedundantZipsUnzips(Forsyde::Composite* parent)
     throw(InvalidArgumentException, OutOfMemoryException);

     bool foundDependencyDownstream(Leaf* current_process, std::map<Id, SY::Comb*> to_compare_with)
 	 	 throw(RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException);

     bool foundDependencyUpstream(Leaf* current_process, std::map<Id, SY::Comb*> to_compare_with)
 	 	 throw(RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException);

     void createParallelComposite(Composite* parent, std::list<Forsyde::Leaf*> equivalent_processes)
         throw(RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException);

     void prepareLeafForParallel(Forsyde::Leaf* reference_leaf, Forsyde::Composite* parent,
    		 Forsyde::ParallelComposite* new_pcomp, unsigned number_of_processes)
         throw(RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException);

     void moveToParallelComposite(Process* reference_process, Composite* old_parent,
    		 ParallelComposite* new_parent) throw (
    		 InvalidProcessException, OutOfMemoryException);

     void moveToNewParent(Process* reference_process, Composite* old_parent,
    		 Composite* new_parent) throw (
    		 InvalidProcessException, OutOfMemoryException);

     void redirectFlowThroughParallelComposite(Process* old_process, Composite* parent,
    		 ParallelComposite* new_pcomp) throw (
    		 InvalidArgumentException, RuntimeException, InvalidProcessException);

     void redirectFlow(Process::Interface* source, Process::Interface* target) throw (
    		 InvalidArgumentException, RuntimeException, InvalidProcessException);



  private:
    /**
     * ForSyDe processnetwork.
     */
    Forsyde::ProcessNetwork* const processnetwork_;

    /**
     * Logger.
     */
    Logger& logger_;

    Config configuration_;

    std::map<Id, bool> visited_processes_;

};

}
}

#endif
