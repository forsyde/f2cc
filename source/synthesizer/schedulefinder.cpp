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

#include "schedulefinder.h"
#include "../forsyde/SY/delaysy.h"
#include "../tools/tools.h"

using namespace f2cc;
using namespace f2cc::ForSyDe;
using std::string;
using std::list;
using std::set;
using std::pair;
using std::bad_alloc;
using std::queue;

ScheduleFinder::ScheduleFinder(ForSyDe::ProcessNetwork* processnetwork, Logger& logger)
        throw(InvalidArgumentException) : processnetwork_(processnetwork), logger_(logger) {
    if (!processnetwork) {
        THROW_EXCEPTION(InvalidArgumentException, "\"processnetwork\" must not be NULL");
    }
}

ScheduleFinder::~ScheduleFinder() throw() {}

list<Id> ScheduleFinder::findSchedule() throw(IOException, RuntimeException) {
    // Add all leafs at processnetwork outputs to starting point queue
    list<Leaf::Port*> output_ports = processnetwork_->getOutputs();
    logger_.logMessage(Logger::DEBUG, string("Scanning all processnetwork outputs..."));
    for (list<Leaf::Port*>::iterator it = output_ports.begin();
         it != output_ports.end(); ++it) {
        logger_.logMessage(Logger::DEBUG, string("Adding \"")
                           + (*it)->getProcess()->getId()->getString()
                           + "\" to starting point queue...");
        starting_points_.push((*it)->getProcess());
    }
    
    // Iterate over all starting points
    list<Id> schedule;
    globally_visited_.clear();
    while (!starting_points_.empty()) {
        Leaf* next_starting_point = starting_points_.front();
        starting_points_.pop();
        if (!next_starting_point) {
            THROW_EXCEPTION(RuntimeException, "Next starting point is NULL");
        }
        logger_.logMessage(Logger::DEBUG, string("Starting search at \"")
                           + next_starting_point->getId()->getString()
                           + "\"...");

        set<Id> locally_visited;
        PartialSchedule partial = findPartialSchedule(next_starting_point,
                                                      locally_visited);
        if (partial.at_beginning) {
            schedule.insert(schedule.begin(), partial.schedule.begin(),
                            partial.schedule.end());
        }
        else {
            list<Id>::iterator it;
            for (it = schedule.begin(); it != schedule.end(); ++it) {
                if (*it == partial.insertion_point) {
                    ++it;
                    break;
                }
            }
            if (it == schedule.end()) {
                THROW_EXCEPTION(IllegalStateException, string("Failed to add ")
                                + "partial schedule: Insertion point \""
                                + partial.insertion_point.getString()
                                + "\" not found in schedule");
            }

            schedule.insert(it, partial.schedule.begin(),
                            partial.schedule.end());
        }
        
        globally_visited_.insert(locally_visited.begin(),
                                 locally_visited.end());
    }
    return schedule;
}

ScheduleFinder::PartialSchedule ScheduleFinder::findPartialSchedule(
    Leaf* start, set<Id>& locally_visited)
    throw(IOException, RuntimeException) {
    PartialSchedule partial_schedule;

    if (isGloballyVisited(start)) {
        partial_schedule.at_beginning = false;
        partial_schedule.insertion_point = *start->getId();
        return partial_schedule;
    }

    // If this is a delay, add the delay element to the schedule and add its
    // preceding leaf to starting point queue
    if (dynamic_cast<delay*>(start)) {
        Leaf::Port* inport = start->getInPorts().front();
        if (inport->isConnected()) {
            Leaf* preceding_leaf =
                inport->getConnectedPort()->getProcess();
            starting_points_.push(preceding_leaf);
        }
        partial_schedule.schedule.push_back(*start->getId());
        return partial_schedule;
    }

    if (!visitLocally(start, locally_visited)) {
        return partial_schedule;
    }

    // Find partial schedule
    logger_.logMessage(Logger::DEBUG, string("Analyzing leaf \"")
                       + start->getId()->getString() + "\"...");
    list<Leaf::Port*> in_ports = start->getInPorts();
    list<Leaf::Port*>::iterator it;
    for (it = in_ports.begin(); it != in_ports.end(); ++it) {
        if ((*it)->isConnected()) {
            Leaf* next_leaf = (*it)->getConnectedPort()->getProcess();
            PartialSchedule pp_schedule(findPartialSchedule(next_leaf,
                                                            locally_visited));
            tools::append<Id>(partial_schedule.schedule, pp_schedule.schedule);
            if (!pp_schedule.at_beginning) {
                partial_schedule.at_beginning = false;
                partial_schedule.insertion_point = pp_schedule.insertion_point;
            }
        }
    }
    partial_schedule.schedule.push_back(*start->getId());

    return partial_schedule;
}

bool ScheduleFinder::isGloballyVisited(ForSyDe::Leaf* leaf) {
    return globally_visited_.find(*leaf->getId()) != globally_visited_.end();
}

bool ScheduleFinder::visitLocally(ForSyDe::Leaf* leaf,
                                  set<ForSyDe::Id>& visited) {
    return visited.insert(*leaf->getId()).second;
}


ScheduleFinder::PartialSchedule::PartialSchedule() :
        at_beginning(true), insertion_point(Id("")) {}

ScheduleFinder::PartialSchedule::PartialSchedule(list<Id>& schedule,
                                                 bool at_beginning,
                                                 Id insertion_point) :
        schedule(schedule), at_beginning(at_beginning),
        insertion_point(insertion_point) {}
