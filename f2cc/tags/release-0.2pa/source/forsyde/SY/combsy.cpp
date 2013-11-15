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

#include "combsy.h"
#include "../../tools/tools.h"
#include <typeinfo>
#include <list>

using namespace f2cc;
using namespace f2cc::Forsyde::SY;
using std::string;
using std::bad_cast;
using std::list;

Comb::Comb(const Forsyde::Id& id, Forsyde::Hierarchy hierarchy,
 		int cost, CFunction* function) throw()
        : Leaf(id, hierarchy, "sy", cost),  function_(function) {}

Comb::~Comb() throw() {}

CFunction* Comb::getFunction() throw() {
    return function_;
}

bool Comb::operator==(const Leaf& rhs) const throw() {
    if (!Leaf::operator==(rhs)) return false;

    try {
        const Comb& other = dynamic_cast<const Comb&>(rhs);
        if (function_ != other.function_) return false;
    }
    catch (bad_cast&) {
        return false;
    }
    return true;
}

string Comb::type() const throw() {
    return "comb";
}

void Comb::moreChecks() throw(InvalidProcessException) {
    if (getInPorts().size() < 1) {
        THROW_EXCEPTION(InvalidProcessException, string("Leaf \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\" must have at least one (1) in port");
    }
    if (getOutPorts().size() != 1) {
        THROW_EXCEPTION(InvalidProcessException, string("Leaf \"")
                        + getId()->getString() + "\" of type \""
                        + type() + "\" must have exactly one (1) out port");
    }
    checkFunction(function_, getNumInPorts());
    //checkPorts();
}

string Comb::moreToString() const throw() {
    return string("LeafFunction: ") + function_->toString();
}

void Comb::checkFunction(CFunction* function, size_t num_in_ports) const
    throw(InvalidProcessException) {
	if (function->getNumInputParameters() != getNumInPorts()) {
		THROW_EXCEPTION(InvalidProcessException, string("Leaf \"")
						+ getId()->getString() + "\" of type \""
						+ type() + "\": function has "
						+ tools::toString(function->getNumInputParameters())
						+ " but the process has "
						+ tools::toString(getNumInPorts())
		                + " input ports.");
	}
	if (!function->getOutputParameters().front()) {
		THROW_EXCEPTION(InvalidProcessException, string("Leaf \"")
						+ getId()->getString() + "\" of type \""
						+ type() + "\": function has "
						+ "no output parameter");
	}


    size_t i;
    list<CVariable*> input_parameters = function->getInputParameters();
    list<CVariable*>::iterator it;
    for (i = 0, it = input_parameters.begin(); i < num_in_ports; ++i, ++it) {
        CVariable variable = **it;
        CDataType input_data_type = *variable.getDataType();
        if (input_data_type.isArray() && !input_data_type.isConst()) {
            THROW_EXCEPTION(InvalidProcessException, string("Leaf \"")
                            + getId()->getString() + "\" of type \""
                            + type() + "\": input parameter \""
                            + variable.getReferenceString() + "\"is a "
                            "reference or array but not declared const");
        }
    }
}

void Comb::checkPorts()
    throw(InvalidProcessException) {

    list<Leaf::Port*> input_ports = getInPorts();
    for (list<Leaf::Port*>::iterator it = input_ports.begin();
    		it != input_ports.end(); ++it) {
        if (*(*it)->getVariable()->getDataType() != (*it)->getDataType()){
			THROW_EXCEPTION(InvalidProcessException, string("Leaf \"")
							+ getId()->getString() + "\" of type \""
							+ type() + "\": Input port \""
							+ (*it)->toString() + "\" has a different data type "
							"than its pointed variable: \""
							+ (*it)->getVariable()->getLocalVariableDeclarationString()
							+ "\"");
        }
    }
    list<Leaf::Port*> output_ports = getOutPorts();
    for (list<Leaf::Port*>::iterator it = output_ports.begin();
    		it != output_ports.end(); ++it) {
        if (*(*it)->getVariable()->getDataType() != (*it)->getDataType()){
			THROW_EXCEPTION(InvalidProcessException, string("Leaf \"")
							+ getId()->getString() + "\" of type \""
							+ type() + "\": Output port \""
							+ (*it)->toString() + "\" has a different data type "
							"than its pointed variable: \""
							+ (*it)->getVariable()->getLocalVariableDeclarationString()
							+ "\"");
        }
    }
}
