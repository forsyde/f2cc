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

MODULE      = forsyde
MOC			= SY
SRCFILES    = zipxsy.cpp unzipxsy.cpp inport.cpp outport.cpp\
              coalescedmapsy.cpp parallelmapsy.cpp delaysy.cpp fanoutsy.cpp \
              mapsy.cpp combsy.cpp zipwithnsy.cpp
MOCOBJPATH	= $(OBJPATH)/$(MODULE)/$(MOC)
OBJECTS     = $(addprefix $(MOCOBJPATH)/, \
                $(addsuffix .o, $(basename $(SRCFILES))) \
               )

build: $(MOCOBJPATH) prebuild $(OBJECTS)
	@printf $(POSTBUILDMSG)


$(MOCOBJPATH):
	@mkdir -p $@

prebuild:
	@printf $(subst %,$(MOC),$(PREBUILDMSG))

$(MOCOBJPATH)/%.o: %.cpp %.h
	@printf $(subst %,$<,$(ITEMBUILDMSG))
	@$(CC) $(CCFLAGS) -o $@ -c $<
	

.PHONY: prebuild $(MOCOBJPATH)/
