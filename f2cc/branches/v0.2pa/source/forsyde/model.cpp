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

#include "model.h"
#include "../tools/tools.h"
#include <map>
#include <list>
#include <new>

using namespace f2cc::ForSyDe;
using std::string;
using std::map;
using std::list;
using std::pair;
using std::bad_alloc;

Model::Model() throw() {}

Model::~Model() throw() {
    destroyAllLeafs();
}

bool Model::addLeaf(Leaf* leaf)
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

void Model::addLeafs(map<const Id, Leaf*> leafs)
    throw(OutOfMemoryException) {
    try {
        leafs_.insert(leafs.begin(), leafs.end());
    }
    catch(bad_alloc&) {
        THROW_EXCEPTION(OutOfMemoryException);
    }
}

Leaf* Model::getLeaf(const Id& id) throw() {
    map<const Id, Leaf*>::iterator it = findLeaf(id);
    return it != leafs_.end() ? it->second : NULL;
}

int Model::getNumLeafs() const throw() {
    return leafs_.size();
}

list<Leaf*> Model::getLeafs() throw() {
    list<Leaf*> leafs;
    map<const Id, Leaf*>::iterator it;
    for (it = leafs_.begin(); it != leafs_.end(); ++it) {
        leafs.push_back(it->second);
    }
    return leafs;
}

bool Model::deleteLeaf(const Id& id) throw() {
    map<const Id, Leaf*>::iterator it = findLeaf(id);
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


Id Model::getUniqueLeafId() const throw() {
    return getUniqueLeafId("");
}

Id Model::getUniqueLeafId(const string& prefix) const throw() {
    for (int i = 1; ; ++i) {
        Id new_id = Id(prefix + tools::toString(i));
        if (leafs_.find(new_id) == leafs_.end()) return new_id;
    }
}

void Model::destroyAllLeafs() throw() {
    map<const Id, Leaf*>::iterator it;
    for (it=leafs_.begin(); it != leafs_.end(); ++it) {
        delete it->second;
    }
}

map<const Id, Leaf*>::iterator Model::findLeaf(const Id& id) throw() {
    return leafs_.find(id);
}


