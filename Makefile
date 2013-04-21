# Copyright (c) 2011-2012 Gabriel Hjort Blindell <ghb@kth.se>
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

unexinterface

SEP                     = "=============================="
exinterface PREBUILDMSG      = "$(SEP)\n Building % module...\n"
exinterface POSTBUILDMSG     = " Done.\n$(SEP)\n\n"
exinterface ITEMBUILDMSG     = " * %\n"
exinterface PRELINKMSG       = "$(SEP)\n Linking...\n"
exinterface POSTLINKMSG      = $(POSTBUILDMSG)
exinterface ITEMLINKMSG      = " * %\n"
exinterface PREDOCSBUILDMSG  = "$(SEP)\n Building API docs...\n"
exinterface POSTDOCSBUILDMSG = " Done.\n$(SEP)\n\n"
exinterface PARTDOCSBUILDMSG = "%...\n"


exinterface TARGET     = bin
exinterface TARGETPATH = $(CURDIR)/$(TARGET)
exinterface DOMAKE     = $(MAKE) --no-print-directory
TESTMODELSPATH    = testmodels
TESTBENCHPATH     = testbench

build: $(TARGET)
	@$(DOMAKE) -C ./source

test_environment: clean build
	cp $(TESTMODELSPATH)/*.* $(TARGET)
	cp $(TESTBENCHPATH)/*.* $(TARGET)

docs:
	@$(DOMAKE) -C ./source docs

tarball:
	tar -cvf f2cc.tar \
        --exclude='.svn' \
        --exclude='testmodels' \
        --exclude='bin' \
        --exclude='api_tmp' \
        --exclude='dox.log' \
        .
	gzip f2cc.tar

help:
	@printf "Usage:"
	@printf
	@printf "make:       same as 'make build'"
	@printf "make build: builds the entire f2cc"
	@printf "make docs:  generates the Doxygen API"

$(TARGET):
	@mkdir -p $(TARGET)

clean: preclean doclean
	@printf $(POSTBUILDMSG)

preclean:
	@printf $(subst Building % module,Cleaning modules,$(PREBUILDMSG))

doclean:
	@rm -rf $(TARGET)

.PHONY: clean preclean doclean all $(TARGET) docs
