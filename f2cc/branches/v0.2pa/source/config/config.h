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

#ifndef F2CC_SOURCE_CONFIG_CONFIG_H_
#define F2CC_SOURCE_CONFIG_CONFIG_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines a class for containing program settings.
 */

#include "../logger/logger.h"
#include "../exceptions/invalidformatexception.h"
#include "../exceptions/invalidargumentexception.h"
#include <string>

namespace f2cc {

/**
 * @brief Defines a class for containing program settings.
 *
 * The \c Config class provides methods for accessing the program-related
 * settings. The settings are usually given through the command-line and there
 * is a special constructor for parsing the command-line into a \c Config
 * object.
 */
class Config {
  public:
    /**
     * Denotes the target platform to which the ForSyDe processnetwork is to be
     * synthesized.
     */
    enum TargetPlatform {
        /**
         * Sequential C code, where no leafs are executed in parallel.
         */
        C,

        /**
         * Sequential C code annotaded with CUDA directives, where data parallel
         * leafs are executed in parallel.
         */
        CUDA
    };

    /**
     * Denotes the input file format that will determine the execution path. \c .graphml
     * files will follow the v0.1 execution path while \c .xml files will follow v0.2
     */
    enum InputFormat {
        /**
         * New ForSyDe-SystemC intermediate XML representation.
         */
        XML,

        /**
         * ForSyDe-Haskell intermediate GraphML representation.
         */
        GraphML
    };

    struct Costs {
        /**
         * Contains the code for the header file.
         */
        int k_H2D;

        int k_D2H;

        int k_D2D;

        int k_H2H;

        unsigned min_parallel;
    };

    /**
     * Creates a configuration with default settings.
     */
    Config() throw();

    /**
     * Creates a configuration using settings from the command
     * line. Non-specified settings use default values.
     *
     * @param argc
     *        Number of arguments.
     * @param argv
     *        Array of char arrays.
     * @throws InvalidArgumentException
     *         When \c argc is less than 1 or when \c argv is NULL.
     * @throws InvalidFormatException
     *         When the command line contains errors.
     */
    Config(int argc, const char** argv)
        throw(InvalidArgumentException, InvalidFormatException);

    /**
     * Destroys this configuration.
     */
    ~Config() throw();

    /**
     * Gets whether the user requested the help menu be printed. Default value
     * is \b false.
     *
     * @returns \b true if the help menu should be printed.
     */
    bool doPrintHelpMenu() const throw();

    /**
     * Gets whether the user requested the version be printed. Default value is
     * \b false.
     *
     * @returns \b true if the help menu should be printed.
     */
    bool doPrintVersion() const throw();

    /**
     * Gets the help menu.
     *
     * @returns Help menu.
     */
    std::string getHelpMenu() const throw();

    /**
     * Gets the input file path. Default value is empty string.
     *
     * @returns Input file path.
     */
    std::string getInputFile() const throw();

    /**
     * Sets the input file path.
     *
     * @param file
     *        Input file path.
     */
    void setInputFile(std::string file) throw();

    /**
     * Gets the header output file path. Default value is the same name
     * as the input file but with a different extension.
     *
     * @returns Header output file path.
     */
    std::string getHeaderOutputFile() const throw();

    /**
     * Gets the implementation output file path. Default value is the same name
     * as the input file but with a different extension.
     *
     * @returns Implementation output file path.
     */
    std::string getImplementationOutputFile() const throw();

    /**
     * Sets the output file path.
     *
     * @param file
     *        Output file path.
     */
    void setOutputFile(std::string file) throw();

    /**
     * Gets the log file path. Default file path is "output.log".
     *
     * @returns Log file path
     */
    std::string getLogFile() const throw();

    /**
     * Sets the log file path
     *
     * @param file
     *        Log file path.
     */
    void setLogFile(std::string file) throw();

    /**
     * Gets the log level. Default level is Logger::INFO.
     *
     * @returns Log level.
     */
    Logger::LogLevel getLogLevel() const throw();

    /**
     * Sets the log level.
     *
     * @param level
     *        Log level.
     */
    void setLogLevel(Logger::LogLevel level) throw();

    /**
     * Gets whether data parallel leafs in the processnetwork should be coalesced.
     * Default setting is \b true.
     *
     * @returns \b true if such action should be performed.
     */
    bool doDataParallelLeafCoalesing() const throw();

    /**
     * Sets whether data parallel leafs in the processnetwork should be coalesced.
     *
     * @param setting
     *        New setting.
     */
    void setDoDataParallelLeafCoalesing(bool setting) throw();

    /**
     * Gets whether the shared memory on the device shall be used for input data
     * in the synthesized CUDA code. Default setting is \b false.
     *
     * @returns \b true if the shared memory is to be used.
     */
    bool useSharedMemoryForInput() const throw();

    /**
     * Sets whether the shared memory should be used for input data.
     *
     * @param setting
     *        New setting.
     */
    void setUseSharedMemoryForInput(bool setting) throw();

    /**
     * Same as useSharedMemoryForInput() but for output data.
     *
     * @returns \b true if the shared memory is to be used.
     */
    bool useSharedMemoryForOutput() const throw();

    /**
     * Same as setUseSharedMemoryForInput(bool) but for output data.
     *
     * @param setting
     *        New setting.
     */
    void setUseSharedMemoryForOutput(bool setting) throw();

    /**
     * Gets the target platform. Default platform is Config::CUDA.
     *
     * @returns Target platform.
     */
    TargetPlatform getTargetPlatform() const throw();

    /**
     * Sets the target platform.
     *
     * @param platform
     *        Target platform.
     */
    void setTargetPlatform(TargetPlatform platform) throw();

    /**
     * Gets the target platform. Default platform is Config::CUDA.
     *
     * @returns Target platform.
     */
    Costs getCosts() const throw();

    /**
     * Sets the target platform.
     *
     * @param platform
     *        Target platform.
     */
    void setCosts(const std::string& file) throw();

    /**
     * Gets the input file format, that determines the execution path.
     *
     * @returns Input format.
     */
    InputFormat getInputFormat() const throw();

    /**
     * Sets the input file format, that determines the execution path.
     *
     * @param format
     *        Input format.
     */
    void setTargetPlatform(InputFormat format) throw();

    /**
     * Parses the command line and sets its specified settings to this
     * configuration.
     *
     * @param argc
     *        Number of arguments.
     * @param argv
     *        Array of char arrays.
     * @throws InvalidArgumentException
     *         When \c argc is less than 1 or when \c argv is NULL.
     * @throws InvalidFormatException
     *         When the command line contains errors.
     */
    void setFromCommandLine(int argc, const char** argv)
        throw(InvalidArgumentException, InvalidFormatException);

    /**
     * Gets the program version.
     *
     * @returns Program version.
     */
    static std::string getVersion() throw();

    /**
     * Gets the SVN revision number.
     *
     * @returns SVN revision.
     */
    static std::string getSvnRevision() throw();

  private:
    /**
     * Sets all settings to default values.
     */
    void setDefaults() throw();

    /**
     * Checks whether a string is an option.
     * 
     * @param str
     *        String to check.
     * @returns \b true if the string starts with a '-'.
     */
    bool isOption(const std::string& str) const throw();

    /**
     * Checks whether a string is a composite option, which is of the format
     * "--<option>=<value>". A composite option is also an option
     * (see isOption(const std::string&)).
     * 
     * @param str
     *        String to check.
     * @returns \b true if the string starts with a '--'.
     * @see isOption(const std::string&)
     */
    bool isCompositeOption(const std::string& str) const throw();

  private:
    /**
     * Specifies whether the user requested that the help menu be printed or
     * not.
     */
    bool do_print_help_;

    /**
     * Specifies whether the user requested that the version be printed or not.
     */
    bool do_print_version_;

    /**
     * Specifies the input file path.
     */
    std::string input_file_;

    /**
     * Specifies the output file path.
     */
    std::string output_file_;

    /**
     * Specifies the log file path.
     */
    std::string log_file_;

    /**
     * Specifies the log level used in the logger.
     */
    Logger::LogLevel log_level_;

    /**
     * Specifies data parallel leaf coalescing setting.
     */
    bool do_data_parallel_leaf_coalescing_;

    /**
     * Specifies shared memory usage setting.
     */
    bool use_shared_memory_for_input_;

    /**
     * @copydoc ::use_shared_memory_for_input_
     */
    bool use_shared_memory_for_output_;

    /**
     * Specifies the target platform.
     */
    TargetPlatform target_platform_;

    /**
     * Specifies the input format, thus the execution path.
     */
    InputFormat format_;

    Costs costs_;
};

}

#endif
