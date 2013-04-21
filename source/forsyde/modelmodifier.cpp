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

#include "processnetworkmodifier.h"
#include "zipxsy.h"
#include "unzipxsy.h"
#include "parallelmapsy.h"
#include "coalescedmapsy.h"
#include "zipwithnsy.h"
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
using std::string;
using std::list;
using std::set;
using std::vector;
using std::bad_alloc;
using std::pair;

ProcessNetworkModifier::ProcessNetworkModifier(ProcessNetwork* processnetwork, Logger& logger)
        throw(InvalidArgumentException) : processnetwork_(processnetwork), logger_(logger) {
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }
}

ProcessNetworkModifier::~ProcessNetworkModifier() throw() {}

void ProcessNetworkModifier::coalesceDataParallelLeafs()
    throw(IOException, RuntimeException) {
    list<ContainedSection> sections = findDataParallelSections();
    list<ContainedSection>::iterator it;
    for (it = sections.begin(); it != sections.end(); ++it) {
        list<Leaf::Port*> ports = it->start->getOutPorts();
        list<Leaf::Port*>::iterator port_it;
        for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
            list<Leaf*> chain = getProcessChain(*port_it, it->end);
            if (chain.size() > 1) {
                logger_.logMessage(Logger::INFO, string("Coalescing leaf ")
                                   + "chain " + leafChainToString(chain)
                                   + "...");
                coalesceLeafChain(chain);
            }
            else {
                logger_.logMessage(Logger::INFO, string("Data parallel ")
                                   + "section " + it->toString()
                                   + " only consists of one segment - no "
                                   + "leaf coalescing needed");
                break;
            }
        }
    }
}

void ProcessNetworkModifier::coalesceParallelMapSyLeafs()
    throw(IOException, RuntimeException) {
    list< list<ParallelMap*> > chains = findParallelMapSyChains();
    if (chains.size() == 0) {
        logger_.logMessage(Logger::INFO, "No ParallelMap chains found");
        return;
    }

    list< list<ParallelMap*> >::iterator it;
    for (it = chains.begin(); it != chains.end(); ++it) {
        if (!isParallelMapSyChainCoalescable(*it)) continue;
        logger_.logMessage(Logger::INFO, string("Coalescing leaf chain ")
                           + leafChainToString(*it) + "...");
        coalesceParallelMapSyChain(*it);
    }
}

void ProcessNetworkModifier::splitDataParallelSegments()
    throw(IOException, RuntimeException) {
    list<ContainedSection> sections = findDataParallelSections();
    list<ContainedSection>::iterator it;
    for (it = sections.begin(); it != sections.end(); ++it) {
        bool aborted = false;
        vector< vector<Leaf*> > chains;
        list<Leaf::Port*> ports = it->start->getOutPorts();
        list<Leaf::Port*>::iterator port_it;
        for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
            list<Leaf*> chain = getProcessChain(*port_it, it->end);
            if (chain.size() <= 1) {
                logger_.logMessage(Logger::INFO, string("Data parallel ")
                                   + "section " + it->toString()
                                   + " only consists of one segment - no "
                                   + "splitting needed");
                aborted = true;
                break;
            }
            vector<Leaf*> chain_as_vector(chain.begin(), chain.end());
            chains.push_back(chain_as_vector);
        }
        if (!aborted) {
            logger_.logMessage(Logger::INFO, string("Splitting segments in ")
                               + "section " + it->toString() + "...");
            splitDataParallelSegments(chains);
        }
    }
}

void ProcessNetworkModifier::fuseUnzipMapZipLeafs()
    throw(IOException, RuntimeException) {
    list<ContainedSection> sections = findDataParallelSections();
    list<ContainedSection>::iterator it;
    for (it = sections.begin(); it != sections.end(); ++it) {
        ContainedSection section = *it;
        logger_.logMessage(Logger::INFO, string("Fusing data parallel section ")
                           + section.toString() + "...");

        // Get function arguments of mapSY or coalescedmapSY leafs
        if (getProcessChain(section.start->getOutPorts().front(),
                            section.end).size() != 1) {
            THROW_EXCEPTION(IllegalStateException, "Leaf chain is not of "
                            "length 1");
        }
        Leaf* data_leaf = section.start->getOutPorts().front()
            ->getConnectedPort()->getProcess();
        list<CFunction> functions;
        CoalescedMap* cmapsy_leaf =
            dynamic_cast<CoalescedMap*>(data_leaf);
        if (cmapsy_leaf) {
            list<CFunction*> functions_to_copy = cmapsy_leaf->getFunctions();
            list<CFunction*>::iterator func_it;
            for (func_it = functions_to_copy.begin();
                 func_it != functions_to_copy.end(); ++func_it) {
                functions.push_back(**func_it);
            }
        }
        else {
            Map* mapsy_leaf = dynamic_cast<Map*>(data_leaf);
            if (!mapsy_leaf) THROW_EXCEPTION(CastException);
            functions.push_back(*mapsy_leaf->getFunction());
        }

        // Create new parallelmapSY leaf to replace the data parallel section
        int num_leafs = section.start->getOutPorts().size();
        ParallelMap* new_leaf = new (std::nothrow) ParallelMap(
            processnetwork_->getUniqueProcessId("_parallelmapSY_"), num_leafs,
            functions);
        if (!new_leaf) THROW_EXCEPTION(OutOfMemoryException);
        logger_.logMessage(Logger::DEBUG, string("New ParallelMap leaf \"")
                           + new_leaf->getId()->getString() + "\" created");

        redirectDataFlow(section.start, section.end, new_leaf, new_leaf);

        // Add new leaf to the processnetwork
        if (processnetwork_->addProcess(new_leaf)) {
            logger_.logMessage(Logger::INFO, string("Data parallel section ")
                               + section.toString() + " replaced by new "
                               "leaf \""
                               + new_leaf->getId()->getString() + "\"");
        }
        else {
            THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                            + "new leaf: Leaf with ID \""
                            + new_leaf->getId()->getString()
                            + "\" already existed");
        }

        // Destroy and delete the section from the processnetwork
        logger_.logMessage(Logger::DEBUG, string("Destroying section \"")
                           + section.toString() + "...");
        destroyLeafChain(section.start);
    }
}

void ProcessNetworkModifier::convertZipWith1ToMap()
    throw(IOException, RuntimeException) {
    list<Leaf*> leafs = processnetwork_->getProcesses();
    list<Leaf*>::iterator it;
    for (it = leafs.begin(); it != leafs.end(); ++it) {
        logger_.logMessage(Logger::DEBUG, string("Analyzing leaf \"")
                           + (*it)->getId()->getString() + "\"...");

        ZipWithNSY* leaf = dynamic_cast<ZipWithNSY*>(*it);
        if (leaf && leaf->getNumInPorts() == 1) {
            Map* new_leaf = new (std::nothrow) Map(
                processnetwork_->getUniqueProcessId("_mapSY_"), *leaf->getFunction());
            if (!new_leaf) THROW_EXCEPTION(OutOfMemoryException);
            logger_.logMessage(Logger::DEBUG, string("New Map leaf \"")
                               + new_leaf->getId()->getString()
                               + "\" created");

            redirectDataFlow(leaf, leaf, new_leaf, new_leaf);

            // Add new leaf to the processnetwork
            if (processnetwork_->addProcess(new_leaf)) {
                logger_.logMessage(Logger::INFO, string("Leaf \"")
                                   + leaf->getId()->getString() + "\" "
                                   + "replaced by new leaf \""
                                   + new_leaf->getId()->getString() + "\"");
            }
            else {
                THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                                + "new leaf: Leaf with ID \""
                                + new_leaf->getId()->getString()
                                + "\" already existed");
            }

            // Destroy and delete the old leaf from the processnetwork
            logger_.logMessage(Logger::DEBUG, string("Destroying leaf \"")
                               + leaf->getId()->getString() + "...");
            processnetwork_->deleteProcess(*leaf->getId());
        }
    }
}

void ProcessNetworkModifier::removeRedundantLeafs()
    throw(IOException, RuntimeException) {
    list<Leaf*> leafs = processnetwork_->getProcesses();
    list<Leaf*>::iterator it;
    for (it = leafs.begin(); it != leafs.end(); ++it) {
        Leaf* leaf = *it;
        logger_.logMessage(Logger::DEBUG, string("Analyzing leaf \"")
                           + leaf->getId()->getString() + "\"...");

        // Remove zipx and unzipx leafs which have only one in and out
        // port
        bool is_zipxsy = dynamic_cast<zipx*>(leaf);
        bool is_unzipxsy = !is_zipxsy && dynamic_cast<unzipx*>(leaf);
        if (is_zipxsy || is_unzipxsy) {
            if (leaf->getNumInPorts() == 1
                && leaf->getNumOutPorts() == 1) {
                string leaf_name = leaf->getId()->getString();

                // Redirect the leaf' in and out ports to unconnect it from
                // the network
                Leaf::Port* in_port = 
                    leaf->getInPorts().front();
                Leaf::Port* out_port = 
                    leaf->getOutPorts().front();
                Leaf::Port* other_end_at_in_port =
                    in_port->getConnectedPort();
                Leaf::Port* other_end_at_out_port =
                    out_port->getConnectedPort();
                if (other_end_at_in_port && other_end_at_out_port) {
                    other_end_at_in_port->connect(other_end_at_out_port);
                }

                // Update processnetwork in- and ouputs, if necessary
                logger_.logMessage(Logger::DEBUG, "Updating processnetwork in- and "
                                   "outputs...");
                if (other_end_at_in_port == NULL) {
                    replaceProcessNetworkInput(in_port,
                                      other_end_at_out_port);
                }
                if (other_end_at_out_port == NULL) {
                    replaceProcessNetworkOutput(out_port,
                                      other_end_at_in_port);
                }

                // Delete leaf from processnetwork
                if (!processnetwork_->deleteProcess(*leaf->getId())) {
                    THROW_EXCEPTION(IllegalStateException, string("Could not ") 
                                    + "delete leaf \"" + leaf_name
                                    + "\"");
                }
                
                if (is_zipxsy) {
                    logger_.logMessage(Logger::INFO, string("Removed ")
                                       + "redundant zipx leaf \""
                                       + leaf_name + "\" (had only 1 in "
                                       + "port)");
                }
                else {
                    logger_.logMessage(Logger::INFO, string("Removed ")
                                       + "redundant unzipx leaf \""
                                       + leaf_name + "\" (had only 1 out "
                                       + "port)");
                }
            }
        }
    }
}

list<ProcessNetworkModifier::ContainedSection> ProcessNetworkModifier::findDataParallelSections()
    throw(IOException, RuntimeException) {
    list<ContainedSection> sections;

    // Find contained sections sections
    logger_.logMessage(Logger::INFO, "Searching for contained sections...");
    sections = findContainedSections();
    if (sections.size() == 0) {
        logger_.logMessage(Logger::INFO, "No contained (and thus no data "
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
    logger_.logMessage(Logger::INFO, message);

    // Check which sections are data parallel and remove those that aren't
    for (it = sections.begin(); it != sections.end(); ) {
        if(isContainedSectionDataParallel(*it)) {
            logger_.logMessage(Logger::INFO, it->toString()
                               + " is data parallel");
            ++it;
        }
        else {
            logger_.logMessage(Logger::INFO, it->toString()
                               + " is not data parallel");
            it = sections.erase(it);
        }
    }

    return sections;
}

list<ProcessNetworkModifier::ContainedSection>
ProcessNetworkModifier::findContainedSections() throw(IOException, RuntimeException) {
    list<ContainedSection> sections;
    set<Id> visited;
    list<Leaf::Port*> output_ports = processnetwork_->getOutputs();
    list<Leaf::Port*>::iterator it;
    for (it = output_ports.begin(); it != output_ports.end(); ++it) {
        logger_.logMessage(Logger::DEBUG, string("Entering at output port \"")
                           + (*it)->toString() + "\"");
        tools::append<ContainedSection>(sections, findContainedSections(
                                            (*it)->getProcess(), visited));
    }
    return sections;
}

list<ProcessNetworkModifier::ContainedSection>
ProcessNetworkModifier::findContainedSections(Leaf* begin, set<Id> visited)
    throw(IOException, RuntimeException) {
    list<ContainedSection> sections;
    bool not_already_visited = visited.insert(*begin->getId()).second;
    if (not_already_visited) {
        logger_.logMessage(Logger::DEBUG, string("Analyzing leaf \"")
                           + begin->getId()->getString() + "\"...");
        zipx* converge_point = dynamic_cast<zipx*>(begin);
        if (converge_point) {
            logger_.logMessage(Logger::DEBUG, string("Discovered zipxSY ")
                               + "leaf \""
                               + converge_point->getId()->getString() + "\"");
            logger_.logMessage(Logger::DEBUG, "Searching for nearest unzipxSY "
                               "leaf...");
            unzipx* diverge_point =
                findNearestunzipxLeaf(converge_point);
            if (diverge_point) {
                logger_.logMessage(Logger::DEBUG, string("Found nearest ")
                                   + "unzipxSY leaf \""
                                   + diverge_point->getId()->getString()
                                   + "\"");
            }
            else {
                logger_.logMessage(Logger::DEBUG, "No unzipxSY leaf found");
                // Return empty list
                return sections;
            }
            logger_.logMessage(Logger::DEBUG, string("Checking that the data ")
                               + "flow between leafs \""
                               + diverge_point->getId()->getString()
                               + "\" and \""
                               + converge_point->getId()->getString()
                               + "\" is contained...");
            if (!isAContainedSection(diverge_point, converge_point)) {
                logger_.logMessage(Logger::DEBUG, string("Section between ")
                                   + "leafs \""
                                   + diverge_point->getId()->getString()
                                   + "\" and \""
                                   + converge_point->getId()->getString()
                                   + "\" is not contained");
                goto continue_search;
            }

            logger_.logMessage(Logger::DEBUG, string("Found contained section ")
                               + "between leafs \""
                               + diverge_point->getId()->getString()
                               + "\" and \""
                               + converge_point->getId()->getString()
                               + "\"");
            sections.push_back(ContainedSection(diverge_point,
                                                converge_point));
            // The converge point need not be set as visited since it is only
            // reachable by an already visited leaf
            tools::append<ContainedSection>(
                sections, findContainedSections(diverge_point, visited));
            return sections;
        }

      continue_search:
        list<Leaf::Port*> in_ports = begin->getInPorts();
        list<Leaf::Port*>::iterator it;
        for (it = in_ports.begin(); it != in_ports.end(); ++it) {
            if ((*it)->isConnected()) {
                Leaf* next_leaf = (*it)->getConnectedPort()->getProcess();
                tools::append<ContainedSection>(
                    sections, findContainedSections(next_leaf, visited));
            }
        }
    }
    return sections;
}

bool ProcessNetworkModifier::isAContainedSection(Leaf* start, Leaf* end)
    throw(InvalidArgumentException, IOException, RuntimeException) {
    if (!start) {
        THROW_EXCEPTION(InvalidArgumentException, "\"start\" must not be NULL");
    }
    if (!end) {
        THROW_EXCEPTION(InvalidArgumentException, "\"end\" must not be NULL");
    }

    if (!checkDataFlowConvergence(start, end, true)) {
        logger_.logMessage(Logger::DEBUG, string("All flow from leaf \"")
                           + start->getId()->getString() + "\" does not "
                           "converge to leaf \""
                           + end->getId()->getString() + "\"");
        return false;
    }
    if (!checkDataFlowConvergence(start, end, false)) {
        logger_.logMessage(Logger::DEBUG, string("All flow to leaf \"")
                           + end->getId()->getString() + "\" does not "
                           "diverge from leaf \""
                           + start->getId()->getString() + "\"");
        return false;
    }
    return true;
}

bool ProcessNetworkModifier::checkDataFlowConvergence(Leaf* start, Leaf* end,
                                             bool forward)
    throw(IOException, RuntimeException) {
    if (start == end) return true;

    if (forward) {
        logger_.logMessage(Logger::DEBUG, string("Analyzing leaf \"")
                           + start->getId()->getString() + "\"...");

        // Check data flow from start to end
        list<Leaf::Port*> out_ports = start->getOutPorts();
        list<Leaf::Port*>::iterator it;
        for (it = out_ports.begin(); it != out_ports.end(); ++it) {
            if (!(*it)->isConnected()) return false;
            if (!checkDataFlowConvergence((*it)->getConnectedPort()
                                          ->getProcess(), end, true)) {
                return false;
            }
        }
    }
    else {
        logger_.logMessage(Logger::DEBUG, string("Analyzing leaf \"")
                           + end->getId()->getString() + "\"...");

        // Check data flow from end to start
        list<Leaf::Port*> in_ports = end->getInPorts();
        list<Leaf::Port*>::iterator it;
        for (it = in_ports.begin(); it != in_ports.end(); ++it) {
            if (!(*it)->isConnected()) return false;
            if (!checkDataFlowConvergence(start, (*it)->getConnectedPort()
                                          ->getProcess(), false)) {
                return false;
            }
        }
    }

    return true;
}

unzipx* ProcessNetworkModifier::findNearestunzipxLeaf(ForSyDe::Leaf* begin)
    throw(IOException, RuntimeException) {
    if (!begin) return NULL;

    logger_.logMessage(Logger::DEBUG, string("Analyzing leaf \"")
                       + begin->getId()->getString() + "\"...");
    unzipx* sought_leaf = dynamic_cast<unzipx*>(begin);
    if (sought_leaf) return sought_leaf;

    list<Leaf::Port*> in_ports = begin->getInPorts();
    list<Leaf::Port*>::iterator it;
    for (it = in_ports.begin(); it != in_ports.end(); ++it) {
        if ((*it)->isConnected()) {
            Leaf* next_leaf = (*it)->getConnectedPort()->getProcess(); 
            sought_leaf = findNearestunzipxLeaf(next_leaf);
            if (sought_leaf) return sought_leaf;
        }
    }

    return NULL;
}

bool ProcessNetworkModifier::isContainedSectionDataParallel(
    const ContainedSection& section) throw(IOException, RuntimeException) {
    list<Leaf::Port*> ports = section.start->getOutPorts();
    list<Leaf::Port*>::iterator port_it;
    bool first = true;
    list<Leaf*> first_chain;
    for (port_it = ports.begin(); port_it != ports.end(); ++port_it) {
        list<Leaf*> current_chain = getProcessChain(*port_it, section.end);
        if (!hasOnlyMapSys(current_chain)) {
            logger_.logMessage(Logger::DEBUG,
                               string("Contained section ")
                               + section.toString() + " does not consist of "
                               + "only Map leafs");
            return false;
        }
        if (first) {
            if (current_chain.size() == 0) {
                logger_.logMessage(Logger::DEBUG,
                                   string("No leafs within the contained ")
                                   + "section " + section.toString());
                return false;
            }
            first_chain = current_chain;
            first = false;
        }
        else {
            logger_.logMessage(Logger::DEBUG,
                               string("Comparing leaf chains ")
                               + leafChainToString(first_chain) 
                               + " and "
                               + leafChainToString(current_chain) + "...");
            if (!areLeafChainsEqual(first_chain, current_chain)) {
                return false;
            }
        }
    }
    return true;
}

bool ProcessNetworkModifier::hasOnlyMapSys(std::list<ForSyDe::Leaf*> chain) const
    throw() {
    list<Leaf*>::const_iterator it;
    for (it = chain.begin(); it != chain.end(); ++it) {
        if (!dynamic_cast<Map*>(*it)) return false;
    }
    return true;
}

bool ProcessNetworkModifier::areLeafChainsEqual(list<Leaf*> first,
                                          list<Leaf*> second)
    throw(IOException, RuntimeException) {
    if (first.size() != second.size()) {
        logger_.logMessage(Logger::INFO,
                           string("Leaf chains ")
                           + leafChainToString(first) + " and "
                           + leafChainToString(second)
                           + " are not of equal length");
        return false;
    }
    list<Leaf*>::iterator first_it;
    list<Leaf*>::iterator second_it;
    for (first_it = first.begin(), second_it = second.begin();
         first_it != first.end(); ++first_it, ++second_it) {
        if (**first_it != **second_it) {
            logger_.logMessage(Logger::INFO,
                               string("Leafs \"")
                               + (*first_it)->getId()->getString() + "\" and \""
                               + (*second_it)->getId()->getString()
                               + "\" in chains "
                               + leafChainToString(first) + " and "
                               + leafChainToString(second)
                               + " are not equal");
            return false;
        }
    }

    return true;
}

list<Leaf*> ProcessNetworkModifier::getProcessChain(Leaf::Port* start,
                                              Leaf* end)
    throw(OutOfMemoryException) {
    try {
        list<Leaf*> chain;
        Leaf::Port* port = start;
        while (true) {
            if (!port->isConnected()) break;
            Leaf* next_leaf = port->getConnectedPort()->getProcess();
            if (next_leaf == end) break;
            chain.push_back(next_leaf);
            port = next_leaf->getOutPorts().front();
        }

        return chain;
    }
    catch (bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException, "Failed to create leaf chain");
    }
}

list< list<ParallelMap*> > ProcessNetworkModifier::findParallelMapSyChains()
    throw(IOException, RuntimeException) {
    list< list<ParallelMap*> > chains;
    set<Id> visited;
    list<Leaf::Port*> output_ports = processnetwork_->getOutputs();
    list<Leaf::Port*>::iterator it;
    for (it = output_ports.begin(); it != output_ports.end(); ++it) {
        logger_.logMessage(Logger::DEBUG, string("Entering at output port \"")
                           + (*it)->toString() + "\"");
        tools::append< list<ParallelMap*> >(
            chains, findParallelMapSyChains((*it)->getProcess(), visited));
    }
    return chains;
}

list< list<ParallelMap*> > ProcessNetworkModifier::findParallelMapSyChains(
    Leaf* begin, set<Id> visited) throw(IOException, RuntimeException) {
    list< list<ParallelMap*> > chains;
    bool not_already_visited = visited.insert(*begin->getId()).second;
    if (not_already_visited) {
        logger_.logMessage(Logger::DEBUG, string("Analyzing leaf \"")
                           + begin->getId()->getString() + "\"...");

        // If this is a beginning of a chain, find the entire chain
        Leaf* continuation_point = begin;
        ParallelMap* parallelmapsy = dynamic_cast<ParallelMap*>(begin);
        if (parallelmapsy) {
            logger_.logMessage(Logger::DEBUG, string("Found begin of chain at ")
                               + "leafs \""
                               + begin->getId()->getString() + "\"");

            list<ParallelMap*> chain;
            while (parallelmapsy) {
                // Since we are searching the processnetwork from outputs to inputs,
                // the leaf must be added to the top of the list to avoid the
                // chain from being reversed
                chain.push_front(parallelmapsy);
                continuation_point = parallelmapsy;
                Leaf::Port* out_port = parallelmapsy->getInPorts().front();
                if (!out_port->isConnected()) break;
                Leaf* next_leaf = out_port->getConnectedPort()
                    ->getProcess();
                parallelmapsy = dynamic_cast<ParallelMap*>(next_leaf);
            }
            logger_.logMessage(Logger::DEBUG, string("Chain ended at leaf ")
                               + "\"" + chain.back()->getId()->getString()
                               + "\"");
            chains.push_back(chain);

            logger_.logMessage(Logger::DEBUG, string("ParallelMap leaf ")
                               + "chain found: " + leafChainToString(chain));
        }

        // Continue the search
        list<Leaf::Port*> in_ports = continuation_point->getInPorts();
        list<Leaf::Port*>::iterator it;
        for (it = in_ports.begin(); it != in_ports.end(); ++it) {
            if ((*it)->isConnected()) {
                Leaf* next_leaf = (*it)->getConnectedPort()->getProcess();
                tools::append< list<ParallelMap*> >(
                    chains, findParallelMapSyChains(next_leaf, visited));
            }
        }
    }
    return chains;
}

void ProcessNetworkModifier::coalesceLeafChain(list<Leaf*> chain)
    throw(RuntimeException) {
    // Build function argument list
    list<CFunction> functions;
    for (list<Leaf*>::iterator it = chain.begin(); it != chain.end(); ++it) {
        Map* mapsy = dynamic_cast<Map*>(*it);
        if (!mapsy) THROW_EXCEPTION(CastException);
        functions.push_back(*mapsy->getFunction());
    }

    // Create new coalescedmapSY leaf
    CoalescedMap* new_leaf = new (std::nothrow) CoalescedMap(
        processnetwork_->getUniqueProcessId("_coalescedmapSY_"), functions);
    if (!new_leaf) THROW_EXCEPTION(OutOfMemoryException);
    
    redirectDataFlow(chain.front(), chain.back(), new_leaf, new_leaf);

    // Add new leaf to the processnetwork
    if (processnetwork_->addProcess(new_leaf)) {
        logger_.logMessage(Logger::INFO, string("Leaf chain ")
                           + leafChainToString(chain)
                           + " replaced by new "
                           "leaf \""
                           + new_leaf->getId()->getString() + "\"");
    }
    else {
        THROW_EXCEPTION(IllegalStateException, string("Failed to create new ")
                        + "leaf: Leaf with ID \""
                        + new_leaf->getId()->getString()
                        + "\" already existed");
    }

    // Destroy and delete the section from the processnetwork
    logger_.logMessage(Logger::DEBUG, string("Destroying leaf chain ")
                       + leafChainToString(chain) + "...");
    destroyLeafChain(chain.front());
}

bool ProcessNetworkModifier::isParallelMapSyChainCoalescable(list<ParallelMap*> chain)
    throw(RuntimeException) {
    if (chain.size() <= 1) {
        logger_.logMessage(Logger::INFO, string("ParallelMap chain ")
                           + leafChainToString(chain)
                           + " only consists of one leaf - no "
                           + "leaf coalescing needed");
        return false;
    }

    list<ParallelMap*>::iterator it;
    bool first = true;
    int first_num_leafs;
    CDataType prev_output_data_type;
    for (it = chain.begin(); it != chain.end(); ++it) {
        ParallelMap* current_leaf = *it;
        CFunction* function = current_leaf->getFunction();
        if (first) {
            first_num_leafs = current_leaf->getNumProcesses();
            first = false;
        }
        else {
            // Check that number of leafs are equal
            if (current_leaf->getNumProcesses() != first_num_leafs) {
                logger_.logMessage(Logger::WARNING,
                                   string("Number of leafs are not equal ")
                                   + "for all leafs in  ParallelMap "
                                   + "chain " + leafChainToString(chain));
                return false;
            }

            // Check that the input data type of this leaf matches the
            // output data type of the previous leaf
            CDataType input_data_type = *function->getInputParameters().front()
                ->getDataType();
            input_data_type.setIsConst(false);
            if (input_data_type != prev_output_data_type) {
                logger_.logMessage(Logger::WARNING,
                                   string("Non-matching data types in "
                                          "ParallelMap chain ")
                                   + leafChainToString(chain));
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

void ProcessNetworkModifier::coalesceParallelMapSyChain(list<ParallelMap*> chain)
    throw(RuntimeException) {
    // Build function argument list
    list<CFunction> functions;
    for (list<ParallelMap*>::iterator it = chain.begin(); it != chain.end();
         ++it) {
        functions.push_back(*(*it)->getFunction());
    }

    // Create new ParallelMap leaf
    int num_leafs = chain.front()->getNumProcesses();
    ParallelMap* new_leaf = new (std::nothrow) ParallelMap(
        processnetwork_->getUniqueProcessId("_parallelmapSY_"), num_leafs,
        functions);
    if (!new_leaf) THROW_EXCEPTION(OutOfMemoryException);
    
    redirectDataFlow(chain.front(), chain.back(), new_leaf, new_leaf);

    // Add new leaf to the processnetwork
    if (processnetwork_->addProcess(new_leaf)) {
        logger_.logMessage(Logger::INFO, string("Leaf chain ")
                           + leafChainToString(chain)
                           + " replaced by new "
                           "leaf \""
                           + new_leaf->getId()->getString() + "\"");
    }
    else {
        THROW_EXCEPTION(IllegalStateException, string("Failed to create new ")
                        + "leaf: Leaf with ID \""
                        + new_leaf->getId()->getString()
                        + "\" already existed");
    }

    // Destroy and delete the section from the processnetwork
    logger_.logMessage(Logger::DEBUG, string("Destroying leaf chain ")
                       + leafChainToString(chain) + "...");
    destroyLeafChain(chain.front());
}

string ProcessNetworkModifier::leafChainToString(list<Leaf*> chain) const throw() {
    string str;
    list<Leaf*>::iterator it;
    bool first = true;
    for (it = chain.begin(); it != chain.end(); ++it) {
        if (first) first = false;
        else       str += "--";
        str += string("\"") + (*it)->getId()->getString() + "\"";
    }
    return str;
}

string ProcessNetworkModifier::leafChainToString(list<ParallelMap*> chain)
    const throw() {
    list<Leaf*> new_list;
    list<ParallelMap*>::iterator it;
    for (it = chain.begin(); it != chain.end(); ++it) {
        new_list.push_back(*it);
    }
    return leafChainToString(new_list);
}

void ProcessNetworkModifier::destroyLeafChain(ForSyDe::Leaf* start)
    throw(InvalidArgumentException) {
    if (!start) {
        THROW_EXCEPTION(InvalidArgumentException, "\"start\" must not be NULL");
    }

    list<Leaf::Port*> ports = start->getOutPorts();
    list<Leaf::Port*>::iterator it;
    for (it = ports.begin(); it != ports.end(); ++it) {
        if ((*it)->isConnected()) {
            destroyLeafChain((*it)->getConnectedPort()->getProcess());
        }
    }
    processnetwork_->deleteProcess(*start->getId());
}

void ProcessNetworkModifier::splitDataParallelSegments(
    vector< vector<ForSyDe::Leaf*> > chains)
    throw(IOException, RuntimeException) {
    try {
        size_t num_segments = chains.front().size();
        for (size_t current_segment = 1; current_segment < num_segments;
             ++current_segment) {
            logger_.logMessage(Logger::INFO, string("Splitting leaf chains ")
                               + "between positions "
                               + tools::toString(current_segment - 1) + " and "
                               + tools::toString(current_segment) + "...");

            // Create new leafs zipxSY and unzipxSY
            zipx* new_zipxSY = new (std::nothrow) zipx(
                processnetwork_->getUniqueProcessId("_zipxSY_"));
            if (!new_zipxSY) THROW_EXCEPTION(OutOfMemoryException);
            logger_.logMessage(Logger::DEBUG, string("New zipx leaf \"")
                               + new_zipxSY->getId()->getString()
                               + "\" created");
            unzipx* new_unzipxSY = new (std::nothrow) unzipx(
                processnetwork_->getUniqueProcessId("_unzipxSY_"));
            if (!new_unzipxSY) THROW_EXCEPTION(OutOfMemoryException);
            logger_.logMessage(Logger::DEBUG, string("New unzipx leaf \"")
                               + new_zipxSY->getId()->getString()
                               + "\" created");

            // Connect the zipxSY to the unzipxSY
            if (!new_zipxSY->addOutPort(Id("out"))) {
                THROW_EXCEPTION(IllegalStateException, "Failed to add port");
            }
            if (!new_unzipxSY->addInPort(Id("in"))) {
                THROW_EXCEPTION(IllegalStateException, "Failed to add port");
            }
            new_zipxSY->getOutPort(Id("out"))->connect(
                new_unzipxSY->getInPort(Id("in")));
            logger_.logMessage(Logger::DEBUG, "Ports added");

            // Insert the zipxSY and unzipxSY leaf in between the current
            // data parallel segment
            for (size_t i = 0; i < chains.size(); ++i) {
                string num(tools::toString(i + 1));

                // Connect left mapSY with zipxSY
                if (!new_zipxSY->addInPort(Id(string("in") + num))) {
                    THROW_EXCEPTION(IllegalStateException, "Failed to add "
                                    "port");
                }
                Leaf::Port* left_mapSY_out_port = 
                    chains[i][current_segment - 1]->getOutPorts().front();
                Leaf::Port* zipxSY_in_port = new_zipxSY->getInPorts().back();
                logger_.logMessage(Logger::DEBUG, string("Connecting \"")
                                   + left_mapSY_out_port->toString()
                                   + "\" with \""
                                   + zipxSY_in_port->toString() + "\"...");
                left_mapSY_out_port->connect(zipxSY_in_port);

                // Connect right mapSY with unzipxSY
                if (!new_unzipxSY->addOutPort(Id(string("out") + num))) {
                    THROW_EXCEPTION(IllegalStateException, "Failed to add "
                                    "port");
                }
                Leaf::Port* right_mapSY_in_port = 
                    chains[i][current_segment]->getInPorts().front();
                Leaf::Port* unzipxSY_out_port = 
                    new_unzipxSY->getOutPorts().back();
                logger_.logMessage(Logger::DEBUG, string("Connecting \"")
                                   + right_mapSY_in_port->toString()
                                   + "\" with \""
                                   + unzipxSY_out_port->toString() + "\"...");
                right_mapSY_in_port->connect(unzipxSY_out_port);
            }

            // Add new leafs to the processnetwork
            if (!processnetwork_->addProcess(new_zipxSY)) {
                THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                                + "new leaf: Leaf with ID "
                                + "\"" + new_zipxSY->getId()->getString()
                                + "\" already existed");
            }
            if (!processnetwork_->addProcess(new_unzipxSY)) {
                THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                                + "new leaf: Leaf with ID "
                                + "\"" + new_unzipxSY->getId()->getString()
                                + "\" already existed");
            }

            logger_.logMessage(Logger::DEBUG, string("New leafs \"")
                               + new_zipxSY->getId()->getString()
                               + "\" and \""
                               + new_unzipxSY->getId()->getString()
                               + "\" added to the processnetwork");
        }
    }
    catch (std::out_of_range&) {
        THROW_EXCEPTION(IndexOutOfBoundsException);
    }
}

ProcessNetworkModifier::ContainedSection::ContainedSection(Leaf* start, Leaf* end)
        throw(InvalidArgumentException) : start(start), end(end) {
    if (!start) {
        THROW_EXCEPTION(InvalidArgumentException, "\"start\" must not be NULL");
    }
    if (!end) {
        THROW_EXCEPTION(InvalidArgumentException, "\"end\" must not be NULL");
    }
}

string ProcessNetworkModifier::ContainedSection::toString() const throw() {
    return string("\"") + start->getId()->getString() + "--" + 
        end->getId()->getString() + "\"";
}

void ProcessNetworkModifier::redirectDataFlow(Leaf* old_start, Leaf* old_end,
                                     Leaf* new_start, Leaf* new_end)
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

    string message("Redirecting data flow through leaf(es) ");
    if (old_start == old_end) {
        message += string("\"") + old_start->getId()->getString() + "\"";
    }
    else {
        message += string("\"") + old_start->getId()->getString() + "\" and \""
            + old_end->getId()->getString() + "\"";
    }
    message += " to leaf(es) ";
    if (new_start == new_end) {
        message += string("\"") + new_start->getId()->getString() + "\"";
    }
    else {
        message += string("\"") + new_start->getId()->getString() + "\" and \""
            + new_end->getId()->getString() + "\"";
    }
    logger_.logMessage(Logger::INFO, message);

    // Add in ports of old_start to the new_start
    logger_.logMessage(Logger::DEBUG, string("Adding in ports from leaf \"")
                       + old_start->getId()->getString() + "\" to leaf \""
                       + new_start->getId()->getString() + "\"");
    list<Leaf::Port*> in_ports = old_start->getInPorts();
    list<Leaf::Port*>::iterator it;
    for (it = in_ports.begin(); it != in_ports.end(); ++it) {
        if (!new_start->addInPort(**it)) {
            THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                            + "in port \"" + (*it)->toString() + "\" to "
                            + "leaf \"" + new_start->getId()->getString()
                            + "\"");
        }
        replaceProcessNetworkInput(*it, new_start->getInPorts().back());
    }

    // Add out ports of old_end to the new_end
    logger_.logMessage(Logger::DEBUG, string("Adding out ports from leaf \"")
                       + old_end->getId()->getString() + "\" to leaf \""
                       + new_end->getId()->getString() + "\"");
    list<Leaf::Port*> out_ports = old_end->getOutPorts();
    for (it = out_ports.begin(); it != out_ports.end(); ++it) {
        if (!new_end->addOutPort(**it)) {
            THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                            + "out port \"" + (*it)->toString() + "\" to "
                            + "leaf \"" + new_start->getId()->getString()
                            + "\"");
        }
        replaceProcessNetworkOutput(*it, new_end->getOutPorts().back());
    }
}

void ProcessNetworkModifier::replaceProcessNetworkInput(Leaf::Port* old_port,
                                      Leaf::Port* new_port)
    throw(RuntimeException) {
    list<Leaf::Port*> inputs(processnetwork_->getInputs());
    list<Leaf::Port*>::iterator it;
    for (it = inputs.begin(); it != inputs.end(); ++it) {
        if (*it == old_port) {
            processnetwork_->deleteInput(*it);
            processnetwork_->addInput(new_port);
            break;
        }
    }
}

void ProcessNetworkModifier::replaceProcessNetworkOutput(Leaf::Port* old_port,
                                       Leaf::Port* new_port)
    throw(RuntimeException) {
    list<Leaf::Port*> outputs(processnetwork_->getOutputs());
    list<Leaf::Port*>::iterator it;
    for (it = outputs.begin(); it != outputs.end(); ++it) {
        if (*it == old_port) {
            processnetwork_->deleteOutput(*it);
            processnetwork_->addOutput(new_port);
            break;
        }
    }
}
