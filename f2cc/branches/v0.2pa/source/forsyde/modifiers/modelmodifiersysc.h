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
#include <map>
#include <utility>
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
  public:
	class DataPath;

  public:
	/**
	 * List for enumerating cost types.
	 */
    enum CostType {
   	 IN_COST,
   	 OUT_COST,
   	 PROCESS_COST,
    };

    /**
     * Creates a model modifier.
     *
     * @param processnetwork
     *        ForSyDe process network.
     * @param logger
     *        Reference to the logger.
     * @param costs
     *        Object modeling the platform costs.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     */
    ModelModifierSysC(Forsyde::ProcessNetwork* processnetwork,
    		Logger& logger, Config::Costs costs)
        throw(InvalidArgumentException);

    /**
     * Destroys this model modifier. The logger remains open.
     */
    ~ModelModifierSysC() throw();

    /**
	  * Public method called for flattening a ForSyDe model and identifying data
	  * parallel sections. It performs the following steps, in order:
	  * 	-# parse through composite processes and flatten them one by one;
	  * 	-# extract equivalent \c Comb processes and group them to
	  * 	\c Forsyde::ParallelComposite processes.
	  * 	-# extract the remaining equivalent \c Leaf processes and group them to
	  * 	\c Forsyde::ParallelComposite processes.
	  * 	-# remove redundant \c Zipx and \c Unzipx processes.
	  *
	  * @throws RuntimeException
	  *         When a program error has occurred. This most likely indicates a
	  *         bug.
	  * @throws InvalidModelException
	  *         When an error related to the ForSyDe model occured.
	  * @throws InvalidProcessException
	  *         When an error related to a ForSyDe process occured.
	  * @throws OutOfMemoryException
	  *         When there is not enough memory for creating a new object.
	  */
    void flattenAndParallelize() throw(
    		RuntimeException, InvalidModelException, InvalidProcessException, OutOfMemoryException);

    /**
      * Public method called for optimizing the target platform: CPU or GPU. Initially,
      * all \c ParallelComposite processes are mapped for GPU execution while all other
      * ones are mapped for CPU execution. Based upon a local cost calculation of
      * execution and communication for each process, some processes may be redirected
      * to optimize throughput.
      *
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      * @throws InvalidModelException
      *         When an error related to the ForSyDe model occurred.
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      */
    void optimizePlatform() throw(
    		RuntimeException, InvalidModelException, InvalidProcessException, OutOfMemoryException);

    /**
      * Public method called for balancing the load for efficient pipeline execution.
      * It performs the following actions:
      * 	-# extracts all possible data paths in the process network.
      * 	-# finds the maximum single cost by analyzing the datapaths, which will
      * 	become the quantum cost for balancing the process network load against.
      * 	-# extracts he contained sections from de data paths and sorts them by their maximum
      * 	cost.
      * 	-# splits the data paths into pipeline stages, and assigns a stage number
      * 	for each process.
      *
      * @returns The new set of costs, useful for the synthesis module.
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      * @throws InvalidModelException
      *         When an error related to the ForSyDe model occurred.
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      */
    Config::Costs loadBalance() throw(
    		RuntimeException, InvalidModelException, InvalidProcessException, OutOfMemoryException);

    /**
      * Public method called for grouping all processes that are associated to GPU
      * pipeline stages into separate ParallelComposite processes, for easy code
      * synthesis later on.
      *
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      * @throws InvalidModelException
      *         When an error related to the ForSyDe model occurred.
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      */
    void wrapPipelineStages() throw(
    		RuntimeException, InvalidModelException, InvalidProcessException, OutOfMemoryException,
    		InvalidArgumentException);

  private:
    /**
      * Set of algorithms for calculating the maximum cost from a list of datapaths.
      * The costs to take into consideration are:
      * 	- execution cost for processes mapped for parallel execution;
      * 	- input/output channel communication cost;
      * 	- sum of execution costs for processes mapped for sequential execution.
      * 	- sum of the costs inside a loop, between to \c Delay elements, since
      * 	they cannot be split.
      * 	- transfer costs per data burst, as seen in
      * 	SynthesizerExperimental::generateCudaKernelWrapper
      *
      * @param root
      *        The root composite process.
      * @param datapaths
      *        The list of datapaths to be analyzed.
      *
      * @returns A string stating the owner of the maximum cost.
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      * @throws InvalidModelException
      *         When an error related to the ForSyDe model occurred.
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      * @throws InvalidArgumentException
      *         If \c root is \c NULL;
      *
      * @see SynthesizerExperimental::generateCudaKernelWrapper
      */
    std::string findMaximumCost(Composite* root, std::list<DataPath> datapaths) throw (
		 RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException,
		InvalidModelException);

    /**
      * Extracts a list of individual datapaths from the process network.
      *
      * @param root
      *        The root composite process.
      *
      * @returns A list with datapaths.
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      * @throws InvalidArgumentException
      *         If \c root is \c NULL;
      */
    std::list<DataPath> extractDataPaths(Composite* root) throw (
		 RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException);

    /**
      * Recursively parses a process network branch and returns its associated list of
      * data paths.
      *
      * @param process
      *        The current process being visited.
      * @param current_path
      *        The datapath which is currently being formed.
      * @param root
      *        The root composite process.
      *
      * @returns A list with datapaths.
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      * @throws InvalidArgumentException
      *         If \c root or \c process is \c NULL;
      */
    std::list<DataPath> parsePath(Process* process, DataPath current_path,
    		Composite* root) throw(
		 RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException);

    /**
      * Calculates and arranges the contained sections into combsets indexed by their
      * costs, so that we can force a priority for accessing them in a defined order.
      *
      * @param datapaths
      *        List of contained sections as datapath objects
      *
      * @returns Combset of contained section as ID lists indexed by their costs.
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      * @throws InvalidArgumentException
      *         If \c root or \c process is \c NULL;
      */
    std::map<unsigned long long, std::list<Id> > sortContainedSectionsByCost(
    		std::list<DataPath> datapaths) throw (
		RuntimeException, InvalidProcessException, OutOfMemoryException, InvalidModelException);

    /**
      * Builds pipeline stages by adding processes so that their computation and
      * communication costs do not exceed the quantum cost. The stages are parsed
      * in reverse order of their maximum cost, so that processes with higher cost have
      * higher priority, and are resolved first.
      *
      * @param contained_sections
      *        List (as vector, for easy indexed accessing) with all the contained
      *        contained sections, in reverse order of their maximum cost.
      *
      * @returns \c False if a new quantum cost is discovered (due to unforseen
      *          communication costs that have to be added). If so, this method
      *          is aborted, invalidated, and the new quantum cost is updated. The
      *          caller has to take care of running this method again with the new
      *          quantum cost.
      *
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      * @throws InvalidArgumentException
      *         If \c root or \c process is \c NULL;
      */
    bool splitPipelineStages(std::vector<Id> contained_sections)
    throw (RuntimeException, InvalidProcessException, OutOfMemoryException, InvalidModelException);

    /**
      * Arranges the newly built pipeline stages into combsets indexed by their
      * stage number, for easy accessing.
      *
      * @returns combset of process ID lists, indexed by the pipeline stage associated
      * with them.
      *
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      * @throws InvalidArgumentException
      *         If \c root or \c process is \c NULL;
      */
    std::map<unsigned, std::list<Forsyde::Id> > orderStages() throw(
    		RuntimeException, InvalidModelException, InvalidProcessException, OutOfMemoryException,
    		InvalidArgumentException);

    /**
      * Bulds a \c ParallelComposite process out of all the processes associated with a
      * pipeline stage. All the processes are moved into the ParallelComposite, and
      * connections with the rest of the process network are taken care of.
      *
      * @param stage
      *        List of process IDs associated with a pipeline stage.
      *
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      * @throws InvalidArgumentException
      *         If \c root or \c process is \c NULL;
      */
    void groupIntoPipelineComposites(std::list<Forsyde::Id> stage) throw(
    		RuntimeException, InvalidModelException, InvalidProcessException, OutOfMemoryException,
    		    		InvalidArgumentException);

    /**
      * Recursive function that flattens the contents of a composite process. When
      * this function finishes execution, the composite process will be gone, and
      * all its first children's hierarchy will be raised one level.
      *
      * @param composite
      *        Composite process which needs to be flattened.
      * @param parent
      *        The parent composite process which will hold the children.
      *
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      * @throws InvalidArgumentException
      *         If \c root or \c process is \c NULL;
      */
    void flattenCompositeProcess(Composite* composite, Composite* parent) throw(
   		 RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException);


    /**
      * Groups equivalent \c SY::Comb processes into lists, so that they can be
      * transformed into ParallelComposite processes.
      *
      * @param parent
      *        The parent composite process which holds these equivalent processes.
      *
      * @returns List of lists with equivalent \c Comb processes.
      *
      * @throws InvalidArgumentException
      *         If \c parent or is \c NULL;
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      */
     std::list<std::list<Leaf*> > extractEquivalentCombs(Forsyde::Composite* parent)
         throw(InvalidArgumentException, OutOfMemoryException);

     /**
       * Groups equivalent Leaf processes into lists, so that they can be
       * transformed into ParallelComposite processes.
       *
       * @param parent
       *        The parent composite process which holds these equivalent processes.
       *
       * @returns List of lists with equivalent \c Comb processes.
       *
       * @throws InvalidArgumentException
       *         If \c parent or is \c NULL;
       * @throws OutOfMemoryException
       *         When there is not enough memory for creating a new object.
       */
     std::list<std::list<Leaf*> > extractEquivalentLeafs(Forsyde::Composite* parent)
         throw(InvalidArgumentException, OutOfMemoryException);

     /**
       * Calculates the cost of a process in a process network
       *
       * @param process
       *        The parent composite process which holds these equivalent processes.
       * @param on_device
       *        Determines whether the process is to execute on a parallel platform.
       *
       * @returns Combset of cost indexed by their type.
       *
       * @throws InvalidArgumentException
       *         If \c parent or is \c NULL;
       * @throws OutOfMemoryException
       *         When there is not enough memory for creating a new object.
       */
     std::map<CostType, unsigned long long> calculateCostInNetwork(Process* process, bool on_device)
     	 throw(RuntimeException, InvalidProcessException, InvalidArgumentException);

     /**
       * Removes redundant \c Zipx and \c Unzipx processes, rebuilding the connections
       * with the rest of the process network.
       *
       * @param parent
       *        The parent composite process which holds the redundant processes.
       *
       * @throws InvalidArgumentException
       *         If \c parent or is \c NULL;
       * @throws OutOfMemoryException
       *         When there is not enough memory for creating a new object.
       */
     void removeRedundantZipsUnzips(Forsyde::Composite* parent)
     	 throw(InvalidArgumentException, OutOfMemoryException);

     /**
      * Recursive function that checks for data dependency in a process network, by
      * parsing in downstream direction.
      *
      * @param current_process
      *        The current process being parsed.
      * @param to_compare_with
      *        Combset of actor processes that have to be checked for data dependencies
      *        agains the current one.
      *
      * @returns \c True if a data dependency has been found.
      *
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      * @throws InvalidArgumentException
      *         If \c root or \c process is \c NULL;
      */
     bool foundDependencyDownstream(Leaf* current_process, std::map<Id, SY::Comb*> to_compare_with)
 	 	 throw(RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException);

     /**
      * Recursive function that checks for data dependency in a process network, by
      * parsing in upstream direction.
      *
      * @param current_process
      *        The current process being parsed.
      * @param to_compare_with
      *        Combset of actor processes that have to be checked for data dependencies
      *        agains the current one.
      *
      * @returns \c True if a data dependency has been found.
      *
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      * @throws InvalidArgumentException
      *         If \c root or \c process is \c NULL;
      */
     bool foundDependencyUpstream(Leaf* current_process, std::map<Id, SY::Comb*> to_compare_with)
 	 	 throw(RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException);

     /**
      * Creates a new \c ParallelComposite process process from a list of equivalent
      * leaf processes.
      *
      * @param parent
      *        The parent composite process.
      * @param equivalent_processes
      *        List of equivalent Leaf processes.
      *
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      * @throws InvalidArgumentException
      *         If \c root or \c process is \c NULL;
      */
     void createParallelComposite(Composite* parent, std::list<Forsyde::Leaf*> equivalent_processes)
         throw(RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException);

     /**
      * Equips a ParallelComposite with its proper semantics and assigns it with the
      * desired functionality. It also takes care of adding Zipx and Unzipx processes
      * to preserve the semantics of the process network outside this ParallelComposite.
      *
      * @param reference_leaf
      *        The leaf process which is used as reference to equip the ParallelComposite.
      * @param parent
      *        Parent composite process.
      * @param new_pcomp
      *        The new parallel composite process, just created.
      * @param number_of_processes
      *        The number of processes that the new ParallelComposite will replace.
      *
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      * @throws InvalidArgumentException
      *         If \c root or \c process is \c NULL;
      */
     void prepareLeafForParallel(Forsyde::Leaf* reference_leaf, Forsyde::Composite* parent,
    		 Forsyde::ParallelComposite* new_pcomp, unsigned number_of_processes)
         throw(RuntimeException, InvalidProcessException, InvalidArgumentException, OutOfMemoryException);

     /**
      * Moves one process from its parent into a ParallelComposite process.
      *
      * @param reference_process
      *        The parent composite process.
      * @param old_parent
      *        The old parent composite process.
      * @param new_parent
      *        The new parallel composite process, acting as a new parent.
      *
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      */
     void moveToParallelComposite(Process* reference_process, Composite* old_parent,
    		 ParallelComposite* new_parent) throw (
    		 InvalidProcessException, OutOfMemoryException);

     /**
      * Moves one process from its parent to another parent.
      *
      * @param reference_process
      *        The reference process.
      * @param old_parent
      *        The old parent composite process.
      * @param new_parent
      *        The new parent composite process.
      *
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      */
     void moveToNewParent(Process* reference_process, Composite* old_parent,
    		 Composite* new_parent) throw (
    		 InvalidProcessException, OutOfMemoryException);

     /**
      * Redirects the dataflow in a process network through a Parallel Composite
      * process.
      *
      * @param old_process
      *        The process whose dataflow has to be redirected.
      * @param parent
      *        Its parent.
      * @param new_pcomp
      *        The ParallelComposite process which the dataflow is redirected to.
      *
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      */
     void redirectFlowThroughParallelComposite(Process* old_process, Composite* parent,
    		 ParallelComposite* new_pcomp) throw (
    		 InvalidArgumentException, RuntimeException, InvalidProcessException);

     /**
      * Redirects the dataflow inside a ParallelComposite process.
      *
      * @param source
      *        The source interface.
      * @param target
      *        The target interface.
      * @param reference
      *        The reference ParallelComposite process.
      * @param input
      *        \c True if the interface is considered an input for the ParallelComposite.
      *
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      * @throws InvalidProcessException
      *         When an error related to a ForSyDe process occurred.
      * @throws OutOfMemoryException
      *         When there is not enough memory for creating a new object.
      */
     void redirectFlow(Process::Interface* source, Process::Interface* target,
    		 ParallelComposite* reference, bool input) throw (
    		 InvalidArgumentException, RuntimeException, InvalidProcessException);

     /**
      * Helper function that returns the transfer coefficent between two processes.
      *
      * @param source_on_device
      *        \c True if the signal originates in a process mapped for parallel execution.
      * @param target_on_device
      *        \c True if the signal is targeted to a process mapped for parallel execution.
      * @param same_stream
      *        \c True if both source and target belong to the same pipeline stage.
      *
      * @returns Coefficient for calculating transfer cost.
      */
     int transferCoefficient(bool source_on_device, bool target_on_device, bool same_stream)
          	 throw();

     /**
      * Returns the index of an ID in a list of IDs.
      *
      * @param id
      *        \c True if the signal originates in a process mapped for parallel execution.
      * @param list
      *        \c True if the signal is targeted to a process mapped for parallel execution.
      *
      * @returns Iterator index for the position of this Id.
      */
     std::list<Id>::iterator getIdFromList(Id id, std::list<Id> list) throw();

     /**
      * Returns the index of an ID in a vector of IDs.
      *
      * @param id
      *        \c True if the signal originates in a process mapped for parallel execution.
      * @param vector
      *        \c True if the signal is targeted to a process mapped for parallel execution.
      *
      * @returns Index for the position of this Id.
      *
      * @throws RuntimeException
      *         When a program error has occurred. This most likely indicates a
      *         bug.
      */
     unsigned getPosOf(Id id, std::vector<Id> vector) throw(RuntimeException);
		/*if (pos_csec < 0 || pos_csec >=contained_s.size()){
			THROW_EXCEPTION(RuntimeException, string("The Id  \"")
							+ remaining[lrit].getString()
							+ "\" has index " + tools::toString(pos_csec)
							+ " in the contained section list:\n"
							+ printVector(contained_s));*/

     /**
      * Gets the portion from a data path between two defined processes.
      *
      * @param start
      *        Left margin for the desired portion.
      * @param stop
      *        Right margin for the desired portion.
      * @param list
      *        List of process IDs representing a data path.
      *
      * @returns List with IDs representing the desired portion from the data path.
      */
     std::list<Id> getPortionOfPath(Id start, Id stop, std::list<Id> list) throw();

     /**
      * Calculates the total cost of a loop in the process network.
      *
      * @param divergent_proc
      *        The process which closes the loop.
      * @param list
      *        List of IDs representing a data path.
      *
      * @returns Loop cost.
      */
     unsigned long long calculateLoopCost(Id divergent_proc, std::list<Id> list) throw();

     /**
      * Calculates the cost of a signal in the process network.
      *
      * @param source
      *        The source process.
      * @param target
      *        The target process.
      * @param sync
      *        \c True if global synchronization is used on the device.
      *
      * @returns Signal cost.
      */
     unsigned long long getSignalCost(Process* source, Process* target, bool sync) throw();

     /**
      * Helper function that prints a vector of IDs.
      *
      * @param vector
      *        The vector containing a data path.
      *
      * @returns Vector as string.
      */
     std::string printVector(std::vector<Id> vector) throw();


  private:


    /**
     * ForSyDe process network.
     */
    Forsyde::ProcessNetwork* const processnetwork_;

    /**
     * Logger.
     */
    Logger& logger_;

    /**
     * Cost coefficients.
     */
    Config::Costs costs_;

    /**
     * \c True if loop cost is calculated taking into account the number of delays.
     */
    bool delay_dependency_;

    /**
     * Container for accessing the quantum cost easily.
     */
    unsigned long long quantum_cost_;

    /**
     * Temporary container used for process network parsing purposes.
     */
    std::map<Id, bool> visited_processes_;

    /**
     * Combset of pipeline stages associated with their stage costs.
     */
    std::map<unsigned, unsigned long long> stage_costs_; // stage : trasfer cost

  public:

    /**
     * @brief Container class for storing a data path.
     *
     * Container class for storing a data path.
     */
	class DataPath {
	  public:
	    /**
	     * Creates a data path object.
	     */
    	DataPath() throw();

	    /**
	     * Default destructor.
	     */
    	~DataPath() throw();

	    /**
	     * Creates a string for printing the data path.
	     *
	     * @returns Data path as string.
	     */
    	std::string printDataPath() throw();

	    /**
	     * Checks if a process has already been visited in creating this data path.
	     *
		 * @param id
		 *        The process' ID.
	     *
	     * @returns \c True if the process already exists in the data path.
	     */
    	bool wasVisited(Id id) throw();

	    /**
	     * Finds the contained paths (for mapping to parallel execution) inside a data
	     * path.
	     *
	     * @returns List of contained paths.
	     */
    	std::list<std::list<Id> > getContainedPaths() throw();

	    /**
	     * Compares if two data paths are equivalent.
	     *
		 * @param rhs
		 *        DataPath object to compare with.
	     */
    	void operator=(const DataPath& rhs) throw();

	    /**
	     * \c True if the data path contains a loop.
	     */
    	bool is_loop_;

	    /**
	     * ID of the input first process.
	     */
    	Id input_process_;

	    /**
	     * ID of the last process.
	     */
    	Id output_process_;

	    /**
	     * The data path container.
	     */
		std::list<std::pair<Id, bool> > path_;

	};


};

}
}

#endif
