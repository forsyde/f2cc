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

#ifndef F2CC_SOURCE_NTHESIZER_SCHEDULEFINDER_H_
#define F2CC_SOURCE_NTHESIZER_SCHEDULEFINDER_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines the \c ScheduleFinder class.
 */

#include "../logger/logger.h"
#include "../forsyde/id.h"
#include "../forsyde/composite.h"
#include "../forsyde/processnetwork.h"
#include "../forsyde/process.h"
#include "../exceptions/ioexception.h"
#include "../exceptions/outofmemoryexception.h"
#include "../exceptions/runtimeexception.h"
#include "../exceptions/invalidargumentexception.h"
#include "../exceptions/invalidmodelexception.h"
#include "../exceptions/illegalstateexception.h"
#include <string>
#include <list>
#include <set>
#include <queue>

namespace f2cc {

/**
 * @brief A class for finding a process schedule for a given \c ForSyDe::Processnetwork
 *        instance.
 *
 * The \c ScheduleFinder class implements an algorithm for finding a process
 * schedule for a given instance of a \c ForSyDe::Processnetwork.
 *
 * The algorithm is a recursive DFS algorithm which traverses over the processes
 * in the process network. It starts by building a \em starting \em point \em queue,
 * containing all processes connected directly to the process network outputs. It then
 * pops a process from the head of the queue, and creates a \em partial \em
 * process \em schedule. The partial process schedule is created by recursively
 * traversing upwards along the data flow, moving via the in ports (\c
 * ForSyDe::Process::Port) of a \c ForSyDe::Process. When no more traversing can
 * be done, it rewinds the stack, and adds the current process to the
 * schedule. If a process has more than one in port, then a partial schedule is
 * generated for each, concatenated together, and then the current process is
 * appended to the end of the partial process schedule. Throughout the
 * traversing, a set of already-visited processes is maintained. If an
 * already-visited process is reached, then an empty schedule is returned and
 * the function stack starts to rewind.
 *
 * This works very well as long as the process network contains no loops. However, if it
 * does, then more needs to be done to get a correct schedule. First, the
 * visited process set is split into a \em global and a \em local set. Whenever
 * a process is popped from the starting point queue, the local set is reset,
 * and once the partial search has finished for that starting point process, the
 * local set is added to the global set. In addition to halting the search
 * whenever no more traversing can be done (i.e. when reaching a processnetwork input)
 * and when a process has already been visited, the search also halts whenever a
 * delay element is hit. In such instances, the preceding process (if any) is
 * added to the starting point queue, the delay element is added to the partial
 * element, and the function stack then rewinds.
 *
 * Lastly, for a given partial schedule, we need to know where to insert it
 * into the final schedule. If the partial search was halted due to hitting a
 * processnetwork input, then the partial schedule is inserted at the beginning of the
 * schedule. If the partial search was halted due to hitting a globally-visited
 * process \em P, then the partial schedule is inserted after the process \em P
 * in the schedule.
 */
class ScheduleFinder {
  private:
    struct PartialSchedule;

  public:
    /**
     * Creates a schedule finder.
     *
     * @param processnetwork
     *        ForSyDe processnetwork.
     * @param logger
     *        Reference to the logger object.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     */
    ScheduleFinder(ForSyDe::Processnetwork* processnetwork, Logger& logger)
        throw(InvalidArgumentException);

    /**
     * Destroys this schedule finder.
     */
    ~ScheduleFinder() throw();

    /**
     * Finds a process schedule for the process network. The schedule is such that if the
     * processes are executed one by one the result will be the same as if the
     * perfect synchrony hypothesis still applied.
     *
     * See detailed class description for information on how the algorithm
     * works.
     *
     * @returns Process schedule.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    std::list<ForSyDe::Id> findSchedule() throw(IOException, RuntimeException);

    /**
     * Finds a partial schedule for unvisited processes when traversing from a
     * given process to an input port of the process network.
     *
     * See detailed class description for information on how the algorithm
     * works.
     *
     * @param start
     *        Process to start from.
     * @param locally_visited
     *        Set of already locally visited processes.
     * @returns Partial process schedule.
     * @throws IOException
     *         When access to the log file fails.
     * @throws RuntimeException
     *         When a program error occurs. This most likely indicates a bug.
     */
    PartialSchedule findPartialSchedule(
        ForSyDe::Process* start, std::set<ForSyDe::Id>& locally_visited)
        throw(IOException, RuntimeException);

    /**
     * Checks if a process has already been visited in a global sense. This
     * does \em not, however, \em set the process as globally visited.
     *
     * @param process
     *        Process.
     * @returns \c true if the process has already been visited.
     */
    bool isGloballyVisited(ForSyDe::Process* process);

    /**
     * Visits a process in a local sense.
     *
     * @param process
     *        Process to visit.
     * @param visited
     *        Set of locally visited processes.
     * @returns \c true if the process has not previously been locally visisted.
     */
    bool visitLocally(ForSyDe::Process* process,
                      std::set<ForSyDe::Id>& visited);

  private:
    /**
     * ForSyDe processnetwork.
     */
    ForSyDe::Processnetwork* const processnetwork_;

    /**
     * Logger.
     */
    Logger& logger_;

    /**
     * Set of globally already visited processes.
     */
    std::set<ForSyDe::Id> globally_visited_;

    /**
     * Queue of starting points.
     */
    std::queue<ForSyDe::Process*> starting_points_;

  private:
    /**
     * A structure for describing a partial process schedule and where to 
     * insert it in the final schedule.
     */
    struct PartialSchedule {
      public:
        /**
         * Creates an empty partial schedule, set to be inserted at beginning of
         * schedule.
         */
        PartialSchedule();

        /**
         * Creates a partial schedule.
         *
         * @param schedule
         *        Partial schedule.
         * @param at_beginning
         *        Whether the partial schedule is to be inserted at the
         *        beginning of the global schedule.
         * @param insertion_point
         *        Schedule insertion point (leave undefined if \c at_beginning
         *        is set to \c true).
         */
        PartialSchedule(std::list<ForSyDe::Id>& schedule, bool at_beginning,
                       ForSyDe::Id insertion_point);

        /**
         * Partial schedule.
         */
        std::list<ForSyDe::Id> schedule;

        /**
         * Whether the insertion point is at the beginning of the schedule.
         * Default value is \c true.
         */
        bool at_beginning;

        /**
         * Process ID (only defined if the insertion point is not at the
         * beginning of the schedule).
         */
        ForSyDe::Id insertion_point;
    };
};

}

#endif
