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

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Driver for the f2cc program.
 *
 * This file acts as the driver for f2cc. It performs necessary initializations,
 * invokes the parsing and synthesis process, and writes the generated code to
 * file. It also handles reporting of errors to the user by catching all
 * exceptions.
 */

#include "config/config.h"
#include "tools/tools.h"
#include "logger/logger.h"
#include "frontend/frontend.h"
#include "frontend/graphmlparser.h"
#include "frontend/xmlparser.h"
#include "forsyde/processnetwork.h"
#include "forsyde/process.h"
#include "forsyde/modelmodifier.h"
#include "synthesizer/synthesizer.h"
#include "exceptions/exception.h"
#include "exceptions/ioexception.h"
#include "exceptions/filenotfoundexception.h"
#include "exceptions/parseexception.h"
#include "exceptions/invalidformatexception.h"
#include "exceptions/invalidmodelexception.h"
#include <iostream>
#include <string>
#include <list>
#include <set>

using namespace f2cc;
using namespace f2cc::ForSyDe;
using std::string;
using std::cout;
using std::endl;
using std::list;
using std::set;

string getProcessnetworkInfo(Processnetwork* model) {
    string info;
    info += "Number of processes: ";
    info += tools::toString(model->getNumProcesses());
    info += "\n";
    info += "Number of inputs: ";
    info += tools::toString(model->getNumInputs());
    info += "\n";
    info += "Number of outputs: ";
    info += tools::toString(model->getNumOutputs());
    return info;
}

int main(int argc, const char* argv[]) {
    const string error_abort_str("\nProgram aborted.\n\n");
    const string parse_error_str("PARSE ERROR:\n");
    const string processnetwork_error_str("INVALID MODEL ERROR:\n");
    const string io_error_str("I/O ERROR:\n");
    const string critical_error_str("CRITICAL PROGRAM ERROR:\n");

    string header = string()
        + "f2cc - A CUDA C synthesizer for ForSyDe models\n\n";
    cout << header;

    // Get configuration
    Config config;
    try {
        config.setFromCommandLine(argc, argv);
    }
    catch (InvalidFormatException& ex) {
        cout << ex.getMessage() << endl;
        return 0;
    }
    catch (Exception& ex) {
        cout << ex.toString() << endl;
        cout << error_abort_str << endl;
        return 0;
    }

    if (config.doPrintHelpMenu()) {
        cout << config.getHelpMenu() << endl;
        return 0;
    }

    if (config.doPrintVersion()) {
        cout << "Version: " << config.getVersion() << endl
             << "SVN revision: " << config.getSvnRevision() << endl;
        return 0;
    }

    // Prepare logger
    Logger logger;
    logger.setLogLevel(config.getLogLevel());
    try {
        logger.open(config.getLogFile());
    }
    catch (Exception& ex) {
        cout << ex.toString() << endl;
        cout << error_abort_str << endl;
    }
    logger.logDebugMessage("Logger open");

    // Execute
    try {
        try {
        	Frontend* parser;
        	switch (config.getFrontendFormat()) {
				case Config::GraphML: {
					logger.logInfoMessage("File of type GraphML. Will use the old f2cc execution flow...");
					parser = new (std::nothrow) GraphmlParser(logger);
					break;
				}

				case Config::XML: {
					logger.logInfoMessage("File of type XML. Will use the new f2cc execution flow...");
					parser = new (std::nothrow) XmlParser(logger);
					break;
				}
				case Config::unknown: {
					THROW_EXCEPTION(OutOfMemoryException);
					break;
				}
			}

            if (!parser) THROW_EXCEPTION(OutOfMemoryException);
            logger.logInfoMessage(string("MODEL INPUT FILE: ")
                                  + config.getInputFile());
            logger.logInfoMessage("Parsing input file...");
            Processnetwork* model = parser->parse(config.getInputFile());
            delete parser;

            string processnetwork_info_message("MODEL INFO:\n");
            processnetwork_info_message += getProcessnetworkInfo(model);
            logger.logInfoMessage(processnetwork_info_message);

            string target_platform_message("TARGET PLATFORM: ");
            switch (config.getTargetPlatform()) {
                case Config::C: {
                    target_platform_message += "C";
                    break;
                }

                case Config::CUDA: {
                    target_platform_message += "CUDA";
                    break;
                }
            }
            logger.logInfoMessage(target_platform_message);

            // Make model modifications, if necessary
            ModelModifier modifier(model, logger);
            logger.logInfoMessage("Removing redundant processes...");
            modifier.removeRedundantProcesses();
            logger.logInfoMessage("Converting comb processes "
                              "with one in port to comb processes...");
            modifier.convertZipWith1Tocomb();
            if (config.getTargetPlatform() == Config::CUDA) {
                string process_coalescing_message("DATA PARALLEL PROCESS "
                                                  "COALESCING: ");
                if (config.doDataParallelProcessCoalesing()) {
                    process_coalescing_message += "YES";
                }
                else {
                    process_coalescing_message += "NO";
                }
                logger.logInfoMessage(process_coalescing_message);
                if (config.doDataParallelProcessCoalesing()) {
                    logger.logInfoMessage(""
                                      "Performing data parallel comb process "
                                      "coalescing...");
                    modifier.coalesceDataParallelProcesses();
                }

                logger.logInfoMessage(
                                  "Splitting data parallel segments...");
                modifier.splitDataParallelSegments();

                logger.logMessage(Logger::INFO,
                                  "Fusing chains of unzipx-map-zipx "
                                  "processes...");
                modifier.fuseUnzipcombZipProcesses();

                if (config.doDataParallelProcessCoalesing()) {
                    logger.logInfoMessage(""
                                      "Performing ParallelMap process "
                                      "coalescing...");
                    modifier.coalesceParallelMapSyProcesses();
                }
            }
            processnetwork_info_message = "NEW MODEL INFO:\n";
            processnetwork_info_message += getProcessnetworkInfo(model);
            logger.logInfoMessage(processnetwork_info_message);

            // Generate code and write to file
            Synthesizer synthesizer(model, logger, config);
            Synthesizer::CodeSet code;
            switch (config.getTargetPlatform()) {
                case Config::C: {
                    code = synthesizer.generateCCode();
                    break;
                }

                case Config::CUDA: {
                    code = synthesizer.generateCudaCCode();
                    break;
                }
            }

            logger.logInfoMessage("Writing code to output files...");
            tools::writeFile(config.getHeaderOutputFile(), code.header);
            tools::writeFile(config.getImplementationOutputFile(),
                             code.implementation);

            logger.logInfoMessage("MODEL NTHESIS COMPLETE");

            // Clean up
            delete model;
            logger.logDebugMessage("Closing logger...");
            logger.close();
        } catch (FileNotFoundException& ex) {
            logger.logErrorMessage(ex.getMessage());
        } catch (ParseException& ex) {
            logger.logErrorMessage(parse_error_str + ex.getMessage());
        } catch (InvalidModelException& ex) {
            logger.logErrorMessage(processnetwork_error_str + ex.getMessage());
        } catch (IOException& ex) {
            logger.logErrorMessage(io_error_str + ex.getMessage());
        } catch (Exception& ex) {
            logger.logCriticalMessage(critical_error_str + ex.toString()
                                      + error_abort_str);
        }
    } catch (Exception&) {
        // Ignore
    }

    return 0;
}
