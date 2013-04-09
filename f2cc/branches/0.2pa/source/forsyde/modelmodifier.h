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

#ifndef F2CC_SOURCE_FORSYDE_MODELMODIFIER_H_
#define F2CC_SOURCE_FORSYDE_MODELMODIFIER_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines a class for performing \c Processnetwork modifications.
 */

#include "id.h"
#include "processnetwork.h"
#include "process.h"
#include "SY/combsy.h"
#include "SY/parallelmapsy.h"
#include "SY/unzipxsy.h"
#include "SY/zipxsy.h"
#include "../logger/logger.h"
#include "../exceptions/ioexception.h"
#include "../exceptions/invalidargumentexception.h"
#include "../exceptions/outofmemoryexception.h"
#include <list>
#include <set>
#include <vector>

namespace f2cc {
namespace ForSyDe {

/**
 * @brief Performs semantic-preserving modifications on a \c Processnetwork
 *        object.
 *
 * The \c ModelModifier is a class which provides a set of model modification
 * methods. The modifications are such that they preserve the semantics of the
 * model, and are used to simplify the latter synthesis process or affect the
 * structure of the generated code (i.e. whether to generate sequential C code
 * or parallel CUDA C code).
 */
class ModelModifier {
  private:
    class ContainedSection;

  public:
    /**
     * Creates a model modifier.
     *
     * @param model
     *        ForSyDe model.
     * @param logger
     *        Reference to the logger.
     * @throws InvalidArgumentException
     *         When \c model is \c NULL.
     */
    ModelModifier(ForSyDe::Processnetwork* model, Logger& logger)
        throw(InvalidArgumentException);

    /**
     * Destroys this model modifier. The logger remains open.
     */
    ~ModelModifier() throw();

    /**
     * Coalesces data parallel processes across different segments into a
     * single data parallel process.
     *
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void coalesceDataParallelProcesses() 
        throw(IOException, RuntimeException);

    /**
     * Coalesces \c ParallelMap processes within the model into a single
     * process.
     *
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void coalesceParallelMapSyProcesses() 
        throw(IOException, RuntimeException);

    /**
     * Splits data parallel segments by injecting a \c zipx followed by an
     * \c unzipx process between each segment.
     *
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void splitDataParallelSegments() throw(IOException, RuntimeException);

    /**
     * Fuses a segment of \c unzipx, \c map, and \c zipx processes into a
     * single \c parallelmap process with the same process function argument
     * as the \c map processes.
     *
     * All segments of all data parallel sections \em must have been split
     * prior to invoking this method!
     *
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void fuseUnzipcombZipProcesses()
        throw(IOException, RuntimeException);

    /**
     * Converts processes of type \c comb which have only one in port to
     * \c comb. This is because, in ForSyDe, they are actually the same
     * process type (map is a special case of zipwithN), but in the internal
     * representation they are separated. Converting the processes allows the
     * synthesizer to exploit potential data parallelism with is expressed
     * using \c comb processes.
     *
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void convertZipWith1Tocomb()
        throw(IOException, RuntimeException);

    /**
     * Removes redundant processes from the model. Redundant processes are
     * instances which does not affect the semantic behaviour of the model when
     * removed, such as \c unzipxSy and \c zipxSy processes with only one in
     * and out port.
     *
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void removeRedundantProcesses()
        throw(IOException, RuntimeException);

  private:
    /**
     * Injects a \c zipx followed by an \c unzipx process between each
     * segment (column of map processes) in a section. The section is given
     * as a vector of process chains (which is also given as a vector).
     *
     * @param chains
     *        Vector of process chains.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void splitDataParallelSegments(
        std::vector< std::vector<ForSyDe::Process*> > chains)
            throw(IOException, RuntimeException);

    /**
     * Searches for data parallel sections within the model. A data parallel
     * section is a section which is both contained and passes the check
     * performed in isContainedSectionDataParallel(const ContainedSection&).
     *
     * @returns List of data parallel sections.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    std::list<ContainedSection> findDataParallelSections()
        throw(IOException, RuntimeException);

    /**
     * Searches for contained sections within the model. A contained section is
     * a part of the model which:
     *   - start with a \c unzipx process, and
     *   - ends with a \c zipx process, where
     *   - all data flow diverging from the starting point converges at the end
     *     point, and
     *   - all data flow converging to the end point diverges from the starting
     *     point.
     * This method cannot handle nested levels of containment and thus only
     * finds the closest layer of containment.
     *
     * @returns List of contained sections.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    std::list<ContainedSection> findContainedSections()
        throw(IOException, RuntimeException);

    /**
     * Same as findContainedSections() but accepts a process to start the
     * search from and a set of visited processes to avoid redundant search and
     * infinite loops.
     *
     * @param begin
     *        Process to begin the search from.
     * @param visited
     *        Set of visited processes.
     * @returns List of contained sections.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    std::list<ContainedSection> findContainedSections(
        ForSyDe::Process* begin, std::set<ForSyDe::Id>& visited)
        throw(IOException, RuntimeException);
    
    /**
     * Finds the nearest \c unzipx process that can be found from a given
     * starting point.
     *
     * @param begin
     *        Process to begin from.
     * @param visited
     *        Set of visited processes.
     * @returns A zipx process, if found; otherwise \c NULL.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    ForSyDe::SY::unzipx* findNearestunzipxProcess(
        ForSyDe::Process* begin, std::set<ForSyDe::Id>& visited)
    throw(IOException, RuntimeException);

    /**
     * Checks whether a section is contained, i.e. that all branching paths from
     * a starting point converge at an end point.
     *
     * @param start
     *        Starting point.
     * @param end
     *        End point.
     * @returns \c true if the paths are contained within the same start and 
     *          end points.
     * @throws InvalidArgumentException
     *         When either \c start or \c end is \c NULL.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    bool isAContainedSection(Process* start, Process* end)
        throw(InvalidArgumentException, IOException, RuntimeException);

    /**
     * Checks that the data flow originating from a starting point converges at
     * an end point. The flow can be checked in either forward or backward
     * direction.
     *
     * @param start
     *        Process to start the check from.
     * @param end
     *        Expected end point.
     * @param visited
     *        Set of visited processes.
     * @param forward
     *        Set to \c true for forward data flow check (from start to end),
     *        and \c false for backward data flow check (from end to start).
     * @returns \c true if the check is passed.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    bool checkDataFlowConvergence(
        Process* start,
        Process* end,
        std::set<ForSyDe::Id>& visited,
        bool forward)
        throw(IOException, RuntimeException);

    /**
     * Checks if a contained section is data parallel. A data parallel section
     * is a section:
     *   - where there is at least one process between the start and the end
     *     point,
     *   - where all its diverging paths are of equal lengths,
     *   - which consist only of \c map processes, and
     *   - where all processes at the same distance from the starting point are
     *     pairwise equal (i.e. has the same function argument).
     *
     * @param section
     *        A contained section.
     * @returns \c true if the section is data parallel.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    bool isContainedSectionDataParallel(const ContainedSection& section)
        throw(IOException, RuntimeException);

    /**
     * Checks whether the chain consists of only \c comb processes.
     * 
     * @param chain
     *        Process chain.
     * @returns \c true if the chain consists of only \c comb processes.
     */
    bool hasOnlycombSys(std::list<ForSyDe::Process*> chain) const throw();

    /**
     * Checks if two process chains are of equal lengths and have the same order
     * of process types which are pairwise equal.
     *
     * @param first
     *        First process chain.
     * @param second
     *        Second process chain.
     * @returns \c true if the chains are equal.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    bool areProcessChainsEqual(std::list<ForSyDe::Process*> first,
                               std::list<ForSyDe::Process*> second)
        throw(IOException, RuntimeException);

    /**
     * Gets the process chaining starting from the process connected at the
     * other end of a given port and ends at an end point. The end point will
     * not be included in the chain.
     *
     * The caller of this function must ensure that there is a path from the
     * start to the end point. If the process chain contains loops or diverges
     * then no guarantees are made on the order in which the processes will
     * appear in the returned chain.
     *
     * @param start
     *        Starting point.
     * @param end
     *        End point.
     * @returns Process chain.
     * @throws OutOfMemoryException
     *         When the chain cannot be created due to memory shortage.
     */
    std::list<ForSyDe::Process*> getProcessChain(
        ForSyDe::Process::Port* start, ForSyDe::Process* end)
        throw(OutOfMemoryException);

    /**
     * Help function for getProcessChain(ForSyDe::Process::Port*,
     * ForSyDe::Process*) to allow recursive calls.
     *
     * @param start
     *        Starting point.
     * @param end
     *        End point.
     * @param visited
     *        Set of visited processes.
     * @returns Process chain.
     * @throws OutOfMemoryException
     *         When the chain cannot be created due to memory shortage.
     * @see getProcessChain(ForSyDe::Process::Port*, ForSyDe::Process*)
     */
    std::list<ForSyDe::Process*> getProcessChainR(
        ForSyDe::Process::Port* start,
        ForSyDe::Process* end,
        std::set<ForSyDe::Id>& visited)
        throw(OutOfMemoryException);

    /**
     * Coalesces a process chain into a single new process. The process chain
     * \em must be ordered such that the output of a process is consumed by the
     * following process. The old processes will be removed from the model and
     * replaced by the new process.
     * 
     * @param chain
     *        Process chain.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void coalesceProcessChain(std::list<ForSyDe::Process*> chain)
        throw(RuntimeException);

    /**
     * Checks if a \c ParallelMap chain can be coalesced, which is possible if
     * the chain is longer than 1 process, if all processes represent the same
     * number of parallel processes, and if the output data type matches the
     * input data type of the following process.
     * 
     * @param chain
     *        Process chain.
     * @returns \c true if the chain can be coalesced.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    bool isParallelMapSyChainCoalescable(
        std::list<ForSyDe::SY::ParallelMap*> chain) throw(RuntimeException);

    /**
     * Searches for chains for \c ParallelMap processes within the model.
     *
     * @returns List of \c ParallelMap process chains.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    std::list< std::list<ForSyDe::SY::ParallelMap*> > findParallelMapSyChains()
        throw(IOException, RuntimeException);

    
    /**
     * Same as findParallelMapSyChains() but accepts a process to start the
     * search from and a set of visited processes to avoid redundant search
     * and infinite loops.
     *
     * @param begin
     *        Process to begin the search from.
     * @param visited
     *        Set of visited processes.
     * @returns List of \c ParallelMap process chains.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    std::list< std::list<SY::ParallelMap*> > findParallelMapSyChains(
        ForSyDe::Process* begin, std::set<ForSyDe::Id>& visited)
        throw(IOException, RuntimeException);

    /**
     * Coalesces a chain of \c ParallelMap processes into a single new
     * process. The process chain \em must be ordered such that the output of a
     * process is consumed by the following process. The old processes will be
     * removed from the model and replaced by the new process.
     * 
     * @param chain
     *        Process chain.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void coalesceParallelMapSyChain(std::list<ForSyDe::SY::ParallelMap*> chain)
        throw(RuntimeException);

    /**
     * Converts a process chain into a string representation.
     *
     * @param chain
     *        Process chain.
     * @returns String representation.
     */
    std::string processChainToString(std::list<ForSyDe::Process*> chain)
        const throw();

    /**
     * @copydoc processChainToString(std::list<ForSyDe::Process*>) const
     */
    std::string processChainToString(std::list<ForSyDe::SY::ParallelMap*> chain)
        const throw();

    /**
     * Removes and destroys a process and all its succeeding processes from the
     * model.
     *
     * @param start
     *        Start of process chain to destroy.
     * @throws InvalidArgumentExceptionException
     *         When \c process is \c NULL.
     */
    void destroyProcessChain(ForSyDe::Process* start)
        throw(InvalidArgumentException);

    /**
     * Redirects data flow between two processes \e A and \e B to another two
     * processes \e C and \e D. The data flow is redirected by adding the in
     * ports of \e A to \e C, and the out ports of \e B to \e D. This will
     * break all connections to \e A and from \e B, thus unconnecting that
     * segment from the model (but it does \em not delete it). Processes \e C
     * and \e D can be the same process.
     *
     * @param old_start
     *        Process indicating the start of data flow.
     * @param old_end
     *        Process indicating the end of data flow.
     * @param new_start
     *        Process to which to direct the start of data flow.
     * @param new_end
     *        Process to which to direct the endof data flow.
     * @throws InvalidArgumentException
     *         When either of \c old_start, \c old_end, \c new_start, or
     *         \c new_end is \c NULL.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void redirectDataFlow(Process* old_start, Process* old_end,
                          Process* new_start, Process* new_end)
        throw(InvalidArgumentException, IOException, RuntimeException);

    /**
     * Replaces the old port, if set as part of the inputs for the model, with
     * the new port.
     * 
     * @param old_port
     *        Port to replace.
     * @param new_port
     *        Replacement port.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void replaceProcessnetworkInput(Process::Port* old_port, Process::Port* new_port)
        throw(RuntimeException);

    /**
     * Same as replaceProcessnetworkInput(Process::Port*, Process::Port*) but for
     * outputs.
     * 
     * @param old_port
     *        Port to replace.
     * @param new_port
     *        Replacement port.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void replaceProcessnetworkOutput(Process::Port* old_port, Process::Port* new_port)
        throw(RuntimeException);

    /**
     * Attempts to visit a process. If the process has not already been visited,
     * the process will be added to the set and \c true is returned. Otherwise
     * \c false is returned.
     * 
     * @param visited
     *        Set of visited processes.
     * @param process
     *        Process to visit.
     * @returns \c true if the process had not already been visited and was
     *          successfully added to the set; otherwise \c false.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    bool visitProcess(std::set<ForSyDe::Id>& visited, Process* process)
        throw(RuntimeException);

  private:
    /**
     * @brief Defines a contained section.
     *
     * A contained section is a part of the process network where all data
     * flow diverging from a single point converges at another single point,
     * and vice versa.
     */
    struct ContainedSection {
        /**
         * First process in the chain.
         */
        ForSyDe::Process* start;

        /**
         * Last process in the chain.
         */
        ForSyDe::Process* end;

        /**
         * Creates a contained section.
         *
         * @param start
         *        Starting point.
         * @param end
         *        End point.
         * @throws InvalidArgumentException
         *         When either \c start or \c end is \c NULL.
         */
        ContainedSection(ForSyDe::Process* start, ForSyDe::Process* end)
            throw(InvalidArgumentException);

        /**
         * Converts this section into a string representation.
         *
         * @returns String representation.
         */
        std::string toString() const throw();
    };

  private:
    /**
     * ForSyDe model.
     */
    ForSyDe::Processnetwork* const processnetwork_;

    /**
     * Logger.
     */
    Logger& logger_;
};

}
}

#endif
