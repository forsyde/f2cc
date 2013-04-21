/*
 * Copyright (c) 2011-2012 Gabriel Hjort Blindell <ghb@kth.se>
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

#include "synthesizer.h"
#include "schedulefinder.h"
#include "../forsyde/SY/mapsy.h"
#include "../forsyde/coalescedmapsy.h"
#include "../forsyde/parallelmapsy.h"
#include "../forsyde/SY/delaysy.h"
#include "../forsyde/SY/zipxsy.h"
#include "../forsyde/SY/unzipxsy.h"
#include "../forsyde/SY/fanoutsy.h"
#include "../forsyde/SY/zipwithnsy.h"
#include "../language/cfunction.h"
#include "../language/cdatatype.h"
#include "../tools/tools.h"
#include "../exceptions/unknownarraysizeexception.h"
#include <new>
#include <map>

using namespace f2cc;
using namespace f2cc::ForSyDe;
using std::string;
using std::list;
using std::set;
using std::pair;
using std::bad_alloc;
using std::map;

const string Synthesizer::kIndents = "    ";
const string Synthesizer::kProcessNetworkInputParameterPrefix = "input";
const string Synthesizer::kProcessNetworkOutputParameterPrefix = "output";

Synthesizer::Synthesizer(ProcessNetwork* processnetwork, Logger& logger, Config& config)
        throw(InvalidArgumentException) : processnetwork_(processnetwork), logger_(logger),
                                          config_(config) {
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }
}

Synthesizer::~Synthesizer() throw() {
    set<Signal*>::iterator it;
    for (it = signals_.begin(); it != signals_.end(); ++it) {
        delete *it;
    }
}

Synthesizer::CodeSet Synthesizer::generateCCode()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    target_platform_ = Synthesizer::C;
    return generateCode();
}

Synthesizer::CodeSet Synthesizer::generateCudaCCode()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    target_platform_ = Synthesizer::CUDA;
    return generateCode();
}

Synthesizer::CodeSet Synthesizer::generateCode()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    logger_.logMessage(Logger::INFO, "Checking that the internal processnetwork is "
                       "valid for synthesis...");
    checkProcessNetwork();
    logger_.logMessage(Logger::INFO, "All checks passed");

    logger_.logMessage(Logger::INFO, "Generating leaf schedule...");
    findSchedule();
    logger_.logMessage(Logger::INFO, string("Leaf schedule:\n")
                       + scheduleToString());

    logger_.logMessage(Logger::INFO, "Renaming leaf functions to avoid name "
                       "clashes...");
    renameMapFunctions();
    logger_.logMessage(Logger::INFO, "Combining function duplicates through "
                       "renaming...");
    combineFunctionDuplicates();

    logger_.logMessage(Logger::INFO, "Generating wrapper functions for "
                       "coalesced leafs...");
    generateCoalescedSyWrapperFunctions();
    logger_.logMessage(Logger::INFO, "Combining function duplicates through "
                       "renaming...");
    combineFunctionDuplicates();

    if (target_platform_ == Synthesizer::CUDA) {
        logger_.logMessage(Logger::INFO, "Generating CUDA kernel functions for "
                           "parallel Map leafs...");
        generateCudaKernelFunctions();
        logger_.logMessage(Logger::INFO, "Combining function duplicates "
                           "through renaming...");
        combineFunctionDuplicates();
    }
    else {
        logger_.logMessage(Logger::INFO, "Generating wrapper functions for "
                           "parallel Map leafs...");
        generateParallelMapSyWrapperFunctions();
        logger_.logMessage(Logger::INFO, "Combining function duplicates "
                           "through renaming...");
        combineFunctionDuplicates();
    }

    logger_.logMessage(Logger::INFO, "Creating signal variables...");
    createSignals();

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
    code.header += generateProcessNetworkFunctionDescription() + "\n";
    logger_.logMessage(Logger::DEBUG, "Generating processnetwork function "
                       "prototype...");
    code.header += generateProcessNetworkFunctionPrototypeCode() + ";\n";
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
    code.implementation += generateProcessNetworkFunctionDefinitionCode() + "\n";

    return code;
}

void Synthesizer::checkProcessNetwork()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {}

void Synthesizer::findSchedule() throw (IOException, RuntimeException) {
    schedule_.clear();
    ScheduleFinder schedule_finder(processnetwork_, logger_);
    schedule_ = schedule_finder.findSchedule();
}

Synthesizer::Signal* Synthesizer::registerSignal(Signal* signal)
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

Synthesizer::Signal* Synthesizer::getSignal(Leaf::Port* out_port,
                                           Leaf::Port* in_port)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!out_port && !in_port) {
        THROW_EXCEPTION(InvalidArgumentException, "Both ports cannot be NULL");
    }
    Signal signal(out_port, in_port);
    return registerSignal(&signal);
}

Synthesizer::Signal* Synthesizer::getSignalByOutPort(Leaf::Port* out_port)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!out_port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"out_port\" must not be "
                        "NULL");
    }
    Leaf::Port* in_port = NULL;
    if (out_port->isConnected()) {
        in_port = out_port->getConnectedPort();
    }
    return getSignal(out_port, in_port);
}

Synthesizer::Signal* Synthesizer::getSignalByInPort(Leaf::Port* in_port)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!in_port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"in_port\" must not be "
                        "NULL");
    }
    Leaf::Port* out_port = NULL;
    if (in_port->isConnected()) {
        out_port = in_port->getConnectedPort();
    }
    return getSignal(out_port, in_port);
}

void Synthesizer::renameMapFunctions()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    for (list<Id>::iterator it = schedule_.begin(); it != schedule_.end();
         ++it) {
        Leaf* current_leaf = processnetwork_->getProcess(*it);
        if (!current_leaf) {
            THROW_EXCEPTION(IllegalStateException, string("Leaf \"") +
                            it->getString() + "\" not found");
        }
        logger_.logMessage(Logger::DEBUG, string("Analyzing leaf \"")
                           + current_leaf->getId()->getString() + "\"...");

        Map* mapsy = dynamic_cast<Map*>(current_leaf);
        if (mapsy) {
            list<CFunction*> functions;
            CoalescedMap* cmapsy = dynamic_cast<CoalescedMap*>(mapsy);
            if (cmapsy) {
                functions = cmapsy->getFunctions();
            }
            else {
                functions.push_back(mapsy->getFunction());
            }

            // Set new names to "f<leaf ID>_<function name><counter>"
            list<CFunction*>::iterator func_it;
            int counter;
            for (func_it = functions.begin(), counter = 1;
                 func_it != functions.end(); ++func_it, ++counter) {
                CFunction* function = *func_it;
                string new_name = getGlobalLeafFunctionName(
                    *mapsy->getId(), function->getName()
                    + tools::toString(counter));
                function->setName(new_name);
            }
        }
    }
}

void Synthesizer::combineFunctionDuplicates()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    // The mapset below is used to store the unique functions found across the
    // processnetwork. The body is used as key, and the name as body
    map<string, string> unique_functions;
    for (list<Id>::iterator it = schedule_.begin(); it != schedule_.end();
         ++it) {
        Leaf* current_leaf = processnetwork_->getProcess(*it);
        if (!current_leaf) {
            THROW_EXCEPTION(IllegalStateException, string("Leaf \"") +
                            it->getString() + "\" not found");
        }
        logger_.logMessage(Logger::DEBUG, string("Analyzing leaf \"")
                           + current_leaf->getId()->getString() + "\"...");

        Map* mapsy = dynamic_cast<Map*>(current_leaf);
        if (mapsy) {
            list<CFunction*> functions;
            CoalescedMap* cmapsy = dynamic_cast<CoalescedMap*>(mapsy);
            if (cmapsy) {
                functions = cmapsy->getFunctions();
            }
            else {
                functions.push_back(mapsy->getFunction());
            }

            // Rename duplicate functions
            list<CFunction*>::iterator func_it;
            for (func_it = functions.begin(); func_it != functions.end();
                 ++func_it) {
                CFunction* function = *func_it;
                pair<map<string, string>::iterator, bool> result =
                    unique_functions.insert(pair<string, string>(
                                                function->getBody(),
                                                function->getName()));
                if (!result.second) {
                    string new_name = result.first->second;
                    if (function->getName() != new_name) {
                        logger_.logMessage(Logger::DEBUG, string("Duplicate ")
                                           + "found. Function \""
                                           + function->getName()
                                           + "\" renamed to \"" + new_name
                                           + "\"");
                        function->setName(new_name);
                    }
                }
            }
        }
    }
}

void Synthesizer::generateCoalescedSyWrapperFunctions()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    for (list<Id>::iterator it = schedule_.begin(); it != schedule_.end();
         ++it) {
        Leaf* current_leaf = processnetwork_->getProcess(*it);
        if (!current_leaf) {
            THROW_EXCEPTION(IllegalStateException, string("Leaf \"") +
                            it->getString() + "\" not found");
        }
        logger_.logMessage(Logger::DEBUG, string("Analyzing leaf \"")
                           + current_leaf->getId()->getString() + "\"...");

        CoalescedMap* cmapsy = dynamic_cast<CoalescedMap*>(current_leaf);
        if (cmapsy) {
            list<CFunction*> functions = cmapsy->getFunctions();
            if (functions.size() > 1) {
                try {
                    CFunction wrapper_function =
                        generateCoalescedSyWrapperFunction(functions);
                    wrapper_function.setName(getGlobalLeafFunctionName(
                                                 *cmapsy->getId(),
                                                 wrapper_function.getName()));
                    cmapsy->insertFunctionFirst(wrapper_function);
                }
                catch (InvalidFormatException& ex) {
                    THROW_EXCEPTION(IllegalStateException,
                                    string("Failed to generate wrapper ")
                                    + "function: " + ex.getMessage());
                }
            }
        }
    }
}

CFunction Synthesizer::generateCoalescedSyWrapperFunction(
    list<CFunction*> functions)
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    string new_name = "func_wrapper";
    CDataType new_return_type = *functions.back()->getReturnDataType();
    list<CVariable> new_input_parameters;
    new_input_parameters.push_back(*functions.front()
                                   ->getInputParameters().front());
    if (functions.back()->getNumInputParameters() == 2) {
        new_input_parameters.push_back(*functions.back()
                                       ->getInputParameters().back());
    }

    string new_body("{\n");
    list<CFunction*>::iterator it;
    int id;
    CVariable source_variable(new_input_parameters.front());
    CVariable destination_variable;
    for (it = functions.begin(), id = 1; it != functions.end(); ++it, ++id) {
        string new_variable_name = string("value") + tools::toString(id);
        CDataType new_variable_data_type;
        if ((*it)->getNumInputParameters() == 1) {
            new_variable_data_type = *(*it)->getReturnDataType();
        }
        else {
            new_variable_data_type =
                *(*it)->getInputParameters().back()->getDataType();
        }
        destination_variable = CVariable(new_variable_name,
                                         new_variable_data_type);
        new_body += kIndents
            + destination_variable.getLocalVariableDeclarationString() + ";\n";
        list<CVariable> inputs;
        inputs.push_back(source_variable);
        new_body += generateLeafFunctionExecutionCode(*it, inputs,
                                                         destination_variable);
        source_variable = destination_variable;
    }
    if (new_input_parameters.size() == 1) {
        new_body += kIndents + "return "
            + destination_variable.getReferenceString() + ";\n";
    }
    
    new_body += "}\n";

    return CFunction(new_name, new_return_type, new_input_parameters, new_body);
}

string Synthesizer::generateLeafFunctionDefinitionsCode()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    string code;
    set<string> unique_function_names;
    for (list<Id>::iterator it = schedule_.begin(); it != schedule_.end();
         ++it) {
        Leaf* current_leaf = processnetwork_->getProcess(*it);
        if (!current_leaf) {
            THROW_EXCEPTION(IllegalStateException, string("Leaf \"") +
                            it->getString() + "\" not found");
        }
        logger_.logMessage(Logger::DEBUG, string("Analyzing leaf \"")
                           + current_leaf->getId()->getString() + "\"...");

        list<CFunction*> functions;
        if (Map* mapsy = dynamic_cast<Map*>(current_leaf)) {
            CoalescedMap* cmapsy = dynamic_cast<CoalescedMap*>(mapsy);
            if (cmapsy) {
                functions = cmapsy->getFunctions();
            }
            else {
                functions.push_back(mapsy->getFunction());
            }
        } else if (ZipWithNSY* zipwithnsy =
                   dynamic_cast<ZipWithNSY*>(current_leaf)) {
            functions.push_back(zipwithnsy->getFunction());
        }

        if (functions.size() > 0) {
            // It is important to do this in reversed order as the first
            // function may call the other following functions
            list<CFunction*>::reverse_iterator func_it;
            for (func_it = functions.rbegin(); func_it != functions.rend();
                 ++func_it) {
                CFunction* function = *func_it;
                bool not_yet_defined = 
                    unique_function_names.insert(function->getName()).second;
                if (not_yet_defined) {
                    code += function->getString() + "\n\n";
                }
            }
        }
    }
    
    return code;
}

string Synthesizer::generateProcessNetworkFunctionPrototypeCode()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    string code;
    code += "void executeProcessNetwork("
        + generateProcessNetworkFunctionParameterListCode()
        + ")";
    return code;
}

string Synthesizer::generateProcessNetworkFunctionDefinitionCode()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    string code;
    code += generateProcessNetworkFunctionPrototypeCode() + " {\n";
    code += kIndents + "int i; // Can safely be removed if the compiler warns\n"
        + kIndents + "       // about it being unused\n";
    code += generateSignalVariableDeclarationsCode() + "\n";
    code += generateDelayVariableDeclarationsCode() + "\n";
    code += generateArrayInputOutputsToSignalsAliasingCode() + "\n";
    code += generateInputsToSignalsCopyingCode() + "\n";
    code += kIndents + "// Execute leafs\n";

    // First, execute the first step of all delay leafs
    for (list<Id>::iterator it = schedule_.begin(); it != schedule_.end();
         ++it) {
        Leaf* current_leaf = processnetwork_->getProcess(*it);
        if (!current_leaf) {
            THROW_EXCEPTION(IllegalStateException, string("Leaf \"") +
                            it->getString() + "\" not found");
        }
        if (delay* delaysy = dynamic_cast<delay*>(current_leaf)) {
            try {
                code += generateLeafExecutionCodeFordelayStep1(delaysy);
            }
            catch (InvalidProcessNetworkException& ex) {
                THROW_EXCEPTION(InvalidProcessNetworkException, "Error in leaf \""
                                + current_leaf->getId()->getString() + "\": "
                                + ex.getMessage());
            }
        }
    }

    // Then, execute all leafs in order, but ignore all delay leafs
    for (list<Id>::iterator it = schedule_.begin(); it != schedule_.end();
         ++it) {
        Leaf* current_leaf = processnetwork_->getProcess(*it);
        if (!current_leaf) {
            THROW_EXCEPTION(IllegalStateException, string("Leaf \"") +
                            it->getString() + "\" not found");
        }
        try {
            code += generateLeafExecutionCode(current_leaf);
        }
        catch (InvalidProcessNetworkException& ex) {
            THROW_EXCEPTION(InvalidProcessNetworkException, "Error in leaf \""
                            + current_leaf->getId()->getString() + "\": "
                            + ex.getMessage());
        }
    }

    // After the entire schedule has been executed, execute the second step
    // of all delay leafs
    for (list<Id>::iterator it = schedule_.begin(); it != schedule_.end();
         ++it) {
        Leaf* current_leaf = processnetwork_->getProcess(*it);
        if (!current_leaf) {
            THROW_EXCEPTION(IllegalStateException, string("Leaf \"") +
                            it->getString() + "\" not found");
        }
        if (delay* delaysy = dynamic_cast<delay*>(current_leaf)) {
            try {
                code += generateLeafExecutionCodeFordelayStep2(delaysy);
            }
            catch (InvalidProcessNetworkException& ex) {
                THROW_EXCEPTION(InvalidProcessNetworkException, "Error in leaf \""
                                + current_leaf->getId()->getString() + "\": "
                                + ex.getMessage());
            }
        }
    }

    code += "\n";
    code += generateSignalsToOutputsCopyingCode() + "\n";
    code += "\n";
    code += generateSignalVariableCleanupCode();
    code += "}";
    return code;
}

string Synthesizer::generateProcessNetworkFunctionDescription() 
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    string desc;
    desc += string("/**\n")
        + " * Executes the processnetwork.\n"
        + " *\n";

    // Generate description for the function input parameters
    list<Leaf::Port*> inputs = processnetwork_->getInputs();
    list<Leaf::Port*>::iterator it;
    int id;
    for (it = inputs.begin(), id = 1; it != inputs.end(); ++it, ++id) {
        Signal* signal = getSignalByInPort(*it);
        CDataType data_type = *signal->getDataType();
        string param_name = kProcessNetworkInputParameterPrefix + tools::toString(id);
        string leaf_name =
            signal->getInPort()->getProcess()->getId()->getString();
        desc += string(" * @param ") + param_name + "\n";
        desc += string(" *        Input to leaf \"") + leaf_name
            + "\".\n";
        if (data_type.isArray()) {
            desc += string(" *        Expects an array of size ")
                + tools::toString(data_type.getArraySize()) + ".\n";
        }
    }

    // Generate description for the function output parameters
    list<Leaf::Port*> outputs = processnetwork_->getOutputs();
    for (it = outputs.begin(), id = 1; it != outputs.end(); ++it, ++id) {
        Signal* signal = getSignalByOutPort(*it);
        CDataType data_type = *signal->getDataType();
        string param_name = kProcessNetworkOutputParameterPrefix + tools::toString(id);
        string leaf_name =
            signal->getOutPort()->getProcess()->getId()->getString();
        desc += string(" * @param ") + param_name + "\n";
        desc += string(" *        Output from leaf \"") + leaf_name
            + "\".\n";
        if (data_type.isArray()) {
            desc += string(" *        Expects an array of size ")
                + tools::toString(data_type.getArraySize()) + ".\n";
        }
    }

    desc += " */\n";
    return desc;
}

string Synthesizer::generateProcessNetworkFunctionParameterListCode()
    throw(InvalidProcessNetworkException, RuntimeException) {
    string code;

    // Generate input parameters
    bool has_input_parameter = false;
    list<Leaf::Port*> inputs = processnetwork_->getInputs();
    list<Leaf::Port*>::iterator it;
    int id;
    for (it = inputs.begin(), id = 1; it != inputs.end(); ++it, ++id) {
        if (it != inputs.begin()) code += ", ";
        CDataType data_type = *getSignalByInPort(*it)->getDataType();
        data_type.setIsConst(true);
        CVariable parameter(kProcessNetworkInputParameterPrefix + tools::toString(id),
                            data_type);
        code += parameter.getInputParameterDeclarationString();
        has_input_parameter = true;
    }

    // Generate output parameters
    list<Leaf::Port*> outputs = processnetwork_->getOutputs();
    for (it = outputs.begin(), id = 1; it != outputs.end(); ++it, ++id) {
        if (has_input_parameter || it != outputs.begin()) code += ", ";
        CDataType data_type = *getSignalByOutPort(*it)->getDataType();
        if (!data_type.isArray()) data_type.setIsPointer(true);
        CVariable parameter(kProcessNetworkOutputParameterPrefix + tools::toString(id),
                            data_type);
        code += parameter.getInputParameterDeclarationString();
    }

    return code;
}

string Synthesizer::generateInputsToSignalsCopyingCode()
    throw(InvalidProcessNetworkException, RuntimeException) {
    string code;

    list<Leaf::Port*> inputs = processnetwork_->getInputs();
    list<Leaf::Port*>::iterator it;
    int id;
    bool at_least_one = false;
    for (it = inputs.begin(), id = 1; it != inputs.end(); ++it, ++id) {
        Signal* signal = getSignalByInPort(*it);
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
        code = kIndents + "// Copy processnetwork inputs to signal variables\n" + code;
    }

    return code;
}

string Synthesizer::generateSignalsToOutputsCopyingCode()
    throw(InvalidProcessNetworkException, RuntimeException) {
    string code;

    list<Leaf::Port*> outputs = processnetwork_->getOutputs();
    list<Leaf::Port*>::iterator it;
    int id;
    bool at_least_one = false;
    for (it = outputs.begin(), id = 1; it != outputs.end(); ++it, ++id) {
        Signal* signal = getSignalByOutPort(*it);
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

string Synthesizer::generateArrayInputOutputsToSignalsAliasingCode()
    throw(InvalidProcessNetworkException, RuntimeException) {
    string code;
    bool at_least_one = false;

    // Iterate over the input parameters
    list<Leaf::Port*> inputs = processnetwork_->getInputs();
    list<Leaf::Port*>::iterator it;
    int id;
    for (it = inputs.begin(), id = 1; it != inputs.end(); ++it, ++id) {
        Signal* signal = getSignalByInPort(*it);
        logger_.logMessage(Logger::DEBUG, string("Analyzing signal ")
                           + signal->toString() + "...");

        CDataType data_type = *signal->getDataType();
        if (!data_type.isArray()) continue;
        at_least_one = true;
        CVariable input_parameter(
            kProcessNetworkInputParameterPrefix + tools::toString(id), data_type);
        code += generateVariableCopyingCode(signal->getVariable(),
                                            input_parameter, false);
    }

    // Iterate over the output parameters
    list<Leaf::Port*> outputs = processnetwork_->getOutputs();
    for (it = outputs.begin(), id = 1; it != outputs.end(); ++it, ++id) {
        Signal* signal = getSignalByOutPort(*it);
        logger_.logMessage(Logger::DEBUG, string("Analyzing signal ")
                           + signal->toString() + "...");

        CDataType data_type = *signal->getDataType();
        if (!data_type.isArray()) continue;
        at_least_one = true;
        CVariable output_parameter(
            kProcessNetworkOutputParameterPrefix + tools::toString(id), data_type);
        code += generateVariableCopyingCode(signal->getVariable(),
                                            output_parameter, false);
    }

    if (at_least_one) {
        code = kIndents + "// Alias signal array variables with processnetwork "
            "input/output arrays\n" + code;
    }

    return code;
}

void Synthesizer::createSignals()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    signals_.clear();
    for (list<Id>::iterator it = schedule_.begin(); it != schedule_.end();
         ++it) {
        Leaf* current_leaf = processnetwork_->getProcess(*it);
        if (!current_leaf) {
            THROW_EXCEPTION(IllegalStateException, string("Leaf \"") +
                            it->getString() + "\" not found");
        }
        logger_.logMessage(Logger::DEBUG, string("Analyzing leaf \"")
                           + current_leaf->getId()->getString() + "\"...");

        list<Leaf::Port*> ports = current_leaf->getInPorts();
        list<Leaf::Port*>::iterator port_it;
        for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
            getSignalByInPort(*port_it);
        }
        ports = current_leaf->getOutPorts();
        for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
            getSignalByOutPort(*port_it);
        }
    }

    logger_.logMessage(Logger::INFO, string("Created ")
                       + tools::toString(signals_.size()) + " signal(s)");
}

void Synthesizer::createDelayVariables() throw(IOException, RuntimeException) {
    delay_variables_.clear();

    list<Id>::iterator it;
    int counter;
    for (it = schedule_.begin(), counter = 1; it != schedule_.end(); ++it) {
        Leaf* current_leaf = processnetwork_->getProcess(*it);
        if (!current_leaf) {
            THROW_EXCEPTION(IllegalStateException, string("Leaf \"") +
                            it->getString() + "\" not found");
        }
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

void Synthesizer::setInputArraySignalVariableDataTypesAsConst()
    throw(IOException, RuntimeException) {
    list<Leaf::Port*> inputs = processnetwork_->getInputs();
    for (list<Leaf::Port*>::iterator it = inputs.begin(); it != inputs.end();
         ++it) {
        Signal* signal = getSignalByInPort(*it);
        CDataType data_type = *signal->getDataType();
        if (!data_type.isArray()) continue;
        logger_.logMessage(Logger::DEBUG, string("Modifying data type for ")
                           + "signal " + signal->toString() + "...");
        data_type.setIsConst(true);
        signal->setDataType(data_type);
    }
}

void Synthesizer::discoverSignalDataTypes()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    for (set<Signal*>::iterator it = signals_.begin(); it != signals_.end();
         ++it) {
        try {
            discoverSignalDataTypeBackwardSearch(*it);
        }
        catch (InvalidProcessNetworkException&) {
            // Data type was not found; do second attempt with forward search
            discoverSignalDataTypeForwardSearch(*it);
        }
    }
}

CDataType Synthesizer::discoverSignalDataTypeForwardSearch(Signal* signal)
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    logger_.logMessage(Logger::DEBUG, string("Searching data type for signal ")
                       + signal->toString() + "...");

    if (signal->hasDataType()) {
        logger_.logMessage(Logger::DEBUG, string("Found data type \"")
                           + signal->getVariable().getDataType()->toString()
                           + "\"");
        return *signal->getVariable().getDataType();
    }

    if (!signal->getInPort()) {
        logger_.logMessage(Logger::DEBUG, "Reached end of network");
        THROW_EXCEPTION(InvalidProcessNetworkException, string("No data type for ")
                        + "signal " + signal->toString() + " could be found");
    }

    // Check if the in port leaf is a Map or ZipWithN, and if so, get the
    // data type from the function argument's corresponding input parameter;
    // if not, then the data type of a neighbouring signal is used
    CDataType data_type;
    Leaf* leaf = signal->getInPort()->getProcess();
    if (Map* mapsy = dynamic_cast<Map*>(leaf)) {
        data_type =
            *mapsy->getFunction()->getInputParameters().front()->getDataType();
        data_type.setIsConst(false);
    }
    else if (ZipWithNSY* zipwithnsy = dynamic_cast<ZipWithNSY*>(leaf)) {
        Leaf::Port* sought_port = signal->getInPort();
        list<Leaf::Port*> in_ports = zipwithnsy->getInPorts();
        list<Leaf::Port*>::iterator port_it;
        list<CVariable*> input_parameters =
            zipwithnsy->getFunction()->getInputParameters();
        list<CVariable*>::iterator param_it;
        if (in_ports.size() > input_parameters.size()) {
            THROW_EXCEPTION(IllegalStateException, string("In leaf \"")
                            + zipwithnsy->getId()->getString() + "\": "
                            + "Number of in ports is greater than the number "
                            + "of input parameters");
        }
        bool port_found = false;
        for (port_it = in_ports.begin(), param_it = input_parameters.begin();
             port_it != in_ports.end(); ++port_it, ++param_it) {
            if (*port_it == sought_port) {
                data_type = *(*param_it)->getDataType();
                port_found = true;
                break;
            }
        }

        if (!port_found) {
            THROW_EXCEPTION(IllegalStateException, string("Port \"")
                            + sought_port->toString() + "\" was not found in "
                            + "leaf \""
                            + zipwithnsy->getId()->getString() + "\"");
        }
    }
    else {
        bool data_type_found = false;
        list<Leaf::Port*> out_ports = leaf->getOutPorts();
        for (list<Leaf::Port*>::iterator it = out_ports.begin(); 
             it != out_ports.end(); ++it) {
            Signal* next_signal = getSignalByOutPort(*it);
            try {
                data_type = discoverSignalDataTypeForwardSearch(next_signal);
                data_type_found = true;
            }
            catch (InvalidProcessNetworkException&) {
                // Ignore exception as it only indicates that no data type was
                // found for next signal
            }
        }
        if (!data_type_found) {
            THROW_EXCEPTION(InvalidProcessNetworkException, string("No data type for ")
                            + "signal " + signal->toString()
                            + " could be found");
        }

        if (dynamic_cast<unzipx*>(leaf)) {
            data_type.setIsArray(true);
        }
    }

    // If this leaf is a zipx and the data type is an array, then we cannot
    // be sure of its array size at this point and therefore must make it
    // unknown
    if (dynamic_cast<zipx*>(leaf) && data_type.isArray()) {
        data_type.setIsArray(true);
    }

    signal->setDataType(data_type);
    logger_.logMessage(Logger::DEBUG, string("Found data type \"")
                       + data_type.toString() + "\"");
    return data_type;
}

CDataType Synthesizer::discoverSignalDataTypeBackwardSearch(Signal* signal)
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    logger_.logMessage(Logger::DEBUG, string("Searching data type for signal ")
                       + signal->toString() + "...");

    if (signal->hasDataType()) {
        logger_.logMessage(Logger::DEBUG, string("Found data type \"")
                           + signal->getVariable().getDataType()->toString()
                           + "\"");
        return *signal->getVariable().getDataType();
    }

    if (!signal->getOutPort()) {
        logger_.logMessage(Logger::DEBUG, "Reached end of network");
        THROW_EXCEPTION(InvalidProcessNetworkException, string("No data type for ")
                        + "signal " + signal->toString() + " could be found");
    }

    // Check if the out port leaf is a Map or ZipWithNSY, and if so, get
    // the data type of either its function argument's return value or its
    // function argument's last input parameter; if not, then the data type of
    // a neighbouring signal is used
    CDataType data_type;
    Leaf* leaf = signal->getOutPort()->getProcess();
    if (Map* mapsy = dynamic_cast<Map*>(leaf)) {
        CFunction* function = mapsy->getFunction();
        if (function->getNumInputParameters() == 1) {
            data_type = *mapsy->getFunction()->getReturnDataType();
        }
        else if (function->getNumInputParameters() == 2) {
            data_type = *mapsy->getFunction()->getInputParameters().back()
                ->getDataType();
        }
        else {
            THROW_EXCEPTION(IllegalStateException, string("Function argument ")
                            + "of Map leaf \""
                            + mapsy->getId()->getString() + "\" has too many "
                            + "input parameters");
        }
    }
    else if (ZipWithNSY* zipwithnsy = dynamic_cast<ZipWithNSY*>(leaf)) {
        CFunction* function = zipwithnsy->getFunction();
        if (function->getNumInputParameters() == zipwithnsy->getNumInPorts()) {
            data_type = *zipwithnsy->getFunction()->getReturnDataType();
        }
        else if (function->getNumInputParameters() + 1 
                 == zipwithnsy->getNumInPorts()) {
            data_type = *zipwithnsy->getFunction()->getInputParameters().back()
                ->getDataType();
        }
        else {
            THROW_EXCEPTION(IllegalStateException, string("Function argument ")
                            + "of ZipWithNSY leaf \""
                            + zipwithnsy->getId()->getString() + "\" has an "
                            + "unexpected number of input parameters");
        }
    }
    else {
        bool data_type_found = false;
        list<Leaf::Port*> in_ports = leaf->getInPorts();
        for (list<Leaf::Port*>::iterator it = in_ports.begin(); 
             it != in_ports.end(); ++it) {
            Signal* prev_signal = getSignalByInPort(*it);
            try {
                data_type = discoverSignalDataTypeBackwardSearch(prev_signal);
                data_type_found = true;
            }
            catch (InvalidProcessNetworkException&) {
                // Ignore exception as it only indicates that no data type was
                // found for previous signal
            }
        }
        if (!data_type_found) {
            THROW_EXCEPTION(InvalidProcessNetworkException, string("No data type for ")
                            + "signal " + signal->toString()
                            + " could be found");
        }

        if (dynamic_cast<zipx*>(leaf)) {
            data_type.setIsArray(true);
        }
    }

    // If this leaf is an unzipx and the data type is an array, then we
    // cannot be sure of its array size at this point and therefore must make it
    // unknown
    if (dynamic_cast<unzipx*>(leaf) && data_type.isArray()) {
        data_type.setIsArray(true);
    }

    signal->setDataType(data_type);
    logger_.logMessage(Logger::DEBUG, string("Found data type \"")
                       + data_type.toString() + "\"");
    return data_type;
}

void Synthesizer::propagateArraySizesBetweenSignals()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    for (list<Id>::iterator it = schedule_.begin(); it != schedule_.end();
         ++it) {
        Leaf* current_leaf = processnetwork_->getProcess(*it);
        if (!current_leaf) {
            THROW_EXCEPTION(IllegalStateException, string("Leaf \"") +
                            it->getString() + "\" not found");
        }

        list<Leaf::Port*> ports = current_leaf->getInPorts();
        list<Leaf::Port*>::iterator port_it;
        for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
            Signal* signal = getSignalByInPort(*port_it);
            try {
                discoverSignalArraySizeBackwardSearch(signal);
            }
            catch (InvalidProcessNetworkException&) {
                // Do second attempt with forward search
                discoverSignalArraySizeForwardSearch(signal);
            }
        }
        ports = current_leaf->getOutPorts();
        for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
            Signal* signal = getSignalByOutPort(*port_it);
            try {
                discoverSignalArraySizeForwardSearch(signal);
            }
            catch (InvalidProcessNetworkException&) {
                // Do second attempt with backward search
                discoverSignalArraySizeBackwardSearch(signal);
            }
        }
    }
}

size_t Synthesizer::discoverSignalArraySizeForwardSearch(Signal* signal)
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    logger_.logMessage(Logger::DEBUG, string("Searching array size for signal ")
                       + signal->toString() + "...");

    CDataType data_type = *signal->getVariable().getDataType();
    if (data_type.hasArraySize()) {
        logger_.logMessage(Logger::DEBUG, string("Found array size ")
                           + tools::toString(data_type.getArraySize()));
        return data_type.getArraySize();
    }

    if (!signal->getInPort()) {
        logger_.logMessage(Logger::DEBUG, "Reached end of network");        
        THROW_EXCEPTION(InvalidProcessNetworkException, string("No array size for ")
                        + "signal " + signal->toString() + " could be found");
    }

    // Check if the in port leaf is an unzipx, and if so, get its array
    // size by summing up the array sizes of its out port signals; if it is not
    // an unzipx, get the array size from a neighbouring signal
    size_t array_size = 0;
    Leaf* leaf = signal->getInPort()->getProcess();
    list<Leaf::Port*> out_ports = leaf->getOutPorts();
    if (out_ports.size() == 0) {
        THROW_EXCEPTION(IllegalStateException, string("Leaf \"")
                        + leaf->getId()->getString() + "\" does not "
                        "have any out ports");
    }
    try {
        if (dynamic_cast<unzipx*>(leaf)) {
            logger_.logMessage(Logger::DEBUG, "Found unzipx leaf. Summing "
                               "up array sizes from its out ports...");
            list<Leaf::Port*>::iterator it;
            for (it = out_ports.begin(); it != out_ports.end(); ++it) {
                Signal* next_signal = getSignalByOutPort(*it);
                array_size += discoverSignalArraySizeForwardSearch(next_signal);
            }
        }
        else {
            Signal* next_signal = getSignalByOutPort(out_ports.front());
            array_size = discoverSignalArraySizeForwardSearch(next_signal);
        }
    }
    catch (InvalidProcessNetworkException&) {
        // Throw new exception but for this signal
        THROW_EXCEPTION(InvalidProcessNetworkException, string("No data type for ")
                        + "signal " + signal->toString()
                        + " could be found");
    }
    data_type.setArraySize(array_size);
    signal->setDataType(data_type);
    logger_.logMessage(Logger::DEBUG, string("Found array size ")
                       + tools::toString(data_type.getArraySize()));
    return array_size;
}

size_t Synthesizer::discoverSignalArraySizeBackwardSearch(Signal* signal)
        throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    logger_.logMessage(Logger::DEBUG, string("Searching array size for signal ")
                       + signal->toString() + "...");

    CDataType data_type = *signal->getVariable().getDataType();
    if (data_type.hasArraySize()) {
        logger_.logMessage(Logger::DEBUG, string("Found array size ")
                           + tools::toString(data_type.getArraySize()));
        return data_type.getArraySize();
    }

    if (!signal->getOutPort()) {
        logger_.logMessage(Logger::DEBUG, "Reached end of network");        
        THROW_EXCEPTION(InvalidProcessNetworkException, string("No array size for ")
                        + "signal " + signal->toString() + " could be found");
    }

    // Check if the in port leaf is a zipx, and if so, get its array
    // size by summing up the array sizes of its in port signals; if it is not
    // a zipx, get the array size from a neighbouring signal
    size_t array_size = 0;
    Leaf* leaf = signal->getOutPort()->getProcess();
    list<Leaf::Port*> in_ports = leaf->getInPorts();
    if (in_ports.size() == 0) {
        THROW_EXCEPTION(IllegalStateException, string("Leaf \"")
                        + leaf->getId()->getString() + "\" does not "
                        "have any in ports");
    }
    try {
        if (dynamic_cast<zipx*>(leaf)) {
            logger_.logMessage(Logger::DEBUG, "Found zipx leaf. Summing "
                               "up array sizes from its in ports...");
            list<Leaf::Port*>::iterator it;
            for (it = in_ports.begin(); it != in_ports.end(); ++it) {
                Signal* next_signal = getSignalByInPort(*it);
                array_size +=
                    discoverSignalArraySizeBackwardSearch(next_signal);
            }
        }
        else {
            Signal* next_signal = getSignalByInPort(in_ports.front());
            array_size = discoverSignalArraySizeBackwardSearch(next_signal);
        }
    }
    catch (InvalidProcessNetworkException&) {
        // Throw new exception but for this signal
        THROW_EXCEPTION(InvalidProcessNetworkException, string("No data type for ")
                        + "signal " + signal->toString()
                        + " could be found");
    }
    data_type.setArraySize(array_size);
    signal->setDataType(data_type);
    logger_.logMessage(Logger::DEBUG, string("Found array size ")
                       + tools::toString(data_type.getArraySize()));
    return array_size;
}

void Synthesizer::propagateSignalArraySizesToLeafFunctions()
    throw(IOException, RuntimeException) {
    // @todo implement
    logger_.logMessage(Logger::WARNING, "Signal-to-function array size "
                       "propagation not implemented");
}

string Synthesizer::generateSignalVariableDeclarationsCode()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    try {
        string code;
        code += kIndents + "// Declare signal variables\n";
        set<Signal*>::iterator it;
        for (it = signals_.begin(); it != signals_.end(); ++it) {
            Signal* signal = *it;
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
        return code;
    }
    catch (UnknownArraySizeException& ex) {
        THROW_EXCEPTION(InvalidProcessNetworkException, ex.getMessage());
    }
}

string Synthesizer::generateDelayVariableDeclarationsCode()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
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
            code += kIndents + "static ";
            code += variable.getLocalVariableDeclarationString();
            code += " = ";
            code += initial_value;
            code += ";\n";
        }
        return code;
    }
    catch (UnknownArraySizeException& ex) {
        THROW_EXCEPTION(InvalidProcessNetworkException, ex.getMessage());
    }
}

pair<CVariable, string> Synthesizer::getDelayVariable(delay* leaf)
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

string Synthesizer::generateSignalVariableCleanupCode()
    throw(IOException, RuntimeException) {
    string code;
    set<Signal*>::iterator it;
    bool at_least_one = false;
    for (it = signals_.begin(); it != signals_.end(); ++it) {
        Signal* signal = *it;
        logger_.logMessage(Logger::DEBUG, string("Analyzing signal ")
                           + signal->toString() + "...");

        if (dynamicallyAllocateMemoryForSignalVariable(signal)) {
            at_least_one = true;
            code += kIndents + "delete[] "
                + signal->getVariable().getReferenceString() + ";\n";
        }
    }
    if (at_least_one) code = kIndents + "// Clean up memory\n" + code;
    return code;
}

string Synthesizer::scheduleToString() const throw() {
    string str;
    for (list<Id>::const_iterator it = schedule_.begin(); it != schedule_.end();
         ++it) {
        if (it != schedule_.begin()) str += ", ";
        str += it->getString();
    }
    return str;
}
        
string Synthesizer::generateLeafExecutionCode(Leaf* leaf)
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    logger_.logMessage(Logger::DEBUG, string("Generating execution code for ")
                       + "leaf \"" + leaf->getId()->getString()
                       + "\"...");

    string code;
    if (dynamic_cast<delay*>(leaf)) {
        // Do nothing
        return "";
    }
    else if (Map* cast_leaf = dynamic_cast<Map*>(leaf)) {
        return generateLeafExecutionCodeForMap(cast_leaf);
    }
    else if (ZipWithNSY* cast_leaf = dynamic_cast<ZipWithNSY*>(leaf)) {
        return generateLeafExecutionCodeForZipWithNSY(cast_leaf);
    }
    else if (zipx* cast_leaf = dynamic_cast<zipx*>(leaf)) {
        return generateLeafExecutionCodeForzipx(cast_leaf);
    }
    else if (unzipx* cast_leaf = dynamic_cast<unzipx*>(leaf)) {
        return generateLeafExecutionCodeForunzipx(cast_leaf);
    }
    else if (fanout* cast_leaf = dynamic_cast<fanout*>(leaf)) {
        return generateLeafExecutionCodeForfanout(cast_leaf);
    }
    else {
        THROW_EXCEPTION(InvalidArgumentException, string("Leaf \"")
                        + leaf->getId()->getString() + "\" is of "
                        "unrecognized leaf type \"" + leaf->type()
                        + "\"");
    }
    return code;
}

void Synthesizer::generateCudaKernelFunctions()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    for (list<Id>::iterator it = schedule_.begin(); it != schedule_.end();
         ++it) {
        Leaf* current_leaf = processnetwork_->getProcess(*it);
        if (!current_leaf) {
            THROW_EXCEPTION(IllegalStateException, string("Leaf \"") +
                            it->getString() + "\" not found");
        }
        logger_.logMessage(Logger::DEBUG, string("Analyzing leaf \"")
                           + current_leaf->getId()->getString() + "\"...");

        ParallelMap* parmapsy = dynamic_cast<ParallelMap*>(current_leaf);
        if (parmapsy) {
            // Add "__device__" prefix to all existing functions
            list<CFunction*> functions = parmapsy->getFunctions();
            list<CFunction*>::iterator func_it;
            for (func_it = functions.begin(); func_it != functions.end();
                 ++func_it) {
                (*func_it)->setDeclarationPrefix("__device__");
            }
            try {
                CFunction kernel_function =
                    generateCudaKernelFunction(functions.front(),
                                               parmapsy->getNumProcesses());
                kernel_function.setName(getGlobalLeafFunctionName(
                                            *parmapsy->getId(),
                                            kernel_function.getName()));
                parmapsy->insertFunctionFirst(kernel_function);
                CFunction wrapper_function =
                    generateCudaKernelWrapperFunction(
                        &kernel_function, parmapsy->getNumProcesses());
                wrapper_function.setName(
                    getGlobalLeafFunctionName(*parmapsy->getId(),
                                                 wrapper_function.getName()));
                parmapsy->insertFunctionFirst(wrapper_function);
            }
            catch (InvalidProcessNetworkException& ex) {
                THROW_EXCEPTION(InvalidProcessNetworkException, string("Error in ")
                                + "leaf \"" + parmapsy->getId()->getString() 
                                + "\": " + ex.getMessage());
            }
        }
    }
}

CFunction Synthesizer::generateCudaKernelFunction(CFunction* function,
                                                  size_t num_leafs)
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    string new_name("kernel");
    string input_param_name("input");
    string output_param_name("output");
    string offset_param_name("index_offset");
    CDataType new_return_type(CDataType::VOID, false, false, 0, false, false);
    CDataType offset_param_type(CDataType::INT, false, false, 0, false, false);
    size_t input_data_size, output_data_size;
    list<CVariable*> old_parameters = function->getInputParameters();
    CDataType old_input_param_data_type = *old_parameters.front()
        ->getDataType();

    // Create function parameters
    list<CVariable> new_parameters;
    if (old_parameters.size() == 1) {
        // Create input parameter
        CVariable new_input_param(input_param_name, old_input_param_data_type);
        if (old_input_param_data_type.isArray()) {
            if (!old_input_param_data_type.hasArraySize()) {
                THROW_EXCEPTION(InvalidProcessNetworkException, string("Data type of ")
                                + "first input parameter has no array size");
            }
            input_data_size = num_leafs * old_input_param_data_type
                .getArraySize();
            new_input_param.getDataType()->setArraySize(input_data_size);
        }
        else {
            new_input_param.getDataType()->setIsConst(true);
            new_input_param.getDataType()->setIsArray(true);
            new_input_param.getDataType()->setArraySize(num_leafs);
        }

        // Create output parameter
        CVariable new_output_param(output_param_name,
                                   *function->getReturnDataType());
        output_data_size = num_leafs;
        new_output_param.getDataType()->setIsArray(true);
        new_output_param.getDataType()->setArraySize(num_leafs);
        
        new_parameters.push_back(new_input_param);
        new_parameters.push_back(new_output_param);
    }
    else if (old_parameters.size() == 2) {
        // Create input parameter
        CVariable new_input_param(input_param_name, old_input_param_data_type);
        if (!old_input_param_data_type.hasArraySize()) {
            THROW_EXCEPTION(InvalidProcessNetworkException, string("Data type of ")
                            + "first input parameter has no array size");
        }
        input_data_size = num_leafs * old_input_param_data_type
            .getArraySize();
        new_input_param.getDataType()->setArraySize(input_data_size);

        // Create output parameter
        CDataType old_output_param_data_type = *old_parameters.back()
            ->getDataType();
        CVariable new_output_param(output_param_name,
                                   old_output_param_data_type);
        if (!old_output_param_data_type.hasArraySize()) {
            THROW_EXCEPTION(InvalidProcessNetworkException, string("Data type of ")
                            + "second input parameter has no array size");
        }
        output_data_size = num_leafs * old_output_param_data_type
            .getArraySize();
        new_output_param.getDataType()->setArraySize(output_data_size);
        
        new_parameters.push_back(new_input_param);
        new_parameters.push_back(new_output_param);
    }
    else {
        THROW_EXCEPTION(IllegalStateException, "Function has unexpected "
                        "number of input parameters");
    }
    new_parameters.push_back(CVariable(offset_param_name, offset_param_type));

    // Create body
    string new_body = string("{\n");
    string input_data_variable_name;
    string output_data_variable_name = output_param_name;

    // Generate code for calculating the index using the thread block X and
    // thread X coordinates
    new_body += kIndents + "unsigned int global_index = "
        + "(blockIdx.x * blockDim.x + threadIdx.x) + " + offset_param_name
        + ";\n";
    if (config_.useSharedMemoryForInput()) {
        logger_.logMessage(Logger::INFO, "USING SHARED MEMORY FOR INPUT DATA: "
                           "YES");
        input_data_variable_name = "input_cached";
        new_body += kIndents + "extern __shared__ "
            + CDataType::typeToString(old_input_param_data_type.getType()) + " "
            + input_data_variable_name + "[];\n";
    }
    else {
        logger_.logMessage(Logger::INFO, "USING SHARED MEMORY FOR INPUT DATA: "
                           "NO");
        input_data_variable_name = input_param_name;
    }

    // If too many threads are generated, then we want to avoid them from
    // doing any leafing, and we do this with an IF statement checking if
    // the thread is out of range
    new_body += kIndents + "if (global_index < "
        + tools::toString(num_leafs) + ") {\n";
    string input_index_variable_name = "input_index";
    string output_index_variable_name = "global_index";
    if (old_parameters.size() == 2) {
        output_index_variable_name += " * "
            + tools::toString(old_parameters.back()->getDataType()
                              ->getArraySize());
    }

    if (config_.useSharedMemoryForInput()) {
        // Generate code for copying input data from global memory into shared
        // memory
        new_body += kIndents + kIndents + "int " + input_index_variable_name
            + " = threadIdx.x * "
            + tools::toString(old_input_param_data_type.getArraySize()) + ";\n";
        new_body += kIndents + kIndents + "int global_input_index"
            + " = global_index * "
            + tools::toString(old_input_param_data_type.getArraySize()) + ";\n";
        int num_elements_per_thread = old_input_param_data_type.getArraySize();
        for (int i = 0; i < num_elements_per_thread; ++i) {
            new_body += kIndents + kIndents
                + input_data_variable_name + "[" + input_index_variable_name
                + " + " + tools::toString(i) + "] = " + input_param_name + "["
                + "global_input_index + " + tools::toString(i) + "];\n";
        }
    }
    else {
        new_body += kIndents + kIndents + "int " + input_index_variable_name
            + " = global_index * "
            + tools::toString(old_input_param_data_type.getArraySize()) + ";\n";
    }

    // Generate code for invoking the kernel
    if (old_parameters.size() == 1) {
        new_body += kIndents + kIndents + output_data_variable_name
            + "[" + output_index_variable_name + "]"
            " = " + function->getName() + "(";
        if (old_input_param_data_type.getArraySize() > 0) {
            new_body += "&";
        }
        new_body += input_data_variable_name + "[" + input_index_variable_name 
            + "]);\n";
    }
    else {
        new_body += kIndents + kIndents + function->getName() + "(";
        if (old_input_param_data_type.getArraySize() > 0) {
            new_body += "&";
        }
        new_body += input_data_variable_name + "[" + input_index_variable_name
            + "], "
            + "&" + output_data_variable_name + "["
            + output_index_variable_name + "]);\n";
    }
    new_body += kIndents + "}\n";
    new_body += "}";

    return CFunction(new_name, new_return_type, new_parameters, new_body,
                     string("__global__"));
}

CFunction Synthesizer::generateCudaKernelWrapperFunction(CFunction* function,
    size_t num_leafs)
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    string new_name("kernel_wrapper");
    string input_param_name("input");
    string output_param_name("output");
    string device_input_variable_name("device_input");
    string device_output_variable_name("device_output");
    CDataType new_return_type(CDataType::VOID, false, false, 0, false, false);

    // Create input parameters
    list<CVariable*> old_parameters = function->getInputParameters();
    if (old_parameters.size() != 3) {
        THROW_EXCEPTION(IllegalStateException, "Kernel function has unexpected "
                        "number of input parameters");
    }
    list<CVariable> new_parameters;
    CDataType input_data_type = *old_parameters.front()->getDataType();
    // Output data type is the data type of the second parameter of the kernel
    CDataType output_data_type = *(*++old_parameters.begin())->getDataType();
    new_parameters.push_back(CVariable(input_param_name, input_data_type));
    new_parameters.push_back(CVariable(output_param_name, output_data_type));

    // Create body
    string new_body = string("{\n");
    CVariable device_input_variable(device_input_variable_name,
                                    input_data_type);
    device_input_variable.getDataType()->setIsConst(false);
    CVariable device_output_variable(device_output_variable_name,
                                     output_data_type);

    // Generate code for declaring variables on the device
    size_t input_data_size = input_data_type.getArraySize();
    size_t output_data_size = output_data_type.getArraySize();
    new_body += kIndents
        + device_input_variable.getPointerDeclarationString() + ";\n";
    new_body += kIndents
        + device_output_variable.getPointerDeclarationString() + ";\n";
    new_body += kIndents + "struct cudaDeviceProp prop;\n"
        + kIndents + "int max_threads_per_block;\n"
        + kIndents + "int shared_memory_per_sm;\n"
        + kIndents + "int num_multicores;\n"
        + kIndents + "int full_utilization_thread_count;\n"
        + kIndents + "int is_timeout_activated;\n\n";

    // Generate code for fetching the device information
    new_body += kIndents + "// Get GPGPU device information\n"
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
    new_body += kIndents + "if (" + tools::toString(num_leafs)
        + " < full_utilization_thread_count) {\n"
        + kIndents + kIndents + "// @todo Use some other way of reporting this "
        + "to the user (printf may not always be acceptable)\n"
        + kIndents + kIndents + "printf(\"WARNING: The input data is too small "
        + "to achieve full utilization of this device!\\n\");\n"
        + kIndents + "}\n\n";

    // Generate code for preparing the device and transferring input data
    new_body += kIndents + "// Prepare device and transfer input data\n"
        + kIndents + "// @todo Better error handling\n"
        + kIndents + "if (cudaMalloc((void**) &"
        + device_input_variable.getReferenceString() + ", "
        + tools::toString(input_data_size) + " * sizeof("
        + CDataType::typeToString(input_data_type.getType()) + ")) "
        + "!= cudaSuccess) {\n"
        + kIndents + kIndents + "printf(\"ERROR: Failed to allocate GPU "
        + "memory\\n\");\n"
        + kIndents + kIndents + "exit(-1);\n"
        + kIndents + "}\n";
    new_body += kIndents + "if (cudaMalloc((void**) &"
        + device_output_variable.getReferenceString() + ", "
        + tools::toString(output_data_size) + " * sizeof("
        + CDataType::typeToString(output_data_type.getType()) + ")) "
        + "!= cudaSuccess) {\n"
        + kIndents + kIndents + "printf(\"ERROR: Failed to allocate GPU "
        + "memory\\n\");\n"
        + kIndents + kIndents + "exit(-1);\n"
        + kIndents + "}\n";
    new_body += kIndents + "if (cudaMemcpy((void*) "
        + device_input_variable.getReferenceString() + ", (void*) "
        + input_param_name
        + ", " + tools::toString(input_data_size) + " * sizeof("
        + CDataType::typeToString(input_data_type.getType())
        + "), cudaMemcpyHostToDevice) != cudaSuccess) {\n"
        + kIndents + kIndents + "printf(\"ERROR: Failed to copy data to "
        + "GPU\\n\");\n"
        + kIndents + kIndents + "exit(-1);\n"
        + kIndents + "}\n"
        + "\n";
    // Generate code for executing the kernel
    new_body += kIndents + "// Execute kernel\n"
        + kIndents + "if (is_timeout_activated) {\n"
        + kIndents + kIndents + "// Prevent the kernel from timing out by\n"
        + kIndents + kIndents + "// splitting up the work into smaller pieces\n"
        + kIndents + kIndents + "// through multiple kernel invokations\n"
        + kIndents + kIndents + "int num_threads_left_to_execute = "
        + tools::toString(num_leafs) + ";\n"
        + kIndents + kIndents + "int index_offset = 0;\n"
        + kIndents + kIndents + "while (num_threads_left_to_execute > 0) {\n";
    new_body += kIndents + kIndents + kIndents + "int num_executing_threads = "
        + "num_threads_left_to_execute < full_utilization_thread_count ? "
        + "num_threads_left_to_execute : full_utilization_thread_count;\n";
    new_body += kIndents + kIndents + kIndents + "struct KernelConfig config = "
        + "calculateBestKernelConfig(num_executing_threads, "
        + "max_threads_per_block, "
        + tools::toString(input_data_size / num_leafs) + " * sizeof("
        + CDataType::typeToString(input_data_type.getType())
        + "), shared_memory_per_sm);\n";
    new_body += kIndents + kIndents + kIndents + function->getName()
        + "<<<config.grid, config.threadBlock, config.sharedMemory>>>("
        + device_input_variable_name + ", " + device_output_variable_name
        + ", index_offset);\n";
    new_body += kIndents + kIndents + kIndents + "int num_executed_threads = "
        + "config.grid.x * config.threadBlock.x;\n"
        + kIndents + kIndents + kIndents + "num_threads_left_to_execute -= "
        + "num_executed_threads;\n"
        + kIndents + kIndents + kIndents + "index_offset += "
        + "num_executed_threads;\n";
    new_body += kIndents + kIndents + "}\n";
    new_body += kIndents + "}\n";
    new_body += kIndents + "else {\n";
    new_body += kIndents + kIndents + "struct KernelConfig config = "
        + "calculateBestKernelConfig(" + tools::toString(num_leafs)
        + ", max_threads_per_block, "
        + tools::toString(input_data_size / num_leafs) + " * sizeof("
        + CDataType::typeToString(input_data_type.getType())
        + "), shared_memory_per_sm);\n";
    new_body += kIndents + kIndents + function->getName()
        + "<<<config.grid, config.threadBlock, config.sharedMemory>>>("
        + device_input_variable_name + ", " + device_output_variable_name
        + ", 0);\n";
    new_body += kIndents + "}\n\n";

    // Generate code for transferring back the result and cleaning up
    new_body += kIndents + "// Transfer result back to host and clean up\n"
        + kIndents + "// @todo Better error handling\n"
        + kIndents + "if (cudaMemcpy((void*) "
        + output_param_name + ", (void*) "
        + device_output_variable.getReferenceString()
        + ", " + tools::toString(output_data_size) + " * sizeof("
        + CDataType::typeToString(device_output_variable.getDataType()
                                  ->getType())
        + "), cudaMemcpyDeviceToHost) != cudaSuccess) {\n"
        + kIndents + kIndents + "printf(\"ERROR: Failed to copy data from "
        + "GPU\\n\");\n"
        + kIndents + kIndents + "exit(-1);\n"
        + kIndents + "}\n";
    new_body += kIndents + "if (cudaFree((void*) "
        + device_input_variable.getReferenceString() + ") != cudaSuccess) {\n"
        + kIndents + kIndents + "printf(\"ERROR: Failed to free GPU "
        + "memory\\n\");\n"
        + kIndents + kIndents + "exit(-1);\n"
        + kIndents + "}\n";
    new_body += kIndents + "if (cudaFree((void*) "
        + device_output_variable.getReferenceString() + ") != cudaSuccess) {\n"
        + kIndents + kIndents + "printf(\"ERROR: Failed to free GPU "
        + "memory\\n\");\n"
        + kIndents + kIndents + "exit(-1);\n"
        + kIndents + "}\n";
    new_body += "}";

    return CFunction(new_name, new_return_type, new_parameters, new_body);
}

void Synthesizer::generateParallelMapSyWrapperFunctions()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    for (list<Id>::iterator it = schedule_.begin(); it != schedule_.end();
         ++it) {
        Leaf* current_leaf = processnetwork_->getProcess(*it);
        if (!current_leaf) {
            THROW_EXCEPTION(IllegalStateException, string("Leaf \"") +
                            it->getString() + "\" not found");
        }
        logger_.logMessage(Logger::DEBUG, string("Analyzing leaf \"")
                           + current_leaf->getId()->getString() + "\"...");

        ParallelMap* parmapsy = dynamic_cast<ParallelMap*>(current_leaf);
        if (parmapsy) {
            try {
                CFunction wrapper_function =
                    generateParallelMapSyWrapperFunction(
                        parmapsy->getFunctions().front(), 
                        parmapsy->getNumProcesses());
                wrapper_function.setName(getGlobalLeafFunctionName(
                                             *parmapsy->getId(),
                                             wrapper_function.getName()));
                parmapsy->insertFunctionFirst(wrapper_function);
            }
            catch (InvalidProcessNetworkException& ex) {
                THROW_EXCEPTION(InvalidProcessNetworkException, string("Error in ")
                                + "leaf \"" + parmapsy->getId()->getString() 
                                + "\": " + ex.getMessage());
            }
        }
    }
}

CFunction Synthesizer::generateParallelMapSyWrapperFunction(CFunction* function,
    size_t num_leafs)
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    string new_name("parallel_wrapper");
    string input_param_name("input");
    string output_param_name("output");
    CDataType new_return_type(CDataType::VOID, false, false, 0, false, false);
    list<CVariable*> old_parameters = function->getInputParameters();
    CDataType old_input_param_data_type = *old_parameters.front()
        ->getDataType();

    // Create function parameters
    list<CVariable> new_parameters;
    if (old_parameters.size() == 1) {
        // Create input parameter
        CVariable new_input_param(input_param_name, old_input_param_data_type);
        if (old_input_param_data_type.isArray()) {
            if (!old_input_param_data_type.hasArraySize()) {
                THROW_EXCEPTION(InvalidProcessNetworkException, string("Data type of ")
                                + "first input parameter has no array size");
            }
            int input_data_size = num_leafs * old_input_param_data_type
                .getArraySize();
            new_input_param.getDataType()->setArraySize(input_data_size);
        }
        else {
            new_input_param.getDataType()->setIsConst(true);
            new_input_param.getDataType()->setIsArray(true);
            new_input_param.getDataType()->setArraySize(num_leafs);
        }

        // Create output parameter
        CVariable new_output_param(output_param_name,
                                   *function->getReturnDataType());
        new_output_param.getDataType()->setIsArray(true);
        new_output_param.getDataType()->setArraySize(num_leafs);
        
        new_parameters.push_back(new_input_param);
        new_parameters.push_back(new_output_param);
    }
    else if (old_parameters.size() == 2) {
        // Create input parameter
        CVariable new_input_param(input_param_name, old_input_param_data_type);
        if (!old_input_param_data_type.hasArraySize()) {
            THROW_EXCEPTION(InvalidProcessNetworkException, string("Data type of ")
                            + "first input parameter has no array size");
        }
        int input_data_size = num_leafs * old_input_param_data_type
            .getArraySize();
        new_input_param.getDataType()->setArraySize(input_data_size);

        // Create output parameter
        CDataType old_output_param_data_type = *old_parameters.back()
            ->getDataType();
        CVariable new_output_param(output_param_name,
                                   old_output_param_data_type);
        if (!old_output_param_data_type.hasArraySize()) {
            THROW_EXCEPTION(InvalidProcessNetworkException, string("Data type of ")
                            + "second input parameter has no array size");
        }
        int output_data_size = num_leafs * old_output_param_data_type
            .getArraySize();
        new_output_param.getDataType()->setArraySize(output_data_size);
        
        new_parameters.push_back(new_input_param);
        new_parameters.push_back(new_output_param);
    }
    else {
        THROW_EXCEPTION(IllegalStateException, "Function has unexpected "
                        "number of input parameters");
    }

    // Create body
    string new_body = string("{\n");
    new_body += kIndents + "int i;\n"
        + kIndents + "for (i = 0; i < " + tools::toString(num_leafs)
        + "; ++i) {\n";
    if (old_parameters.size() == 1) {
        new_body += kIndents + kIndents + output_param_name + "[i] = "
            + function->getName() + "(";
        if (old_input_param_data_type.getArraySize() > 0) {
            new_body += "&" + input_param_name + "[i * "
                + tools::toString(old_input_param_data_type.getArraySize())
                + "]";
        }
        else {
            new_body += input_param_name + "[i]";
        }
        new_body += ");\n";
    }
    else {
        new_body += kIndents + kIndents + function->getName() + "(";
        if (old_input_param_data_type.getArraySize() > 0) {
            new_body += "&" + input_param_name + "[i * "
                + tools::toString(old_input_param_data_type.getArraySize())
                + "]";
        }
        else {
            new_body += input_param_name + "[i]";
        }
        CDataType old_output_param_data_type =
            *old_parameters.back()->getDataType();
        new_body += ", &" + output_param_name + "[i * "
                + tools::toString(old_output_param_data_type.getArraySize())
                + "]);\n";
    }
    new_body += kIndents + "}\n"
        + "}";

    return CFunction(new_name, new_return_type, new_parameters, new_body);
}

string Synthesizer::generateVariableCopyingCode(CVariable to, CVariable from,
                                                bool do_deep_copy) 
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
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

string Synthesizer::generateVariableCopyingCode(CVariable to,
                                                list<CVariable>& from) 
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
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
    catch (InvalidProcessNetworkException& ex) {
        THROW_EXCEPTION(InvalidProcessNetworkException, string("Error between ")
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

string Synthesizer::generateVariableCopyingCode(list<CVariable>& to,
                                                CVariable from)
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
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
    catch (InvalidProcessNetworkException& ex) {
        THROW_EXCEPTION(InvalidProcessNetworkException, string("Error between ")
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

string Synthesizer::generateLeafFunctionExecutionCode(
    CFunction* function, list<CVariable> inputs, CVariable output)
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    ensureVariableIsNotConst(output);

    string code;

    // Add function call
    if (function->getNumInputParameters() == inputs.size()) {
        CVariable function_return("return", *function->getReturnDataType());
        try {
            ensureVariableDataTypeCompatibilities(output, function_return);
            ensureVariableArrayCompatibilities(output, function_return);
        }
        catch (InvalidProcessNetworkException& ex) {
            THROW_EXCEPTION(InvalidProcessNetworkException, string("Error in function, ")
                            + "return value: " + ex.getMessage());
        }

        code += kIndents + output.getReferenceString() + " = "
            + function->getName() + "(";
    }
    else if (function->getNumInputParameters() == inputs.size() + 1) {
        CVariable function_output = *function->getInputParameters().back();
        try {
            ensureVariableDataTypeCompatibilities(function_output, output);
            ensureVariableArrayCompatibilities(function_output, output);
        }
        catch (InvalidProcessNetworkException& ex) {
            THROW_EXCEPTION(InvalidProcessNetworkException, string("Error in function, ")
                            + "last parameter: " + ex.getMessage());
        }

        code += kIndents + function->getName() + "(";
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

void Synthesizer::ensureVariableIsNotConst(CVariable variable)
    throw(InvalidProcessNetworkException) {
    if (variable.getDataType()->isConst()) {
        THROW_EXCEPTION(InvalidProcessNetworkException, string("Variable \"") +
                        variable.getReferenceString() + "\" is a const");
    }
}

void Synthesizer::ensureVariableDataTypeCompatibilities(CVariable lhs,
                                                        CVariable rhs)
    throw(InvalidProcessNetworkException) {
    CDataType lhs_data_type = *lhs.getDataType();
    CDataType rhs_data_type = *rhs.getDataType();
    if (lhs_data_type.getType() != rhs_data_type.getType()) {
            THROW_EXCEPTION(InvalidProcessNetworkException, string("Error between ")
                            + "variables " + rhs.getReferenceString()
                            + " and " + lhs.getReferenceString() + ": "
                            + "mismatched data types (from "
                            + CDataType::typeToString(rhs_data_type.getType())
                            + " to "
                            + CDataType::typeToString(lhs_data_type.getType())
                            + ")");
    }
}

void Synthesizer::ensureVariableIsArray(CVariable variable)
    throw(InvalidProcessNetworkException) {
    if (!variable.getDataType()->isArray()) {
        THROW_EXCEPTION(InvalidProcessNetworkException, string("Variable \"") +
                        variable.getReferenceString() + "\" is not an array");
    }    
}

void Synthesizer::ensureArraySizes(size_t lhs, size_t rhs)
    throw(InvalidProcessNetworkException) {
    if (lhs != rhs) {
        THROW_EXCEPTION(InvalidProcessNetworkException, string("Mismatched array ")
                        + "sizes (from size " + tools::toString(rhs)
                        + " to size " + tools::toString(lhs) + ")");
    }
}

void Synthesizer::ensureVariableArrayCompatibilities(CVariable lhs,
                                                     CVariable rhs)
    throw(InvalidProcessNetworkException) {
    CDataType lhs_data_type = *lhs.getDataType();
    CDataType rhs_data_type = *rhs.getDataType();
    if (lhs_data_type.isArray()) {
        if (!rhs_data_type.isArray()) {
            THROW_EXCEPTION(InvalidProcessNetworkException, string("Error between ")
                            + "variables " + rhs.getReferenceString()
                            + " and " + lhs.getReferenceString() + ": "
                            + "mismatched data types (from scalar to array)");
        }
        if (!lhs_data_type.hasArraySize()) {
            THROW_EXCEPTION(InvalidProcessNetworkException, string("Variable \"") +
                            lhs.getReferenceString() + "\" has no array "
                            + "size");
        }
        if (!rhs_data_type.hasArraySize()) {
            THROW_EXCEPTION(InvalidProcessNetworkException, string("Variable \"") +
                            rhs.getReferenceString() + "\" has no array "
                            + "size");
        }
        try {
            ensureArraySizes(lhs_data_type.getArraySize(),
                             rhs_data_type.getArraySize());
        }
        catch (InvalidProcessNetworkException& ex) {
            THROW_EXCEPTION(InvalidProcessNetworkException, string("Error between ")
                            + "variables " + rhs.getReferenceString()
                            + " and " + lhs.getReferenceString() + ": "
                            + ex.getMessage());
        }
    }
    else {
        if (rhs_data_type.isArray()) {
            THROW_EXCEPTION(InvalidProcessNetworkException, string("Error between ")
                            + "variables " + rhs.getReferenceString()
                            + " and " + lhs.getReferenceString() + ": "
                            + "mismatched data types (from array to scalar)");
        }
    }
}

string Synthesizer::generateKernelConfigStructDefinitionCode()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    string code;
    code += string("/**\n")
        + " * C struct for returning the calculated kernel configuration for \n"
        + " * best performance.\n"
        + " */\n";
    code += string("struct KernelConfig {\n")
        + kIndents + "dim3 grid;\n"
        + kIndents + "dim3 threadBlock;\n"
        + kIndents + "size_t sharedMemory;\n"
        + "};\n";
    return code;
}

string Synthesizer::generateKernelConfigFunctionDefinitionCode()
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    string code;
    code += string("/**\n")
        + " * Calculate the best kernel configuration of grid and thread\n"
        + " * blocks for best performance. The aim is to maximize the number\n"
        + " * of threads available for each CUDA multi-leafor.\n"
        + " *\n"
        + " * When no shared memory is used:\n"
        + " * The configuration is calculated by using the maximum number of\n"
        + " * threads per thread block, and then the grid is set to the\n"
        + " * lowest number of thread blocks which will accommodate the\n"
        + " * desired thread count.\n"
        + " * \n"
        + " * When shared memory is used:\n"
        + " * The configuration is calculated by starting with as large a\n"
        + " * thread block as possible. If the thread block uses too much\n"
        + " * shared memory, the size is decreased until it does fit. If \n"
        + " * the shared memory is not optimally used, the thread block\n"
        + " * continues until either all shared memory is used optimally or\n"
        + " * until the shared memory can fit more than 8 thread blocks\n"
        + " * (there is no point in going further since no more than 8 thread\n"
        + " * blocks can be scheduled on an SM). If no optimal configuration\n"
        + " * has been found, the best one is selected.\n"
        + " *\n"
        + " * @param num_threads\n"
        + " *        Number of threads to execute in the kernel invocation.\n"
        + " * @param max_threads_per_block\n"
        + " *        Maximum number of threads per block on this device.\n"
        + " * @param shared_memory_used_per_thread\n"
        + " *        Amount of shared memory used per thread.\n"
        + " * @param shared_memory_per_sm\n"
        + " *        Amount of shared memory available per streaming \n"
        + " *        multi-leafor.\n"
        + " */\n";
    code += string("struct KernelConfig calculateBestKernelConfig(")
        + "int num_threads, int max_threads_per_block, "
        + "int shared_memory_used_per_thread, "
        + "int shared_memory_per_sm) {\n";
    if (config_.useSharedMemoryForInput()) {
        code += string()
            + kIndents + "int threads_per_block_best;\n"
            + kIndents + "int unused_shared_memory_best = "
            + "shared_memory_per_sm;\n"
            + kIndents + "for (int threads_per_block = max_threads_per_block; "
            + "; --threads_per_block) {\n"
            + kIndents + kIndents + "int num_blocks_per_sm = "
            + "shared_memory_per_sm"
            + " / (threads_per_block * shared_memory_used_per_thread);\n"
            + kIndents + kIndents + "if (num_blocks_per_sm == 0) continue;\n"
            + kIndents + kIndents + "int total_shared_memory_used = "
            + "num_blocks_per_sm * threads_per_block"
            + " * shared_memory_used_per_thread;\n"
            + kIndents + kIndents + "int unused_shared_memory = "
            + "shared_memory_per_sm - total_shared_memory_used;\n"
            + kIndents + kIndents + "if (unused_shared_memory"
            + " < unused_shared_memory_best) {\n"
            + kIndents + kIndents + kIndents + "threads_per_block_best = "
            + "threads_per_block;\n"
            + kIndents + kIndents + kIndents + "unused_shared_memory_best = "
            + "unused_shared_memory;\n"
            + kIndents + kIndents + "}\n"
            + kIndents + kIndents + "// Stop if this is optimal or as good as "
            "it gets\n"
            + kIndents + kIndents + "if (unused_shared_memory == 0 "
            "|| num_blocks_per_sm > 8) break;\n"
            + kIndents + "}\n"
            + "\n"
            + kIndents + "int num_blocks = (num_threads + "
            + "threads_per_block_best - 1) / threads_per_block_best;\n"
            + kIndents + "struct KernelConfig config;\n"
            + kIndents + "config.grid = dim3(num_blocks, 1);\n"
            + kIndents + "config.threadBlock = dim3(threads_per_block_best, "
            + "1);\n"
            + kIndents + "config.sharedMemory = "
            + "threads_per_block_best * shared_memory_used_per_thread;\n"
            + kIndents + "return config;\n";
    }
    else {
        code += kIndents + "int num_blocks = "
            + "(num_threads + max_threads_per_block - 1)"
            + " / max_threads_per_block;\n"
            + kIndents + "struct KernelConfig config;\n"
            + kIndents + "config.grid = dim3(num_blocks, 1);\n"
            + kIndents + "config.threadBlock = "
            + "dim3(max_threads_per_block, 1);\n"
            + kIndents + "config.sharedMemory = 0;\n"
            + kIndents + "return config;\n";
    }
    code += "}\n";
    return code;
}

string Synthesizer::getGlobalLeafFunctionName(
    ForSyDe::Id leaf_id, const string& function_name) const throw() {
    return string("f") + leaf_id.getString() + "_" + function_name;
}

bool Synthesizer::dynamicallyAllocateMemoryForSignalVariable(Signal* signal) {
    // If the signal has an in and out port, then the signal is not written to
    // from any processnetwork input parameter nor read from for the processnetwork output
    // parameters
    return signal->getOutPort() && signal->getInPort()
        && signal->getVariable().getDataType()->isArray();
}

string Synthesizer::generateLeafExecutionCodeFordelayStep1(
    delay* leaf)
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    CVariable output =
        getSignalByOutPort(leaf->getOutPorts().front())->getVariable();
    CVariable delay_variable = getDelayVariable(leaf).first;
    return generateVariableCopyingCode(output, delay_variable);
}

string Synthesizer::generateLeafExecutionCodeFordelayStep2(
    delay* leaf)
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    CVariable input = 
        getSignalByInPort(leaf->getInPorts().front())->getVariable();
    CVariable delay_variable = getDelayVariable(leaf).first;
    return generateVariableCopyingCode(delay_variable, input);
}

string Synthesizer::generateLeafExecutionCodeForMap(Map* leaf)
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
    list<CVariable> inputs;
    inputs.push_back(getSignalByInPort(leaf->getInPorts().front())
                     ->getVariable());
    CVariable output =
        getSignalByOutPort(leaf->getOutPorts().front())->getVariable();
    CFunction* function = leaf->getFunction();
    return generateLeafFunctionExecutionCode(function, inputs, output);
}

string Synthesizer::generateLeafExecutionCodeForZipWithNSY(
    ZipWithNSY* leaf)
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
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

string Synthesizer::generateLeafExecutionCodeForunzipx(unzipx* leaf)
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
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

string Synthesizer::generateLeafExecutionCodeForzipx(zipx* leaf)
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
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

string Synthesizer::generateLeafExecutionCodeForfanout(fanout* leaf)
    throw(InvalidProcessNetworkException, IOException, RuntimeException) {
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

Synthesizer::Signal::Signal(Leaf::Port* out_port, Leaf::Port* in_port)
        throw(InvalidArgumentException)
        : out_port_(out_port), in_port_(in_port), has_data_type_(false) {
    if (!out_port_ && !in_port_) {
        THROW_EXCEPTION(InvalidArgumentException, "Both ports cannot be NULL");
    }
}

Synthesizer::Signal::~Signal() throw() {}

bool Synthesizer::Signal::hasDataType() const throw() {
    return has_data_type_;
}

CDataType* Synthesizer::Signal::getDataType() throw(IllegalStateException) {
    return &data_type_;
}

void Synthesizer::Signal::setDataType(const CDataType& type) throw() {
    has_data_type_ = true;
    data_type_ = type;
}

string Synthesizer::Signal::getVariableName() const throw() {
    string name("v");
    if (out_port_) {
        name += out_port_->getProcess()->getId()->getString();
        name += "_";
        name += out_port_->getId()->getString();
    }
    else {
        name += "processnetwork_input";
    }
    name += "_to_";
    if (in_port_) {
        name += in_port_->getProcess()->getId()->getString();
        name += "_";
        name += in_port_->getId()->getString();
    }
    else {
        name += "processnetwork_output";
    }
    return name;
}

bool Synthesizer::Signal::operator==(const Signal& rhs) const throw() {
    return out_port_ == rhs.out_port_ && in_port_ == rhs.in_port_;
}

bool Synthesizer::Signal::operator!=(const Signal& rhs) const throw() {
    return !operator==(rhs);
}

bool Synthesizer::Signal::operator<(const Signal& rhs) const throw() {
    return toString().compare(rhs.toString()) < 0;
}

string Synthesizer::Signal::toString() const throw() {
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

CVariable Synthesizer::Signal::getVariable() const
    throw(IllegalStateException) {
    if (!has_data_type_) THROW_EXCEPTION(IllegalStateException,
                                         string("Signal ") + toString()
                                         + " has no data type");
    return CVariable(getVariableName(), data_type_);
}

Leaf::Port* Synthesizer::Signal::getOutPort() const throw() {
    return out_port_;
}

Leaf::Port* Synthesizer::Signal::getInPort() const throw() {
    return in_port_;
}

bool Synthesizer::SignalComparator::operator() (const Signal* lhs,
                                                const Signal* rhs) const
    throw() {
    return *lhs < *rhs;
}
