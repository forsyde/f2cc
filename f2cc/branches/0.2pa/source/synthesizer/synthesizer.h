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

#ifndef F2CC_SOURCE_NTHESIZER_NTHESIZER_H_
#define F2CC_SOURCE_NTHESIZER_NTHESIZER_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines the \c Synthesizer class.
 */

#include "../logger/logger.h"
#include "../config/config.h"
#include "../forsyde/id.h"
#include "../forsyde/processnetwork.h"
#include "../forsyde/process.h"
#include "../forsyde/SY/delaysy.h"
#include "../forsyde/SY/combsy.h"
#include "../forsyde/SY/unzipxsy.h"
#include "../forsyde/SY/zipxsy.h"
#include "../forsyde/SY/fanoutsy.h"
#include "../forsyde/SY/combsy.h"
#include "../language/cfunction.h"
#include "../language/cvariable.h"
#include "../language/cdatatype.h"
#include "../exceptions/ioexception.h"
#include "../exceptions/outofmemoryexception.h"
#include "../exceptions/runtimeexception.h"
#include "../exceptions/invalidargumentexception.h"
#include "../exceptions/invalidmodelexception.h"
#include "../exceptions/illegalstateexception.h"
#include <string>
#include <list>
#include <set>
#include <map>
#include <stack>

namespace f2cc {

/**
 * @brief A class for synthesizing a ForSyDe processnetwork into executable code.
 *
 * The \c Synthesizer class provides methods for synthesizing a ForSyde processnetwork
 * into either sequential C or parallel CUDA C code. The executable processnetwork is
 * invoked as a function call, with its input as function parameters and its
 * returned as a function return value. The code is generated as a single source
 * file which can be compiled without modifications by standard C or CUDA C
 * compiler.
 */
class Synthesizer {
  public:
    /**
     * @brief Contains the code for the header and implementation file.
     *
     * The generated code is split into a header and an implementation file.
     * But as a function can only return a single value, this struct was devised
     * to be able to return both from the same function.
     */
    struct CodeSet {
        /**
         * Contains the code for the header file.
         */
        std::string header;

        /**
         * Contains the code for the implementation file.
         */
        std::string implementation;
    };

  private:
    /**
     * Indentation string.
     */
    static const std::string kIndents;

    /**
     * Prefix to use for the input parameters in the process network C function.
     */
    static const std::string kProcessnetworkInputParameterPrefix;

    /**
     * Prefix to use for the output parameters in the process network C function.
     */
    static const std::string kProcessnetworkOutputParameterPrefix;

    /**
     * Code target platforms.
     */
    enum TargetPlatform {
        C,
        CUDA
    };

    class Signal;

    /**
     * Class for comparing two signal pointers in an std::set.
     */
    class SignalComparator {
      public:
        /**
         * Compares one signal pointer with another. Both signals are
         * dereferenced before comparison.
         *
         * @param lhs
         *        First signal.
         * @param rhs
         *        First signal.
         * @returns \c true if \code *lhs < *rhs \endcode.
         */
        bool operator() (const Signal* lhs, const Signal* rhs) const throw();
    };

  public:
    /**
     * Creates a synthesizer.
     *
     * @param processnetwork
     *        ForSyDe processnetwork.
     * @param logger
     *        Reference to the logger object.
     * @param config
     *        Reference to the config object.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     */
    Synthesizer(ForSyDe::Processnetwork* processnetwork, Logger& logger, Config& config)
        throw(InvalidArgumentException);

    /**
     * Destroys this synthesizer.
     */
    ~Synthesizer() throw();

    /**
     * Generates sequential C code.
     *
     * @returns Generated code.
     * @throws InvalidProcessnetworkException
     *         When the process network is such that it cannot be synthesized.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When something goes wrong during the synthesis process.
     */
    CodeSet generateCCode()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates CUDA C code.
     *
     * @returns Generated code.
     * @throws InvalidProcessnetworkException
     *         When the process network is such that it cannot be synthesized.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When something goes wrong during the synthesis process.
     */
    CodeSet generateCudaCCode()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);
    
  private:
    /**
     * Checks that the process network is valid from the synthesizer's point of view.
     * Currently, this does nothing (i.e. all parsed processnetworks are valid processnetworks).
     *
     * @throws InvalidProcessnetworkException
     *         When the checks fail.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When something goes wrong during the synthesis process.
     */
    void checkProcessnetwork()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates code for the currently set target platform.
     *
     * @returns Generated code.
     * @throws InvalidProcessnetworkException
     *         When the process network is such that it cannot be synthesized.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When something goes wrong during the synthesis process.
     */
    CodeSet generateCode()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Finds a process schedule for the process network. 
     *
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    void findSchedule() throw(IOException, RuntimeException);

    /**
     * Registers a new signal. If no such signal is registred, it is registred
     * and the method returns the new signal (note that this is not the input
     * parameter as the signal is copied). If such a signal is already
     * registred, nothing is registred and the old signal is returned.
     *
     * @param signal
     *        New signal to register
     * @returns New signal if registred; otherwise old signal.
     * @throws InvalidArgumentException
     *         When \c signal is \c NULL.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    Signal* registerSignal(Signal* signal)
        throw(InvalidArgumentException, IOException, RuntimeException);

    /**
     * Gets the signal associated with a given out and in port. If no such
     * signal is found in the register, a new signal is registred.
     *
     * @param out_port
     *        Out port of one process.
     * @param in_port
     *        In port of another process.
     * @returns Registred signal.
     * @throws InvalidArgumentException
     *         When both \c out_port and \c in_port are \c NULL.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    Signal* getSignal(ForSyDe::Process::Port* out_port,
                     ForSyDe::Process::Port* in_port)
        throw(InvalidArgumentException, IOException, RuntimeException);

    /**
     * Same as getSignal(const ForSyDe::Process::Port*, const
     * ForSyDe::Process::Port*) but only requires the out port. The method takes
     * care of finding the in port and invokes getSignal(const
     * ForSyDe::Process::Port*, const ForSyDe::Process::Port*) with the correct
     * parameters.
     *
     * @param out_port
     *        Out port of one process.
     * @returns Registred signal.
     * @throws InvalidArgumentException
     *         When \c out_port is \c NULL.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    Signal* getSignalByOutPort(ForSyDe::Process::Port* out_port)
        throw(InvalidArgumentException, IOException, RuntimeException);

    /**
     * Same as getSignal(const ForSyDe::Process::Port*, const
     * ForSyDe::Process::Port*) but only requires the in port. The method takes
     * care of finding the out port and invokes getSignal(const
     * ForSyDe::Process::Port*, const ForSyDe::Process::Port*) with the correct
     * parameters.
     *
     * @param in_port
     *        In port of one process.
     * @returns Registred signal.
     * @throws InvalidArgumentException
     *         When \c in_port is \c NULL.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    Signal* getSignalByInPort(ForSyDe::Process::Port* in_port)
        throw(InvalidArgumentException, IOException, RuntimeException);

    /**
     * Renames the functions of all comb processes present in the schedule to
     * avoid name clashes in the generated code. Also, C is a bit picky about
     * variable and function names (for instance, they must not start with a
     * number).
     *
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    void renamecombFunctions()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Combines functions between comb processes which are identical by
     * renaming the duplicates. Functions are compared using the \c == operator.
     *
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    void combineFunctionDuplicates()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Processes of type \c CoalescedMap may contain more than one process
     * function argument. In order to be able to generate correct code and still
     * treating them like any other \c comb process, wrapper functions need to
     * be created which invoke the other function arguments in subsequent order.
     * The wrapper function are then added to the \c CoalescedMap process
     * such that it is the function returned when calling comb::getFunction().
     *
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    void generateCoalescedSyWrapperFunctions()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates a wrapping function which invokes each function in a list,
     * passing the result from one to the next. See
     * generateCoalescedSyWrapperFunctions() for more information.
     *
     * @param functions
     *        List of functions.
     * @returns Wrapper function.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     * @see generateCoalescedWrapperFunctions()
     */
    CFunction generateCoalescedSyWrapperFunction(
        std::list<CFunction*> functions)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates CUDA kernel functions for \c ParallelMap processes. The
     * kernel function is added to the process as first function, which will
     * cause it to be retrieved when comb::getFunction() is invoked and thus
     * the process can be handled like any other \c comb process.
     *
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    void generateCudaKernelFunctions()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates a CUDA kernel function which in turn invokes a given function.
     * The resultant kernel function accepts 3 parameters:
     *    - The first parameter is the input parameter to the given function.
     *    - The second parameter is the output parameter to the given function.
     *    - The third parameter is an integer specifying the offset to add to
     *      the index when accessing the input and output arrays. This is needed
     *      as the computation may need to be split into multiple kernel
     *      invocations in order to avoid time out.
     * The kernel function expects that the thread blocks are configured in a
     * 1-dimensional setting along the X axis, and that each block size is 
     * configured as 1xN, where \em N is calculated from the size of the input
     * array for best performance.
     *
     * @param function
     *        Function to generate kernel function for.
     * @param num_processes
     *        Number of processes which the kernel function encompasses.
     * @returns Kernel function.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    CFunction generateCudaKernelFunction(CFunction* function,
                                         size_t num_processes)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates a wrapper function which invokes a CUDA kernel function with
     * appropriate grid and thread block configuration. The kernel function is
     * left intact.
     *
     * @param function
     *        Kernel function.
     * @param num_processes
     *        Number of processes which the kernel function encompasses.
     * @returns Wrapper function.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    CFunction generateCudaKernelWrapperFunction(CFunction* function,
                                                size_t num_processes)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates wrapper functions for \c ParallelMap processes. This is only
     * done when synthesizing C code. The wrapper function is added to the
     * process as first function, which will cause it to be retrieved when
     * comb::getFunction() is invoked and thus the process can be handled like
     * any other \c comb process.
     *
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    void generateParallelMapSyWrapperFunctions()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates a wrapper function which executes a given function for the
     * entire input array. The wrapping function is left intact.
     *
     * @param function
     *        Function to execute.
     * @param num_processes
     *        Number of processes which the function encompasses.
     * @returns Wrapper function.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    CFunction generateParallelMapSyWrapperFunction(CFunction* function,
                                                   size_t num_processes)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates code for the function definitions for the processes present
     * in the schedule.
     *
     * @returns Process function definitions code.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateProcessFunctionDefinitionsCode()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates code for the process network function definition, which implements the
     * schedule.
     *
     * Note that \c delay processes are executed in two steps. The first step
     * of all \c delay processes is executed before all other processes. Then,
     * the processes are executed in order as defined by the schedule but the \c
     * delay processes are ignored. Once the schedule has been executed, the
     * second step of all \c delay processes is executed. This must be done in
     * order to first propagate the values of the delay variables to the signal
     * variables, and then save the new values in the delay variables until the
     * next processnetwork invocation.
     *
     * @returns Processnetwork function definition code.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateProcessnetworkFunctionDefinitionCode()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates code for the process network function prototype. This is used for the
     * header file.
     *
     * @returns Processnetwork function prototype.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateProcessnetworkFunctionPrototypeCode()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates a method description (Java style) for the process network function.
     *
     * @returns Processnetwork function description.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateProcessnetworkFunctionDescription() 
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates code for copying the input parameter values of the process network
     * function to the appropriate signals. Input array parameters are ignored
     * (see generateArrayInputOutputsToSignalsAliasingCode()).
     *
     * @returns fanouting code.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateInputsToSignalsfanoutingCode()
        throw(InvalidProcessnetworkException, RuntimeException);

    /**
     * Generates code for copying the appropriate signal values to the output
     * parameters of the process network function. Signal array variables associated with
     * output parameters are ignored (see
     * generateArrayInputOutputsToSignalsAliasingCode()).
     *
     * @returns fanouting code.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateSignalsToOutputsfanoutingCode()
        throw(InvalidProcessnetworkException, RuntimeException);

    /**
     * Generates code which aliases the input and output array parameters with
     * the corresponding signal array variables. This reduces the amount of
     * memory copying needed.
     *
     * @returns fanouting code.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateArrayInputOutputsToSignalsAliasingCode()
        throw(InvalidProcessnetworkException, RuntimeException);

    /**
     * Generates code for declaring the signal variables. Non-array data types
     * will be allocated locally on the stack and arrays will be allocated on
     * the heap. Also, signal variables which receives its value from an input
     * array parameter, or will copy its value to an output array parameter,
     * will simply be declared but not be allocated any memory as its address
     * will be set to the address of the input array.
     *
     * @returns Variable declarations code.
     * @throws InvalidProcessnetworkException
     *         When a variable cannot be declared due to lacking information.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateSignalVariableDeclarationsCode()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates code for declaring the delay variables. A delay variable is
     * always declared as \c static as they need to retain their values between
     * processnetwork invocations. The variables will also be initialized with the
     * initial values specified in the process network.
     *
     * @returns Variable declarations code.
     * @throws InvalidProcessnetworkException
     *         When a variable cannot be declared due to lacking information.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generatedelayVariableDeclarationsCode()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Gets the corresponding delay variable and initial value for a process.
     *
     * @param process
     *        delay process.
     * @returns Pair where the first value is the variable and the second the
     *          initial value.
     * @throws InvalidArgumentException
     *         When \c process is \c NULL.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::pair<CVariable, std::string> getdelayVariable(
        ForSyDe::SY::delay* process)
        throw(InvalidArgumentException, RuntimeException);

    /**
     * Generates code which deletes dynamically allocated signal variables.
     * Signal variables which gets its input from input array parameters or copy
     * their value to output array parameters are \b not deleted as no memory is
     * allocated for them (they simply take the address of the array input
     * parameters).
     *
     * @returns Cleanup code.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateSignalVariableCleanupCode()
        throw(IOException, RuntimeException);

    /**
     * Generates code for the process network input parameters. Each parameter will have
     * prefix specified by Synthesizer::kProcessnetworkInputParameterPrefix_ or
     * Synthesizer::kProcessnetworkOutputParameterPrefix_, followed by an integer value.
     * All output parameters will be declared as pointers (except arrays, which
     * are already pointers).
     *
     * @returns Function parameter list code.
     * @throws InvalidProcessnetworkException
     *         When a variable cannot be declared due to lacking information.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateProcessnetworkFunctionParameterListCode()
        throw(InvalidProcessnetworkException, RuntimeException);

    /**
     * Creates all signals needed for the processes present in the
     * schedule. This is necessary in order be able to declare all variables at
     * the process network of the function definition in C. However, the data type of all
     * signals are \em not detected. The method also clears any previously
     * generated signals.
     *
     * @throws InvalidProcessnetworkException
     *         When a signal cannot be created due to lacking information.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    void createSignals()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Creates all delay variables needed for the delay processes present in the
     * schedule. This is necessary in order be able to declare all variables at
     * the process network of the function definition in C. The method also clears any
     * previously generated variables.
     *
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    void createdelayVariables() throw(IOException, RuntimeException);

    /**
     * Sets data types of array input signal variables as "const".  The
     * "constness" is removed by default when the signal is created, but for
     * input array signals the values are copied shallowly by simply reassigning
     * the address of the array pointer. However, to do that, the signal
     * variable needs to be set as "const" or the compiler will complain, and
     * simply throwing away the constness with a cast is ugly.
     *
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    void setInputArraySignalVariableDataTypesAsConst()
        throw(IOException, RuntimeException);

    /**
     * Attempts to discover and set the data types of all signals. If the data
     * type is an array, its size may still be unknown.
     *
     * @throws InvalidProcessnetworkException
     *         When a data type cannot be found for all signals.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    void discoverSignalDataTypes()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Attempts to discover and set the data type for a signal by doing
     * process-to-process search in the forward data flow direction. This means
     * the method only looks at the processes of the in ports. If the data type
     * is an array, its size may still be unknown.
     *
     * @param signal
     *        Signal for which to discover data type.
     * @returns Discovered data type.
     * @throws InvalidProcessnetworkException
     *         When a data type cannot be found for this signal.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    CDataType discoverSignalDataTypeForwardSearch(Signal* signal)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Same as discoverSignalDataTypeForwardSearch(Signal&) but does backward
     * search.  This means the method only looks at the processes of the out
     * ports.
     *
     * @param signal
     *        Signal for which to discover data type.
     * @returns Discovered data type.
     * @throws InvalidProcessnetworkException
     *         When a data type cannot be found for this signal.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    CDataType discoverSignalDataTypeBackwardSearch(Signal* signal)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Propagates known array sizes between the signals.
     *
     * @throws InvalidProcessnetworkException
     *         When an array size cannot be propagated to all signals.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     * @todo Implement this method (currently it does nothing).
     */
    void propagateArraySizesBetweenSignals()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Searches for the array size of a signal in the forward data flow search.
     *
     * @param signal
     *        Signal for which to discover array size.
     * @returns Discovered array size.
     * @throws InvalidProcessnetworkException
     *         When an array size cannot be found for this signal.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    size_t discoverSignalArraySizeForwardSearch(Signal* signal)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Same as discoverSignalArraySizeForwardSearch(Signal&) but does backward
     * search.
     *
     * @param signal
     *        Signal for which to discover array size.
     * @returns Discovered array size.
     * @throws InvalidProcessnetworkException
     *         When an array size cannot be found for this signal.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    size_t discoverSignalArraySizeBackwardSearch(Signal* signal)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Propagates the array sizes discovered for the signals to the process
     * functions.
     *
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    void propagateSignalArraySizesToProcessFunctions()
        throw(IOException, RuntimeException);

    /**
     * Generates code which execute the semantic meaning of a process. Executing
     * a \c delay process with this method has no effect (i.e. the process
     * is ignored).
     *
     * @param process
     *        Process to execute.
     * @returns Execution code.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateProcessExecutionCode(ForSyDe::Process* process)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates code for copying the content of one variable to another. Both
     * variables must be either non-arrays or array types. Scalar variables
     * which also are pointers will be dereferenced.
     *
     * @param to
     *        Destination variable.
     * @param from
     *        Source variable.
     * @param do_deep_copy
     *        If both variables are arrays, the copying can be done either
     *        shallowly or deeply. A shallow copy means only the destination's
     *        array address is assigned to the source's, meaning that the
     *        values are not truly copied but it would appear that way when
     *        using the destination variable afterwards. By default, all array
     *        variables copying is deep. Scalar variables are not affected by
     *        this parameter.
     * @returns fanouting code.
     * @throws InvalidProcessnetworkException
     *         When there is a data type or array size mismatch, or when the
     *         array size of either is unknown.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateVariablefanoutingCode(CVariable to, CVariable from, bool
        do_deep_copy = true) 
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates code for copying the contents of a list of non-array variables
     * to another variable of array type.
     *
     * @param to
     *        Destination variable.
     * @param from
     *        Source variables.
     * @returns fanouting code.
     * @throws InvalidProcessnetworkException
     *         When the \c to variable is not an array, or when its array size
     *         is unknown, or when there is a data type or array size mismatch.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateVariablefanoutingCode(CVariable to, 
                                            std::list<CVariable>& from)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates code for copying the content of a variable of array type to a
     * list of other non-array variables.
     *
     * @param to
     *        Destination variables.
     * @param from
     *        Source variable
     * @returns fanouting code.
     * @throws InvalidProcessnetworkException
     *         When the \c from variable is not an array, or when its array size
     *         is unknown, or when there is a data type or array size mismatch.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateVariablefanoutingCode(std::list<CVariable>& to,
                                            CVariable from)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates code for executing a process function.
     *
     * @param function
     *        Function to invoke.
     * @param inputs
     *        List of input variables.
     * @param output
     *        Destination variable.
     * @returns Execution code.
     * @throws InvalidProcessnetworkException
     *         When the function has unexpected number of input parameters, or
     *         when there is a data type or array size mismatch.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateProcessFunctionExecutionCode(CFunction* function,
                                                     std::list<CVariable>
                                                     inputs,
                                                     CVariable output)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Checks that a variable is not set as \c const. If it is, an exception is
     * thrown.
     *
     * @param variable
     *        Variable to check.
     * @throws InvalidProcessnetworkException
     *         When the check fails.
     */
    void ensureVariableIsNotConst(CVariable variable)
        throw(InvalidProcessnetworkException);

    /**
     * Checks that two variables are of the same types. If they are not, an
     * exception is thrown.
     *
     * @param lhs
     *        First variable (used on the left-hand side in an expression).
     * @param rhs
     *        Second variable  (used on the right-hand side in an expression).
     * @throws InvalidProcessnetworkException
     *         When the check fails.
     */
    void ensureVariableDataTypeCompatibilities(CVariable lhs, CVariable rhs)
        throw(InvalidProcessnetworkException);

    /**
     * Checks that a variable is an array. If it is not, an exception is thrown.
     *
     * @param variable
     *        Variable to check.
     * @throws InvalidProcessnetworkException
     *         When the check fails.
     */
    void ensureVariableIsArray(CVariable variable)
        throw(InvalidProcessnetworkException);

    /**
     * Checks that two array sizes are equal. If they are not, an exception is
     * thrown.
     *
     * @param lhs
     *        First size (used on the left-hand side in an expression).
     * @param rhs
     *        Second size  (used on the right-hand side in an expression).
     * @throws InvalidProcessnetworkException
     *         When the check fails.
     */
    void ensureArraySizes(size_t lhs, size_t rhs)
        throw(InvalidProcessnetworkException);

    /**
     * Checks that two variables are either both arrays or not arrays. If both
     * variables are arrays, then the method also checks that both array sizes
     * are equal. If any of the checks fails, an exception is thrown.
     *
     * @param lhs
     *        First variable (used on the left-hand side in an expression).
     * @param rhs
     *        Second variable  (used on the right-hand side in an expression).
     * @throws InvalidProcessnetworkException
     *         When a check fails.
     */
    void ensureVariableArrayCompatibilities(CVariable lhs, CVariable rhs)
        throw(InvalidProcessnetworkException);

    /**
     * Converts schedule into a string representation.
     *
     * @returns Schedule as string.
     */
    std::string scheduleToString() const throw();

    /**
     * Generates code for the kernel config struct definition. The kernel config
     * struct is used for calculating the best kernel configuration of grids and
     * blocks at runtime for optimal performance.
     *
     * @returns Struct definition code.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateKernelConfigStructDefinitionCode()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates code for the kernel config function definition. The kernel
     * config function calculates the best kernel configuration of grids and
     * blocks at runtime for optimal performance.
     *
     * @returns Function definition code.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateKernelConfigFunctionDefinitionCode()
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Gets a function name which can be used globally in the synthesized code
     * file. The format of the resultant name is
     * \c "f<process_id>_<function_name>".
     *
     * @param process_id
     *        Process ID.
     * @param function_name
     *        Name of the function.
     * @returns Global function name.
     */
    std::string getGlobalProcessFunctionName(ForSyDe::Id process_id,
                                             const std::string& function_name)
        const throw();

    /**
     * Checks whether to allocate dynamic memory for the signal variable.
     * 
     * @param signal
     *        Signal whose variable to check.
     * @returns \c true if the data type is an array and it is not written to by
     *          the process network input parameters or read from for the process network output
     *          parameters.
     */
    bool dynamicallyAllocateMemoryForSignalVariable(Signal* signal);

    /**
     * Generates code which execute the first step of given \c delay
     * process. The generated code copies the value from the delay variable to
     * the out signal.
     *
     * @param process
     *        Process to execute.
     * @returns Execution code.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateProcessExecutionCodeFordelayStep1(
        ForSyDe::SY::delay* process)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates code which execute the second step of given \c delay
     * process. The generated code copies the value from the in signal to the
     * delay variable.
     *
     * @param process
     *        Process to execute.
     * @returns Execution code.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateProcessExecutionCodeFordelayStep2(
        ForSyDe::SY::delay* process)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates code which execute a given \c comb process. The generated code
     * uses the process' in signal as input parameter to its function argument,
     * and then writes the result to its out signal.
     *
     * @param process
     *        Process to execute.
     * @returns Execution code.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateProcessExecutionCodeForcomb(
        ForSyDe::SY::comb* process)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates code which execute a given \c unzipx process. The generated
     * code copies each value from its in signal (which is expected to be an
     * array) to all of its out signals. The index of the out port list is used
     * to decide which value it will receive from the input array.
     *
     * @param process
     *        Process to execute.
     * @returns Execution code.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateProcessExecutionCodeForunzipx(
        ForSyDe::SY::unzipx* process)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates code which execute a given \c zipx process. The generated
     * code copies all values from its in signals to its out signal, which is
     * expected to be an array. The index of the in port list is used to decide
     * where an in value ends up in the output array.
     *
     * @param process
     *        Process to execute.
     * @returns Execution code.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateProcessExecutionCodeForzipx(ForSyDe::SY::zipx* process)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

    /**
     * Generates code which execute a given \c fanout process. The generated
     * code copies the value from its in signal to all of its out signals.
     *
     * @param process
     *        Process to execute.
     * @returns Execution code.
     * @throws InvalidProcessnetworkException
     *         When something is wrong with the process network.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::string generateProcessExecutionCodeForfanout(ForSyDe::SY::fanout* process)
        throw(InvalidProcessnetworkException, IOException, RuntimeException);

  private:
    /**
     * ForSyDe processnetwork.
     */
    ForSyDe::Processnetwork* const processnetwork_;

    /**
     * Logger.
     */
    Logger& logger_;

    /**
     * Config.
     */
    Config& config_;

    /**
     * Process schedule.
     */
    std::list<ForSyDe::Id> schedule_;

    /**
     * Set of processnetwork signals.
     */
    std::set<Signal*, SignalComparator> signals_;

    /**
     * Specifies the code target platform.
     */
    TargetPlatform target_platform_;

    /**
     * combset of delay variables. The delay process is used as key, and the
     * value is a pair of a \c CVariable and its initial value.
     */
    std::map< ForSyDe::SY::delay*, std::pair<CVariable, std::string> >
    delay_variables_;

  private:
    /**
     * @brief Manages data storage between processes.
     *
     * The \c Signal class is used to manage the variables needed for
     * transferring data from one process to another. A signal consists of an in
     * port and out port from two separate processes. A signal copied from
     * another will produce the exact same results as the original signal for
     * whatever method invoked.
     */
    class Signal {
      public:
        /**
         * Creates a signal between two processes.
         *
         * @param out_port
         *        Out port of one process.
         * @param in_port
         *        In port of another process.
         * @throws InvalidArgumentException
         *         When \c out_port and \c in_port are \c NULL.
         */
        Signal(ForSyDe::Process::Port* out_port,
               ForSyDe::Process::Port* in_port)
            throw(InvalidArgumentException);
        
        /**
         * Destroys this signal.
         */
        ~Signal() throw();

        /**
         * Gets the variable of this signal. If the signal has not been set a
         * data type, the data type of the variable is of default value (usually
         * \c void).
         *
         * @returns Variable.
         * @throws IllegalStateException
         *         When the signal has no data type.
         */
        CVariable getVariable() const throw(IllegalStateException);

        /**
         * Checks whether this signal has a data type set.
         * 
         * @returns \c true if the does.
         */
        bool hasDataType() const throw();

        /**
         * Gets the data type of this signal.
         * 
         * @returns Data type.
         * @throws IllegalStateException
         *         When the signal has no data type.
         */
        CDataType* getDataType() throw(IllegalStateException);

        /**
         * Sets the data type for this signal.
         *
         * @param type
         *        Data type to set.
         */
        void setDataType(const CDataType& type) throw();

        /**
         * Gets the out port of this signal.
         *
         * @returns Out port, if any; otherwise \c NULL.
         */
        ForSyDe::Process::Port* getOutPort() const throw();

        /**
         * Gets the in port of this signal.
         *
         * @returns In port, if any; otherwise \c NULL.
         */
        ForSyDe::Process::Port* getInPort() const throw();

        /**
         * Checks equality between this signal and another
         *
         * @param rhs
         *        Other signal to compare with.
         * @returns \c true if both signals have the same out and in ports.
         */
        bool operator==(const Signal& rhs) const throw();

        /**
         * Checks inequality between this signal and another
         *
         * @param rhs
         *        Other signal to compare with.
         * @returns \c true if both signals have the different out or in ports.
         */
        bool operator!=(const Signal& rhs) const throw();

        /**
         * Checks if this signal is "less" than another signal. The comparison
         * is done by comparing the string representations of both signals.
         *
         * @param rhs
         *        Other signal to compare with.
         * @returns \c true if 
         */
        bool operator<(const Signal& rhs) const throw();

        /**
         * Converts this signal into a string representation.
         *
         * @returns String representation.
         */
        std::string toString() const throw();

      private:
        /**
         * Gets the variable name for this signal.
         *
         * @returns Variable name.
         */
        std::string getVariableName() const throw();

      private:
        /**
         * Out port of one signal.
         */
        ForSyDe::Process::Port* out_port_;

        /**
         * In port of another signal.
         */
        ForSyDe::Process::Port* in_port_;

        /**
         * Flag for checking if the signal has a data type set.
         */
        bool has_data_type_;

        /**
         * Data type.
         */
        CDataType data_type_;
    };
};

}

#endif
