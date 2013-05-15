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

#include "model.h"
#include "composite.h"
#include "../tools/tools.h"
#include <map>
#include <list>
#include <new>

using namespace f2cc::Forsyde;
using std::string;
using std::map;
using std::list;
using std::pair;
using std::bad_alloc;

Model::Model() throw() {}

Model::~Model() throw() {
    destroyAllProcesses();
    destroyAllComposites();
}

bool Model::addProcess(Leaf* leaf)
    throw(InvalidArgumentException, OutOfMemoryException) {
    if (!leaf) {
        THROW_EXCEPTION(InvalidArgumentException, "\"leaf\" must not be "
                        "NULL");
    }

    try {
        pair<map<const Id, Leaf*>::iterator, bool>
            result = leafs_.insert(
                pair<const Id, Leaf*>(
                    *leaf->getId(), leaf));
        return result.second;
    }
    catch(bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

void Model::addProcesses(map<const Id, Leaf*> leafes)
    throw(OutOfMemoryException) {
    try {
        leafs_.insert(leafes.begin(), leafes.end());
    }
    catch(bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}


Leaf* Model::getProcess(const Id& id) throw() {
    map<const Id, Leaf*>::iterator it = findProcess(id);
    return it != leafs_.end() ? it->second : NULL;
}

int Model::getNumProcesses() const throw() {
    return leafs_.size();
}

list<Leaf*> Model::getProcesses() throw() {
    list<Leaf*> leafes;
    map<const Id, Leaf*>::iterator it;
    for (it = leafs_.begin(); it != leafs_.end(); ++it) {
        leafes.push_back(it->second);
    }
    return leafes;
}

map<const Id, Leaf*> Model::getProcessesMap() throw() {
    return leafs_;
}

bool Model::deleteProcess(const Id& id) throw() {
    map<const Id, Leaf*>::iterator it = findProcess(id);
    if (it != leafs_.end()) {
    	Leaf* removed_leaf = it->second;
        leafs_.erase(it);
        delete removed_leaf;
        return true;
    }
    else {
        return false;
    }
}
bool Model::removeProcess(const Id& id) throw() {
    map<const Id, Leaf*>::iterator it = findProcess(id);
    if (it != leafs_.end()) {
    	Leaf* saved_leaf = it->second;
        leafs_.erase(it);
        return saved_leaf;
    }
    else {
        return false;
    }
}

Id Model::getUniqueProcessId() const throw() {
    return getUniqueProcessId("");
}

Id Model::getUniqueProcessId(const string& prefix) const throw() {
    for (int i = 1; ; ++i) {
        Id new_id = Id(prefix + tools::toString(i));
        if (leafs_.find(new_id) == leafs_.end()) return new_id;
    }
}

void Model::destroyAllProcesses() throw() {
    map<const Id, Leaf*>::iterator it;
    for (it=leafs_.begin(); it != leafs_.end(); ++it) {
        delete it->second;
    }
}

map<const Id, Leaf*>::iterator Model::findProcess(const Id& id) throw() {
    return leafs_.find(id);
}

///////////////////////////////////////////

bool Model::addComposite(Composite* composite)
    throw(InvalidArgumentException, OutOfMemoryException) {
    if (!composite) {
        THROW_EXCEPTION(InvalidArgumentException, "\"composite\" must not be "
                        "NULL");
    }

    try {
        pair<map<const Id, Composite*>::iterator, bool>
            result = composites_.insert(
                pair<const Id, Composite*>(
                    *composite->getId(), composite));
        return result.second;
    }
    catch(bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

void Model::addComposites(map<const Id, Composite*> compositees)
    throw(OutOfMemoryException) {
    try {
        composites_.insert(compositees.begin(), compositees.end());
    }
    catch(bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}


Composite* Model::getComposite(const Id& id) throw() {
    map<const Id, Composite*>::iterator it = findComposite(id);
    return it != composites_.end() ? it->second : NULL;
}

int Model::getNumComposites() const throw() {
    return composites_.size();
}

list<Composite*> Model::getComposites() throw() {
    list<Composite*> compositees;
    map<const Id, Composite*>::iterator it;
    for (it = composites_.begin(); it != composites_.end(); ++it) {
        compositees.push_back(it->second);
    }
    return compositees;
}

map<const Id, Composite*> Model::getCompositesMap() throw() {
    return composites_;
}

bool Model::deleteComposite(const Id& id) throw() {
    map<const Id, Composite*>::iterator it = findComposite(id);
    if (it != composites_.end()) {
    	Composite* removed_composite = it->second;
        composites_.erase(it);
        delete removed_composite;
        return true;
    }
    else {
        return false;
    }
}
bool Model::removeComposite(const Id& id) throw() {
    map<const Id, Composite*>::iterator it = findComposite(id);
    if (it != composites_.end()) {
    	Composite* saved_composite = it->second;
        composites_.erase(it);
        removeRecursive(saved_composite);
        return saved_composite;
    }
    else {
        return false;
    }
}


Id Model::getUniqueCompositeId() const throw() {
    return getUniqueCompositeId("");
}

Id Model::getUniqueCompositeId(const string& prefix) const throw() {
    for (int i = 1; ; ++i) {
        Id new_id = Id(prefix + tools::toString(i));
        if (composites_.find(new_id) == composites_.end()) return new_id;
    }
}

void Model::destroyAllComposites() throw() {
    map<const Id, Composite*>::iterator it;
    for (it=composites_.begin(); it != composites_.end(); ++it) {
        delete it->second;
    }
}

map<const Id, Composite*>::iterator Model::findComposite(const Id& id) throw() {
    return composites_.find(id);
}

void Model::removeRecursive(Composite* root) throw(){
	list<Leaf*> leafs = root->getProcesses();
	for (list<Leaf*>::iterator it = leafs.begin(); it != leafs.end(); ++it){
		removeProcess(*(*it)->getId());
	}
	list<Composite*> composites = root->getComposites();
	for (list<Composite*>::iterator it = composites.begin(); it != composites.end(); ++it){
		removeComposite(*(*it)->getId());
	}
}


