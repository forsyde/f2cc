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

#include "frontend.h"
#include "dumper.h"
#include <list>

using namespace f2cc;
using namespace f2cc::Forsyde;
using std::string;
using std::list;

Frontend::Frontend(Logger& logger) throw() : logger_(logger) {}

Frontend::~Frontend() throw() {}

ProcessNetwork* Frontend::parse(const string& file)
    throw(InvalidArgumentException, FileNotFoundException, IOException,
          ParseException, InvalidModelException, RuntimeException) {
    if (file.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"file\" must not be empty "
                        "string");
    }

    ProcessNetwork* processnetwork = createProcessNetwork(file);

    XmlDumper* dumper;
    dumper = new (std::nothrow) XmlDumper(logger_);
    dumper->dump(processnetwork,"hallo.xml");

    logger_.logMessage(Logger::INFO, "Checking that the internal processnetwork is "
                       "sane...");
    checkProcessNetwork(processnetwork);
    logger_.logMessage(Logger::INFO, "All checks passed");

    postCheckFixes(processnetwork);

    return processnetwork;
}

void Frontend::checkProcessNetwork(ProcessNetwork* processnetwork)
    throw(InvalidArgumentException, InvalidModelException, IOException,
          RuntimeException) {
    list<Leaf*> leafs = processnetwork->getProcesses();
    list<Leaf*>::iterator leaf_it;
    for (leaf_it = leafs.begin(); leaf_it != leafs.end();
         ++leaf_it) {
        Leaf* leaf = *leaf_it;
        logger_.logMessage(Logger::DEBUG,
                           string("Checking leaf \"")
                           + leaf->getId()->getString() + "\"");

        // Leaf type-related check
        try {
            leaf->check();
        } catch (InvalidProcessException& ex) {
            THROW_EXCEPTION(InvalidModelException, ex.getMessage());
        }

        // Port checks
        list<Leaf::Port*> ports = leaf->getInPorts();
        list<Leaf::Port*>::iterator port_it;
        for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
            checkPort(*port_it, processnetwork);
        }
        ports = leaf->getOutPorts();
        for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
            checkPort(*port_it, processnetwork);
        }
    }
}

void Frontend::checkPort(Leaf::Port* port, ProcessNetwork* processnetwork)
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
                        + "\" in leaf \""
                        + port->getProcess()->getId()->getString()
                        + "\" is unconnected");
    }

    // Check that the port is not connected to its own leaf
    if (port->getConnectedPort()->getProcess() == port->getProcess()) {
        THROW_EXCEPTION(InvalidModelException, string("Port \"")
                        + port->getId()->getString()
                        + "\" in leaf \""
                        + port->getProcess()->getId()->getString()
                        + "\" is connected to its own process "
                        + "(Combinatorial looping)");
    }

    // Check that the other port belongs to a process in the processnetwork
    if (
    	(!processnetwork->getProcess(*port->getConnectedPort()->getProcess()->getId())) &&
    	(!processnetwork->getComposite(*port->getConnectedPort()->getProcess()->getId()))
    	) {
        THROW_EXCEPTION(InvalidModelException, string("Port \"")
                        + port->getId()->getString()
                        + "\" in leaf \""
                        + port->getProcess()->getId()->getString()
                        + "\" is connected to a process outside the "
                        + "process network");
    }
}

void Frontend::checkProcessNetworkMore(Forsyde::ProcessNetwork* processnetwork)
    throw(InvalidArgumentException, InvalidModelException, IOException,
          RuntimeException) {}


void Frontend::postCheckFixes(Forsyde::ProcessNetwork* processnetwork)
    throw(InvalidArgumentException, IOException, RuntimeException) {}
