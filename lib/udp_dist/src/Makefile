#
# %CopyrightBegin%
# 
# Copyright Ericsson AB 1997-2012. All Rights Reserved.
# 
# The contents of this file are subject to the Erlang Public License,
# Version 1.1, (the "License"); you may not use this file except in
# compliance with the License. You should have received a copy of the
# Erlang Public License along with this software. If not, it can be
# retrieved online at http://www.erlang.org/.
# 
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
# the License for the specific language governing rights and limitations
# under the License.
# 
# %CopyrightEnd%
#
#
# Copyright (C) 1997, Ericsson Telecommunications
# Author: Kenneth Lundin 
#
include $(ERL_TOP)/make/target.mk
include $(ERL_TOP)/make/$(TARGET)/otp.mk

# ----------------------------------------------------
# Application version
# ----------------------------------------------------
include ../vsn.mk
VSN=$(UDP_VSN)

# ----------------------------------------------------
# Release directory specification
# ----------------------------------------------------
RELSYSDIR = $(RELEASE_PATH)/lib/udp_dist-$(VSN)

#
# Common Macros
#

EBIN = ../ebin
MODULES= \
	udp \
	udp_dist \
	udp_server

ERL_FILES = $(MODULES:%=%.erl)

TARGET_FILES = $(MODULES:%=$(EBIN)/%.$(EMULATOR))

HRL_FILE = ../include/udp.hrl

APP_FILE = udp_dist.app

APP_SRC = $(APP_FILE).src
APP_TARGET = $(EBIN)/$(APP_FILE)

# ----------------------------------------------------
# FLAGS
# ----------------------------------------------------

ERL_COMPILE_FLAGS += \
	-I$(ERL_TOP)/lib/kernel/include \
	-I$(HRL_FILE) \
	+warn_unused_vars

YRL_FLAGS = 

# ----------------------------------------------------
# Targets
# ----------------------------------------------------

debug opt: $(TARGET_FILES) $(APP_TARGET)


clean:
	rm -f $(TARGET_FILES) $(APP_TARGET)
	rm -f core *~

docs:


info:
	@echo "PARSER_SRC: $(PARSER_SRC)"
	@echo "INSTALL_DATA: $(INSTALL_DATA)"

$(APP_TARGET): $(APP_SRC) ../vsn.mk
	sed -e 's;%VSN%;$(VSN);' $< > $@

# ----------------------------------------------------
# Release Target
# ----------------------------------------------------
include $(ERL_TOP)/make/otp_release_targets.mk

release_spec: opt
	$(INSTALL_DIR) "$(RELSYSDIR)/ebin"
	$(INSTALL_DATA) $(TARGET_FILES) $(APP_TARGET) "$(RELSYSDIR)/ebin"
	$(INSTALL_DIR) "$(RELSYSDIR)/src"
	$(INSTALL_DATA)  $(ERL_FILES) $(HRL_FILES) $(APP_SRC) "$(RELSYSDIR)/src"

# there are no include files to be used by the user
#$(INSTALL_DIR) "$(RELSYSDIR)/include"
#$(INSTALL_DATA) $(HRL_FILES) "$(RELSYSDIR)/include"

release_docs_spec:
