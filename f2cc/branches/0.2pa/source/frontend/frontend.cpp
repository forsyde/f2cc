/*
 * Copyright (c) 2011-2013 Gabriel Hjort Blindell <ghb@kth.se>
 *                          George Ungureanu <ugeorge@kth.se>
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
#include "../forsyde/SY/inport.h"
#include "../forsyde/SY/outport.h"
#include <list>

using namespace f2cc;
using namespace f2cc::ForSyDe;
using namespace f2cc::ForSyDe::SY;
using std::string;
using std::list;

Frontend::Frontend(Logger& logger) throw() : logger_(logger) {}

Frontend::~Frontend() throw() {}

Processnetwork* Frontend::parse(const string& file)
    throw(InvalidArgumentException, FileNotFoundException, IOException,
          ParseException, InvalidModelException, RuntimeException) {
    if (file.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"file\" must not be empty "
                        "string");
    }

    Processnetwork* processnetwork = createProcessnetwork(file);

    logger_.logInfoMessage("Checking that the internal process network is sane...");
    checkProcessnetwork(processnetwork);
    logger_.logInfoMessage("All checks passed");

    logger_.logInfoMessage("Running post-check fixes...");
    postCheckFixes(processnetwork);
    logger_.logInfoMessage("Post-check fixes done");

    ensureNoInPorts(processnetwork);
    ensureNoOutPorts(processnetwork);

    return processnetwork;
}

void Frontend::checkProcessnetwork(Processnetwork* processnetwork)
    throw(InvalidArgumentException, InvalidModelException, IOException,
          RuntimeException) {
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }

    // Check processes
    list<Process*> processes = processnetwork->getProcesses();
    list<Process*>::iterator process_it;
    for (process_it = processes.begin(); process_it != processes.end();
         ++process_it) {
        checkProcess(*process_it, processnetwork);
    }

    logger_.logInfoMessage("Running additional processnetwork checks...");
    checkProcessnetworkMore(processnetwork);
    logger_.logInfoMessage("Additional processnetwork checks passed");    
}

void Frontend::ensureNoInPorts(Processnetwork* processnetwork)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }

    logger_.logDebugMessage("Checking that there are no InPort processes in "
                            "the process network at this stage...");
    list<Process*> processes = processnetwork->getProcesses();
    list<Process*>::iterator process_it;
    for (process_it = processes.begin(); process_it != processes.end();
         ++process_it) {
        Process* process = *process_it;
        logger_.logDebugMessage(string("Checking process \"")
                                + process->getId()->getString() + "\"...");
        if(dynamic_cast<InPort*>(process)) {
            logger_.logDebugMessage("Is an InPort process");
            THROW_EXCEPTION(IllegalStateException, string("Process \"")
                            + process->getId()->getString()
                            + "\" is an InPort - no InPort processes are "
                            + "allowed at this stage");
        }
        else {
            logger_.logDebugMessage("Not an InPort process");
        }
    }
    logger_.logDebugMessage("No InPort processes in the process network");
}

void Frontend::ensureNoOutPorts(Processnetwork* processnetwork)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }

    logger_.logDebugMessage("Checking that there are no OutPort processes in "
                            "the process network at this stage...");
    list<Process*> processes = processnetwork->getProcesses();
    list<Process*>::iterator process_it;
    for (process_it = processes.begin(); process_it != processes.end();
         ++process_it) {
        Process* process = *process_it;
        logger_.logDebugMessage(string("Checking process \"")
                                + process->getId()->getString() + "\"...");
        if(dynamic_cast<OutPort*>(process)) {
            logger_.logDebugMessage("Is an OutPort process");
            THROW_EXCEPTION(IllegalStateException, string("Process \"")
                            + process->getId()->getString()
                            + "\" is an OutPort - no OutPort processes are "
                            + "allowed at this stage");
        }
        else {
            logger_.logDebugMessage("Not an OutPort process");
        }
    }
    logger_.logDebugMessage("No OutPort processes in the process network");
}

void Frontend::checkProcess(Process* process, Processnetwork* processnetwork)
    throw(InvalidArgumentException, InvalidModelException, IOException,
          RuntimeException) {
    if (!process) {
        THROW_EXCEPTION(InvalidArgumentException,
                        "\"process\" must not be NULL");
    }
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }

    logger_.logDebugMessage(string("Checking process \"")
                            + process->getId()->getString() + "\"...");

    // Process type-related check
    try {
        process->check();
    } catch (InvalidProcessException& ex) {
        THROW_EXCEPTION(InvalidModelException, ex.getMessage());
    }

    // Port checks
    logger_.logDebugMessage("Checking ports...");
    list<Process::Port*> ports = process->getInPorts();
    list<Process::Port*>::iterator port_it;
    for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
        checkPort(*port_it, processnetwork);
    }
    ports = process->getOutPorts();
    for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
        checkPort(*port_it, processnetwork);
    }

    logger_.logDebugMessage(string("Process \"")
                            + process->getId()->getString()
                            + "\" passed all checks");
}

void Frontend::checkPort(Process::Port* port, Processnetwork* processnetwork)
    throw(InvalidArgumentException, InvalidModelException, IOException,
          RuntimeException) {
    if (!port) {
        THROW_EXCEPTION(InvalidArgumentException, "\"port\" must not be NULL");
    }
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
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
                        + "\" is connected to its own processnetwork "
                        + "(Mapinatorial looping)");
    }

    // Check that the other port belongs to a process in the process network
    if (!processnetwork->getProcess(*port->getConnectedPort()->getProcess()->getId())) {
        THROW_EXCEPTION(InvalidModelException, string("Port \"")
                        + port->getId()->getString()
                        + "\" in process \""
                        + port->getProcess()->getId()->getString()
                        + "\" is connected to a process outside the "
                        + "processnetwork");
    }
}

void Frontend::checkProcessnetworkMore(ForSyDe::Processnetwork* processnetwork)
    throw(InvalidArgumentException, InvalidModelException, IOException,
          RuntimeException) {}


void Frontend::postCheckFixes(ForSyDe::Processnetwork* processnetwork)
    throw(InvalidArgumentException, IOException, RuntimeException) {}
