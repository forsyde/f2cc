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

#include "synthesizer02.h"
#include "schedulefinder.h"
#include "../forsyde/SY/delaysy.h"
#include "../forsyde/SY/zipxsy.h"
#include "../forsyde/SY/unzipxsy.h"
#include "../forsyde/SY/fanoutsy.h"
#include "../forsyde/SY/combsy.h"
#include "../language/cfunction.h"
#include "../language/cdatatype.h"
#include "../tools/tools.h"
#include "../exceptions/unknownarraysizeexception.h"
#include <new>
#include <map>
#include <vector>
#include <cmath>

using namespace f2cc;
using namespace f2cc::Forsyde;
using namespace f2cc::Forsyde::SY;
using std::string;
using std::list;
using std::vector;
using std::set;
using std::pair;
using std::make_pair;
using std::bad_alloc;
using std::map;
using std::ceil;

const string SynthesizerExperimental::kIndents = "    ";
const string SynthesizerExperimental::kExecSuffix = "_exec_wrapper";
const string SynthesizerExperimental::kKernelFuncSuffix = "_kernel";
const string SynthesizerExperimental::kKernelStageSuffix = "_kernel_stage";
const string SynthesizerExperimental::kKernelWrapSuffix = "_kernel_wrapper";
const string SynthesizerExperimental::kProcessNetworkInputParameterPrefix = "input";
const string SynthesizerExperimental::kProcessNetworkOutputParameterPrefix = "output";

SynthesizerExperimental::SynthesizerExperimental(ProcessNetwork* processnetwork, Logger& logger, Config& config)
        throw(InvalidArgumentException) : processnetwork_(processnetwork), logger_(logger),
                                          config_(config), in_kernel_(false),
                                          enable_device_sync_(true){
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }
}

SynthesizerExperimental::~SynthesizerExperimental() throw() {
    set<Signal*>::iterator it;
    for (it = signals_.begin(); it != signals_.end(); ++it) {
        delete *it;
    }
}

SynthesizerExperimental::CodeSet SynthesizerExperimental::generateCCode()
    throw(InvalidModelException, IOException, RuntimeException) {
    target_platform_ = SynthesizerExperimental::C;
    return generateCode();
}

SynthesizerExperimental::CodeSet SynthesizerExperimental::generateCudaCCode()
    throw(InvalidModelException, IOException, RuntimeException) {
    target_platform_ = SynthesizerExperimental::CUDA;
    return generateCode();
}

SynthesizerExperimental::CodeSet SynthesizerExperimental::generateCode()
    throw(InvalidModelException, IOException, RuntimeException) {
    logger_.logMessage(Logger::INFO, "Checking that the internal processnetwork is "
                       "valid for synthesis...");
    checkProcessNetwork();
    logger_.logMessage(Logger::INFO, "All checks passed");

	Composite* root = processnetwork_->getComposite(Id("f2cc0"));
	if(!root){
		THROW_EXCEPTION(InvalidModelException, string("Process network ")
						+ "does not have a root process");
	}
    schedule_.clear();
    logger_.logMessage(Logger::INFO, "Generating sequential schedules for all composite processes...");

    list<Composite*> stage_pcomps  = processnetwork_->getComposites();
	for (list<Composite*>::iterator pcit = stage_pcomps.begin(); pcit != stage_pcomps.end(); ++pcit){
		logger_.logMessage(Logger::INFO, "Generating process schedule for "
				+ (*pcit)->getName().getString()
				+ "...");
		findSchedule(*pcit);
		logger_.logMessage(Logger::INFO, string("Process schedule for "
				+ (*pcit)->getName().getString() + ":\n")
				+ scheduleToString(schedule_[*(*pcit)->getId()] ));

	}

	functions_ = processnetwork_->getFunctions();
    logger_.logMessage(Logger::INFO, "Generating wrapper functions for "
                       "composite processes...");
    generateCompositeWrapperFunctions();

    CodeSet code;
    code.implementation = "";

    for (list<CFunction*>::iterator fit = functions_.begin(); fit != functions_.end(); ++fit){
    	if ((*fit) == functions_.back()) code.implementation += (*fit)->getStringNewRoot() + "\n\n";
    	else code.implementation += (*fit)->getStringNew() + "\n\n";
    }

    return code;



/*////////////TODO!!!!!!!!!!!!!!!!!!!!!!!!!!!!! execute model functions (in, out, N)
    logger_.logMessage(Logger::INFO, "Discovering signal variable data "
                       "types...");
    discoverSignalDataTypes();

    logger_.logMessage(Logger::INFO, "Propagating array sizes...");
    propagateArraySizesBetweenSignals();
    propagateSignalArraySizesToLeafFunctions();

    logger_.logMessage(Logger::INFO, "Setting data types of array input signal "
                       "variables as 'const'...");
    setInputArraySignalVariableDataTypesAsConst();

    logger_.logMessage(Logger::INFO, "Creating delay variables...");
    createDelayVariables();

    switch (target_platform_) {
        case C: {
            logger_.logMessage(Logger::INFO, "Generating C code...");
            break;
        }

        case CUDA: {
            logger_.logMessage(Logger::INFO, "Generating CUDA C code...");
            break;
        }

        default:
            // Should never get here
            break;
    }

    CodeSet code;
    string boiler_plate = string()
        + "////////////////////////////////////////////////////////////\n"
        + "// AUTO-GENERATED BY F2CC " + config_.getVersion() + "\n"
        + "////////////////////////////////////////////////////////////\n";
    code.header = boiler_plate + "\n";
    logger_.logMessage(Logger::DEBUG, "Generating processnetwork function "
                       "description...");
    code.header += generateCompositeDescription() + "\n";
    logger_.logMessage(Logger::DEBUG, "Generating processnetwork function "
                       "prototype...");
    code.header += generateCompositePrototypeCode() + ";\n";
    code.implementation = boiler_plate
        + "\n"
        + "#include \"" + config_.getHeaderOutputFile() + "\"\n";
    if (target_platform_ == CUDA) {
        code.implementation += string()
            + "#include <stdio.h> // Remove when error handling and "
            + "reporting of too small input data is fixed\n"
            + "\n";
        logger_.logMessage(Logger::DEBUG, "Generating kernel config struct "
                           "definition...");
        code.implementation += generateKernelConfigStructDefinitionCode()
            + "\n";
        logger_.logMessage(Logger::DEBUG, "Generating kernel config function "
                           "definition...");
        code.implementation += generateKernelConfigFunctionDefinitionCode()
            + "\n";
    }
    else {
        code.implementation += "\n";
    }

    logger_.logMessage(Logger::DEBUG, "Generating leaf function "
                       "definitions...");
    code.implementation += generateLeafFunctionDefinitionsCode() + "\n";
    logger_.logMessage(Logger::DEBUG, "Generating processnetwork function "
                       "definition...");
    code.implementation += generateCompositeDefinitionCode() + "\n";

    return code;*/
}

void SynthesizerExperimental::checkProcessNetwork()
    throw(InvalidModelException, IOException, RuntimeException) {}


void SynthesizerExperimental::findSchedule(Forsyde::Composite* stage) throw (
				IOException, RuntimeException) {
    ScheduleFinder schedule_finder(processnetwork_, logger_);
    schedule_.insert(make_pair(*stage->getId(), schedule_finder.findSchedule(stage)));
}

void SynthesizerExperimental::generateCompositeWrapperFunctions()
    throw(InvalidModelException, IOException, RuntimeException) {
    list<Composite*> stage_pcomps  = processnetwork_->getComposites();
	for (list<Composite*>::reverse_iterator pcit = stage_pcomps.rbegin();
			pcit != stage_pcomps.rend(); ++pcit){
		if ( (target_platform_ == CUDA) && (*(*pcit)->getId() == Id("f2cc0")) ){
			logger_.logMessage(Logger::INFO, "Generating streamed CUDA kernel functions for "
							   "adjacent parallel composite functions...");
			(*pcit)->setWrapper(generateCudaKernelWrapper(*pcit, schedule_[*(*pcit)->getId()]));
		}
		else (*pcit)->setWrapper(generateWrapperForComposite(*pcit, schedule_[*(*pcit)->getId()]));
	}
}


CFunction* SynthesizerExperimental::generateCudaKernelWrapper(Forsyde::Composite* composite,
		std::list<Forsyde::Id> schedule)
    throw(InvalidModelException, IOException, RuntimeException) {

    signals_.clear();
    delay_variables_.clear();
    logger_.logMessage(Logger::INFO, string() + "Creating signal variables for \""
    		+ composite->getId()->getString() + "\"...");
     createSignals(composite);

     logger_.logMessage(Logger::INFO, string() + "Creating delay variables for \""
     		+ composite->getId()->getString() + "\"...");
     createDelayVariables(schedule);

    unsigned count;
    list<Composite::IOPort*>::iterator iit;
    list<CVariable> in_vars;
	list<Composite::IOPort*> inputs  = composite->getInIOPorts();
	for (count = 0, iit  = inputs.begin(); iit != inputs.end(); ++iit){
		Signal* sig = getSignalInsideByInPort(*iit);
		string new_varname = string ("in" + tools::toString(++count));
		CVariable new_variable(new_varname, *sig->getDataType());
		in_vars.push_back(new_variable);
	}

    list<CVariable> out_vars;
	list<Composite::IOPort*> outputs  = composite->getOutIOPorts();
	for (count = 0, iit = outputs.begin(); iit != outputs.end(); ++iit){
		Signal* sig = getSignalInsideByOutPort(*iit);
		string new_varname = string ("out" + tools::toString(++count));
		CDataType* var_dt = sig->getVariable().getDataType();
		var_dt->setIsConst(false);
		sig->setDataType(*var_dt);
		CVariable new_variable(new_varname, *sig->getDataType());
		out_vars.push_back(new_variable);
	}

	list<Leaf*> leafs  = composite->getProcesses();
	for (list<Leaf*>::iterator lit  = leafs.begin(); lit != leafs.end(); ++lit){
		SY::Comb* comb = dynamic_cast<SY::Comb*>(*lit);
		if (comb){
			if (composite->findRelation(comb) != Hierarchy::FirstChild){
				comb->getFunction()->setDeclarationPrefix("__device__");
			}
		}
	}

	Config::Costs costs = config_.getCosts();
	costs.n_stages = 0;
	bool previous_kernel = false;
	unsigned max_num_proc = 0;
	list<list<Id> > kernel_schedules;
	list<Id> current_kernel_schedule;
	string current_kernel_id;
    for (list<Id>::iterator sit = schedule.begin(); sit != schedule.end(); ++sit) {
    	Composite* comp = composite->getComposite(*sit);
    	if (comp){
    		ParallelComposite* pcomp = dynamic_cast<ParallelComposite*>(comp);
    		if (pcomp){
    			costs.n_stages++;
    			if (!previous_kernel) {
    				costs.n_stages += 2;
    				current_kernel_id = pcomp->getId()->getString();
    			}
    			previous_kernel = true;
    			current_kernel_schedule.push_back(*pcomp->getId());
    			if ((unsigned)pcomp->getNumProcesses() > max_num_proc)
    				max_num_proc = pcomp->getNumProcesses();
    		}
    		else if (previous_kernel){
    			unsigned n_proc = (unsigned)ceil(
    					(float)max_num_proc / (float)config_.getCosts().n_bursts);
    		     logger_.logMessage(Logger::DEBUG, string() + "Creating a CUDA kernel from the contained"
    		    		 "section \""
    		     		+ current_kernel_schedule.front().getString() + "--"
    		     		+ current_kernel_schedule.back().getString() + "\"...");
    			previous_kernel = false;
    			CFunction* kernel_exec = generateWrapperForKernelComposite(current_kernel_id, composite,
    					current_kernel_schedule, n_proc);
    			CFunction* kernel_func = generateCudaKernelFunction(kernel_exec, n_proc);
    			functions_.push_back(kernel_func);
    			std::cout<<kernel_func->getStringNew()<<"\n";
    			kernel_schedules.push_back(current_kernel_schedule);
    			current_kernel_schedule.empty();

    		    signals_.clear();
    		    logger_.logMessage(Logger::INFO, string() + "Creating signal variables for \""
    		    		+ composite->getId()->getString() + "\"...");
    		     createSignals(composite);

    			//max_num_proc = 0;
    		}
    	}
    }

	if (previous_kernel){
		unsigned n_proc = (unsigned)ceil(
				(float)max_num_proc / (float)config_.getCosts().n_bursts);
	     logger_.logMessage(Logger::INFO, string() + "Creating a CUDA kernel from the contained"
	    		 "section \""
	     		+ current_kernel_schedule.front().getString() + "--"
	     		+ current_kernel_schedule.back().getString() + "\"...");
		previous_kernel = false;
		CFunction* kernel_exec = generateWrapperForKernelComposite(current_kernel_id, composite,
				current_kernel_schedule, n_proc);
		CFunction* kernel_func = generateCudaKernelFunction(kernel_exec, n_proc);
		functions_.push_back(kernel_func);
		std::cout<<kernel_func->getStringNew()<<"\n";
		kernel_schedules.push_back(current_kernel_schedule);
		current_kernel_schedule.empty();

	    signals_.clear();
	    logger_.logMessage(Logger::INFO, string() + "Creating signal variables for \""
	    		+ composite->getId()->getString() + "\"...");
	     createSignals(composite);
	}

    config_.setCosts(costs);

	logger_.logMessage(Logger::INFO, string() + "Optimizing kernel for "
			+ tools::toString(config_.getCosts().n_bursts) + " burst(s) and "
			+ tools::toString(config_.getCosts().n_stages) + " stage(s)..." );


    string new_body = generateCudaRootWrapperCode(composite, schedule, kernel_schedules, max_num_proc);
	new_body = renameVariables(new_body, composite);

	CDataType n_param_type(CDataType::UNSIGNED_LONG_INT, false, false, 0, false, false);
	in_vars.push_back(CVariable("N", n_param_type));

	CFunction* wrapper_function = new CFunction(string ("cuda" + composite->getName().getString()),
			out_vars, in_vars, new_body);

	processnetwork_->addFunction(wrapper_function);
	functions_.push_back(wrapper_function);

	std::cout<<wrapper_function->getStringNew()<<"\n";

	return wrapper_function;

}


CFunction* SynthesizerExperimental::generateCudaKernelFunction(CFunction* function,
                                                  size_t num_leafs)
    throw(InvalidModelException, IOException, RuntimeException) {
    string new_name = function->getName() + kKernelFuncSuffix;
    string offset_param_name("index_offset");
    CDataType offset_param_type(CDataType::INT, false, false, 0, false, false);
    list<CVariable*> in_parameters = function->getInputParameters();
    list<CVariable*> out_parameters = function->getOutputParameters();
    vector<string> input_data_variable_name;
    vector<string> output_data_variable_name;


    list<CVariable> new_in_parameters;
    for (list<CVariable*>::iterator pit = in_parameters.begin(); pit != in_parameters.end(); ++pit) {
    	new_in_parameters.push_back(**pit);
    }
    list<CVariable> new_out_parameters;
    for (list<CVariable*>::iterator pit = out_parameters.begin(); pit != out_parameters.end(); ++pit) {
    	new_out_parameters.push_back(**pit);
    }

    // Create body
    string new_body = string("{\n");

    unsigned count;
    list<CVariable*>::iterator pit;
    list<CVariable>::iterator npit;
    // Generate code for calculating the index using the thread block X and
    // thread X coordinates
    new_body += kIndents + "unsigned int global_index = "
        + "(blockIdx.x * blockDim.x + threadIdx.x) + " + offset_param_name
        + ";\n";
    if (config_.useSharedMemoryForInput()) {
    	for (count = 0, pit = in_parameters.begin(); pit != in_parameters.end(); ++pit, ++count) {
			logger_.logMessage(Logger::INFO, "USING SHARED MEMORY FOR INPUT DATA: "
							   "YES");
			input_data_variable_name.push_back(string("input_cached" + tools::toString(count)));
			new_body += kIndents + "extern __shared__ "
				+ CDataType::typeToString((*pit)->getDataType()->getType()) + " "
				+ input_data_variable_name.back() + "[];\n";
    	}
    	for (count = 0, pit = out_parameters.begin(); pit != out_parameters.end(); ++pit, ++count) {
			logger_.logMessage(Logger::INFO, "USING SHARED MEMORY FOR INPUT DATA: "
							   "YES");
			output_data_variable_name.push_back(string("output_cached" + tools::toString(count)));
			new_body += kIndents + "extern __shared__ "
				+ CDataType::typeToString((*pit)->getDataType()->getType()) + " "
				+ output_data_variable_name.back() + "[];\n";
    	}
    }
    else {
        logger_.logMessage(Logger::INFO, "USING SHARED MEMORY FOR INPUT DATA: "
                           "NO");
        for (count = 0, pit = in_parameters.begin(); pit != in_parameters.end(); ++pit, count++) {
        	input_data_variable_name.push_back((*pit)->getReferenceString());
        }
        for (count = 0, pit = out_parameters.begin(); pit != out_parameters.end(); ++pit, count++) {
        	output_data_variable_name.push_back((*pit)->getReferenceString());
        }
    }

    // If too many threads are generated, then we want to avoid them from
    // doing any processing, and we do this with an IF statement checking if
    // the thread is out of range
    new_body += kIndents + "if (global_index < "
        + tools::toString(num_leafs) + ") {\n";
    vector<string> in_param_index_variable_name;
    vector<string> out_param_index_variable_name;
    string global_index_variable_name = "global_index";
    for (count = 0, pit = in_parameters.begin(); pit != in_parameters.end(); ++pit, count++) {
    	in_param_index_variable_name.push_back(string(input_data_variable_name[count] + "_index"));
    }
    for (count = 0, pit = out_parameters.begin(); pit != out_parameters.end(); ++pit, count++) {
    	out_param_index_variable_name.push_back(string(output_data_variable_name[count] + "_index"));
    }

    if (config_.useSharedMemoryForInput()) {
        // Generate code for copying input data from global memory into shared
        // memory
        for (count = 0, npit = new_in_parameters.begin();
        		npit != new_in_parameters.end(); ++npit, count++) {
        	new_body += kIndents + kIndents + "int " + in_param_index_variable_name[count]
				+ " = threadIdx.x * "
				+ tools::toString((*npit).getDataType()->getArraySize() / num_leafs) + ";\n";

            new_body += kIndents + kIndents + "int global_" + in_param_index_variable_name[count]
                + " = global_index * "
                + tools::toString((*npit).getDataType()->getArraySize() / num_leafs) + ";\n";
            int num_elements_per_thread = (*npit).getDataType()->getArraySize();
            for (int i = 0; i < num_elements_per_thread; ++i) {
                new_body += kIndents + kIndents
                    + input_data_variable_name[count] + "[" + in_param_index_variable_name[count]
                    + " + " + tools::toString(i) + "] = " + (*npit).getReferenceString() + "["
                    + "global_" + in_param_index_variable_name[count]
                    +  " + " + tools::toString(i) + "];\n";
            }
        }
        for (count = 0, npit = new_out_parameters.begin();
        		npit != new_out_parameters.end(); ++npit, count++) {
        	new_body += kIndents + kIndents + "int " + out_param_index_variable_name[count]
				+ " = threadIdx.x * "
				+ tools::toString((*npit).getDataType()->getArraySize() / num_leafs) + ";\n";

            new_body += kIndents + kIndents + "int global_" + out_param_index_variable_name[count]
                + " = global_index * "
                + tools::toString((*npit).getDataType()->getArraySize() / num_leafs) + ";\n";
            int num_elements_per_thread = (*npit).getDataType()->getArraySize();
            for (int i = 0; i < num_elements_per_thread; ++i) {
                new_body += kIndents + kIndents
                    + output_data_variable_name[count] + "[" + out_param_index_variable_name[count]
                    + " + " + tools::toString(i) + "] = " + (*npit).getReferenceString() + "["
                    + "global_" + out_param_index_variable_name[count]
                    +  " + " + tools::toString(i) + "];\n";
            }
        }
    }
    else {
        for (count = 0, npit = new_in_parameters.begin();
        		npit != new_in_parameters.end(); ++npit, count++) {
        	std::cout<<count<<" , "<<in_param_index_variable_name[count]<<"\n";
			new_body += kIndents + kIndents + "int " + in_param_index_variable_name[count]
				+ " = global_index * "
				+ tools::toString((*npit).getDataType()->getArraySize() / num_leafs) + ";\n";
        }
        for (count = 0, npit = new_out_parameters.begin();
        		npit != new_out_parameters.end(); ++npit, count++) {
        	std::cout<<count<<" , "<<out_param_index_variable_name[count]<<"\n";
			new_body += kIndents + kIndents + "int " + out_param_index_variable_name[count]
				+ " = global_index * "
				+ tools::toString((*npit).getDataType()->getArraySize() / num_leafs) + ";\n";
        }
    }

    // Generate code for invoking the kernel
    new_body += kIndents + kIndents + function->getName() + "(";
    for (count = 0, npit = new_out_parameters.begin();
    		npit != new_out_parameters.end(); ++npit, count++) {
    	//if ((*npit).getDataType()->getArraySize() / num_leafs > 1)
    	new_body += "&";
    	new_body += output_data_variable_name[count] + "["
    			+ out_param_index_variable_name[count] + "], ";
    }
    for (count = 0, npit = new_in_parameters.begin();
    		npit != new_in_parameters.end(); ++npit, count++) {
    	if ((*npit).getDataType()->isArray() / num_leafs > 1) new_body += "&";
    	new_body += input_data_variable_name[count] + "["
    			+ in_param_index_variable_name[count] + "]";
    	if (count < in_param_index_variable_name.size() - 1) new_body += ", ";
    }
    new_body += ");\n";
    new_body += kIndents + "}\n";
    new_body += "}";

    new_in_parameters.push_back(CVariable(offset_param_name, offset_param_type));

    CFunction* new_function = new CFunction(new_name, new_out_parameters, new_in_parameters, new_body,
            string("__global__"));

    return new_function;
}


CFunction* SynthesizerExperimental::generateWrapperForComposite(Composite* composite,
		std::list<Forsyde::Id> schedule)
    throw(InvalidModelException, IOException, RuntimeException) {
    //string new_name = "_func_exec";

    signals_.clear();
    delay_variables_.clear();
    logger_.logMessage(Logger::INFO, string() + "Creating signal variables for \""
    		+ composite->getId()->getString() + "\"...");
     createSignals(composite);

     logger_.logMessage(Logger::INFO, string() + "Creating delay variables for \""
     		+ composite->getId()->getString() + "\"...");
     createDelayVariables(schedule);

    unsigned count;
    list<Composite::IOPort*>::iterator iit;
    list<CVariable> in_vars;
	list<Composite::IOPort*> inputs  = composite->getInIOPorts();
	for (count = 0, iit  = inputs.begin(); iit != inputs.end(); ++iit){
		Signal* sig = getSignalInsideByInPort(*iit);
		string new_varname = string ("in" + tools::toString(++count));
		CVariable new_variable(new_varname, *sig->getDataType());
		in_vars.push_back(new_variable);
	}

    list<CVariable> out_vars;
	list<Composite::IOPort*> outputs  = composite->getOutIOPorts();
	for (count = 0, iit = outputs.begin(); iit != outputs.end(); ++iit){
		Signal* sig = getSignalInsideByOutPort(*iit);
		string new_varname = string ("out" + tools::toString(++count));
		CDataType* var_dt = sig->getVariable().getDataType();
		var_dt->setIsConst(false);
		sig->setDataType(*var_dt);
		CVariable new_variable(new_varname, *sig->getDataType());
		out_vars.push_back(new_variable);
	}

    string new_body = generateCompositeDefinitionCode(composite, schedule);
    new_body = renameVariables(new_body, composite);

    CFunction* wrapper_function = new CFunction(string (composite->getName().getString() + kExecSuffix ),
			out_vars, in_vars, new_body);

    functions_.push_back(wrapper_function);

	std::cout<<wrapper_function->getStringNew()<<"\n";

	if (*composite->getId() == Id("f2cc0")){
		string exec_body = generateRootExecutionCode(*wrapper_function, in_vars, out_vars);
		CFunction* execution = new CFunction(string ("sequential" +  composite->getName().getString()),
					out_vars, in_vars, exec_body);

		functions_.push_back(execution);
		std::cout<<execution->getStringNewRoot()<<"\n";
	}
    return wrapper_function;

}

CFunction* SynthesizerExperimental::generateWrapperForKernelComposite(string current_id,
		Composite* composite, std::list<Forsyde::Id> schedule, unsigned n_procs)
    throw(InvalidModelException, IOException, RuntimeException) {
    //string new_name = "_func_exec";

    unsigned count;
    list<Composite::IOPort*>::iterator iit;

    set<Signal*>::iterator it;
    for (it = signals_.begin(); it != signals_.end(); ++it) {
    	CDataType new_type = *(*it)->getDataType();
    	new_type.setArraySize(new_type.getArraySize() / n_procs);
    	if (new_type.getArraySize() <= 1)  new_type.setIsArray(false);
    	(*it)->setDataType(new_type);
    }
    in_kernel_ = true;

    Composite* entrance = composite->getComposite(schedule.front());
    list<CVariable> in_vars;
	list<Composite::IOPort*> inputs  = entrance->getInIOPorts();
	for (count = 0, iit  = inputs.begin(); iit != inputs.end(); ++iit){
		Signal* sig = getSignalByInPort(*iit);
		string new_varname = string ("in" + tools::toString(++count));
		CDataType new_type = *sig->getDataType();
		CVariable new_variable(new_varname, *sig->getDataType());
		in_vars.push_back(new_variable);
	}

	Composite* exit = composite->getComposite(schedule.back());
    list<CVariable> out_vars;
	list<Composite::IOPort*> outputs  = exit->getOutIOPorts();
	for (count = 0, iit = outputs.begin(); iit != outputs.end(); ++iit){
		Signal* sig = getSignalByOutPort(*iit);
		string new_varname = string ("out" + tools::toString(++count));
		CDataType* var_dt = sig->getVariable().getDataType();
		var_dt->setIsConst(false);
		sig->setDataType(*var_dt);
		CVariable new_variable(new_varname, *sig->getDataType());
		out_vars.push_back(new_variable);
	}

    string new_body = generateCompositeDefinitionCode(composite, schedule);
    new_body = renameVariables(new_body, composite);

    CFunction* wrapper_function = new CFunction(string (current_id + kKernelStageSuffix ),
			out_vars, in_vars, new_body, string("__device__"));

    functions_.push_back(wrapper_function);

	std::cout<<wrapper_function->getStringNew()<<"\n";

    return wrapper_function;

}


string SynthesizerExperimental::generateCompositeDefinitionCode(Composite* composite,
		std::list<Forsyde::Id> schedule)
    throw(InvalidModelException, IOException, RuntimeException) {
    string code;
    code += " {\n";
    code += kIndents + "int i; // Can safely be removed if the compiler warns\n"
        + kIndents + "       // about it being unused\n";
    code += generateSignalVariableDeclarationsCode(composite) + "\n";
    code += generateDelayVariableDeclarationsCode() + "\n";
    code += kIndents + "// Execute leafs\n";

    // First, execute the first step of all delay leafs
    for (list<Id>::iterator it = schedule.begin(); it != schedule.end(); ++it) {
        Leaf* current_leaf = composite->getProcess(*it);
        Composite* current_composite = composite->getComposite(*it);
        if (!current_leaf && !current_composite) {
            THROW_EXCEPTION(IllegalStateException, string("Leaf \"") +
                            it->getString() + "\" not found");
        }
        if (current_leaf) {
			if (composite->findRelation(current_leaf) == Hierarchy::FirstChild){
				if (delay* delaysy = dynamic_cast<delay*>(current_leaf)) {
					try {
						code += generateLeafExecutionCodeFordelayStep1(delaysy);
					}
					catch (InvalidModelException& ex) {
						THROW_EXCEPTION(InvalidModelException, "Error in leaf \""
										+ current_leaf->getId()->getString() + "\": "
										+ ex.getMessage());
					}
				}
			}
        }
    }

    // Then, execute all leafs in order, but ignore all delay leafs

    if (in_kernel_ && enable_device_sync_) code += "\n" + kIndents + "cudaDeviceSync();\n\n";

    for (list<Id>::iterator it = schedule.begin(); it != schedule.end(); ++it) {
        Leaf* current_leaf = composite->getProcess(*it);
        Composite* current_composite = composite->getComposite(*it);
        if (!current_leaf && !current_composite) {
            THROW_EXCEPTION(IllegalStateException, string("Leaf \"") +
                            it->getString() + "\" not found");
        }
        try {
        	if (current_leaf) code += generateProcessExecutionCode(current_leaf);
        	else if (current_composite) code += generateProcessExecutionCode(current_composite);
        }
        catch (InvalidModelException& ex) {
            THROW_EXCEPTION(InvalidModelException, "Error in leaf \""
                            + current_leaf->getId()->getString() + "\": "
                            + ex.getMessage());
        }

        if (in_kernel_ && enable_device_sync_) code += "\n" + kIndents + "cudaDeviceSync();\n\n";

    }

    // After the entire schedule has been executed, execute the second step
    // of all delay leafs
    for (list<Id>::iterator it = schedule.begin(); it != schedule.end(); ++it) {
        Leaf* current_leaf = composite->getProcess(*it);
        Composite* current_composite = composite->getComposite(*it);
        if (!current_leaf && !current_composite) {
            THROW_EXCEPTION(IllegalStateException, string("Leaf \"") +
                            it->getString() + "\" not found");
        }
        if (current_leaf) {
			if (composite->findRelation(current_leaf) == Hierarchy::FirstChild){
				if (delay* delaysy = dynamic_cast<delay*>(current_leaf)) {
					try {
						code += generateLeafExecutionCodeFordelayStep2(delaysy);
					}
					catch (InvalidModelException& ex) {
						THROW_EXCEPTION(InvalidModelException, "Error in leaf \""
										+ current_leaf->getId()->getString() + "\": "
										+ ex.getMessage());
					}
				}
			}
        }
    }

    code += "\n";
    code += generateSignalVariableCleanupCode(composite);
    code += "}";
    return code;
}


std::string SynthesizerExperimental::generateCudaRootWrapperCode(Composite* composite,
		std::list<Forsyde::Id> schedule, std::list<std::list<Forsyde::Id> > k_schedules,
		unsigned n_proc)
    throw(InvalidModelException, IOException, RuntimeException) {
    string code;


    unsigned burst_size = 0;
    CVariable max_input;
    list<Composite::IOPort*>::iterator iit;
    list<Composite::IOPort*> inputs  = composite->getInIOPorts();
    for (iit = inputs.begin(); iit != inputs.end(); ++iit){
    	Signal* sig = getSignalInsideByInPort(*iit);
    	if (sig->getVariable().getDataType()->getArraySize() > burst_size){
    		burst_size = sig->getVariable().getDataType()->getArraySize();
    		max_input = sig->getVariable();
    	}
    }
	burst_size = burst_size / n_proc;

    unsigned n_streams = config_.getCosts().n_stages;
    unsigned n_bursts = config_.getCosts().n_bursts;

    code += " {\n";
    code += kIndents + "int i; // Can safely be removed if the compiler warns\n"
        + kIndents + "       // about it being unused\n";

    code += kIndents + "cudaStream_t stream[" + tools::toString(n_streams) + "];\n";

    code += kIndents + "struct cudaDeviceProp prop;\n"
        + kIndents + "int max_threads_per_block;\n"
        + kIndents + "int shared_memory_per_sm;\n"
        + kIndents + "int num_multicores;\n"
        + kIndents + "int full_utilization_thread_count;\n"
        + kIndents + "int is_timeout_activated;\n\n";

    // Generate code for fetching the device information
    code += kIndents + "// Get GPGPU device information\n"
        + kIndents + "// @todo Better error handling\n"
        + kIndents + "if (cudaGetDeviceProperties(&prop, 0) != cudaSuccess) {\n"
        + kIndents + kIndents + "printf(\"ERROR: Failed to allocate GPU "
        + "memory\\n\");\n"
        + kIndents + kIndents + "exit(-1);\n"
        + kIndents + "}\n"
        + kIndents + "max_threads_per_block = prop.maxThreadsPerBlock;\n"
        + kIndents + "shared_memory_per_sm = (int) "
        + "prop.sharedMemPerBlock;\n"
        + kIndents + "num_multicores = prop.multiLeaforCount;\n"
        + kIndents + "is_timeout_activated = "
        + "prop.kernelExecTimeoutEnabled;\n"
        + kIndents + "full_utilization_thread_count = max_threads_per_block * "
        + "num_multicores;\n";

    // Generate code for checking whether the data input is enough for full
    // utilization of this device
    code += kIndents + "if (" + tools::toString(n_proc)
        + " < full_utilization_thread_count) {\n"
        + kIndents + kIndents + "// @todo Use some other way of reporting this "
        + "to the user (printf may not always be acceptable)\n"
        + kIndents + kIndents + "printf(\"WARNING: The input data is too small "
        + "to achieve full utilization of this device!\\n\");\n"
        + kIndents + "}\n\n";


    code += generateCudaVariableDeclarationsCode(composite, k_schedules) + "\n";

    std::cout<<code<<"\n";

    code += kIndents + "unsigned long data_index[" +tools::toString(n_streams)+ "] = {0 code +=";
    for (unsigned i = 1; i < n_streams; i++){
    	code += ", " + tools::toString(i);
    }
    code += "};\n";
    code += kIndents + "unsigned long number_of_bursts = N * " + tools::toString(n_bursts) + ";\n\n";
    code += kIndents + "char finished = 0;\n\n";
    code += kIndents + "while (!finished) {\n";
    code += kIndents + kIndents + "for (i = 0; i < "+ tools::toString(n_streams) +"; i++) {\n";

    code += kIndents + kIndents + kIndents + "if ((data_index[i] < number_of_bursts) && "
    		"(cudaStreamQuery(stream[i] == cudaSuccess) {\n\n";

    string nIndents = kIndents + kIndents + kIndents + kIndents;
    code += generateCudaH2DCopyCode(composite, k_schedules) + "\n";

    // Generate code for executing the kernel
    code += nIndents + "// Execute kernel\n"
        + nIndents + "if (is_timeout_activated) {\n"
        + nIndents + kIndents + "// Prevent the kernel from timing out by\n"
        + nIndents + kIndents + "// splitting up the work into smaller pieces\n"
        + nIndents + kIndents + "// through multiple kernel invokations\n"
        + nIndents + kIndents + "int num_threads_left_to_execute = "
        + tools::toString(n_proc) + ";\n"
        + nIndents + kIndents + "int index_offset = 0;\n"
        + nIndents + kIndents + "while (num_threads_left_to_execute > 0) {\n";
    code += nIndents + kIndents + kIndents + "int num_executing_threads = "
        + "num_threads_left_to_execute < full_utilization_thread_count ? "
        + "num_threads_left_to_execute : full_utilization_thread_count;\n";
    code += nIndents + kIndents + kIndents + "struct KernelConfig config = "
        + "calculateBestKernelConfig(num_executing_threads, "
        + "max_threads_per_block, "
        + tools::toString(burst_size) + " * sizeof("
        + CDataType::typeToString(max_input.getDataType()->getType())
        + "), shared_memory_per_sm);\n";
    code += nIndents + kIndents + kIndents + functions_.back()->getName()
        + "<<<config.grid, config.threadBlock, config.sharedMemory, stream[i]>>>(";

    list<CVariable*> output_params = functions_.back()->getOutputParameters();
    list<CVariable*> input_params = functions_.back()->getInputParameters();
    for (list<CVariable*>::iterator it = output_params.begin(); it != output_params.end(); ++it){
    	if (it != output_params.begin()) code += ", ";
    	code += (*it)->getReferenceString() + "_device[i] ";
    }
    for (list<CVariable*>::iterator it = input_params.begin(); it != input_params.end(); ++it){
    	if ((*it)->getReferenceString() != "index_offset");
    	code += ", " + (*it)->getReferenceString() + "_device[i]";
    }
    code += ", index_offset);\n";
    code += nIndents + kIndents + kIndents + "int num_executed_threads = "
        + "config.grid.x * config.threadBlock.x;\n"
        + nIndents + kIndents + kIndents + "num_threads_left_to_execute -= "
        + "num_executed_threads;\n"
        + nIndents + kIndents + kIndents + "index_offset += "
        + "num_executed_threads;\n";
    code += nIndents + kIndents + "}\n";
    code += nIndents + "}\n";
    code += nIndents + "else {\n";
    code += nIndents + kIndents + "struct KernelConfig config = "
        + "calculateBestKernelConfig(" + tools::toString(n_proc)
        + ", max_threads_per_block, "
        + tools::toString(burst_size) + " * sizeof("
        + CDataType::typeToString(max_input.getDataType()->getType())
        + "), shared_memory_per_sm);\n";
    code += nIndents + kIndents + functions_.back()->getName()
        + "<<<config.grid, config.threadBlock, config.sharedMemory, stream[i]>>>(";
    for (list<CVariable*>::iterator it = output_params.begin(); it != output_params.end(); ++it){
    	if (it != output_params.begin()) code += ", ";
    	code += (*it)->getReferenceString() + "_device[i] ";
    }
    for (list<CVariable*>::iterator it = input_params.begin(); it != input_params.end(); ++it){
    	if ((*it)->getReferenceString() != "index_offset");
    	code += ", " + (*it)->getReferenceString() + "_device[i]";
    }
    code += ", 0);\n";
    code += nIndents + "}\n\n";
    code += generateCudaD2HCopyCode(composite, k_schedules) + "\n";

    code += nIndents + "data_index[i] += " + tools::toString(n_streams) + ";\n";

    code += kIndents + kIndents + kIndents +"}\n";

    code += kIndents + kIndents +"}\n";
    code += kIndents + kIndents +"finished = (index[0] >= number_of_bursts)";
    for (unsigned i = 1; i < n_streams; i++){
    	code += "\n" + kIndents + kIndents + kIndents + "&& (index[" + tools::toString(i) + "]"
    			" >= number_of_bursts)";
    }
    code += ";\n" + kIndents + "}\n\n";

    code += generateCudaVariableCleanupCode(composite, k_schedules);

    code += "}";
    return code;
}

string SynthesizerExperimental::generateProcessExecutionCode(Process* process)
    throw(InvalidModelException, IOException, RuntimeException) {
    logger_.logMessage(Logger::DEBUG, string("Generating execution code for ")
                       + "process \"" + process->getId()->getString()
                       + "\"...");

    string code;
    if (ParallelComposite* cast_composite = dynamic_cast<ParallelComposite*>(process)) {
    	if (in_kernel_) return  generateCompositeExecutionCode(cast_composite);
    	return  generateParallelCompositeExecutionCode(cast_composite);
    }
    else if (Composite* cast_composite = dynamic_cast<Composite*>(process)) {
    	return  generateCompositeExecutionCode(cast_composite);
    }
    else if (dynamic_cast<delay*>(process)) {
        // Do nothing
        return "";
    }
    else if (Comb* cast_leaf = dynamic_cast<Comb*>(process)) {
        return generateLeafExecutionCodeForComb(cast_leaf);
    }
    else if (Zipx* cast_leaf = dynamic_cast<Zipx*>(process)) {
        return generateLeafExecutionCodeForZipx(cast_leaf);
    }
    else if (Unzipx* cast_leaf = dynamic_cast<Unzipx*>(process)) {
        return generateLeafExecutionCodeForUnzipx(cast_leaf);
    }
    else if (Fanout* cast_leaf = dynamic_cast<Fanout*>(process)) {
        return generateLeafExecutionCodeForFanout(cast_leaf);
    }
    else {
        THROW_EXCEPTION(InvalidArgumentException, string("Leaf \"")
                        + process->getId()->getString() + "\" is of "
                        "unrecognized leaf type \"" + process->type()
                        + "\"");
    }
    return code;
}

string SynthesizerExperimental::generateLeafExecutionCodeFordelayStep1(
    delay* leaf)
    throw(InvalidModelException, IOException, RuntimeException) {
    CVariable output =
        getSignalByOutPort(leaf->getOutPorts().front())->getVariable();
    CVariable delay_variable = getDelayVariable(leaf).first;
    return generateVariableCopyingCode(output, delay_variable);
}

string SynthesizerExperimental::generateLeafExecutionCodeFordelayStep2(
    delay* leaf)
    throw(InvalidModelException, IOException, RuntimeException) {
    CVariable input =
        getSignalByInPort(leaf->getInPorts().front())->getVariable();
    CVariable delay_variable = getDelayVariable(leaf).first;
    return generateVariableCopyingCode(delay_variable, input);
}

string SynthesizerExperimental::generateCompositeExecutionCode(Composite* composite)
    throw(InvalidModelException, IOException, RuntimeException) {
    list<CVariable> inputs;
    list<CVariable> outputs;
    list<Composite::IOPort*> in_ports = composite->getInIOPorts();
    list<Composite::IOPort*>::iterator it;
    for (it = in_ports.begin(); it != in_ports.end(); ++it) {
        inputs.push_back(getSignalByInPort(*it)->getVariable());
    }
    list<Composite::IOPort*> out_ports = composite->getOutIOPorts();
    for (it = out_ports.begin(); it != out_ports.end(); ++it) {
    	outputs.push_back(getSignalByOutPort(*it)->getVariable());
    }
    CFunction* function = composite->getWrapper();
    return generateCompositeWrapperExecutionCode(*function, inputs, outputs);
}

string SynthesizerExperimental::generateParallelCompositeExecutionCode(
		ParallelComposite* composite)
    throw(InvalidModelException, IOException, RuntimeException) {
    list<CVariable> inputs;
    list<CVariable> outputs;
    list<Composite::IOPort*> in_ports = composite->getInIOPorts();
    list<Composite::IOPort*>::iterator it;
    for (it = in_ports.begin(); it != in_ports.end(); ++it) {
        inputs.push_back(getSignalByInPort(*it)->getVariable());
    }
    list<Composite::IOPort*> out_ports = composite->getOutIOPorts();
    for (it = out_ports.begin(); it != out_ports.end(); ++it) {
    	outputs.push_back(getSignalByOutPort(*it)->getVariable());
    }
    unsigned nproc = composite->getNumProcesses();
    CFunction* function = composite->getWrapper();
    return generateParallelCompositeWrapperExecutionCode(*function, nproc, inputs, outputs);
}

string SynthesizerExperimental::generateLeafExecutionCodeForComb(SY::Comb* leaf)
    throw(InvalidModelException, IOException, RuntimeException) {
    list<CVariable> inputs;
    list<Leaf::Port*> in_ports = leaf->getInPorts();
    list<Leaf::Port*>::iterator it;
    for (it = in_ports.begin(); it != in_ports.end(); ++it) {
        inputs.push_back(getSignalByInPort(*it)->getVariable());
    }
    CVariable output =
        getSignalByOutPort(leaf->getOutPorts().front())->getVariable();
    CFunction* function = leaf->getFunction();
    return generateLeafFunctionExecutionCode(function, inputs, output);
}

string SynthesizerExperimental::generateLeafExecutionCodeForUnzipx(Unzipx* leaf)
    throw(InvalidModelException, IOException, RuntimeException) {
    CVariable input =
        getSignalByInPort(leaf->getInPorts().front())->getVariable();
    list<CVariable> outputs;
    list<Leaf::Port*> out_ports = leaf->getOutPorts();
    list<Leaf::Port*>::iterator it;
    for (it = out_ports.begin(); it != out_ports.end(); ++it) {
        outputs.push_back(getSignalByOutPort(*it)->getVariable());
    }
    return generateVariableCopyingCode(outputs, input);
}

string SynthesizerExperimental::generateLeafExecutionCodeForZipx(Zipx* leaf)
    throw(InvalidModelException, IOException, RuntimeException) {
    CVariable output =
    		getSignalByOutPort(leaf->getOutPorts().front())->getVariable();
    list<CVariable> inputs;
    list<Leaf::Port*> in_ports = leaf->getInPorts();
    list<Leaf::Port*>::iterator it;

    string code;
    for (it = in_ports.begin(); it != in_ports.end(); ++it) {
        inputs.push_back(getSignalByInPort(*it)->getVariable());
    }
    code += generateVariableCopyingCode(output, inputs);
    return code;
}

string SynthesizerExperimental::generateLeafExecutionCodeForFanout(Fanout* leaf)
    throw(InvalidModelException, IOException, RuntimeException) {
    CVariable input =
        getSignalByInPort(leaf->getInPorts().front())->getVariable();
    list<Leaf::Port*> out_ports = leaf->getOutPorts();
    list<Leaf::Port*>::iterator it;

    string code;
    for (it = out_ports.begin(); it != out_ports.end(); ++it) {
        CVariable output = getSignalByOutPort(*it)->getVariable();
        code += generateVariableCopyingCode(output, input);
    }
    return code;
}

string SynthesizerExperimental::generateLeafFunctionExecutionCode(
    CFunction* function, list<CVariable> inputs, CVariable output)
    throw(InvalidModelException, IOException, RuntimeException) {
    ensureVariableIsNotConst(output);

    string code;

    // Add function call
    if (function->getNumInputParameters() == inputs.size()) {
        CVariable function_return = *function->getOutputParameter();
        try {
            ensureVariableDataTypeCompatibilities(output, function_return);
            ensureVariableArrayCompatibilities(output, function_return);
        }
        catch (InvalidModelException& ex) {
            THROW_EXCEPTION(InvalidModelException, string("Error in function, ")
                            + "return value: " + ex.getMessage());
        }

        code += kIndents + function->getName() + "(" +  output.getReferenceString() + ",";
    }
    else if (function->getNumInputParameters() == inputs.size() + 1) {
        CVariable function_output = *function->getInputParameters().back();
        try {
            ensureVariableDataTypeCompatibilities(function_output, output);
            ensureVariableArrayCompatibilities(function_output, output);
        }
        catch (InvalidModelException& ex) {
            THROW_EXCEPTION(InvalidModelException, string("Error in function, ")
                            + "last parameter: " + ex.getMessage());
        }

        code += kIndents + function->getName() + "(" +  output.getReferenceString() + ",";
    }
    else {
        THROW_EXCEPTION(IllegalStateException, "Function has unexpected "
                        "number of input parameters");
    }

    // Add parameters
    list<CVariable>::iterator input_it;
    list<CVariable*> input_parameters = function->getInputParameters();
    list<CVariable*>::iterator param_it;
    for (input_it = inputs.begin(), param_it = input_parameters.begin();
         input_it != inputs.end(); ++input_it, ++param_it) {
        CVariable input = *input_it;
        CVariable param = **param_it;
        ensureVariableDataTypeCompatibilities(param, input);
        ensureVariableArrayCompatibilities(param, input);

        if (input_it != inputs.begin()) code += ", ";
        code += input.getReferenceString();
    }
    if (function->getNumInputParameters() == inputs.size() + 1) {
        code += string(", ") + output.getReferenceString();
    }
    code += ");\n";

    return code;
}

std::string SynthesizerExperimental::generateCompositeWrapperExecutionCode(CFunction function,
		std::list<CVariable> inputs, std::list<CVariable> outputs)
    throw(InvalidModelException, IOException, RuntimeException){

	string code;
	code += kIndents + function.getName() + "(";
	list<CVariable>::iterator output_it;
	list<CVariable*> output_parameters = function.getOutputParameters();
	list<CVariable*>::iterator param_it;
	for (output_it = outputs.begin(), param_it = output_parameters.begin();
			output_it != outputs.end(); ++output_it, ++param_it) {
		CVariable output = *output_it;
		CVariable param = **param_it;
		ensureVariableIsNotConst(output);
		ensureVariableDataTypeCompatibilities(param, output);
		ensureVariableArrayCompatibilities(output, output);

		if (output_it != outputs.begin()) code += ", ";
		code += output.getReferenceString();
	}
	code += ", ";
    list<CVariable>::iterator input_it;
    list<CVariable*> input_parameters = function.getInputParameters();
    for (input_it = inputs.begin(), param_it = input_parameters.begin();
         input_it != inputs.end(); ++input_it, ++param_it) {
        CVariable input = *input_it;
        CVariable param = **param_it;
        ensureVariableDataTypeCompatibilities(param, input);
        ensureVariableArrayCompatibilities(input, input);

        if (input_it != inputs.begin()) code += ", ";
        code += input.getReferenceString();
    }

	code += ");\n";
    return code;
}

std::string SynthesizerExperimental::generateParallelCompositeWrapperExecutionCode(
		CFunction function, unsigned nproc, std::list<CVariable> inputs, std::list<CVariable> outputs)
    throw(InvalidModelException, IOException, RuntimeException){

	string code;
	code += kIndents + "for (i = 0; i < " + tools::toString(nproc)  + "; ++i) {\n";

	code += kIndents + kIndents + function.getName() + "(";
	list<CVariable>::iterator output_it;
	list<CVariable*> output_parameters = function.getOutputParameters();
	list<CVariable*>::iterator param_it;
	for (output_it = outputs.begin(), param_it = output_parameters.begin();
			output_it != outputs.end(); ++output_it, ++param_it) {
		CVariable output = *output_it;
		CVariable param = **param_it;
		ensureVariableIsNotConst(output);
		ensureVariableDataTypeCompatibilities(param, output);
		ensureVariableArrayCompatibilities(output, output);

		if (output_it != outputs.begin()) code += ", ";
		code +=  "&"  + output.getReferenceString() + "[i * "
				+ tools::toString(output.getDataType()->getArraySize() / nproc) + "]";
	}
	code += ", ";
    list<CVariable>::iterator input_it;
    list<CVariable*> input_parameters = function.getInputParameters();
    for (input_it = inputs.begin(), param_it = input_parameters.begin();
         input_it != inputs.end(); ++input_it, ++param_it) {
        CVariable input = *input_it;
        CVariable param = **param_it;
        ensureVariableDataTypeCompatibilities(param, input);
        ensureVariableArrayCompatibilities(input, input);

        if (input_it != inputs.begin()) code += ", ";
        code += "&" + input.getReferenceString() + "[i * "
				+ tools::toString(input.getDataType()->getArraySize() / nproc) + "]";
    }

	code += ");\n" + kIndents + "}\n";
    return code;
}

std::string SynthesizerExperimental::generateRootExecutionCode(
		CFunction function, std::list<CVariable> inputs, std::list<CVariable> outputs)
    throw(InvalidModelException, IOException, RuntimeException){

	string code;
	code += "{\n" + kIndents + "for (i = 0; i < N ; ++i) {\n";

	code += kIndents + kIndents + function.getName() + "(";
	list<CVariable>::iterator output_it;
	list<CVariable*> output_parameters = function.getOutputParameters();
	list<CVariable*>::iterator param_it;
	for (output_it = outputs.begin(), param_it = output_parameters.begin();
			output_it != outputs.end(); ++output_it, ++param_it) {
		CVariable output = *output_it;
		CVariable param = **param_it;
		ensureVariableIsNotConst(output);
		ensureVariableDataTypeCompatibilities(param, output);
		ensureVariableArrayCompatibilities(output, output);

		if (output_it != outputs.begin()) code += ", ";
		code +=  "&"  + output.getReferenceString() + "[i * "
				+ tools::toString(output.getDataType()->getArraySize() ) + "]";
	}
	code += ", ";
    list<CVariable>::iterator input_it;
    list<CVariable*> input_parameters = function.getInputParameters();
    for (input_it = inputs.begin(), param_it = input_parameters.begin();
         input_it != inputs.end(); ++input_it, ++param_it) {
        CVariable input = *input_it;
        CVariable param = **param_it;
        ensureVariableDataTypeCompatibilities(param, input);
        ensureVariableArrayCompatibilities(input, input);

        if (input_it != inputs.begin()) code += ", ";
        code += "&" + input.getReferenceString() + "[i * "
				+ tools::toString(input.getDataType()->getArraySize()) + "]";
    }

	code += ");\n" + kIndents + "}\n}\n";
    return code;
}



string SynthesizerExperimental::generateSignalVariableDeclarationsCode(Composite* composite)
    throw(InvalidModelException, IOException, RuntimeException) {
    try {
        string code;
        code += kIndents + "// Declare signal variables\n";
        set<Signal*>::iterator it;
        for (it = signals_.begin(); it != signals_.end(); ++it) {
            Signal* signal = *it;
            if (!isInSignal(signal,composite) && !isOutSignal(signal,composite)){
				logger_.logMessage(Logger::DEBUG, string("Generating variable ")
								   + "declaration for signal "
								   + signal->toString() + "...");

				code += kIndents;
				if (signal->getVariable().getDataType()->isArray()) {
					if (dynamicallyAllocateMemoryForSignalVariable(signal)) {
						code += signal->getVariable()
							.getDynamicVariableDeclarationString();
					}
					else {
						code += signal->getVariable()
							.getPointerDeclarationString();
					}
				}
				else {
					code += signal->getVariable()
						.getLocalVariableDeclarationString();
				}
				code += ";\n";
            }
        }
        return code;
    }
    catch (UnknownArraySizeException& ex) {
        THROW_EXCEPTION(InvalidModelException, ex.getMessage());
    }
}

string SynthesizerExperimental::generateCudaVariableDeclarationsCode(Composite* composite,
		std::list<std::list<Forsyde::Id> > k_schedules)
    throw(InvalidModelException, IOException, RuntimeException) {

	set<Id> inside_kernels;
	for (list<list<Forsyde::Id> >::iterator it = k_schedules.begin(); it != k_schedules.end(); ++it) {
		for (list<Forsyde::Id>::iterator iit = it->begin(); iit != it->end(); ++iit) {
			inside_kernels.insert(*iit);
		}
	}

    try {
        string code;
        code += kIndents + "// Declare signal variables\n";
        set<Signal*>::iterator it;
        for (it = signals_.begin(); it != signals_.end(); ++it) {
            Signal* signal = *it;
            if ((	inside_kernels.find(*signal->getInPort()->getProcess()->getId()) !=
            		inside_kernels.end())
            		&& (inside_kernels.find(*signal->getOutPort()->getProcess()->getId()) !=
            			inside_kernels.end()) )
            	continue;
            /*if (isInSignal(signal,composite)) {
            	code += kIndents;
            	code += signal->getVariable().getPointerDeclarationString() + "Ptr = "
            			+ signal->getVariable().getPointerDeclarationString() + ";\n";
            	code += kIndents;
            	code += "cudaMallocHost(&" + signal->getVariable().getReferenceString() + "Ptr, "
            			+ tools::toString(signal->getVariable().getDataType()->getArraySize())
            			+ " * sizeof("
            			+ signal->getVariable().getDataType()->getVariableDataTypeString()
            			+ ") * N);\n";
            	continue;
            }
            if (isOutSignal(signal,composite)){
            	code += kIndents;
            	code += signal->getVariable().getPointerDeclarationString() + "Ptr = "
            			+ signal->getVariable().getPointerDeclarationString() + ";\n";
            	code += kIndents;
            	code += "cudaMallocHost(&" + signal->getVariable().getReferenceString() + "Ptr, "
            			+ tools::toString(signal->getVariable().getDataType()->getArraySize())
            			+ " * sizeof("
            			+ signal->getVariable().getDataType()->getVariableDataTypeString()
            			+ ") * N);\n";
            	continue;
            }*/

			logger_.logMessage(Logger::DEBUG, string("Generating variable ")
							   + "declaration for signal "
							   + signal->toString() + "...");

			string new_var_name = signal->getVariable().getReferenceString() + "_device";
			code += kIndents + CDataType::typeToString(signal->getVariable().getDataType()->getType())
					+ "* " + new_var_name + "["
					+ tools::toString(config_.getCosts().n_stages) + "];\n";
			for (unsigned i = 1; i <= config_.getCosts().n_stages; i++){
				CVariable device_variable(string (new_var_name +
						 + "[" +tools::toString(i)+ "]"), *signal->getVariable().getDataType());
				device_variable.getDataType()->setIsConst(false);
				unsigned vector_size = (unsigned)ceil(
						(float)device_variable.getDataType()->getArraySize() /
						(float)config_.getCosts().n_bursts );
				code +=  kIndents + "// @todo Better error handling\n"
						+ kIndents + "if (cudaMalloc((void**) &"
						+ device_variable.getReferenceString() + ", "
						+ tools::toString(vector_size) + " * sizeof("
						+ CDataType::typeToString(device_variable.getDataType()->getType()) + ")) "
						+ "!= cudaSuccess) {\n"
						+ kIndents + kIndents + "printf(\"ERROR: Failed to allocate GPU "
						+ "memory\\n\");\n"
						+ kIndents + kIndents + "exit(-1);\n"
						+ kIndents + "}\n";
			}

        }
        return code;
    }
    catch (UnknownArraySizeException& ex) {
        THROW_EXCEPTION(InvalidModelException, ex.getMessage());
    }
}

string SynthesizerExperimental::generateCudaVariableCleanupCode(Composite* composite,
		std::list<std::list<Forsyde::Id> > k_schedules)
    throw(InvalidModelException, IOException, RuntimeException) {

	set<Id> inside_kernels;
	for (list<list<Forsyde::Id> >::iterator it = k_schedules.begin(); it != k_schedules.end(); ++it) {
		for (list<Forsyde::Id>::iterator iit = it->begin(); iit != it->end(); ++iit) {
			inside_kernels.insert(*iit);
		}
	}

    try {
        string code;
        set<Signal*>::iterator it;
        for (it = signals_.begin(); it != signals_.end(); ++it) {
			Signal* signal = *it;
			if (isOutSignal(signal,composite)) {
			if ((	inside_kernels.find(*signal->getInPort()->getProcess()->getId()) !=
					inside_kernels.end())
					&& (inside_kernels.find(*signal->getOutPort()->getProcess()->getId()) !=
						inside_kernels.end()) )
				continue;
			logger_.logMessage(Logger::DEBUG, string("Generate H2D copy transfer ")
							   + "declaration for signal "
							   + signal->toString() + "...");


			string varname = signal->getVariable().getReferenceString();
			for (unsigned i = 1; i <= config_.getCosts().n_stages; i++){
				CVariable device_variable(string (varname +
						 + "[" +tools::toString(i)+ "]"), *signal->getVariable().getDataType());
				device_variable.getDataType()->setIsConst(false);
				code += kIndents + "if (cudaFree((void*) "
			        + device_variable.getReferenceString() + ") != cudaSuccess) {\n"
			        + kIndents + kIndents + "printf(\"ERROR: Failed to free GPU "
			        + "memory\\n\");\n"
			        + kIndents + kIndents + "exit(-1);\n"
			        + kIndents + "}\n";
				}
			}
        }
        return code;
    }
    catch (UnknownArraySizeException& ex) {
        THROW_EXCEPTION(InvalidModelException, ex.getMessage());
    }
}

string SynthesizerExperimental::generateCudaH2DCopyCode(Composite* composite,
		std::list<std::list<Forsyde::Id> > k_schedules)
    throw(InvalidModelException, IOException, RuntimeException) {

	set<Id> inside_kernels;
	for (list<list<Forsyde::Id> >::iterator it = k_schedules.begin(); it != k_schedules.end(); ++it) {
		for (list<Forsyde::Id>::iterator iit = it->begin(); iit != it->end(); ++iit) {
			inside_kernels.insert(*iit);
		}
	}

    try {
        string code;
        set<Signal*>::iterator it;
        for (it = signals_.begin(); it != signals_.end(); ++it) {
			Signal* signal = *it;
			if (isInSignal(signal,composite)) {
			if ((	inside_kernels.find(*signal->getInPort()->getProcess()->getId()) !=
					inside_kernels.end())
					&& (inside_kernels.find(*signal->getOutPort()->getProcess()->getId()) !=
						inside_kernels.end()) )
				continue;
			logger_.logMessage(Logger::DEBUG, string("Generate H2D copy transfer ")
							   + "declaration for signal "
							   + signal->toString() + "...");

			string new_var_name = signal->getVariable().getReferenceString() + "_device";

			CVariable device_variable(string (new_var_name +
					 + "[i]"), *signal->getVariable().getDataType());
			device_variable.getDataType()->setIsConst(false);
			unsigned vector_size = (unsigned)ceil(
					(float)device_variable.getDataType()->getArraySize() /
					(float)config_.getCosts().n_bursts );
			code += kIndents + kIndents + kIndents + kIndents +"if (cudaMemcpyAsync((void*) "
				+ device_variable.getReferenceString() + ", (void*) "
				+ signal->getVariable().getReferenceString()
				+ "[data_index[i]]"
				+ ", " + tools::toString(vector_size) + " * sizeof("
				+ CDataType::typeToString(device_variable.getDataType()->getType())
				+ "), cudaMemcpyHostToDevice, stream[i]) != cudaSuccess) {\n"
				+ kIndents + kIndents + kIndents + kIndents + kIndents
				+ "printf(\"ERROR: Failed to copy data to "
				+ "GPU\\n\");\n"
				+ kIndents + kIndents + kIndents + kIndents + kIndents + "exit(-1);\n"
				+ kIndents + kIndents + kIndents + kIndents + "}\n"
				+ "\n";
			}

        }
        return code;
    }
    catch (UnknownArraySizeException& ex) {
        THROW_EXCEPTION(InvalidModelException, ex.getMessage());
    }
}

string SynthesizerExperimental::generateCudaD2HCopyCode(Composite* composite,
		std::list<std::list<Forsyde::Id> > k_schedules)
    throw(InvalidModelException, IOException, RuntimeException) {

	set<Id> inside_kernels;
	for (list<list<Forsyde::Id> >::iterator it = k_schedules.begin(); it != k_schedules.end(); ++it) {
		for (list<Forsyde::Id>::iterator iit = it->begin(); iit != it->end(); ++iit) {
			inside_kernels.insert(*iit);
		}
	}

    try {
        string code;
        set<Signal*>::iterator it;
        for (it = signals_.begin(); it != signals_.end(); ++it) {
			Signal* signal = *it;
			if (isOutSignal(signal,composite)) {
			if ((	inside_kernels.find(*signal->getInPort()->getProcess()->getId()) !=
					inside_kernels.end())
					&& (inside_kernels.find(*signal->getOutPort()->getProcess()->getId()) !=
						inside_kernels.end()) )
				continue;
			logger_.logMessage(Logger::DEBUG, string("Generate H2D copy transfer ")
							   + "declaration for signal "
							   + signal->toString() + "...");

			string new_var_name = signal->getVariable().getReferenceString() + "_device";

			CVariable device_variable(string (new_var_name +
					 + "[i]"), *signal->getVariable().getDataType());
			device_variable.getDataType()->setIsConst(false);
			unsigned vector_size = (unsigned)ceil(
					(float)device_variable.getDataType()->getArraySize() /
					(float)config_.getCosts().n_bursts );
			code += kIndents + kIndents + kIndents + kIndents +"if (cudaMemcpyAsync((void*) "
				+ signal->getVariable().getReferenceString() + "[data_index[i]], "
				+ "(void*) " + device_variable.getReferenceString()
				+ ", " + tools::toString(vector_size) + " * sizeof("
				+ CDataType::typeToString(device_variable.getDataType()->getType())
				+ "), cudaMemcpyDeviceToHost, stream[i]) != cudaSuccess) {\n"
				+ kIndents + kIndents + kIndents + kIndents + kIndents
				+ "printf(\"ERROR: Failed to copy data to "
				+ "GPU\\n\");\n"
				+ kIndents + kIndents + kIndents + kIndents + kIndents + "exit(-1);\n"
				+ kIndents + kIndents + kIndents + kIndents + "}\n"
				+ "\n";
			}

        }
        return code;
    }
    catch (UnknownArraySizeException& ex) {
        THROW_EXCEPTION(InvalidModelException, ex.getMessage());
    }
}

string SynthesizerExperimental::generateSignalVariableCleanupCode(Composite* composite)
    throw(IOException, RuntimeException) {
    string code;
    set<Signal*>::iterator it;
    bool at_least_one = false;
    for (it = signals_.begin(); it != signals_.end(); ++it) {
        Signal* signal = *it;
        if (!isInSignal(signal,composite) && !isOutSignal(signal,composite)){
			Signal* signal = *it;
			logger_.logMessage(Logger::DEBUG, string("Analyzing signal ")
							   + signal->toString() + "...");

			if (dynamicallyAllocateMemoryForSignalVariable(signal)) {
				at_least_one = true;
				code += kIndents + "delete[] "
					+ signal->getVariable().getReferenceString() + ";\n";
			}
        }
    }
    if (at_least_one) code = kIndents + "// Clean up memory\n" + code;
    return code;
}

string SynthesizerExperimental::generateDelayVariableDeclarationsCode()
    throw(InvalidModelException, IOException, RuntimeException) {
    try {
        string code;
        if (delay_variables_.size() > 0) {
            code += kIndents + "// Declare delay variables\n";
        }
        map< delay*, pair<CVariable, std::string> >::iterator it;
        for (it = delay_variables_.begin(); it != delay_variables_.end();
             ++it) {
            CVariable variable = it->second.first;
            string initial_value = it->second.second;
            initial_value = tools::searchReplace(initial_value, "[", "{");
            initial_value = tools::searchReplace(initial_value, "]", "}");
            code += kIndents + "static ";
            code += variable.getLocalVariableDeclarationString();
            code += " = ";
            code += initial_value;
            code += ";\n";
        }
        return code;
    }
    catch (UnknownArraySizeException& ex) {
        THROW_EXCEPTION(InvalidModelException, ex.getMessage());
    }
}

string SynthesizerExperimental::generateInputsToSignalsCopyingCode(Composite* composite)
    throw(InvalidModelException, RuntimeException) {
    string code;

    list<Composite::IOPort*> inputs = composite->getInIOPorts();
    list<Composite::IOPort*>::iterator it;
    int id;
    bool at_least_one = false;
    for (it = inputs.begin(), id = 1; it != inputs.end(); ++it, ++id) {
        Signal* signal = getSignalInsideByInPort(*it);
        logger_.logMessage(Logger::DEBUG, string("Analyzing signal ")
                           + signal->toString() + "...");

        CDataType data_type = *signal->getDataType();
        if (data_type.isArray()) continue;
        at_least_one = true;
        CVariable input_parameter(
            kProcessNetworkInputParameterPrefix + tools::toString(id), data_type);
        code += generateVariableCopyingCode(signal->getVariable(),
                                            input_parameter, false);
    }

    if (at_least_one) {
        code = kIndents + "// Copy composite inputs to signal variables\n" + code;
    }

    return code;
}

string SynthesizerExperimental::generateSignalsToOutputsCopyingCode(Composite* composite)
    throw(InvalidModelException, RuntimeException) {
    string code;

    list<Composite::IOPort*> outputs = composite->getOutIOPorts();
    list<Composite::IOPort*>::iterator it;
    int id;
    bool at_least_one = false;
    for (it = outputs.begin(), id = 1; it != outputs.end(); ++it, ++id) {
        Signal* signal = getSignalInsideByOutPort(*it);
        logger_.logMessage(Logger::DEBUG, string("Analyzing signal ")
                           + signal->toString() + "...");

        CDataType data_type = *signal->getDataType();
        if (data_type.isArray()) continue;
        at_least_one = true;
        data_type.setIsPointer(true);
        CVariable output_parameter(
            kProcessNetworkOutputParameterPrefix + tools::toString(id), data_type);
        code += generateVariableCopyingCode(output_parameter,
                                            signal->getVariable(), false);
    }

    if (at_least_one) {
        code = kIndents + "// Copy signal variables to processnetwork outputs\n" + code;
    }

    return code;
}


void SynthesizerExperimental::createSignals(Forsyde::Composite* composite)
    throw(InvalidModelException, IOException, RuntimeException) {
    signals_.clear();

    list<Leaf*> leafs = composite->getProcesses();
    for (list<Leaf*>::iterator it = leafs.begin(); it != leafs.end(); ++it) {
    	if (composite->findRelation(*it) == Hierarchy::FirstChild){
			logger_.logMessage(Logger::DEBUG, string("Analyzing leaf \"")
							   + (*it)->getId()->getString() + "\"...");

			list<Leaf::Port*> iports = (*it)->getInPorts();
			list<Leaf::Port*>::iterator port_it;
			for (port_it = iports.begin(); port_it != iports.end(); ++port_it) {
				CDataType new_type = (*port_it)->getDataType();
				new_type.setIsConst(false);
				getSignalByInPort(*port_it)->setDataType(new_type);
			}
			list<Leaf::Port*> oports = (*it)->getOutPorts();
			for (port_it = oports.begin(); port_it != oports.end(); ++port_it) {
				CDataType new_type = (*port_it)->getDataType();
				new_type.setIsConst(false);
				getSignalByOutPort(*port_it)->setDataType(new_type);
			}
    	}
    }

    list<Composite*> composites = composite->getComposites();
    for (list<Composite*>::iterator it = composites.begin(); it != composites.end(); ++it) {
    	if (composite->findRelation(*it) == Hierarchy::FirstChild){
			logger_.logMessage(Logger::DEBUG, string("Analyzing composite \"")
							   + (*it)->getId()->getString() + "\"...");

			list<Composite::IOPort*> iports = (*it)->getInIOPorts();
			list<Composite::IOPort*>::iterator port_it;
			for (port_it = iports.begin(); port_it != iports.end(); ++port_it) {
				CDataType new_type = (*port_it)->getDataType().first;
				new_type.setIsConst(false);
				Signal* new_out_signal = getSignalByInPort(*port_it);
				new_out_signal->setDataType(new_type);
			}
			list<Composite::IOPort*> oports = (*it)->getOutIOPorts();
			for (port_it = oports.begin(); port_it != oports.end(); ++port_it) {
				CDataType new_type = (*port_it)->getDataType().first;
				new_type.setIsConst(false);
				Signal* new_out_signal = getSignalByOutPort(*port_it);
				new_out_signal->setDataType(new_type);
			}
    	}
    }

    logger_.logMessage(Logger::INFO, string("Created ")
                       + tools::toString(signals_.size()) + " signal(s).");
    for (set<Signal*>::iterator it = signals_.begin(); it != signals_.end(); ++it) {
        logger_.logMessage(Logger::DEBUG, string("\t ")
                           + (*it)->getVariable().getLocalVariableDeclarationString());
    }
}

void SynthesizerExperimental::createDelayVariables(list<Id> schedule) throw(
		IOException, RuntimeException) {
    delay_variables_.clear();

    list<Id>::iterator it;
    int counter;
    for (it = schedule.begin(), counter = 1; it != schedule.end(); ++it) {
        Leaf* current_leaf = processnetwork_->getProcess(*it);
        if (!current_leaf) continue;
        logger_.logMessage(Logger::DEBUG, string("Analyzing leaf \"")
                           + current_leaf->getId()->getString() + "\"...");

        delay* delay_leaf = dynamic_cast<delay*>(current_leaf);
        if (delay_leaf) {
            string name = string("v_delay_element") + tools::toString(counter);
            ++counter;
            CDataType data_type =
                *getSignalByInPort(delay_leaf->getInPorts().front())
                ->getDataType();
            CVariable variable(name, data_type);
            pair<CVariable, string> value(variable,
                                          delay_leaf->getInitialValue());
            pair< delay*, pair<CVariable, string> > key_value(delay_leaf,
                                                                value);
            pair<map< delay*, pair<CVariable, string> >::iterator, bool>
                result = delay_variables_.insert(key_value);
            if (!result.second) {
                THROW_EXCEPTION(IllegalStateException, string("Delay variable ")
                                + "\" " + name + "\" already exist");
            }
        }
    }

    logger_.logMessage(Logger::INFO, string("Created ")
                       + tools::toString(delay_variables_.size())
                       + " delay variable(s)");
}

string SynthesizerExperimental::renameVariables(string body, Composite* composite)
    throw(InvalidModelException, RuntimeException) {

	unsigned in_vars;
	unsigned out_vars;
	unsigned vars;

	string new_body = body;

    set<Signal*>::iterator it;
	for (in_vars = 0, out_vars = 0, vars = 0, it = signals_.begin(); it != signals_.end(); ++it){
		Signal* signal = *it;
		if (isInSignal(*it, composite)){
			string new_varname = string ("in" + tools::toString(++in_vars));
			new_body = tools::searchReplace(new_body, signal->getVariable().getReferenceString(),
					new_varname);
	        logger_.logMessage(Logger::DEBUG, string("Renamed variable name ")
	                           + signal->getVariable().getReferenceString() + " with "
	                           + new_varname);
		}

		else if (isOutSignal(*it, composite)){
			string new_varname = string ("out" + tools::toString(++out_vars));
			new_body = tools::searchReplace(new_body, signal->getVariable().getReferenceString(),
					new_varname);
	        logger_.logMessage(Logger::DEBUG, string("Renamed variable name ")
	                           + signal->getVariable().getReferenceString() + " with "
	                           + new_varname);
		}
		else {
			string new_varname = string (signal->getInPort()->getProcess()->type()
					+ tools::toString(++vars));
			new_body = tools::searchReplace(new_body, signal->getVariable().getReferenceString(),
					new_varname);
	        logger_.logMessage(Logger::DEBUG, string("Renamed variable name ")
	                           + signal->getVariable().getReferenceString() + " with "
	                           + new_varname);
		}
	}


    return new_body;
}

string SynthesizerExperimental::generateVariableCopyingCode(CVariable to, CVariable from,
                                                bool do_deep_copy)
    throw(InvalidModelException, IOException, RuntimeException) {
    ensureVariableDataTypeCompatibilities(from, to);
    ensureVariableArrayCompatibilities(from, to);

    string code;
    if (to.getDataType()->isArray()) {
        if (do_deep_copy) {
            ensureVariableIsNotConst(to);

            size_t array_size = to.getDataType()->getArraySize();
            code += kIndents + "for (i = 0; i < " + tools::toString(array_size)
                + "; ++i) {\n"
                + kIndents + kIndents + to.getReferenceString() + "[i] = "
                + from.getReferenceString() + "[i];\n"
                + kIndents + "}\n";
        }
        else {
            code += kIndents + to.getReferenceString() + " = "
                + from.getReferenceString() + ";\n";
        }
    }
    else {
        ensureVariableIsNotConst(to);

        code += kIndents;
        if (to.getDataType()->isPointer()) code += "*";
        code += to.getReferenceString() + " = ";
        if (from.getDataType()->isPointer()) code += "*";
        code += from.getReferenceString() + ";\n";
    }
    return code;
}


string SynthesizerExperimental::generateVariableCopyingCode(CVariable to,
                                                list<CVariable>& from)
    throw(InvalidModelException, IOException, RuntimeException) {
    ensureVariableIsNotConst(to);
    ensureVariableIsArray(to);
    size_t num_from_elements = 0;
    for (list<CVariable>::iterator it = from.begin(); it != from.end(); ++it) {
        ensureVariableDataTypeCompatibilities(to, *it);
        num_from_elements += it->getDataType()->getArraySize();
    }
    try {
        ensureArraySizes(to.getDataType()->getArraySize(), num_from_elements);
    }
    catch (InvalidModelException& ex) {
        THROW_EXCEPTION(InvalidModelException, string("Error between ")
                        + "list of variables and variable \""
                        + to.getReferenceString() + "\": " + ex.getMessage());
    }

    string code;
    list<CVariable>::iterator it;
    size_t to_index = 0;
    for (it = from.begin(); it != from.end(); ++it) {
        if (!it->getDataType()->isArray()) {
            code += kIndents + to.getReferenceString()
                + "[" + tools::toString(to_index) + "] = "
                + it->getReferenceString() + ";\n";
            ++to_index;
        }
        else {
            size_t from_array_size = it->getDataType()->getArraySize();
            code += kIndents + "for (i = " + tools::toString(to_index)
                + ", j = 0; i < "
                + tools::toString(to_index + from_array_size)
                + "; ++i, ++j) {\n"
                + kIndents + kIndents + to.getReferenceString() + "[i] = "
                + it->getReferenceString() + "[j];\n"
                + kIndents + "}\n";
            to_index += from_array_size;
        }
    }
    return code;
}

string SynthesizerExperimental::generateVariableCopyingCode(list<CVariable>& to,
                                                CVariable from)
    throw(InvalidModelException, IOException, RuntimeException) {
    size_t num_to_elements = 0;
    for (list<CVariable>::iterator it = to.begin(); it != to.end(); ++it) {
        ensureVariableIsNotConst(*it);
        ensureVariableDataTypeCompatibilities(*it, from);
        num_to_elements += it->getDataType()->getArraySize();
    }
    ensureVariableIsArray(from);
    try {
        ensureArraySizes(num_to_elements, from.getDataType()->getArraySize());
    }
    catch (InvalidModelException& ex) {
        THROW_EXCEPTION(InvalidModelException, string("Error between ")
                        + "variable \"" + from.getReferenceString() + "\" and "
                        + "list of variables: " + ex.getMessage());
    }

    string code;
    list<CVariable>::iterator it;
    size_t from_index = 0;
    for (it = to.begin(); it != to.end(); ++it) {
        if (!it->getDataType()->isArray()) {
            code += kIndents + it->getReferenceString() + " = "
                + from.getReferenceString()
                + "[" + tools::toString(from_index) + "];\n";
            ++from_index;
        }
        else {
            size_t to_array_size = it->getDataType()->getArraySize();
            code += kIndents + "for (i = " + tools::toString(from_index)
                + ", j = 0; i < "
                + tools::toString(from_index + to_array_size)
                + "; ++i, ++j) {\n"
                + kIndents + kIndents + it->getReferenceString() + "[j] = "
                + from.getReferenceString() + "[i];\n"
                + kIndents + "}\n";
            from_index += to_array_size;
        }
    }
    return code;
}


void SynthesizerExperimental::ensureVariableDataTypeCompatibilities(CVariable lhs,
                                                        CVariable rhs)
    throw(InvalidModelException) {
    CDataType lhs_data_type = *lhs.getDataType();
    CDataType rhs_data_type = *rhs.getDataType();
    if (lhs_data_type.getType() != rhs_data_type.getType()) {
            THROW_EXCEPTION(InvalidModelException, string("Error between ")
                            + "variables " + rhs.getReferenceString()
                            + " and " + lhs.getReferenceString() + ": "
                            + "mismatched data types (from "
                            + CDataType::typeToString(rhs_data_type.getType())
                            + " to "
                            + CDataType::typeToString(lhs_data_type.getType())
                            + ")");
    }
}

void SynthesizerExperimental::ensureVariableIsNotConst(CVariable variable)
    throw(InvalidModelException) {
    if (variable.getDataType()->isConst()) {
        THROW_EXCEPTION(InvalidModelException, string("Variable \"") +
                        variable.getReferenceString() + "\" is a const");
    }
}

void SynthesizerExperimental::ensureVariableIsArray(CVariable variable)
    throw(InvalidModelException) {
    if (!variable.getDataType()->isArray()) {
        THROW_EXCEPTION(InvalidModelException, string("Variable \"") +
                        variable.getReferenceString() + "\" is not an array");
    }
}

void SynthesizerExperimental::ensureArraySizes(size_t lhs, size_t rhs)
    throw(InvalidModelException) {
    if (lhs != rhs) {
        THROW_EXCEPTION(InvalidModelException, string("Mismatched array ")
                        + "sizes (from size " + tools::toString(rhs)
                        + " to size " + tools::toString(lhs) + ")");
    }
}

void SynthesizerExperimental::ensureVariableArrayCompatibilities(CVariable lhs,
                                                     CVariable rhs)
    throw(InvalidModelException) {
    CDataType lhs_data_type = *lhs.getDataType();
    CDataType rhs_data_type = *rhs.getDataType();
    if (lhs_data_type.isArray()) {
        if (!rhs_data_type.isArray()) {
            THROW_EXCEPTION(InvalidModelException, string("Error between ")
                            + "variables " + rhs.getReferenceString()
                            + " and " + lhs.getReferenceString() + ": "
                            + "mismatched data types (from scalar to array)");
        }
        if (!lhs_data_type.hasArraySize()) {
            THROW_EXCEPTION(InvalidModelException, string("Variable \"") +
                            lhs.getReferenceString() + "\" has no array "
                            + "size");
        }
        if (!rhs_data_type.hasArraySize()) {
            THROW_EXCEPTION(InvalidModelException, string("Variable \"") +
                            rhs.getReferenceString() + "\" has no array "
                            + "size");
        }
        try {
            ensureArraySizes(lhs_data_type.getArraySize(),
                             rhs_data_type.getArraySize());
        }
        catch (InvalidModelException& ex) {
            THROW_EXCEPTION(InvalidModelException, string("Error between ")
                            + "variables " + rhs.getReferenceString()
                            + " and " + lhs.getReferenceString() + ": "
                            + ex.getMessage());
        }
    }
    else {
        if (rhs_data_type.isArray()) {
            THROW_EXCEPTION(InvalidModelException, string("Error between ")
                            + "variables " + rhs.getReferenceString()
                            + " and " + lhs.getReferenceString() + ": "
                            + "mismatched data types (from array to scalar)");
        }
    }
}

pair<CVariable, string> SynthesizerExperimental::getDelayVariable(delay* leaf)
    throw(InvalidArgumentException, RuntimeException) {
    if (!leaf) {
        THROW_EXCEPTION(InvalidArgumentException, "leaf must not be NULL");
    }

    map< delay*, pair<CVariable, string> >::iterator it =
        delay_variables_.find(leaf);
    if (it != delay_variables_.end()) {
        return it->second;
    }
    else {
        THROW_EXCEPTION(IllegalStateException, string("Delay variable for ")
                        + "leaf \"" + leaf->getId()->getString()
                        + "\" not found");
    }
}

SynthesizerExperimental::Signal* SynthesizerExperimental::registerSignal(Signal* signal)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!signal) {
        THROW_EXCEPTION(InvalidArgumentException, "\"signal\" must not be "
                        "NULL");
    }

    try {
        set<Signal*>::iterator it = signals_.find(signal);
        if (it != signals_.end()) return *it;
        Signal* new_signal = new Signal(*signal);
        signals_.insert(new_signal);

        logger_.logMessage(Logger::DEBUG, string("Registred new signal ")
                           + new_signal->toString());

        return new_signal;
    } catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

SynthesizerExperimental::Signal* SynthesizerExperimental::getSignal(Process::Interface* out_port,
                                           Process::Interface* in_port)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!out_port && !in_port) {
        THROW_EXCEPTION(InvalidArgumentException, "Both ports cannot be NULL");
    }
    Signal signal(out_port, in_port);
    return registerSignal(&signal);
}

SynthesizerExperimental::Signal* SynthesizerExperimental::getSignalByInPort(
		Process::Interface* out_port)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!out_port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"out_port\" must not be "
                        "NULL");
    }
    Process::Interface* in_port = NULL;
    Leaf::Port* port =  dynamic_cast<Leaf::Port*>(out_port);
    Composite::IOPort* ioport =  dynamic_cast<Composite::IOPort*>(out_port);
    if (port){
		if (port->isConnected()) {
			in_port = port->getConnectedPort();
		}
    }
    else if (ioport){
		if (ioport->isConnectedOutside()) {
			in_port = ioport->getConnectedPortOutside();
		}
    }
    return getSignal(out_port, in_port);
}

SynthesizerExperimental::Signal* SynthesizerExperimental::getSignalByOutPort(
		Process::Interface* in_port)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!in_port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"in_port\" must not be "
                        "NULL");
    }
    Process::Interface* out_port = NULL;
    Leaf::Port* port =  dynamic_cast<Leaf::Port*>(in_port);
    Composite::IOPort* ioport =  dynamic_cast<Composite::IOPort*>(in_port);
    if (port){
		if (port->isConnected()) {
			out_port = port->getConnectedPort();
		}
    }
    else if (ioport){
		if (ioport->isConnectedOutside()) {
			out_port = ioport->getConnectedPortOutside();
		}
    }
    return getSignal(out_port, in_port);
}

SynthesizerExperimental::Signal* SynthesizerExperimental::getSignalInsideByOutPort(
		Composite::IOPort* out_port)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!out_port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"out_port\" must not be "
                        "NULL");
    }
    Process::Interface* in_port = NULL;

	if (out_port->isConnectedInside()) {
		in_port = out_port->getConnectedPortInside();
	}

    return getSignal(out_port, in_port);
}

SynthesizerExperimental::Signal* SynthesizerExperimental::getSignalInsideByInPort(
		Composite::IOPort* in_port)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!in_port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"in_port\" must not be "
                        "NULL");
    }
    Process::Interface* out_port = NULL;
	if (in_port->isConnectedInside()) {
		out_port = in_port->getConnectedPortInside();
	}
    return getSignal(out_port, in_port);
}


bool SynthesizerExperimental::isOutSignal(Signal* signal, Composite* composite)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!signal) {
        THROW_EXCEPTION(InvalidArgumentException, "\"signal\" must not be "
                        "NULL");
    }
    if (!composite) {
        THROW_EXCEPTION(InvalidArgumentException, "\"signal\" must not be "
                        "NULL");
    }

	set<Signal*>::iterator it = signals_.find(signal);
	if (it == signals_.end()){
        THROW_EXCEPTION(RuntimeException, "\"signal\" does not exist. ");
	}
	else {
		list<Composite::IOPort*> ports = composite->getOutIOPorts();
		list<Composite::IOPort*>::iterator port_it;
		for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
			if (getSignalInsideByOutPort(*port_it) == signal) {
				return true;
			}
		}
	}
	return false;
}

bool SynthesizerExperimental::isInSignal(Signal* signal, Composite* composite)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!signal) {
        THROW_EXCEPTION(InvalidArgumentException, "\"signal\" must not be "
                        "NULL");
    }
    if (!composite) {
        THROW_EXCEPTION(InvalidArgumentException, "\"signal\" must not be "
                        "NULL");
    }

	set<Signal*>::iterator it = signals_.find(signal);
	if (it == signals_.end()){
        THROW_EXCEPTION(RuntimeException, "\"signal\" does not exist. ");
	}
	else {
		list<Composite::IOPort*> ports = composite->getInIOPorts();
		list<Composite::IOPort*>::iterator port_it;
		for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
			if (getSignalInsideByInPort(*port_it) == signal) {
				return true;

			}
		}
	}
	return false;
}


string SynthesizerExperimental::scheduleToString(std::list<Forsyde::Id> schedule) const throw() {
    string str;
    for (list<Id>::const_iterator it = schedule.begin(); it != schedule.end();
         ++it) {
        if (it != schedule.begin()) str += ", ";
        str += it->getString();
    }
    return str;
}

bool SynthesizerExperimental::dynamicallyAllocateMemoryForSignalVariable(Signal* signal) {
    // If the signal has an in and out port, then the signal is not written to
    // from any processnetwork input parameter nor read from for the processnetwork output
    // parameters
    return signal->getOutPort() && signal->getInPort()
        && signal->getVariable().getDataType()->isArray();
}

////////////////////////////////////////////////////////////////////////////////////////////

SynthesizerExperimental::Signal::Signal(Process::Interface* out_port, Process::Interface* in_port)
        throw(InvalidArgumentException)
        : out_port_(out_port), in_port_(in_port), has_data_type_(false) {
    if (!out_port_ && !in_port_) {
        THROW_EXCEPTION(InvalidArgumentException, "Both ports cannot be NULL");
    }
}

SynthesizerExperimental::Signal::~Signal() throw() {}

bool SynthesizerExperimental::Signal::hasDataType() const throw() {
    return has_data_type_;
}

CDataType* SynthesizerExperimental::Signal::getDataType() throw(IllegalStateException) {
    return &data_type_;
}

void SynthesizerExperimental::Signal::setDataType(const CDataType& type) throw() {
    has_data_type_ = true;
    data_type_ = type;
}

string SynthesizerExperimental::Signal::getVariableName() const throw() {
    string name("v");
    if (out_port_) {
    	name += out_port_->getProcess()->getId()->getString();
        name += out_port_->getId()->getString();
    }
    else {
        name += "model_input";
    }
    name += "_to_";
    if (in_port_) {
    	name += in_port_->getProcess()->getId()->getString();
        name += in_port_->getId()->getString();
    }
    else {
        name += "model_output";
    }
    return name;
}

bool SynthesizerExperimental::Signal::operator==(const Signal& rhs) const throw() {
    return out_port_ == rhs.out_port_ && in_port_ == rhs.in_port_;
}

bool SynthesizerExperimental::Signal::operator!=(const Signal& rhs) const throw() {
    return !operator==(rhs);
}

bool SynthesizerExperimental::Signal::operator<(const Signal& rhs) const throw() {
    return toString().compare(rhs.toString()) < 0;
}

string SynthesizerExperimental::Signal::toString() const throw() {
    string str;
    str += "\"";
    if (out_port_) {
        str += out_port_->getProcess()->getId()->getString();
        str += ":";
        str += out_port_->getId()->getString();
    }
    str += "\"--\"";
    if (in_port_) {
        str += in_port_->getProcess()->getId()->getString();
        str += ":";
        str += in_port_->getId()->getString();
    }
    str += "\"";
    return str;
}

CVariable SynthesizerExperimental::Signal::getVariable() const
    throw(IllegalStateException) {
    if (!has_data_type_) THROW_EXCEPTION(IllegalStateException,
                                         string("Signal ") + toString()
                                         + " has no data type");
    return CVariable(getVariableName(), data_type_);
}

Process::Interface* SynthesizerExperimental::Signal::getOutPort() const throw() {
    return out_port_;
}

Process::Interface* SynthesizerExperimental::Signal::getInPort() const throw() {
    return in_port_;
}

bool SynthesizerExperimental::SignalComparator::operator() (const Signal* lhs,
                                                const Signal* rhs) const
    throw() {
    return *lhs < *rhs;
}

