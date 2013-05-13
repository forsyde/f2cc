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

#include "config.h"
#include "../tools/tools.h"
#include "../exceptions/invalidargumentexception.h"
#include <vector>

using namespace f2cc;
using std::string;
using std::vector;

Config::Config() throw() {
    setDefaults();
}

Config::Config(int argc, const char** argv) throw(InvalidArgumentException,
                                                  InvalidFormatException) {
    setDefaults();
    setFromCommandLine(argc, argv);
}

Config::~Config() throw() {}

bool Config::doPrintHelpMenu() const throw() {
    return do_print_help_;
}

string Config::getHelpMenu() const throw() {
    const size_t maximum_line_length = 80;
    string part;
    string str = string()
        + "Developers: Gabriel Hjort Blindell <ghb@kth.se>\n"
        + "            George Ungureanu <ugeorge@kth.se>\n"
        + "KTH - Royal Institute of Technology, Stockholm, Sweden\n"
        + "Copyright (c) 2011-2013\n"
        + "\n";

    part = "This tool is part of the ForSyDe framework for synthesizing "
        "programs processnetworked at a high level of abstraction into compilable C or "
        "CUDA C code. The synthesis leaf is semantic-preserving which means "
        "that a processnetwork which is proven to be correct will also yield correct C "
        "or CUDA C code.\n"
        "\n"
        "The tool expects the processnetwork to be represented as a GraphML file. "
        "Currently, the tool supports the following leafs:\n";
    tools::breakLongLines(part, maximum_line_length, 0);
    part += ""
        "   * Map\n"
        "   * ParallelMap\n"
        "   * ZipWithNSY\n"
        "   * Unzipx\n"
        "   * Zipx\n"
        "   * delay\n"
        "\n";
    str += part;

    part = "f2cc accepts the following command options:\n";
    str += part;

    size_t indents = 6;

    part = "   -o FILE, --output-file=FILE\n"
        "      Specifies the output files. Default file names are the same as "
        "the input file but with different file extensions."
        "\n\n";
    tools::breakLongLines(part, maximum_line_length, indents);
    str += part;

    part = "   -tp PLATFORM, --target-platform=PLATFORM\n"
        "      Specifies the target platform which will affect the kind of "
        "code generated. Valid options are C and CUDA."
        "\n\n";
    tools::breakLongLines(part, maximum_line_length, indents);
    str += part;

    part = "   -no-pc, --no-leaf-coalescing\n"
        "      CUDA ONLY. Specifies that the tool should not coalesce "
        "leafs, even "
        "when it is possible to do so for the given input processnetwork."
        "\n\n";
    tools::breakLongLines(part, maximum_line_length, indents);
    str += part;

    part = "   -use-sm-i, --use-shared-memory-for-input\n"
        "      CUDA ONLY. Specifies that the synthesized code should make use "
        "of shared memory for the input data."
        "\n\n";
    tools::breakLongLines(part, maximum_line_length, indents);
    str += part;

    part = "   -lf FILE, --log-file=FILE\n"
        "      Specifies the path to the log file. Default setting is "
        "output.log."
        "\n\n";
    tools::breakLongLines(part, maximum_line_length, indents);
    str += part;

    part = "   -ll LEVEL, --log-level=LEVEL\n"
        "      Specifies the log level. This affects how verbose the tool is "
        "in its logging and prompt output. Valid options are CRITICAL, ERROR, "
        "WARNING, INFO, and DEBUG. Default setting is INFO."
        "\n\n";
    tools::breakLongLines(part, maximum_line_length, indents);
    str += part;

    part = "   -v, --version\n"
        "      Prints the version."
        "\n\n";
    tools::breakLongLines(part, maximum_line_length, indents);
    str += part;

    part = "   -h, --help\n"
        "      Prints this menu."
        "\n\n";
    tools::breakLongLines(part, maximum_line_length, indents);
    str += part;

    part = "Report bugs to: <ghb@kth.se>\n";
    str += part;

    return str;
}

bool Config::doPrintVersion() const throw() {
    return do_print_version_;
}

string Config::getInputFile() const throw() {
    return input_file_;
}

void Config::setInputFile(string file) throw() {
    input_file_ = file;
}

string Config::getHeaderOutputFile() const throw() {
    return tools::getFileName(output_file_) + ".h";
}

string Config::getImplementationOutputFile() const throw() {
    return output_file_;
}

void Config::setOutputFile(string file) throw() {
    output_file_ = file;
}

string Config::getLogFile() const throw() {
    return log_file_;
}

void Config::setLogFile(string file) throw() {
    log_file_ = file;
}

Logger::LogLevel Config::getLogLevel() const throw() {
    return log_level_;
}

void Config::setLogLevel(Logger::LogLevel level) throw() {
    log_level_ = level;
}

void Config::setDefaults() throw() {
    do_print_help_ = false;
    do_print_version_ = false;
    log_file_ = "output.log";
    log_level_ = Logger::INFO;
    do_data_parallel_leaf_coalescing_ = true;
    use_shared_memory_for_input_ = false;
    use_shared_memory_for_output_ = false;
    target_platform_ = Config::CUDA;
    format_ = Config::XML;
}

void Config::setFromCommandLine(int argc, const char** argv)
    throw(InvalidArgumentException, InvalidFormatException) {
    if (!argv) {
        THROW_EXCEPTION(InvalidArgumentException, "\"argv\" must not be NULL");
    }

    try {
        bool found_input_file = false;
        bool is_output_file_set = false;
        for (int index = 1; index < argc; ++index) {
            string current_str = argv[index];
            bool isLast = index == argc - 1;
            if (!isOption(current_str) && isLast) {
                input_file_ = current_str;
                found_input_file = true;
            }
            else {
                if (!isOption(current_str)) {
                    THROW_EXCEPTION(InvalidFormatException, string("\"")
                                    + current_str + "\" is not an option");
                }

                // Extract option (and argument if it is composite) from string
                string option, argument;
                if (isCompositeOption(current_str)) {
                    vector<string> splitted = tools::split(current_str, '=');
                    if (splitted.size() > 2) {
                        THROW_EXCEPTION(InvalidFormatException,
                                        string("Multiple assignments ('=') on ")
                                        + "option \"" + splitted[0] + "\"");
                    }
                    option = splitted[0];
                    if (splitted.size() == 2) argument = splitted[1];
                }
                else {
                    option = current_str;
                }

                // Set boolean flags
                bool isNextStrOption = isLast ? false
                    : isOption(argv[index + 1]);
                bool has_argument;
                if (isCompositeOption(option)) {
                    has_argument = argument.size() > 0;
                }
                else {
                    has_argument = !isLast && !isNextStrOption;
                }

                // Analyze option
                if (option == "-h" || option == "--help") {
                    do_print_help_ = true;
                }
                else if (option == "-v" || option == "--version") {
                    do_print_version_ = true;
                }
                else if (option == "-ll" || option == "--log-level") {
                    if (has_argument) {
                        if (!isCompositeOption(option)) {
                            argument = argv[index + 1];
                            ++index;
                        }
                    }
                    else {
                        THROW_EXCEPTION(InvalidFormatException,
                                        "No log level argument");
                    }
            
                    try {
                        tools::toUpperCase(argument);
                        log_level_ = Logger::stringToLogLevel(argument);
                    }
                    catch (InvalidArgumentException& ex) {
                        THROW_EXCEPTION(InvalidFormatException,
                                        ex.getMessage());
                    }
                }
                else if (option == "-lf" || option == "--log-file") {
                    if (has_argument) {
                        if (!isCompositeOption(option)) {
                            argument = argv[index + 1];
                            ++index;
                        }
                    }
                    else {
                        THROW_EXCEPTION(InvalidFormatException,
                                        "No log file argument");
                    }

                    log_file_ = argument;
                }
                else if (option == "-tp" || option == "--target-platform") {
                    if (has_argument) {
                        if (!isCompositeOption(option)) {
                            argument = argv[index + 1];
                            ++index;
                        }
                    }
                    else {
                        THROW_EXCEPTION(InvalidFormatException,
                                        "No target platform argument");
                    }
            
                    tools::toLowerCase(argument);
                    if (argument == "c") {
                        target_platform_ = Config::C;
                    }
                    else if (argument == "cuda") {
                        target_platform_ = Config::CUDA;
                    }
                    else {
                        THROW_EXCEPTION(InvalidFormatException,
                                        "Invalid target platform argument");
                    }
                }
                else if (option == "-o" || option == "--output-file") {
                    if (has_argument) {
                        if (!isCompositeOption(option)) {
                            argument = argv[index + 1];
                            ++index;
                        }
                    }
                    else {
                        THROW_EXCEPTION(InvalidFormatException,
                                        "No output file argument");
                    }

                    output_file_ = argument;
                    is_output_file_set = true;
                }
                else if (option == "-no-pc" 
                         || option == "--no-leaf-coalescing") {
                    do_data_parallel_leaf_coalescing_ = false;
                }
                else if (option == "-use-sm-i" 
                         || option == "--use-shared-memory-for-input") {
                    use_shared_memory_for_input_ = true;
                }
                /*
                 // Usage of shared memory for output data is not yet supported
                else if (option == "-use-sm-o" 
                         || option == "--use-shared-memory-for-output") {
                    use_shared_memory_for_output_ = true;
                }
                */
                else {
                    THROW_EXCEPTION(InvalidFormatException,
                                    string("Unknown command option \"") + option
                                    + "\"");
                }
            }
        }
        
        if (!found_input_file && !do_print_help_ && !do_print_version_) {
            THROW_EXCEPTION(InvalidFormatException, "No input file");
        }
        if (!is_output_file_set) {
            output_file_ = tools::getFileName(input_file_);
            switch (target_platform_) {
                case C: {
                    output_file_ += ".c";
                    break;
                }
                case CUDA: {
                    output_file_ += ".cu";
                    break;
                }
            }
        }
        if (found_input_file){
        	std::string extension = tools::getExtension(input_file_);
			if (extension == "xml") format_ = XML;
			else if (extension == "graphml") format_ = GraphML;
			else THROW_EXCEPTION(InvalidFormatException, "Input format not recognized");
        }
    }
    catch(InvalidFormatException& ex) {
        THROW_EXCEPTION(InvalidFormatException, ex.getMessage()
                        + "\nRerun program with option \"-h\" for help menu");
    }
}

bool Config::doDataParallelLeafCoalesing() const throw() {
    return do_data_parallel_leaf_coalescing_;
}

void Config::setDoDataParallelLeafCoalesing(bool setting) throw() {
    do_data_parallel_leaf_coalescing_ = setting;
}

bool Config::useSharedMemoryForInput() const throw() {
    return use_shared_memory_for_input_;
}

void Config::setUseSharedMemoryForInput(bool setting) throw() {
    use_shared_memory_for_input_ = setting;
}

bool Config::useSharedMemoryForOutput() const throw() {
    return use_shared_memory_for_output_;
}

void Config::setUseSharedMemoryForOutput(bool setting) throw() {
    use_shared_memory_for_output_ = setting;
}

Config::TargetPlatform Config::getTargetPlatform() const throw() {
    return target_platform_;
}

void Config::setTargetPlatform(Config::TargetPlatform platform) throw() {
    target_platform_ = platform;
}

Config::Costs Config::getCosts() const throw(){
	return costs_;
}

void Config::setCosts(const string& file) throw(){
//TODO::IMPLEMENT
}

Config::InputFormat Config::getInputFormat() const throw() {
    return format_;
}

void Config::setTargetPlatform(Config::InputFormat format) throw() {
	format_ = format;
}

bool Config::isOption(const string& str) const throw() {
    return str.substr(0, 1) == "-";
}

bool Config::isCompositeOption(const string& str) const throw() {
    if (str.length() < 2) return false;
    return str.substr(0, 2) == "--";
}

string Config::getVersion() throw() {
    return "0.1";
}

string Config::getSvnRevision() throw() {
#ifdef SVNVERSION
    string svn_str(SVNVERSION);

    // If the SVN revision string consists of two numbers, remove the first
    size_t colon_pos = svn_str.find(':');
    if (colon_pos != string::npos) {
        svn_str = svn_str.erase(0, colon_pos + 1);
    }

    // Remove trailing no-numeric characters, if any
    while (svn_str.length() > 0
           && !tools::isNumeric(svn_str.substr(svn_str.length() - 1))) {
        svn_str.erase(svn_str.length() - 1);
    }

    return svn_str;
#else
    return "Unknown";
#endif
}
