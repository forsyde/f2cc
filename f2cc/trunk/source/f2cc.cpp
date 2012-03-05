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
#include "forsyde/model.h"
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
using namespace f2cc::Forsyde;
using std::string;
using std::cout;
using std::endl;
using std::list;
using std::set;

string getModelInfo(Model* model) {
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
    const string model_error_str("INVALID MODEL ERROR:\n");
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
    logger.logMessage(Logger::DEBUG, "Logger open");

    // Execute
    try {
        try {
            Frontend* parser = new (std::nothrow) GraphmlParser(logger);
            if (!parser) THROW_EXCEPTION(OutOfMemoryException);
            logger.logMessage(Logger::INFO, string("MODEL INPUT FILE: ")
                              + config.getInputFile());
            logger.logMessage(Logger::INFO, "Parsing input file...");
            Model* model = parser->parse(config.getInputFile());
            delete parser;

            string model_info_message("MODEL INFO:\n");
            model_info_message += getModelInfo(model);
            logger.logMessage(Logger::INFO, model_info_message);

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
            logger.logMessage(Logger::INFO, target_platform_message);

            // Make model modifications, if necessary
            ModelModifier modifier(model, logger);
            logger.logMessage(Logger::INFO, "Removing redundant processes...");
            modifier.removeRedundantProcesses();
            logger.logMessage(Logger::INFO, "Converting ZipWithN processes "
                              "with one in port to MapSY processes...");
            modifier.convertZipWith1ToMapSY();
            if (config.getTargetPlatform() == Config::CUDA) {
                string process_coalescing_message("DATA PARALLEL PROCESS "
                                                  "COALESCING: ");
                if (config.doDataParallelProcessCoalesing()) {
                    process_coalescing_message += "YES";
                }
                else {
                    process_coalescing_message += "NO";
                }
                logger.logMessage(Logger::INFO, process_coalescing_message);
                if (config.doDataParallelProcessCoalesing()) {
                    logger.logMessage(Logger::INFO, ""
                                      "Performing data parallel MapSY process "
                                      "coalescing...");
                    modifier.coalesceDataParallelProcesses();
                }

                logger.logMessage(Logger::INFO, 
                                  "Splitting data parallel segments...");
                modifier.splitDataParallelSegments();

                logger.logMessage(Logger::INFO,
                                  "Fusing chains of unzipxSY-mapSY-zipxSY "
                                  "processes...");
                modifier.fuseUnzipMapZipProcesses();

                if (config.doDataParallelProcessCoalesing()) {
                    logger.logMessage(Logger::INFO, ""
                                      "Performing ParallelMapSY process "
                                      "coalescing...");
                    modifier.coalesceParallelMapSyProcesses();
                }
            }
            model_info_message = "NEW MODEL INFO:\n";
            model_info_message += getModelInfo(model);
            logger.logMessage(Logger::INFO, model_info_message);

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

            logger.logMessage(Logger::INFO, "Writing code to output files...");
            tools::writeFile(config.getHeaderOutputFile(), code.header);
            tools::writeFile(config.getImplementationOutputFile(),
                             code.implementation);

            logger.logMessage(Logger::INFO, "MODEL SYNTHESIS COMPLETE");

            // Clean up
            delete model;
            logger.logMessage(Logger::DEBUG, "Closing logger...");
            logger.close();
        } catch (FileNotFoundException& ex) {
            logger.logMessage(Logger::ERROR, ex.getMessage());
        } catch (ParseException& ex) {
            logger.logMessage(Logger::ERROR, parse_error_str + ex.getMessage());
        } catch (InvalidModelException& ex) {
            logger.logMessage(Logger::ERROR, model_error_str + ex.getMessage());
        } catch (IOException& ex) {
            logger.logMessage(Logger::ERROR, io_error_str + ex.getMessage());
        } catch (Exception& ex) {
            logger.logMessage(Logger::CRITICAL, critical_error_str
                              + ex.toString() + error_abort_str);
        }
    } catch (Exception&) {
        // Ignore
    }

    return 0;
}
