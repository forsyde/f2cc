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

#ifndef F2CC_SOURCE_FRONTEND_DUMPER_H_
#define F2CC_SOURCE_FRONTEND_DUMPER_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines the frontend port.
 */

#include "../logger/logger.h"
#include "../forsyde/processnetwork.h"
#include "../forsyde/composite.h"
#include "../forsyde/leaf.h"
#include "../ticpp/ticpp.h"
#include "../exceptions/filenotfoundexception.h"
#include "../exceptions/invalidmodelexception.h"
#include "../exceptions/runtimeexception.h"
#include <string>
#include <list>

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
class XmlDumper {
  public:
    /**
     * Creates a frontend.
     *
     * @param logger
     *        Reference to the logger.
     */
	XmlDumper(Logger& logger) throw();

    /**
     * Destroys this frontend. The logger remains open.
     */
    virtual ~XmlDumper() throw();

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
    void dump(ForSyDe::ProcessNetwork* pn, const std::string& file)
        throw(InvalidArgumentException, IOException,
              InvalidModelException, RuntimeException);

  private:
    /**
     * Creates a new ForSyDe processnetwork by parsing a given input file. This method is
     * responsible of dynamically allocating and returning a new \c
     * ForSyDe::ProcessNetwork object.
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
    void dumpProcessNetwork(ForSyDe::ProcessNetwork* pn,
    		ticpp::Document doc)
        throw(InvalidArgumentException, FileNotFoundException,
              InvalidModelException, RuntimeException);

    /**
     * Creates a new ForSyDe processnetwork by parsing a given input file. This method is
     * responsible of dynamically allocating and returning a new \c
     * ForSyDe::ProcessNetwork object.
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
    void dumpComposite(ForSyDe::Composite* composite, ticpp::Element* parent)
        throw(InvalidArgumentException,
              InvalidModelException, RuntimeException);

    /**
     * Creates a new ForSyDe processnetwork by parsing a given input file. This method is
     * responsible of dynamically allocating and returning a new \c
     * ForSyDe::ProcessNetwork object.
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
    void dumpLeaf(ForSyDe::Leaf* leaf, ticpp::Element* parent)
        throw(InvalidArgumentException, InvalidModelException, RuntimeException);

    /**
     * Creates a new ForSyDe processnetwork by parsing a given input file. This method is
     * responsible of dynamically allocating and returning a new \c
     * ForSyDe::ProcessNetwork object.
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
    void dumpPort(ForSyDe::Composite::IOPort* port, ticpp::Element* composite,
    		const char* direction)
        throw(InvalidArgumentException, InvalidModelException, RuntimeException);

    /**
     * Creates a new ForSyDe processnetwork by parsing a given input file. This method is
     * responsible of dynamically allocating and returning a new \c
     * ForSyDe::ProcessNetwork object.
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
    void dumpSignal(ForSyDe::Leaf::Port* port, ticpp::Element* composite)
        throw(InvalidArgumentException, InvalidModelException, RuntimeException);

    /**
     * Creates a new ForSyDe processnetwork by parsing a given input file. This method is
     * responsible of dynamically allocating and returning a new \c
     * ForSyDe::ProcessNetwork object.
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
    void dumpIOSignal(ForSyDe::Composite::IOPort* port, ticpp::Element* composite)
        throw(InvalidArgumentException, InvalidModelException, RuntimeException);


    /**
     * Creates a new ForSyDe processnetwork by parsing a given input file. This method is
     * responsible of dynamically allocating and returning a new \c
     * ForSyDe::ProcessNetwork object.
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
    bool isVisitedProcess(ForSyDe::Process* process) throw(InvalidArgumentException);

    /**
     * Creates a new ForSyDe processnetwork by parsing a given input file. This method is
     * responsible of dynamically allocating and returning a new \c
     * ForSyDe::ProcessNetwork object.
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
    bool isVisitedPort(ForSyDe::Process::Interface* port) throw(
    		InvalidArgumentException);

  private:
    /**
     * Logger.
     */
    Logger& logger_;

    /**
     * Logger.
     */
    std::list<ForSyDe::Process*> visited_processes_;

    /**
     * Logger.
     */
    std::list<ForSyDe::Process::Interface*> visited_ports_;
};

}

#endif