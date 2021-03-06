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

#ifndef F2CC_SOURCE_FRONTEND_FRONTEND_H_
#define F2CC_SOURCE_FRONTEND_FRONTEND_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines the frontend port.
 */

#include "../logger/logger.h"
#include "../forsyde/processnetwork.h"
#include "../exceptions/filenotfoundexception.h"
#include "../exceptions/parseexception.h"
#include "../exceptions/invalidmodelexception.h"
#include "../exceptions/runtimeexception.h"
#include <string>

namespace f2cc {

/**
 * @brief An port for defining a frontend. The frontend parses a file of
 *        expected input and converts it into an internal ForSyDe processnetwork
 *        representation.
 *
 * The \c Frontend port specifies the methods required by all frontend
 * implementations. A frontend takes a file as input, and parses the content
 * into an internal ForSyDe processnetwork representation, which can be handled by the
 * later stages of the software synthesis leaf.
 *
 * The port is actually an abstract base class as it provides some method
 * implementations, but it should really be viewed as an port.
 */
class Frontend {
  public:
    /**
     * Creates a frontend.
     *
     * @param logger
     *        Reference to the logger.
     */
    Frontend(Logger& logger) throw();

    /**
     * Destroys this frontend. The logger remains open.
     */
    virtual ~Frontend() throw();

    /**
     * Parses a file converts it into a corresponding internal representation of
     * the ForSyDe processnetwork. The processnetwork will also be checked so that it appears
     * sane for the later stages of the software synthesis leaf.
     *
     * The receiver of the returned processnetwork is responsible of freeing the memory
     * consumed when the processnetwork is no longer needed.
     *
     * @param file
     *        Input file.
     * @returns Parsed processnetwork.
     * @throws InvalidArgumentException
     *         When \c file is an empty string.
     * @throws FileNotFoundException
     *         When the file cannot be found.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws ParseException
     *         When the file fails to be parsed.
     * @throws InvalidModelException
     *         When the processnetwork is invalid (but was successfully parsed).
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    Forsyde::ProcessNetwork* parse(const std::string& file)
        throw(InvalidArgumentException, FileNotFoundException, IOException,
              ParseException, InvalidModelException, RuntimeException);

  protected:
    /**
     * Creates a new ForSyDe processnetwork by parsing a given input file. This method is
     * responsible of dynamically allocating and returning a new \c
     * Forsyde::ProcessNetwork object.
     *
     * @param file
     *        Input file.
     * @returns Parsed processnetwork.
     * @throws InvalidArgumentException
     *         When \c file is an empty string.
     * @throws FileNotFoundException
     *         When the file cannot be found.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws ParseException
     *         When the file fails to be parsed.
     * @throws InvalidModelException
     *         When the processnetwork is invalid (but was successfully parsed).
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    virtual Forsyde::ProcessNetwork* createProcessNetwork(const std::string& file)
        throw(InvalidArgumentException, FileNotFoundException, IOException,
              ParseException, InvalidModelException, RuntimeException) = 0;

    /**
     * Performs more processnetwork checks. By default, this does nothing.
     * 
     * @param processnetwork
     *        ProcessNetwork to check.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     * @throws InvalidModelException
     *         When any of the checks fails.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    virtual void checkProcessNetworkMore(Forsyde::ProcessNetwork* processnetwork)
        throw(InvalidArgumentException, InvalidModelException, IOException,
              RuntimeException);

    /**
     * Performs post-check fixes to the processnetwork, if necessary. By default, this
     * does nothing.
     * 
     * @param processnetwork
     *        ProcessNetwork to fix.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    virtual void postCheckFixes(Forsyde::ProcessNetwork* processnetwork)
        throw(InvalidArgumentException, IOException, RuntimeException);

  private:
    /**
     * Checks that a given processnetwork is a valid ForSyde processnetwork by ensuring that:
     *   - all in and out ports of all leafs in the processnetwork are
     *     connected,
     *   - all ports are connected to leafs that reside within the processnetwork,
     *   - no output of any leaf is connected to the input of the
     *     same leaf (i.e. avoiding Combinatorial loops),
     *   - all leaf type-related checks are passed.
     * 
     * @param processnetwork
     *        ProcessNetwork to check.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     * @throws InvalidModelException
     *         When any of the checks fails.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void checkProcessNetwork(Forsyde::ProcessNetwork* processnetwork)
        throw(InvalidArgumentException, InvalidModelException, IOException,
              RuntimeException);

    /**
     * Checks that a port is:
     *   - is connected,
     *   - is connected to a port belonging to leaf which is part of the
     *     given processnetwork,
     *   - is not connected a port of its own leaf (Combinatorial loops).
     * 
     * @param port
     *        Port to check.
     * @param processnetwork
     *        ProcessNetwork that the leaf using the port should belong to.
     * @throws InvalidArgumentException
     *         When either \c port or \c processnetwork is \c NULL.     
     * @throws InvalidModelException
     *         When any of the checks fails.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void checkPort(Forsyde::Leaf::Port* port, Forsyde::ProcessNetwork* processnetwork)
        throw(InvalidArgumentException, InvalidModelException, IOException,
              RuntimeException);

  protected:
    /**
     * Logger.
     */
    Logger& logger_;
};

}

#endif
