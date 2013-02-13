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

#include "frontend.h"
#include <list>

using namespace f2cc;
using namespace f2cc::Forsyde;
using std::string;
using std::list;

Frontend::Frontend(Logger& logger) throw() : logger_(logger) {}

Frontend::~Frontend() throw() {}

Model* Frontend::parse(const string& file)
    throw(InvalidArgumentException, FileNotFoundException, IOException,
          ParseException, InvalidModelException, RuntimeException) {
    if (file.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"file\" must not be empty "
                        "string");
    }

    Model* model = createModel(file);

    logger_.logInfoMessage("Checking that the internal model is sane...");
    checkModel(model);
    logger_.logInfoMessage("All checks passed");

    postCheckFixes(model);

    return model;
}

void Frontend::checkModel(Model* model)
    throw(InvalidArgumentException, InvalidModelException, IOException,
          RuntimeException) {
    list<Process*> processes = model->getProcesses();
    list<Process*>::iterator process_it;
    for (process_it = processes.begin(); process_it != processes.end();
         ++process_it) {
        Process* process = *process_it;
        logger_.logMessage(Logger::DEBUG,
                           string("Checking process \"")
                           + process->getId()->getString() + "\"");

        // Process type-related check
        try {
            process->check();
        } catch (InvalidProcessException& ex) {
            THROW_EXCEPTION(InvalidModelException, ex.getMessage());
        }

        // Port checks
        list<Process::Port*> ports = process->getInPorts();
        list<Process::Port*>::iterator port_it;
        for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
            checkPort(*port_it, model);
        }
        ports = process->getOutPorts();
        for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
            checkPort(*port_it, model);
        }
    }
}

void Frontend::checkPort(Process::Port* port, Model* model)
    throw(InvalidArgumentException, InvalidModelException, IOException,
          RuntimeException) {
    if (!port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"port\" must not be NULL");
    }
    if (!model) {
        THROW_EXCEPTION(InvalidArgumentException, "\"model\" must not be NULL");
    }

    if (!port->isConnected()) {
        THROW_EXCEPTION(InvalidModelException, string("Port \"")
                        + port->getId()->getString()
                        + "\" in process \""
                        + port->getProcess()->getId()->getString()
                        + "\" is unconnected");
    }

    // Check that the port is not connected to its own process
    if (port->getConnectedPort()->getProcess() == port->getProcess()) {
        THROW_EXCEPTION(InvalidModelException, string("Port \"")
                        + port->getId()->getString()
                        + "\" in process \""
                        + port->getProcess()->getId()->getString()
                        + "\" is connected to its own model "
                        + "(combinatorial looping)");
    }

    // Check that the other port belongs to a process in the model
    if (!model->getProcess(*port->getConnectedPort()->getProcess()->getId())) {
        THROW_EXCEPTION(InvalidModelException, string("Port \"")
                        + port->getId()->getString()
                        + "\" in process \""
                        + port->getProcess()->getId()->getString()
                        + "\" is connected to a process outside the "
                        + "model");
    }
}

void Frontend::checkModelMore(Forsyde::Model* model)
    throw(InvalidArgumentException, InvalidModelException, IOException,
          RuntimeException) {}


void Frontend::postCheckFixes(Forsyde::Model* model)
    throw(InvalidArgumentException, IOException, RuntimeException) {}
