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

#ifndef F2CC_SOURCE_FORSYDE_MODELMODIFIER_H_
#define F2CC_SOURCE_FORSYDE_MODELMODIFIER_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines a class for performing \c ProcessNetwork modifications.
 */

#include "../id.h"
#include "../processnetwork.h"
#include "../leaf.h"
#include "../SY/mapsy.h"
#include "../SY/parallelmapsy.h"
#include "../SY/unzipxsy.h"
#include "../SY/zipxsy.h"
#include "../../logger/logger.h"
#include "../../exceptions/ioexception.h"
#include "../../exceptions/invalidargumentexception.h"
#include "../../exceptions/outofmemoryexception.h"
#include <list>
#include <set>
#include <vector>

namespace f2cc {
namespace Forsyde {

/**
 * @brief Performs semantic-preserving modifications on a \c ProcessNetwork
 *        object.
 *
 * The \c ModelModifier is a class which provides a set of processnetwork modification
 * methods. The modifications are such that they preserve the semantics of the
 * processnetwork, and are used to simplify the latter synthesis leaf or affect the
 * structure of the generated code (i.e. whether to generate sequential C code
 * or parallel CUDA C code).
 */
class ModelModifier {
  private:
    class ContainedSection;

  public:
    /**
     * Creates a processnetwork modifier.
     *
     * @param processnetwork
     *        ForSyDe processnetwork.
     * @param logger
     *        Reference to the logger.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     */
    ModelModifier(Forsyde::ProcessNetwork* processnetwork, Logger& logger)
        throw(InvalidArgumentException);

    /**
     * Destroys this processnetwork modifier. The logger remains open.
     */
    ~ModelModifier() throw();

    /**
     * Coalesces data parallel leafs across different segments into a
     * single data parallel leaf.
     *
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void coalesceDataParallelLeafs() 
        throw(IOException, RuntimeException);

    /**
     * Coalesces \c ParallelMap leafs within the processnetwork into a single
     * leaf.
     *
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void coalesceParallelMapSyLeafs() 
        throw(IOException, RuntimeException);

    /**
     * Splits data parallel segments by injecting a \c ZipxSY followed by an
     * \c UnzipxSY leaf between each segment.
     *
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void splitDataParallelSegments() throw(IOException, RuntimeException);

    /**
     * Fuses a segment of \c UnzipxSY, \c mapSY, and \c ZipxSY leafs into a
     * single \c parallelmapSY leaf with the same leaf function argument
     * as the \c mapSY leafs.
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
    void fuseUnzipMapZipLeafs()
        throw(IOException, RuntimeException);

    /**
     * Converts leafs of type \c ZipWithN which have only one in port to
     * \c Map. This is because, in ForSyDe, they are actually the same
     * leaf type (mapSY is a special case of zipwithN), but in the internal
     * representation they are separated. Converting the leafs allows the
     * synthesizer to exploit potential data parallelism with is expressed
     * using \c Map leafs.
     *
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void convertZipWith1ToMap()
        throw(IOException, RuntimeException);

    /**
     * Removes redundant leafs from the processnetwork. Redundant leafs are
     * instances which does not affect the semantic behaviour of the processnetwork when
     * removed, such as \c UnZipxSy and \c ZipxSy leafs with only one in
     * and out port.
     *
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void removeRedundantLeafs()
        throw(IOException, RuntimeException);

  private:
    /**
     * Injects a \c ZipxSY followed by an \c UnzipxSY leaf between each
     * segment (column of mapSY leafs) in a section. The section is given
     * as a vector of leaf chains (which is also given as a vector).
     *
     * @param chains
     *        Vector of leaf chains.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void splitDataParallelSegments(
        std::vector< std::vector<Forsyde::Leaf*> > chains)
            throw(IOException, RuntimeException);

    /**
     * Searches for data parallel sections within the processnetwork. A data parallel
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
     * Searches for contained sections within the processnetwork. A contained section is
     * a part of the processnetwork which:
     *   - start with a \c UnzipxSY leaf, and
     *   - ends with a \c ZipxSY leaf, where
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
     * Same as findContainedSections() but accepts a leaf to start the
     * search from and a set of visited leafs to avoid redundant search and
     * infinite loops.
     *
     * @param begin
     *        Leaf to begin the search from.
     * @param visited
     *        Set of visited leafs.
     * @returns List of contained sections.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    std::list<ContainedSection> findContainedSections(
        Forsyde::Leaf* begin, std::set<Forsyde::Id> visited)
        throw(IOException, RuntimeException);
    
    /**
     * Finds the nearest \c Unzipx leaf that can be found from a given
     * starting point.
     *
     * @param begin
     *        Leaf to begin from.
     * @returns A Zipx leaf, if found; otherwise \c NULL.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    Forsyde::SY::Unzipx* findNearestUnzipxLeaf(Forsyde::Leaf* begin)
    throw(IOException, RuntimeException);

    /**
     * Checks whether a section is contained, i.e. that all branching paths from
     * a starting point converge at an end point.
     *
     * @param start
     *        Starting point.
     * @param end
     *        End point.
     * @returns \b true if the paths are contained within the same start and 
     *          end points.
     * @throws InvalidArgumentException
     *         When either \c start or \c end is \c NULL.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    bool isAContainedSection(Leaf* start, Leaf* end)
        throw(InvalidArgumentException, IOException, RuntimeException);

    /**
     * Checks that the data flow originating from a starting point converges at
     * an end point. The flow can be checked in either forward or backward
     * direction.
     *
     * @param start
     *        Leaf to start the check from.
     * @param end
     *        Expected end point.
     * @param forward
     *        Set to \b true for forward data flow check (from start to end),
     *        and \b false for backward data flow check (from end to start).
     * @returns \b true if the check is passed.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    bool checkDataFlowConvergence(Leaf* start, Leaf* end, bool forward)
        throw(IOException, RuntimeException);

    /**
     * Checks if a contained section is data parallel. A data parallel section
     * is a section:
     *   - where there is at least one leaf between the start and the end
     *     point,
     *   - where all its diverging paths are of equal lengths,
     *   - which consist only of \c mapSY leafs, and
     *   - where all leafs at the same distance from the starting point are
     *     pairwise equal (i.e. has the same function argument).
     *
     * @param section
     *        A contained section.
     * @returns \b true if the section is data parallel.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    bool isContainedSectionDataParallel(const ContainedSection& section)
        throw(IOException, RuntimeException);

    /**
     * Checks whether the chain consists of only \c Map leafs.
     * 
     * @param chain
     *        Leaf chain.
     * @returns \b true if the chain consists of only \c Map leafs.
     */
    bool hasOnlyMapSys(std::list<Forsyde::Leaf*> chain) const throw();

    /**
     * Checks if two leaf chains are of equal lengths and have the same order
     * of leaf types which are pairwise equal.
     *
     * @param first
     *        First leaf chain.
     * @param second
     *        Second leaf chain.
     * @returns \b true if the chains are equal.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    bool areLeafChainsEqual(std::list<Forsyde::Leaf*> first,
                               std::list<Forsyde::Leaf*> second)
        throw(IOException, RuntimeException);

    /**
     * Gets the leaf chaining starting from the leaf connected at the
     * other end of a given port and ends at an end point. The end point will
     * not be included in the chain. If there are multiple chains, it will
     * select the first out port of the leaf where the flow diverges.
     *
     * @param start
     *        Starting point.
     * @param end
     *        End point.
     * @returns Leaf chain.
     * @throws OutOfMemoryException
     *         When the chain cannot be created due to memory shortage.
     */
    std::list<Forsyde::Leaf*> getProcessChain(
        Forsyde::Leaf::Port* start, Forsyde::Leaf* end)
        throw(OutOfMemoryException);

    /**
     * Coalesces a leaf chain into a single new leaf. The leaf chain
     * \em must be ordered such that the output of a leaf is consumed by the
     * following leaf. The old leafs will be removed from the processnetwork and
     * replaced by the new leaf.
     * 
     * @param chain
     *        Leaf chain.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void coalesceLeafChain(std::list<Forsyde::Leaf*> chain)
        throw(RuntimeException);

    /**
     * Checks if a \c ParallelMap chain can be coalesced, which is possible if
     * the chain is longer than 1 leaf, if all leafs represent the same
     * number of parallel leafs, and if the output data type matches the
     * input data type of the following leaf.
     * 
     * @param chain
     *        Leaf chain.
     * @returns \b true if the chain can be coalesced.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    bool isParallelMapSyChainCoalescable(
        std::list<Forsyde::SY::ParallelMap*> chain) throw(RuntimeException);

    /**
     * Searches for chains for \c ParallelMap leafs within the processnetwork.
     *
     * @returns List of \c ParallelMap leaf chains.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    std::list< std::list<Forsyde::SY::ParallelMap*> > findParallelMapSyChains()
        throw(IOException, RuntimeException);

    
    /**
     * Same as findParallelMapSyChains() but accepts a leaf to start the
     * search from and a set of visited leafs to avoid redundant search
     * and infinite loops.
     *
     * @param begin
     *        Leaf to begin the search from.
     * @param visited
     *        Set of visited leafs.
     * @returns List of \c ParallelMap leaf chains.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    std::list< std::list<SY::ParallelMap*> > findParallelMapSyChains(
        Forsyde::Leaf* begin, std::set<Forsyde::Id> visited)
        throw(IOException, RuntimeException);

    /**
     * Coalesces a chain of \c ParallelMap leafs into a single new
     * leaf. The leaf chain \em must be ordered such that the output of a
     * leaf is consumed by the following leaf. The old leafs will be
     * removed from the processnetwork and replaced by the new leaf.
     * 
     * @param chain
     *        Leaf chain.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void coalesceParallelMapSyChain(std::list<Forsyde::SY::ParallelMap*> chain)
        throw(RuntimeException);

    /**
     * Converts a leaf chain into a string representation.
     *
     * @param chain
     *        Leaf chain.
     * @returns String representation.
     */
    std::string leafChainToString(std::list<Forsyde::Leaf*> chain)
        const throw();

    /**
     * @copydoc leafChainToString(std::list<Forsyde::Leaf*>) const
     */
    std::string leafChainToString(std::list<Forsyde::SY::ParallelMap*> chain)
        const throw();

    /**
     * Removes and destroys a leaf and all its succeeding leafs from the
     * processnetwork.
     *
     * @param start
     *        Start of leaf chain to destroy.
     * @throws InvalidArgumentExceptionException
     *         When \c leaf is \c NULL.
     */
    void destroyLeafChain(Forsyde::Leaf* start)
        throw(InvalidArgumentException);

    /**
     * Redirects data flow between two leafs \e A and \e B to another two
     * leafs \e C and \e D. The data flow is redirected by adding the in
     * ports of \e A to \e C, and the out ports of \e B to \e D. This will
     * break all connections to \e A and from \e B, thus unconnecting that
     * segment from the processnetwork (but it does \em not delete it). Leafs \e C
     * and \e D can be the same leaf.
     *
     * @param old_start
     *        Leaf indicating the start of data flow.
     * @param old_end
     *        Leaf indicating the end of data flow.
     * @param new_start
     *        Leaf to which to direct the start of data flow.
     * @param new_end
     *        Leaf to which to direct the endof data flow.
     * @throws InvalidArgumentException
     *         When either of \c old_start, \c old_end, \c new_start, or
     *         \c new_end is \c NULL.
     * @throws IOException
     *         When access to the log file failed.
     * @throws RuntimeException
     *         When a program error has occurred. This most likely indicates a
     *         bug.
     */
    void redirectDataFlow(Leaf* old_start, Leaf* old_end,
                          Leaf* new_start, Leaf* new_end)
        throw(InvalidArgumentException, IOException, RuntimeException);

    /**
     * Replaces the old port, if set as part of the inputs for the processnetwork, with
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
    void replaceProcessNetworkInput(Leaf::Port* old_port, Leaf::Port* new_port)
        throw(RuntimeException);

    /**
     * Same as replaceProcessNetworkInput(Leaf::Port*, Leaf::Port*) but for
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
    void replaceProcessNetworkOutput(Leaf::Port* old_port, Leaf::Port* new_port)
        throw(RuntimeException);

  private:
    /**
     * @brief Defines a contained section.
     *
     * A contained section is a part of the leaf network where all data
     * flow diverging from a single point converges at another single point,
     * and vice versa.
     */
    struct ContainedSection {
        /**
         * First leaf in the chain.
         */
        Forsyde::Leaf* start;

        /**
         * Last leaf in the chain.
         */
        Forsyde::Leaf* end;

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
        ContainedSection(Forsyde::Leaf* start, Forsyde::Leaf* end)
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
     * ForSyDe processnetwork.
     */
    Forsyde::ProcessNetwork* const processnetwork_;

    /**
     * Logger.
     */
    Logger& logger_;
};

}
}

#endif
