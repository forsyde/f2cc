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

#include "frontend.h"
#include <list>

using namespace f2cc;
using namespace f2cc::ForSyDe;
using std::string;
using std::list;

Frontend::Frontend(Logger& logger) throw() : logger_(logger) {}

Frontend::~Frontend() throw() {}

Model* Frontend::parse(const string& file)
    throw(InvalidArgumentException, FileNotFoundException, IOException,
          ParseException, InvalidModelException, RuntimeException) {
    if (file.length() == 0) {
        THROW_EXCEPTION(InvalidArgumentException, "\"file\" must not be empty "
                        "string");
    }

    Model* model = createModel(file);

    logger_.logMessage(Logger::INFO, "Checking that the internal model is "
                       "sane...");
    checkModel(model);
    logger_.logMessage(Logger::INFO, "All checks passed");

    postCheckFixes(model);

    return model;
}

void Frontend::checkModel(Model* model)
    throw(InvalidArgumentException, InvalidModelException, IOException,
          RuntimeException) {
    list<Leaf*> leafs = model->getLeafs();
    list<Leaf*>::iterator leaf_it;
    for (leaf_it = leafs.begin(); leaf_it != leafs.end();
         ++leaf_it) {
        Leaf* leaf = *leaf_it;
        logger_.logMessage(Logger::DEBUG,
                           string("Checking leaf \"")
                           + leaf->getId()->getString() + "\"");

        // Leaf type-related check
        try {
            leaf->check();
        } catch (InvalidLeafException& ex) {
            THROW_EXCEPTION(InvalidModelException, ex.getMessage());
        }

        // Interface checks
        list<Leaf::Interface*> interfaces = leaf->getInPorts();
        list<Leaf::Interface*>::iterator interface_it;
        for (interface_it = interfaces.begin(); interface_it != interfaces.end(); ++interface_it) {
            checkInterface(*interface_it, model);
        }
        interfaces = leaf->getOutPorts();
        for (interface_it = interfaces.begin(); interface_it != interfaces.end(); ++interface_it) {
            checkInterface(*interface_it, model);
        }
    }
}

void Frontend::checkInterface(Leaf::Interface* interface, Model* model)
    throw(InvalidArgumentException, InvalidModelException, IOException,
          RuntimeException) {
    if (!interface) {
        THROW_EXCEPTION(InvalidArgumentException, "\"interface\" must not be NULL");
    }
    if (!model) {
        THROW_EXCEPTION(InvalidArgumentException, "\"model\" must not be NULL");
    }

    if (!interface->isConnected()) {
        THROW_EXCEPTION(InvalidModelException, string("Interface \"")
                        + interface->getId()->getString()
                        + "\" in leaf \""
                        + interface->getLeaf()->getId()->getString()
                        + "\" is unconnected");
    }

    // Check that the interface is not connected to its own leaf
    if (interface->getConnectedInterface()->getLeaf() == interface->getLeaf()) {
        THROW_EXCEPTION(InvalidModelException, string("Interface \"")
                        + interface->getId()->getString()
                        + "\" in leaf \""
                        + interface->getLeaf()->getId()->getString()
                        + "\" is connected to its own model "
                        + "(combinatorial looping)");
    }

    // Check that the other interface belongs to a leaf in the model
    if (!model->getLeaf(*interface->getConnectedInterface()->getLeaf()->getId())) {
        THROW_EXCEPTION(InvalidModelException, string("Interface \"")
                        + interface->getId()->getString()
                        + "\" in leaf \""
                        + interface->getLeaf()->getId()->getString()
                        + "\" is connected to a leaf outside the "
                        + "model");
    }
}

void Frontend::checkModelMore(ForSyDe::Model* model)
    throw(InvalidArgumentException, InvalidModelException, IOException,
          RuntimeException) {}


void Frontend::postCheckFixes(ForSyDe::Model* model)
    throw(InvalidArgumentException, IOException, RuntimeException) {}
