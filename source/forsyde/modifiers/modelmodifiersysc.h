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

    bool splitPipelineStages(std::vector<Id> contained_sections)
    throw (RuntimeException, InvalidProcessException, OutOfMemoryException, InvalidModelException);

    std::map<unsigned, std::list<Forsyde::Id> > orderStages() throw(
    		RuntimeException, InvalidModelException, InvalidProcessException, OutOfMemoryException,
    		InvalidArgumentException);

    void groupIntoPipelineComposites(std::list<Forsyde::Id> stage) throw(
    		RuntimeException, InvalidModelException, InvalidProcessException, OutOfMemoryException,
    		    		InvalidArgumentException);

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

     std::map<CostType, unsigned long long> calculateCostInNetwork(Process* process, bool on_device)
     	 throw(RuntimeException, InvalidProcessException, InvalidArgumentException);

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

     void redirectFlow(Process::Interface* source, Process::Interface* target,
    		 ParallelComposite* reference, bool input) throw (
    		 InvalidArgumentException, RuntimeException, InvalidProcessException);

     void convertPCompToLeaf(ParallelComposite* reference) throw (
    		 InvalidArgumentException, RuntimeException, InvalidProcessException);

     int transferCoefficient(bool source_on_device, bool target_on_device, bool same_stream)
          	 throw();

     std::list<Id>::iterator getIdFromList(Id id, std::list<Id> list) throw();

     unsigned getPosOf(Id id, std::vector<Id> vector) throw(RuntimeException);
		/*if (pos_csec < 0 || pos_csec >=contained_s.size()){
			THROW_EXCEPTION(RuntimeException, string("The Id  \"")
							+ remaining[lrit].getString()
							+ "\" has index " + tools::toString(pos_csec)
							+ " in the contained section list:\n"
							+ printVector(contained_s));*/

     std::list<Id> getPortionOfPath(Id start, Id stop, std::list<Id> list) throw();

     unsigned long long calculateLoopCost(Id divergent_proc, std::list<Id> list) throw();

     unsigned long long getSignalCost(Process* source, Process* target, bool sync) throw();

     std::string printVector(std::vector<Id> vector) throw();


  private:


    /**
     * ForSyDe processnetwork.
     */
    Forsyde::ProcessNetwork* const processnetwork_;

    /**
     * Logger.
     */
    Logger& logger_;

    Config::Costs costs_;

    bool delay_dependency_;

    unsigned long long quantum_cost_;

    std::map<Id, bool> visited_processes_;

    std::map<unsigned, unsigned long long> stage_costs_; // stage : trasfer cost

  public:
	class DataPath {
	  public:
    	DataPath() throw();

    	~DataPath() throw();

    	std::string printDataPath() throw();

    	bool wasVisited(Id id) throw();

    	std::list<std::list<Id> > getContainedPaths() throw();

    	void operator=(const DataPath& rhs) throw();

    	bool is_loop_;

    	Id input_process_;

    	Id output_process_;

		std::list<std::pair<Id, bool> > path_;

	};


};

}
}

#endif
