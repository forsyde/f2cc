# Copyright (c) 2011-2013
#     Gabriel Hjort Blindell <ghb@kth.se>
#     George Ungureanu <ugeorge@kth.se>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright notice,
#       this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OF THIS SOFTWARE NOR THE
# COPYRIGHT HOLDERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

SRCFILES = f2cc.cpp
OBJECTS  = $(addprefix $(OBJPATH)/, \
             $(addsuffix .o, $(basename $(SRCFILES))) \
            )
EXEC_ = f2cc
EXEC  = $(addprefix $(TARGETPATH)/, $(EXEC_))

DEPENDENCIES = ticpp exceptions tools logger forsyde forsyde/SY language frontend config \
               synthesizer  
DEPOBJECTS   = $(addprefix $(OBJPATH)/, \
                 $(addsuffix /*.o, $(basename $(DEPENDENCIES))) \
                )

export LIBPATH = $(TARGETPATH)/lib
export OBJPATH = $(TARGETPATH)/obj

export CC      = g++
export CCFLAGS = -Wall \
                 -DSVNVERSION="\"`svnversion`\""
export LDFLAGS =
export AR      = ar
export ARFLAGS = crf

all: $(LIBPATH) $(OBJPATH) link

link: build prelink
	@printf $(subst %,$(EXEC_),$(ITEMLINKMSG))
	@$(CC) $(CCFLAGS) $(LDFLAGS) -o $(TARGETPATH)/$(EXEC_) $(OBJECTS) \
           $(DEPOBJECTS)
	@printf $(POSTLINKMSG)

prelink:
	@printf $(PRELINKMSG)

build: $(DEPENDENCIES) prebuild $(OBJECTS)
	@printf $(POSTBUILDMSG)

prebuild:
	@printf $(subst %,core,$(PREBUILDMSG))

$(LIBPATH) $(OBJPATH):
	@mkdir -p $@

$(DEPENDENCIES):
	@$(DOMAKE) -C ./$@

$(OBJPATH)/%.o: %.cpp
	@printf $(subst %,$<,$(ITEMBUILDMSG))
	@$(CC) $(CCFLAGS) -o $@ -c $<

docs: predocs
	@doxygen dox
	@printf $(POSTDOCSBUILDMSG)
	@cat dox.log

predocs:
	@printf $(PREDOCSBUILDMSG)

.PHONY: prebuild $(LIBPATH) $(OBJPATH) docs predocs $(DEPENDENCIES)
