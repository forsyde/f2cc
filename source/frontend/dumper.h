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
 * @brief An XML dumper for the internam \c Forsyde model.
 *
 * The \c XmlDumper class parses the internal representation of the ForSyDe model in a
 * \c Forsyde::ProcessNetwork and dumps it into a single \c XML file. It is useful for debugging
 * purposes and plotting the intermediate stages in the \c ModelModifier and
 * \c SyscModelModifier.
 */
class XmlDumper {
  public:
    /**
     * Creates a dumper.
     *
     * @param logger
     *        Reference to the logger.
     */
	XmlDumper(Logger& logger) throw();

    /**
     * Destroys this dumper. The logger remains open.
     */
    virtual ~XmlDumper() throw();

    /**
     * Dumps a \c Forsyde::ProcessNetwork into an XML file.
     *
     * @param pn
     *        The \c Forsyde::ProcessNetwork that needs to be dumped.
     * @param file
     *        Output file.
     *
     * @throws InvalidArgumentException
     *         When \c file is an empty string or \c pn doesn't exist.
     * @throws IOException
     *         When the file cannot be read or the log file cannot be written.
     * @throws ParseException
     *         When the file fails to be parsed.
     * @throws InvalidModelException
     *         When a process in the process network is invalid.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void dump(Forsyde::ProcessNetwork* pn, const std::string& file)
        throw(InvalidArgumentException, IOException,
              InvalidModelException, RuntimeException);

  private:
    /**
     * Builds an XML \c Document object from a \c Forsyde::ProcessNetwork object.
     *
     * @param pn
     *        The \c Forsyde::ProcessNetwork that needs to be dumped.
     * @param doc
     *        The \c Document object that will hold the XML data.
     *
     * @throws InvalidArgumentException
     *         When \c \c pn doesn't exist.
     * @throws ParseException
     *         When the file fails to be parsed.
     * @throws InvalidModelException
     *         When an element of the process network is invalid.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void dumpProcessNetwork(Forsyde::ProcessNetwork* pn,
    		ticpp::Document doc)
        throw(InvalidArgumentException, InvalidModelException, RuntimeException);

    /**
     * Dumps a \c Forsyde::Composite process into an XML \c Element object.
     *
     * @param composite
     *        The \c Forsyde::Composite process that needs to be dumped.
     * @param parent
     *        The \c Element object that will hold its XML data.
     *
     * @throws InvalidArgumentException
     *         When \c parent or \c composite doesn't exist.
     * @throws ParseException
     *         When the file fails to be parsed.
     * @throws InvalidModelException
     *         When an element of the process network is invalid.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void dumpComposite(Forsyde::Composite* composite, ticpp::Element* parent)
        throw(InvalidArgumentException, InvalidModelException, RuntimeException);

    /**
     * Dumps a \c Forsyde::Leaf process into an XML \c Element object.
     *
     * @param leaf
     *        The \c Forsyde::Leaf process that needs to be dumped.
     * @param parent
     *        The \c Element object that will hold its XML data.
     *
     * @throws InvalidArgumentException
     *         When \c parent or \c leaf doesn't exist.
     * @throws ParseException
     *         When the file fails to be parsed.
     * @throws InvalidModelException
     *         When an element of the process network is invalid.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void dumpLeaf(Forsyde::Leaf* leaf, ticpp::Element* parent)
        throw(InvalidArgumentException, InvalidModelException, RuntimeException);

    /**
     * Dumps a \c Forsyde::Composite::IOPort and its connection inside, and exports them into
     * an XML \c Element object.
     *
     * @param port
     *        The \c Forsyde::Composite::IOPort process that that to be dumped.
     * @param composite
     *        The \c Element object that holds the data for its parent \c Forsyde::Composite process.
     * @param direction
     *        The port's direction (in or out).
     *
     * @throws InvalidArgumentException
     *         When \c port or \c composite doesn't exist.
     * @throws ParseException
     *         When the file fails to be parsed.
     * @throws InvalidModelException
     *         When an element of the process network is invalid.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void dumpPort(Forsyde::Composite::IOPort* port, ticpp::Element* composite,
    		const char* direction)
        throw(InvalidArgumentException, InvalidModelException, RuntimeException);

    /**
     * Dumps a link between two \c Forsyde::Leaf::Port into an XML \c Element object called "signal".
     *
     * @param port
     *        The \c Forsyde::Leaf::Port that that to be dumped, and its connection at the other end.
     * @param composite
     *        The \c Element object that holds the data for its parent \c Forsyde::Composite process.
     *
     * @throws InvalidArgumentException
     *         When \c port or \c composite doesn't exist.
     * @throws ParseException
     *         When the file fails to be parsed.
     * @throws InvalidModelException
     *         When an element of the process network is invalid.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void dumpSignal(Forsyde::Leaf::Port* port, ticpp::Element* composite, bool is_in)
        throw(InvalidArgumentException, InvalidModelException, RuntimeException);

    /**
     * Dumps a link between a \c Forsyde::Composite::IOPort and its connection outside into an XML
     * \c Element object called "signal". It behaves like
     * dumpSignal(Forsyde::Leaf::Port*, ticpp::Element*), but it takes a \c Forsyde::Composite::IOPort
     * as argument, since there may exist uncovered signals with an IOPort as source.
     *
     * @param port
     *        The \c Forsyde::Composite::IOPort that that to be dumped, and its connection at the other end.
     * @param composite
     *        The \c Element object that holds the data for its parent \c Forsyde::Composite process.
     *
     * @throws InvalidArgumentException
     *         When \c port or \c composite doesn't exist.
     * @throws ParseException
     *         When the file fails to be parsed.
     * @throws InvalidModelException
     *         When an element of the process network is invalid.
     * @throws RuntimeException
     *         When something unexpected occurs. This is most likely due to a
     *         bug.
     */
    void dumpIOSignal(Forsyde::Composite::IOPort* port, ticpp::Element* composite, bool is_in)
        throw(InvalidArgumentException, InvalidModelException, RuntimeException);


    /**
     * Checks whether a \c Forsyde::Process was visited during the parsing.
     *
     * @param process
     *        The \c Forsyde::Process that is checked.
     *
	 * @returns \b true if the process was visited.
     *
     * @throws InvalidArgumentExceptison
     *         When \c process doesn't exist.
     */
    bool isVisitedProcess(Forsyde::Process* process) throw(InvalidArgumentException);

    /**
     * Checks whether a \c Forsyde::Process::Interface was visited during the parsing.
     *
     * @param port
     *        The \c Forsyde::Process::Interface that is checked.
     *
	 * @returns \b true if the interface was visited.
     *
     * @throws InvalidArgumentException
     *         When \c process doesn't exist.
     */
    bool isVisitedPort(Forsyde::Process::Interface* port) throw(
    		InvalidArgumentException);

  private:
    /**
     * Logger.
     */
    Logger& logger_;

    /**
     * A list with visited processes.
     */
    std::list<Forsyde::Process*> visited_processes_;

    /**
     * A list with visited interfaces.
     */
    std::list<Forsyde::Process::Interface*> visited_ports_;
};

}

#endif
