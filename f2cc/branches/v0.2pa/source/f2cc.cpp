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

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Driver for the f2cc program.
 *
 * This file acts as the driver for f2cc. It performs necessary initializations,
 * invokes the parsing and synthesis leaf, and writes the generated code to
 * file. It also handles reporting of errors to the user by catching all
 * exceptions.
 */

#include "config/config.h"
#include "tools/tools.h"
#include "logger/logger.h"
#include "frontend/frontend.h"
#include "frontend/graphmlparser.h"
#include "frontend/xmlparser.h"
#include "frontend/dumper.h"
#include "forsyde/processnetwork.h"
#include "forsyde/leaf.h"
#include "forsyde/modifiers/modelmodifier.h"
#include "forsyde/modifiers/modelmodifiersysc.h"
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
using namespace f2cc::Forsyde;
using std::string;
using std::cout;
using std::endl;
using std::list;
using std::set;

string getProcessNetworkInfo(ProcessNetwork* processnetwork) {
    string info;
    info += "Number of leafs: ";
    info += tools::toString(processnetwork->getNumProcesses());
    info += "\n";
    info += "Number of composites: ";
    info += tools::toString(processnetwork->getNumComposites());
    info += "\n";
    info += "Number of inputs: ";
    info += tools::toString(processnetwork->getNumInputs());
    info += "\n";
    info += "Number of outputs: ";
    info += tools::toString(processnetwork->getNumOutputs());
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
            switch (config.getInputFormat()) {
                case Config::XML: {
                	logger.logInfoMessage(string("New XML format assumed.")
                			+ " The execution will follow the path from v0.2...");
                	parser = new (std::nothrow) XmlParser(logger);
                    break;
                }

                case Config::GraphML: {
                	logger.logInfoMessage(string("Old GraphML format assumed.")
                	        + " The execution will follow the path from v0.1...");
                	parser = new (std::nothrow) GraphmlParser(logger);
                    break;
                }
            }
            if (!parser) THROW_EXCEPTION(OutOfMemoryException);
            logger.logInfoMessage(string("MODEL INPUT FILE: ")
                                  + config.getInputFile());
            logger.logInfoMessage("Parsing input file...");
            ProcessNetwork* processnetwork = parser->parse(config.getInputFile());
            delete parser;

            string processnetwork_info_message("MODEL INFO:\n");
            processnetwork_info_message += getProcessNetworkInfo(processnetwork);
            logger.logInfoMessage(processnetwork_info_message);

            XmlDumper dumper(logger);
            dumper.dump(processnetwork,"hallo.xml");


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

            switch (config.getInputFormat()) {
				case Config::GraphML: {
					// Make processnetwork modifications, if necessary
					ModelModifier modifier(processnetwork, logger);
					logger.logInfoMessage("Removing redundant leafs...");
					modifier.removeRedundantLeafs();
					logger.logInfoMessage("Converting Comb leafs "
									  "with one in port to Comb leafs...");
					modifier.convertZipWith1ToMap();
					if (config.getTargetPlatform() == Config::CUDA) {
						string leaf_coalescing_message("DATA PARALLEL PROCESS "
														  "COALESCING: ");
						if (config.doDataParallelLeafCoalesing()) {
							leaf_coalescing_message += "YES";
						}
						else {
							leaf_coalescing_message += "NO";
						}
						logger.logInfoMessage(leaf_coalescing_message);
						if (config.doDataParallelLeafCoalesing()) {
							logger.logInfoMessage(""
											  "Performing data parallel Comb leaf "
											  "coalescing...");
							modifier.coalesceDataParallelLeafs();
						}

						logger.logInfoMessage(
										  "Splitting data parallel segments...");
						modifier.splitDataParallelSegments();

						logger.logMessage(Logger::INFO,
										  "Fusing chains of Unzipx-map-Zipx "
										  "leafs...");
						modifier.fuseUnzipMapZipLeafs();

						if (config.doDataParallelLeafCoalesing()) {
							logger.logInfoMessage(""
											  "Performing ParallelMap leaf "
											  "coalescing...");
							modifier.coalesceParallelMapSyLeafs();
						}
					}
					processnetwork_info_message = "NEW MODEL INFO:\n";
					processnetwork_info_message += getProcessNetworkInfo(processnetwork);
					logger.logInfoMessage(processnetwork_info_message);
					break;
				}

				case Config::XML: {
					config.setCosts(string("config/costs.xml"));
					ModelModifierSysC modifier(processnetwork, logger, config.getCosts());
					modifier.flattenAndParallelize();

					XmlDumper dumper1(logger);
					dumper1.dump(processnetwork,"flattenAndParallelize.xml");

					modifier.optimizePlatform();

					XmlDumper dumper2(logger);
					dumper2.dump(processnetwork,"optimizePlatform.xml");

					modifier.loadBalance();

					processnetwork_info_message = "NEW MODEL INFO:\n";
					processnetwork_info_message += getProcessNetworkInfo(processnetwork);
					logger.logInfoMessage(processnetwork_info_message);

					break;
				}
			}



            // Generate code and write to file
            Synthesizer synthesizer(processnetwork, logger, config);
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

            logger.logInfoMessage("MODEL SYNTHESIS COMPLETE");

            // Clean up
            delete processnetwork;
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
