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

#ifndef F2CC_SOURCE_FRONTEND_FRONTEND_H_
#define F2CC_SOURCE_FRONTEND_FRONTEND_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines the frontend interface.
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
 * @brief An interface for defining a frontend. The frontend parses a file of
 *        expected input and converts it into an internal ForSyDe processnetwork
 *        representation.
 *
 * The \c Frontend interface specifies the methods required by all frontend
 * implementations. A frontend takes a file as input, and parses the content
 * into an internal ForSyDe processnetwork representation, which can be handled by the
 * later stages of the software synthesis process.
 *
 * The interface is actually an abstract base class as it provides some method
 * implementations, but it should really be viewed as an interface.
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
     * sane for the later stages of the software synthesis process.
     *
     * The receiver of the returned processnetwork is responsible of freeing the memory
     * consumed when the process network is no longer needed.
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
     * @throws InvalidProcessnetworkException
     *         When the process network is invalid (but was successfully parsed).
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    ForSyDe::Processnetwork* parse(const std::string& file)
        throw(InvalidArgumentException, FileNotFoundException, IOException,
              ParseException, InvalidProcessnetworkException, RuntimeException);

  protected:
    /**
     * Creates a new ForSyDe processnetwork by parsing a given input file. This method is
     * responsible of dynamically allocating and returning a new \c
     * ForSyDe::Processnetwork object. After the process network has been created, it is subjected
     * to a series of checks and hook invocations to ensure that the process network is
     * sane and valid for the later steps in the synthesis procedure.
     *
     * The order of the hook calls are:
     *   - checkProcessnetwork(ForSyDe::Processnetwork*)
     *   - checkProcessnetworkMore(ForSyDe::Processnetwork*)
     *   - postCheckFixes(ForSyDe::Processnetwork*)
     *   - ensureNoInPorts(ForSyDe::Processnetwork*)
     *   - ensureNoOutPorts(ForSyDe::Processnetwork*)
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
     * @throws InvalidProcessnetworkException
     *         When the process network is invalid (but was successfully parsed).
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    virtual ForSyDe::Processnetwork* createProcessnetwork(const std::string& file)
        throw(InvalidArgumentException, FileNotFoundException, IOException,
              ParseException, InvalidProcessnetworkException, RuntimeException) = 0;

    /**
     * Performs more processnetwork checks. By default, this does nothing.
     * 
     * @param processnetwork
     *        Processnetwork to check.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     * @throws InvalidProcessnetworkException
     *         When any of the checks fails.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    virtual void checkProcessnetworkMore(ForSyDe::Processnetwork* processnetwork)
        throw(InvalidArgumentException, InvalidProcessnetworkException, IOException,
              RuntimeException);

    /**
     * Performs post-check fixes to the process network, if necessary. By default, this
     * does nothing.
     * 
     * @param processnetwork
     *        Processnetwork to fix.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    virtual void postCheckFixes(ForSyDe::Processnetwork* processnetwork)
        throw(InvalidArgumentException, IOException, RuntimeException);

  private:
    /**
     * Runs standard checks on the process network by invoking (in this order):
     *   - checkProcess(ForSyDe::Process*, ForSyDe::Processnetwork*) on each process, and
     *   - checkProcessnetworkMore(ForSyDe::Processnetwork*).
     * 
     * @param processnetwork
     *        Processnetwork to check.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     * @throws InvalidProcessnetworkException
     *         When any of the checks fails.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     * @see checkProcess(ForSyDe::Process*, ForSyDe::Processnetwork*)
     * @see checkProcessnetworkMore(ForSyDe::Processnetwork*)
     */
    void checkProcessnetwork(ForSyDe::Processnetwork* processnetwork)
        throw(InvalidArgumentException, InvalidProcessnetworkException, IOException,
              RuntimeException);

    /**
     * Checks that the process network contains no \c InPort processes at this stage.
     * Since it is the responsibility of the frontend to remove such processes,
     * an exception thrown from here indicates a serious error in the frontend.
     *
     * @param processnetwork
     *        Processnetwork to check.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void ensureNoInPorts(ForSyDe::Processnetwork* processnetwork)
        throw(InvalidArgumentException, IOException, RuntimeException);

    /**
     * Checks that the process network contains no \c OutPort processes at this stage.
     * Since it is the responsibility of the frontend to remove such processes,
     * an exception thrown from here indicates a serious error in the frontend.
     *
     * @param processnetwork
     *        Processnetwork to check.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void ensureNoOutPorts(ForSyDe::Processnetwork* processnetwork)
        throw(InvalidArgumentException, IOException, RuntimeException);

    /**
     * Checks that a process is a valid by ensuring that:
     *   - all process type-related checks are passed, and that
     *   - all its inputs and ouputs passes the
     *     checkPort(ForSyDe::Process::Port*, ForSyDe::Processnetwork*) check.
     * 
     * @param process
     *        Process to check.
     * @param processnetwork
     *        Processnetwork that the process using the port should belong to.
     * @throws InvalidArgumentException
     *         When \c processnetwork is \c NULL.
     * @throws InvalidProcessnetworkException
     *         When any of the checks fails.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     * @see checkPort(ForSyDe::Process::Port*, ForSyDe::Processnetwork*)
     */
    void checkProcess(ForSyDe::Process* process, ForSyDe::Processnetwork* processnetwork)
        throw(InvalidArgumentException, InvalidProcessnetworkException, IOException,
              RuntimeException);

    /**
     * Checks that a port is:
     *   - is connected,
     *   - is connected to a port belonging to process which is part of the
     *     given processnetwork,
     *   - is not connected a port of its own process (combinatorial loops).
     * 
     * @param port
     *        Port to check.
     * @param processnetwork
     *        Processnetwork that the process using the port should belong to.
     * @throws InvalidArgumentException
     *         When either \c port or \c processnetwork is \c NULL.     
     * @throws InvalidProcessnetworkException
     *         When any of the checks fails.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void checkPort(ForSyDe::Process::Port* port, ForSyDe::Processnetwork* processnetwork)
        throw(InvalidArgumentException, InvalidProcessnetworkException, IOException,
              RuntimeException);

  protected:
    /**
     * Logger.
     */
    Logger& logger_;
};

}

#endif
