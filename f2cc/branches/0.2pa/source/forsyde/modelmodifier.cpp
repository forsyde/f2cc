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

#include "modelmodifier.h"
#include "SY/zipxsy.h"
#include "SY/unzipxsy.h"
#include "SY/parallelmapsy.h"
#include "SY/coalescedmapsy.h"
#include "SY/combsy.h"
#include "../language/cfunction.h"
#include "../language/cdatatype.h"
#include "../tools/tools.h"
#include "../exceptions/castexception.h"
#include "../exceptions/indexoutofboundsexception.h"
#include <set>
#include <string>
#include <new>
#include <stdexcept>

using namespace f2cc;
using namespace f2cc::ForSyDe;
using namespace f2cc::ForSyDe::SY;
using std::string;
using std::list;
using std::set;
using std::vector;
using std::bad_alloc;
using std::pair;

ModelModifier::ModelModifier(Processnetwork* processnetwork, Logger& logger)
        throw(InvalidArgumentException) : processnetwork_(processnetwork), logger_(logger) {
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }
}

ModelModifier::~ModelModifier() throw() {}

void ModelModifier::coalesceDataParallelProcesses()
    throw(IOException, RuntimeException) {
    list<ContainedSection> sections = findDataParallelSections();
    list<ContainedSection>::iterator it;
    for (it = sections.begin(); it != sections.end(); ++it) {
        list<Process::Port*> ports = it->start->getOutPorts();
        list<Process::Port*>::iterator port_it;
        for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
            list<Process*> chain = getProcessChain(*port_it, it->end);
            if (chain.size() > 1) {
                logger_.logInfoMessage(string("Coalescing process ")
                                       + "chain " + processChainToString(chain)
                                       + "...");
                coalesceProcessChain(chain);
            }
            else {
                logger_.logInfoMessage(string("Data parallel ")
                                       + "section " + it->toString()
                                       + " only consists of one segment - no "
                                       + "process coalescing needed");
                break;
            }
        }
    }
}

void ModelModifier::coalesceParallelMapSyProcesses()
    throw(IOException, RuntimeException) {
    list< list<ParallelMap*> > chains = findParallelMapSyChains();
    if (chains.size() == 0) {
        logger_.logInfoMessage("No ParallelMap chains found");
        return;
    }

    list< list<ParallelMap*> >::iterator it;
    for (it = chains.begin(); it != chains.end(); ++it) {
        if (!isParallelMapSyChainCoalescable(*it)) continue;
        logger_.logInfoMessage(string("Coalescing process chain ")
                               + processChainToString(*it) + "...");
        coalesceParallelMapSyChain(*it);
    }
}

void ModelModifier::splitDataParallelSegments()
    throw(IOException, RuntimeException) {
    list<ContainedSection> sections = findDataParallelSections();
    list<ContainedSection>::iterator it;
    for (it = sections.begin(); it != sections.end(); ++it) {
        bool aborted = false;
        vector< vector<Process*> > chains;
        list<Process::Port*> ports = it->start->getOutPorts();
        list<Process::Port*>::iterator port_it;
        for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
            list<Process*> chain = getProcessChain(*port_it, it->end);
            if (chain.size() <= 1) {
                logger_.logInfoMessage(string("Data parallel ")
                                       + "section " + it->toString()
                                       + " only consists of one segment - no "
                                       + "splitting needed");
                aborted = true;
                break;
            }
            vector<Process*> chain_as_vector(chain.begin(), chain.end());
            chains.push_back(chain_as_vector);
        }
        if (!aborted) {
            logger_.logInfoMessage(string("Splitting segments in ")
                                   + "section " + it->toString() + "...");
            splitDataParallelSegments(chains);
        }
    }
}

void ModelModifier::fuseUnzipcombZipProcesses()
    throw(IOException, RuntimeException) {
    list<ContainedSection> sections = findDataParallelSections();
    list<ContainedSection>::iterator it;
    for (it = sections.begin(); it != sections.end(); ++it) {
        ContainedSection section = *it;
        logger_.logInfoMessage(string("Fusing data parallel section ")
                               + section.toString() + "...");

        // Get function arguments of map or coalescedmap processes
        if (getProcessChain(section.start->getOutPorts().front(),
                            section.end).size() != 1) {
            THROW_EXCEPTION(IllegalStateException, "Process chain is not of "
                            "length 1");
        }
        Process* data_process = section.start->getOutPorts().front()
            ->getConnectedPort()->getProcess();
        list<CFunction> functions;
        CoalescedMap* cmapsy_process =
            dynamic_cast<CoalescedMap*>(data_process);
        if (cmapsy_process) {
            list<CFunction*> functions_to_copy = cmapsy_process->getFunctions();
            list<CFunction*>::iterator func_it;
            for (func_it = functions_to_copy.begin();
                 func_it != functions_to_copy.end(); ++func_it) {
                functions.push_back(**func_it);
            }
        }
        else {
            comb* mapsy_process = dynamic_cast<comb*>(data_process);
            if (!mapsy_process) THROW_EXCEPTION(CastException);
            functions.push_back(*mapsy_process->getFunction());
        }

        // Create new parallelmap process to replace the data parallel section
        int num_processes = section.start->getOutPorts().size();
        ParallelMap* new_process = new (std::nothrow) ParallelMap(
            processnetwork_->getUniqueProcessId("_parallelmap_"), Id("Process_Network"), num_processes,
            functions);
        if (!new_process) THROW_EXCEPTION(OutOfMemoryException);
        logger_.logDebugMessage(string("New ParallelMap process \"")
                                + new_process->getId()->getString()
                                + "\" created");

        redirectDataFlow(section.start, section.end, new_process, new_process);

        // Add new process to the process network
        if (processnetwork_->addProcess(new_process)) {
            logger_.logInfoMessage(string("Data parallel section ")
                                   + section.toString() + " replaced by new "
                                   "process \""
                                   + new_process->getId()->getString() + "\"");
        }
        else {
            THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                            + "new process: Process with ID \""
                            + new_process->getId()->getString()
                            + "\" already existed");
        }

        // Destroy and delete the section from the process network
        logger_.logDebugMessage(string("Destroying section \"")
                                + section.toString() + "...");
        destroyProcessChain(section.start);
    }
}

void ModelModifier::convertZipWith1Tocomb()
    throw(IOException, RuntimeException) {
    list<Process*> processes = processnetwork_->getProcesses();
    list<Process*>::iterator it;
    for (it = processes.begin(); it != processes.end(); ++it) {
        logger_.logDebugMessage(string("Analyzing process \"")
                                + (*it)->getId()->getString() + "\"...");

        comb* process = dynamic_cast<comb*>(*it);
        if (process && process->getNumInPorts() == 1) {
            comb* new_process = new (std::nothrow) comb(
                processnetwork_->getUniqueProcessId("_map_"), Id("Process_Network"), *process->getFunction());
            if (!new_process) THROW_EXCEPTION(OutOfMemoryException);
            logger_.logDebugMessage(string("New comb process \"")
                                    + new_process->getId()->getString()
                                    + "\" created");

            redirectDataFlow(process, process, new_process, new_process);

            // Add new process to the process network
            if (processnetwork_->addProcess(new_process)) {
                logger_.logInfoMessage(string("Process \"")
                                       + process->getId()->getString() + "\" "
                                       + "replaced by new process \""
                                       + new_process->getId()->getString()
                                       + "\"");
            }
            else {
                THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                                + "new process: Process with ID \""
                                + new_process->getId()->getString()
                                + "\" already existed");
            }

            // Destroy and delete the old process from the process network
            logger_.logDebugMessage(string("Destroying process \"")
                                    + process->getId()->getString() + "...");
            processnetwork_->deleteProcess(*process->getId());
        }
    }
}

void ModelModifier::removeRedundantProcesses()
    throw(IOException, RuntimeException) {
    list<Process*> processes = processnetwork_->getProcesses();
    list<Process*>::iterator it;
    for (it = processes.begin(); it != processes.end(); ++it) {
        Process* process = *it;
        logger_.logDebugMessage(string("Analyzing process \"")
                                + process->getId()->getString() + "\"...");

        // Remove zipx and unzipx processes which have only one in and out
        // port
        bool is_zipxsy = dynamic_cast<zipx*>(process);
        bool is_unzipxsy = !is_zipxsy && dynamic_cast<unzipx*>(process);
        if (is_zipxsy || is_unzipxsy) {
            if (process->getNumInPorts() == 1
                && process->getNumOutPorts() == 1) {
                string process_name = process->getId()->getString();

                // Redirect the process' in and out ports to unconnect it from
                // the network
                Process::Port* in_port = 
                    process->getInPorts().front();
                Process::Port* out_port = 
                    process->getOutPorts().front();
                Process::Port* other_end_at_in_port =
                    in_port->getConnectedPort();
                Process::Port* other_end_at_out_port =
                    out_port->getConnectedPort();
                if (other_end_at_in_port && other_end_at_out_port) {
                    other_end_at_in_port->connect(other_end_at_out_port);
                }

                // Update processnetwork in- and ouputs, if necessary
                logger_.logDebugMessage("Updating processnetwork in- and "
                                        "outputs...");
                if (other_end_at_in_port == NULL) {
                    replaceProcessnetworkInput(in_port,
                                      other_end_at_out_port);
                }
                if (other_end_at_out_port == NULL) {
                    replaceProcessnetworkOutput(out_port,
                                       other_end_at_in_port);
                }

                // Delete process from processnetwork
                if (!processnetwork_->deleteProcess(*process->getId())) {
                    THROW_EXCEPTION(IllegalStateException, string("Could not ") 
                                    + "delete process \"" + process_name
                                    + "\"");
                }
                
                if (is_zipxsy) {
                    logger_.logInfoMessage(string("Removed ")
                                           + "redundant zipx process \""
                                           + process_name + "\" (had only 1 in "
                                           + "port)");
                }
                else {
                    logger_.logInfoMessage(string("Removed ")
                                           + "redundant unzipx process \""
                                           + process_name + "\" (had only 1 "
                                           "out port)");
                }
            }
        }
    }
}

list<ModelModifier::ContainedSection> ModelModifier::findDataParallelSections()
    throw(IOException, RuntimeException) {
    list<ContainedSection> sections;

    // Find contained sections sections
    logger_.logInfoMessage("Searching for contained sections...");
    sections = findContainedSections();
    if (sections.size() == 0) {
        logger_.logInfoMessage("No contained (and thus no data "
                               "parallel) sections found");
        return sections;
    }
    string message = string("Found ") + tools::toString(sections.size())
        + " contained section(s): ";
    bool first = true;
    list<ContainedSection>::iterator it;
    for (it = sections.begin(); it != sections.end(); ++it) {
        if (first) first = false;
        else       message += ", ";
        message += it->toString();
    }
    logger_.logInfoMessage(message);

    // Check which sections are data parallel and remove those that aren't
    logger_.logInfoMessage("Checking which sections are data parallel...");
    for (it = sections.begin(); it != sections.end(); ) {
        if(isContainedSectionDataParallel(*it)) {
            logger_.logInfoMessage(it->toString()
                                   + " is data parallel");
            ++it;
        }
        else {
            logger_.logInfoMessage(it->toString()
                                   + " is not data parallel");
            it = sections.erase(it);
        }
    }

    return sections;
}

list<ModelModifier::ContainedSection>
ModelModifier::findContainedSections() throw(IOException, RuntimeException) {
    list<ContainedSection> sections;
    set<Id> visited;
    list<Process::Port*> output_ports = processnetwork_->getOutputs();
    list<Process::Port*>::iterator it;
    for (it = output_ports.begin(); it != output_ports.end(); ++it) {
        logger_.logDebugMessage(string("Entering at output port \"")
                                + (*it)->toString() + "\"");
        tools::append<ContainedSection>(sections, findContainedSections(
                                            (*it)->getProcess(), visited));
    }
    return sections;
}

list<ModelModifier::ContainedSection>
ModelModifier::findContainedSections(Process* begin, set<Id>& visited)
    throw(IOException, RuntimeException) {
    list<ContainedSection> sections;
    if (visitProcess(visited, begin)) {
        logger_.logDebugMessage(string("Analyzing process \"")
                                + begin->getId()->getString() + "\"...");
        zipx* converge_point = dynamic_cast<zipx*>(begin);
        if (converge_point) {
            logger_.logDebugMessage(string("Discovered zipx ")
                                    + "process \""
                                    + converge_point->getId()->getString()
                                    + "\"");
            logger_.logDebugMessage("Searching for nearest unzipx "
                                    "process...");
            set<Id> visited_for_nearest_unzipxsy;
            unzipx* diverge_point =
                findNearestunzipxProcess(converge_point,
                                           visited_for_nearest_unzipxsy);
            if (diverge_point) {
                logger_.logDebugMessage(string("Found nearest ")
                                        + "unzipx process \""
                                        + diverge_point->getId()->getString()
                                        + "\"");
            }
            else {
                logger_.logDebugMessage("No unzipx process found");
                // Return empty list
                return sections;
            }
            logger_.logDebugMessage(string("Checking that the data ")
                                    + "flow between processes \""
                                    + diverge_point->getId()->getString()
                                    + "\" and \""
                                    + converge_point->getId()->getString()
                                    + "\" is contained...");
            if (!isAContainedSection(diverge_point, converge_point)) {
                logger_.logDebugMessage(string("Section between ")
                                        + "processes \""
                                        + diverge_point->getId()->getString()
                                        + "\" and \""
                                        + converge_point->getId()->getString()
                                        + "\" is not contained");
                goto continue_search;
            }

            logger_.logDebugMessage(string("Found contained section ")
                                    + "between processes \""
                                    + diverge_point->getId()->getString()
                                    + "\" and \""
                                    + converge_point->getId()->getString()
                                    + "\"");
            sections.push_back(ContainedSection(diverge_point,
                                                converge_point));
            // The converge point need not be set as visited since it is only
            // reachable by an already visited process
            tools::append<ContainedSection>(
                sections, findContainedSections(diverge_point, visited));
            return sections;
        }

      continue_search:
        list<Process::Port*> in_ports = begin->getInPorts();
        list<Process::Port*>::iterator it;
        for (it = in_ports.begin(); it != in_ports.end(); ++it) {
            if ((*it)->isConnected()) {
                Process* next_process = (*it)->getConnectedPort()->getProcess();
                tools::append<ContainedSection>(
                    sections, findContainedSections(next_process, visited));
            }
        }
    }
    return sections;
}

bool ModelModifier::isAContainedSection(Process* start, Process* end)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!start) {
        THROW_EXCEPTION(InvalidArgumentException, "\"start\" must not be NULL");
    }
    if (!end) {
        THROW_EXCEPTION(InvalidArgumentException, "\"end\" must not be NULL");
    }

    set<ForSyDe::Id> visited;
    if (!checkDataFlowConvergence(start, end, visited, true)) {
        logger_.logDebugMessage(string("All flow from process \"")
                                + start->getId()->getString() + "\" does not "
                                "converge to process \""
                                + end->getId()->getString() + "\"");
        return false;
    }
    visited.clear();
    if (!checkDataFlowConvergence(start, end, visited, false)) {
        logger_.logDebugMessage(string("All flow to process \"")
                                + end->getId()->getString() + "\" does not "
                                "diverge from process \""
                                + start->getId()->getString() + "\"");
        return false;
    }
    return true;
}

bool ModelModifier::checkDataFlowConvergence(Process* start, Process* end,
                                             set<ForSyDe::Id>& visited,
                                             bool forward)
    throw(IOException, RuntimeException) {
    if (start == end) return true;

    if (forward) {
        if (visitProcess(visited, start)) {
            logger_.logDebugMessage(string("Analyzing process \"")
                                    + start->getId()->getString() + "\"...");

            // Check data flow from start to end
            list<Process::Port*> out_ports = start->getOutPorts();
            list<Process::Port*>::iterator it;
            for (it = out_ports.begin(); it != out_ports.end(); ++it) {
                if (!(*it)->isConnected()) return false;
                bool is_contained =
                    checkDataFlowConvergence((*it)->getConnectedPort()
                                             ->getProcess(), end, visited,
                                             true);
                if (!is_contained) return false;
            }
        }
    }
    else {
        if (visitProcess(visited, end)) {
            logger_.logDebugMessage(string("Analyzing process \"")
                                    + end->getId()->getString() + "\"...");

            // Check data flow from end to start
            list<Process::Port*> in_ports = end->getInPorts();
            list<Process::Port*>::iterator it;
            for (it = in_ports.begin(); it != in_ports.end(); ++it) {
                if (!(*it)->isConnected()) return false;
                if (!checkDataFlowConvergence(start, (*it)->getConnectedPort()
                                              ->getProcess(), visited, false)) {
                    return false;
                }
            }
        }
    }

    return true;
}

unzipx* ModelModifier::findNearestunzipxProcess(ForSyDe::Process* begin,
                                                    set<Id>& visited)
    throw(IOException, RuntimeException) {
    if (!begin) return NULL;
    if (visitProcess(visited, begin)) {
        logger_.logDebugMessage(string("Analyzing process \"")
                                + begin->getId()->getString() + "\"...");
        unzipx* sought_process = dynamic_cast<unzipx*>(begin);
        if (sought_process) return sought_process;

        list<Process::Port*> in_ports = begin->getInPorts();
        list<Process::Port*>::iterator it;
        for (it = in_ports.begin(); it != in_ports.end(); ++it) {
            if ((*it)->isConnected()) {
                Process* next_process = (*it)->getConnectedPort()->getProcess();
                sought_process = findNearestunzipxProcess(next_process,
                                                            visited);
                if (sought_process) return sought_process;
            }
        }
    }

    return NULL;
}

bool ModelModifier::isContainedSectionDataParallel(
    const ContainedSection& section) throw(IOException, RuntimeException) {
    logger_.logDebugMessage(string("Analyzing contained section ")
                            + section.toString() + "...");

    list<Process::Port*> ports = section.start->getOutPorts();
    list<Process::Port*>::iterator port_it;
    bool first = true;
    list<Process*> first_chain;
    for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
        logger_.logDebugMessage(string("Starting at port \"")
                                + (*port_it)->toString() + "\"");
        logger_.logDebugMessage("Getting process chain...");
        list<Process*> current_chain = getProcessChain(*port_it, section.end);
        if (!hasOnlycombSys(current_chain)) {
            logger_.logMessage(Logger::DEBUG,
                               string("Contained section ")
                               + section.toString() + " does not consist of "
                               + "only comb processes");
            return false;
        }
        if (first) {
            if (current_chain.size() == 0) {
                logger_.logMessage(Logger::DEBUG,
                                   string("No processes within the contained ")
                                   + "section " + section.toString());
                return false;
            }
            first_chain = current_chain;
            first = false;
        }
        else {
            logger_.logMessage(Logger::DEBUG,
                               string("Comparing process chains ")
                               + processChainToString(first_chain) 
                               + " and "
                               + processChainToString(current_chain) + "...");
            if (!areProcessChainsEqual(first_chain, current_chain)) {
                return false;
            }
        }
    }
    return true;
}

bool ModelModifier::hasOnlycombSys(std::list<ForSyDe::Process*> chain) const
    throw() {
    list<Process*>::const_iterator it;
    for (it = chain.begin(); it != chain.end(); ++it) {
        if (!dynamic_cast<comb*>(*it)) return false;
    }
    return true;
}

bool ModelModifier::areProcessChainsEqual(list<Process*> first,
                                          list<Process*> second)
    throw(IOException, RuntimeException) {
    if (first.size() != second.size()) {
        logger_.logMessage(Logger::INFO,
                           string("Process chains ")
                           + processChainToString(first) + " and "
                           + processChainToString(second)
                           + " are not of equal length");
        return false;
    }
    list<Process*>::iterator first_it;
    list<Process*>::iterator second_it;
    for (first_it = first.begin(), second_it = second.begin();
         first_it != first.end(); ++first_it, ++second_it) {
        if (**first_it != **second_it) {
            logger_.logMessage(Logger::INFO,
                               string("Processes \"")
                               + (*first_it)->getId()->getString() + "\" and \""
                               + (*second_it)->getId()->getString()
                               + "\" in chains "
                               + processChainToString(first) + " and "
                               + processChainToString(second)
                               + " are not equal");
            return false;
        }
    }

    return true;
}

list<Process*> ModelModifier::getProcessChain(Process::Port* start,
                                              Process* end)
    throw(OutOfMemoryException) {
    logger_.logDebugMessage(string("Getting process chain from \"")
                            + start->toString() + "\" to \"" + end->toString()
                            + "\"...");
    set<Id> visited;
    return getProcessChainR(start, end, visited);
}

list<Process*> ModelModifier::getProcessChainR(Process::Port* start,
                                               Process* end, set<Id>& visited)
    throw(OutOfMemoryException) {
    try {
        logger_.logDebugMessage("At \"" + start->toString() + "\"");

        list<Process*> chain;
        while (true) {
            if (!start->isConnected()) {
                logger_.logDebugMessage(string("\"") + start->toString()
                                        + "\" is not connected");
                break;
            }
            Process* next_process = start->getConnectedPort()->getProcess();
            logger_.logDebugMessage(string("Moved to process \"")
                                    + next_process->getId()->getString()
                                    + "\"");
            if (next_process == end) {
                logger_.logDebugMessage("Found end point");
                break;
            }
            if (visitProcess(visited, next_process)) {
                logger_.logDebugMessage(string("\"") + next_process->getId()
                                        ->getString() + "\" already visited");
                break;
            }
            chain.push_back(next_process);
            logger_.logDebugMessage(string("Pushed process \"")
                                    + next_process->getId()->getString()
                                    + "\" to chain");
            list<Process::Port*> output_ports = next_process->getOutPorts();
            list<Process::Port*>::iterator it;
            for (it = output_ports.begin(); it != output_ports.end(); ++it) {
                list<Process*> subchain = getProcessChainR(*it, end, visited);
                logger_.logDebugMessage(string("Found subchain: ")
                                        + processChainToString(subchain));
                tools::append(chain, subchain);
                logger_.logDebugMessage("Appended to chain");
            }
        }

        return chain;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException, "Failed to create process chain");
    }
}

list< list<ParallelMap*> > ModelModifier::findParallelMapSyChains()
    throw(IOException, RuntimeException) {
    list< list<ParallelMap*> > chains;
    set<Id> visited;
    list<Process::Port*> output_ports = processnetwork_->getOutputs();
    list<Process::Port*>::iterator it;
    for (it = output_ports.begin(); it != output_ports.end(); ++it) {
        logger_.logDebugMessage(string("Entering at output port \"")
                                + (*it)->toString() + "\"");
        tools::append< list<ParallelMap*> >(
            chains, findParallelMapSyChains((*it)->getProcess(), visited));
    }
    return chains;
}

list< list<ParallelMap*> > ModelModifier::findParallelMapSyChains(
    Process* begin, set<Id>& visited) throw(IOException, RuntimeException) {
    list< list<ParallelMap*> > chains;
    if (visitProcess(visited, begin)) {
        logger_.logDebugMessage(string("Analyzing process \"")
                                + begin->getId()->getString() + "\"...");

        // If this is a beginning of a chain, find the entire chain
        Process* continuation_point = begin;
        ParallelMap* parallelmapsy = dynamic_cast<ParallelMap*>(begin);
        if (parallelmapsy) {
            logger_.logDebugMessage(string("Found begin of chain at ")
                                    + "processes \""
                                    + begin->getId()->getString() + "\"");

            list<ParallelMap*> chain;
            while (parallelmapsy) {
                // Since we are searching the process network from outputs to inputs,
                // the process must be added to the process network of the list to avoid the
                // chain from being reversed
                chain.push_front(parallelmapsy);
                continuation_point = parallelmapsy;
                Process::Port* out_port = parallelmapsy->getInPorts().front();
                if (!out_port->isConnected()) break;
                Process* next_process = out_port->getConnectedPort()
                    ->getProcess();
                parallelmapsy = dynamic_cast<ParallelMap*>(next_process);
            }
            logger_.logDebugMessage(string("Chain ended at process ")
                                    + "\"" + chain.back()->getId()->getString()
                                    + "\"");
            chains.push_back(chain);

            logger_.logDebugMessage(string("ParallelMap process ")
                                    + "chain found: "
                                    + processChainToString(chain));
        }

        // Continue the search
        list<Process::Port*> in_ports = continuation_point->getInPorts();
        list<Process::Port*>::iterator it;
        for (it = in_ports.begin(); it != in_ports.end(); ++it) {
            if ((*it)->isConnected()) {
                Process* next_process = (*it)->getConnectedPort()->getProcess();
                tools::append< list<ParallelMap*> >(
                    chains, findParallelMapSyChains(next_process, visited));
            }
        }
    }
    return chains;
}

void ModelModifier::coalesceProcessChain(list<Process*> chain)
    throw(RuntimeException) {
    // Build function argument list
    list<CFunction> functions;
    for (list<Process*>::iterator it = chain.begin(); it != chain.end(); ++it) {
        comb* mapsy = dynamic_cast<comb*>(*it);
        if (!mapsy) THROW_EXCEPTION(CastException);
        functions.push_back(*mapsy->getFunction());
    }

    // Create new coalescedmap process
    CoalescedMap* new_process = new (std::nothrow) CoalescedMap(
        processnetwork_->getUniqueProcessId("_coalescedmap_"), Id("Process_Network"), functions);
    if (!new_process) THROW_EXCEPTION(OutOfMemoryException);
    
    redirectDataFlow(chain.front(), chain.back(), new_process, new_process);

    // Add new process to the process network
    if (processnetwork_->addProcess(new_process)) {
        logger_.logInfoMessage(string("Process chain ")
                               + processChainToString(chain)
                               + " replaced by new "
                               "process \""
                               + new_process->getId()->getString() + "\"");
    }
    else {
        THROW_EXCEPTION(IllegalStateException, string("Failed to create new ")
                        + "process: Process with ID \""
                        + new_process->getId()->getString()
                        + "\" already existed");
    }

    // Destroy and delete the section from the process network
    logger_.logDebugMessage(string("Destroying process chain ")
                            + processChainToString(chain) + "...");
    destroyProcessChain(chain.front());
}

bool ModelModifier::isParallelMapSyChainCoalescable(list<ParallelMap*> chain)
    throw(RuntimeException) {
    if (chain.size() <= 1) {
        logger_.logInfoMessage(string("ParallelMap chain ")
                               + processChainToString(chain)
                               + " only consists of one process - no "
                               + "process coalescing needed");
        return false;
    }

    list<ParallelMap*>::iterator it;
    bool first = true;
    int first_num_processes;
    CDataType prev_output_data_type;
    for (it = chain.begin(); it != chain.end(); ++it) {
        ParallelMap* current_process = *it;
        CFunction* function = current_process->getFunction();
        if (first) {
            first_num_processes = current_process->getNumProcesses();
            first = false;
        }
        else {
            // Check that number of processes are equal
            if (current_process->getNumProcesses() != first_num_processes) {
                logger_.logWarningMessage(string("Number of processes are not ")
                                          + "equal for all processes in "
                                          + "ParallelMap chain "
                                          + processChainToString(chain));
                return false;
            }

            // Check that the input data type of this process matches the
            // output data type of the previous process
            CDataType input_data_type = *function->getInputParameters().front()
                ->getDataType();
            input_data_type.setIsConst(false);
            if (input_data_type != prev_output_data_type) {
                logger_.logWarningMessage(string("Non-matching data types in ")
                                          + "ParallelMap chain "
                                          + processChainToString(chain));
                return false;
            }
        }

        // Set output data type for next iteration
        if (function->getNumInputParameters() == 1) {
            prev_output_data_type = *function->getReturnDataType();
        }
        else {
            prev_output_data_type = *function->getInputParameters().back()
                ->getDataType();
        }
    }

    // All tests passed
    return true;
}

void ModelModifier::coalesceParallelMapSyChain(list<ParallelMap*> chain)
    throw(RuntimeException) {
    // Build function argument list
    list<CFunction> functions;
    for (list<ParallelMap*>::iterator it = chain.begin(); it != chain.end();
         ++it) {
        functions.push_back(*(*it)->getFunction());
    }

    // Create new ParallelMap process
    int num_processes = chain.front()->getNumProcesses();
    ParallelMap* new_process = new (std::nothrow) ParallelMap(
        processnetwork_->getUniqueProcessId("_parallelmap_"), Id("Process_Network"), num_processes,
        functions);
    if (!new_process) THROW_EXCEPTION(OutOfMemoryException);
    
    redirectDataFlow(chain.front(), chain.back(), new_process, new_process);

    // Add new process to the process network
    if (processnetwork_->addProcess(new_process)) {
        logger_.logInfoMessage(string("Process chain ")
                               + processChainToString(chain)
                               + " replaced by new "
                               "process \""
                               + new_process->getId()->getString() + "\"");
    }
    else {
        THROW_EXCEPTION(IllegalStateException, string("Failed to create new ")
                        + "process: Process with ID \""
                        + new_process->getId()->getString()
                        + "\" already existed");
    }

    // Destroy and delete the section from the process network
    logger_.logDebugMessage(string("Destroying process chain ")
                            + processChainToString(chain) + "...");
    destroyProcessChain(chain.front());
}

string ModelModifier::processChainToString(list<Process*> chain) const throw() {
    string str;
    list<Process*>::iterator it;
    bool first = true;
    for (it = chain.begin(); it != chain.end(); ++it) {
        if (first) first = false;
        else       str += "--";
        str += string("\"") + (*it)->getId()->getString() + "\"";
    }
    return str;
}

string ModelModifier::processChainToString(list<ParallelMap*> chain)
    const throw() {
    list<Process*> new_list;
    list<ParallelMap*>::iterator it;
    for (it = chain.begin(); it != chain.end(); ++it) {
        new_list.push_back(*it);
    }
    return processChainToString(new_list);
}

void ModelModifier::destroyProcessChain(ForSyDe::Process* start)
    throw(InvalidArgumentException) {
    if (!start) {
        THROW_EXCEPTION(InvalidArgumentException, "\"start\" must not be NULL");
    }

    list<Process::Port*> ports = start->getOutPorts();
    list<Process::Port*>::iterator it;
    for (it = ports.begin(); it != ports.end(); ++it) {
        if ((*it)->isConnected()) {
            destroyProcessChain((*it)->getConnectedPort()->getProcess());
        }
    }
    processnetwork_->deleteProcess(*start->getId());
}

void ModelModifier::splitDataParallelSegments(
    vector< vector<ForSyDe::Process*> > chains)
    throw(IOException, RuntimeException) {
    try {
        size_t num_segments = chains.front().size();
        for (size_t current_segment = 1; current_segment < num_segments;
             ++current_segment) {
            logger_.logInfoMessage(string("Splitting process chains ")
                                   + "between positions "
                                   + tools::toString(current_segment - 1)
                                   + " and "
                                   + tools::toString(current_segment) + "...");

            // Create new processes zipx and unzipx
            zipx* new_zipx = new (std::nothrow) zipx(
                processnetwork_->getUniqueProcessId("_zipx_"), Id("Process_Network"));
            if (!new_zipx) THROW_EXCEPTION(OutOfMemoryException);
            logger_.logDebugMessage(string("New zipx process \"")
                                    + new_zipx->getId()->getString()
                                    + "\" created");
            unzipx* new_unzipx = new (std::nothrow) unzipx(
                processnetwork_->getUniqueProcessId("_unzipx_"), Id("Process_Network"));
            if (!new_unzipx) THROW_EXCEPTION(OutOfMemoryException);
            logger_.logDebugMessage(string("New unzipx process \"")
                                    + new_zipx->getId()->getString()
                                    + "\" created");

            // Connect the zipx to the unzipx
            if (!new_zipx->addOutPort(Id("out"))) {
                THROW_EXCEPTION(IllegalStateException, "Failed to add port");
            }
            if (!new_unzipx->addInPort(Id("in"))) {
                THROW_EXCEPTION(IllegalStateException, "Failed to add port");
            }
            new_zipx->getOutPort(Id("out"))->connect(
                new_unzipx->getInPort(Id("in")));
            logger_.logDebugMessage("Ports added");

            // Insert the zipx and unzipx process in between the current
            // data parallel segment
            for (size_t i = 0; i < chains.size(); ++i) {
                string num(tools::toString(i + 1));

                // Connect left map with zipx
                if (!new_zipx->addInPort(Id(string("in") + num))) {
                    THROW_EXCEPTION(IllegalStateException, "Failed to add "
                                    "port");
                }
                Process::Port* left_map_out_port = 
                    chains[i][current_segment - 1]->getOutPorts().front();
                Process::Port* zipx_in_port = new_zipx->getInPorts().back();
                logger_.logDebugMessage(string("Connecting \"")
                                        + left_map_out_port->toString()
                                        + "\" with \""
                                        + zipx_in_port->toString() + "\"...");
                left_map_out_port->connect(zipx_in_port);

                // Connect right map with unzipx
                if (!new_unzipx->addOutPort(Id(string("out") + num))) {
                    THROW_EXCEPTION(IllegalStateException, "Failed to add "
                                    "port");
                }
                Process::Port* right_map_in_port = 
                    chains[i][current_segment]->getInPorts().front();
                Process::Port* unzipx_out_port = 
                    new_unzipx->getOutPorts().back();
                logger_.logDebugMessage(string("Connecting \"")
                                        + right_map_in_port->toString()
                                        + "\" with \""
                                        + unzipx_out_port->toString()
                                        + "\"...");
                right_map_in_port->connect(unzipx_out_port);
            }

            // Add new processes to the process network
            if (!processnetwork_->addProcess(new_zipx)) {
                THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                                + "new process: Process with ID "
                                + "\"" + new_zipx->getId()->getString()
                                + "\" already existed");
            }
            if (!processnetwork_->addProcess(new_unzipx)) {
                THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                                + "new process: Process with ID "
                                + "\"" + new_unzipx->getId()->getString()
                                + "\" already existed");
            }

            logger_.logDebugMessage(string("New processes \"")
                                    + new_zipx->getId()->getString()
                                    + "\" and \""
                                    + new_unzipx->getId()->getString()
                                    + "\" added to the process network");
        }
    }
    catch (std::out_of_range&) {
        THROW_EXCEPTION(IndexOutOfBoundsException);
    }
}

ModelModifier::ContainedSection::ContainedSection(Process* start, Process* end)
        throw(InvalidArgumentException) : start(start), end(end) {
    if (!start) {
        THROW_EXCEPTION(InvalidArgumentException, "\"start\" must not be NULL");
    }
    if (!end) {
        THROW_EXCEPTION(InvalidArgumentException, "\"end\" must not be NULL");
    }
}

string ModelModifier::ContainedSection::toString() const throw() {
    return string("\"") + start->getId()->getString() + "--" + 
        end->getId()->getString() + "\"";
}

void ModelModifier::redirectDataFlow(Process* old_start, Process* old_end,
                                     Process* new_start, Process* new_end)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!old_start) {
        THROW_EXCEPTION(InvalidArgumentException, "\"old_start\" must not be "
                        "NULL");
    }
    if (!old_end) {
        THROW_EXCEPTION(InvalidArgumentException, "\"old_end\" must not be "
                        "NULL");
    }
    if (!new_start) {
        THROW_EXCEPTION(InvalidArgumentException, "\"new_start\" must not be "
                        "NULL");
    }
    if (!new_end) {
        THROW_EXCEPTION(InvalidArgumentException, "\"new_end\" must not be "
                        "NULL");
    }

    string message("Redirecting data flow through process(es) ");
    if (old_start == old_end) {
        message += string("\"") + old_start->getId()->getString() + "\"";
    }
    else {
        message += string("\"") + old_start->getId()->getString() + "\" and \""
            + old_end->getId()->getString() + "\"";
    }
    message += " to process(es) ";
    if (new_start == new_end) {
        message += string("\"") + new_start->getId()->getString() + "\"";
    }
    else {
        message += string("\"") + new_start->getId()->getString() + "\" and \""
            + new_end->getId()->getString() + "\"";
    }
    logger_.logInfoMessage(message);

    // Add in ports of old_start to the new_start
    logger_.logDebugMessage(string("Adding in ports from process \"")
                            + old_start->getId()->getString()
                            + "\" to process \""
                            + new_start->getId()->getString() + "\"");
    list<Process::Port*> in_ports = old_start->getInPorts();
    list<Process::Port*>::iterator it;
    for (it = in_ports.begin(); it != in_ports.end(); ++it) {
        if (!new_start->addInPort(**it)) {
            THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                            + "in port \"" + (*it)->toString() + "\" to "
                            + "process \"" + new_start->getId()->getString()
                            + "\"");
        }
        replaceProcessnetworkInput(*it, new_start->getInPorts().back());
    }

    // Add out ports of old_end to the new_end
    logger_.logDebugMessage(string("Adding out ports from process \"")
                            + old_end->getId()->getString() + "\" to process \""
                            + new_end->getId()->getString() + "\"");
    list<Process::Port*> out_ports = old_end->getOutPorts();
    for (it = out_ports.begin(); it != out_ports.end(); ++it) {
        if (!new_end->addOutPort(**it)) {
            THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                            + "out port \"" + (*it)->toString() + "\" to "
                            + "process \"" + new_start->getId()->getString()
                            + "\"");
        }
        replaceProcessnetworkOutput(*it, new_end->getOutPorts().back());
    }
}

void ModelModifier::replaceProcessnetworkInput(Process::Port* old_port,
                                      Process::Port* new_port)
    throw(RuntimeException) {
    list<Process::Port*> inputs(processnetwork_->getInputs());
    list<Process::Port*>::iterator it;
    for (it = inputs.begin(); it != inputs.end(); ++it) {
        if (*it == old_port) {
            processnetwork_->deleteInput(*it);
            processnetwork_->addInput(new_port);
            break;
        }
    }
}

void ModelModifier::replaceProcessnetworkOutput(Process::Port* old_port,
                                       Process::Port* new_port)
    throw(RuntimeException) {
    list<Process::Port*> outputs(processnetwork_->getOutputs());
    list<Process::Port*>::iterator it;
    for (it = outputs.begin(); it != outputs.end(); ++it) {
        if (*it == old_port) {
            processnetwork_->deleteOutput(*it);
            processnetwork_->addOutput(new_port);
            break;
        }
    }
}

bool ModelModifier::visitProcess(set<Id>& visited, Process* process)
    throw(RuntimeException) {
    return visited.insert(*process->getId()).second;
}
