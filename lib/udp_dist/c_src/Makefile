include $(ERL_TOP)/make/target.mk
include $(ERL_TOP)/make/$(TARGET)/otp.mk
include $(ERL_TOP)/make/$(TARGET)/otp_ded.mk

CC = $(DED_CC)
LD = $(DED_LD)
LIBS = $(DED_LIBS)

# ----------------------------------------------------
# Application version
# ----------------------------------------------------
include ../vsn.mk
VSN=$(UDP_VSN)

# ----------------------------------------------------
# Release directory specification
# ----------------------------------------------------
RELSYSDIR = $(RELEASE_PATH)/lib/udp_dist-$(VSN)


# ----------------------------------------------------
# FLAGS misc
# ----------------------------------------------------

EI_LIBDIR = $(ERL_TOP)/lib/erl_interface/obj/$(TARGET)
EI_INCLUDES = $(ERL_TOP)/lib/erl_interface/include
ERTS_INCLUDES = $(ERL_TOP)/erts/emulator/beam

# ----------------------------------------------------
# FLAGS
# ----------------------------------------------------
CFLAGS = $(DED_INCLUDES) $(EI_INCLUDES) $(ERTS_INCLUDES) $(DED_CFLAGS)
LDFLAGS += $(DED_LDFLAGS) -fPIC -Wall -fmessage-length=0 -fno-common

# ----------------------------------------------------
# Target Specs
# ----------------------------------------------------

OBJ_FILES = $(OBJDIR)/eliot_udp.o


ifeq ($(TARGET),win32)
SHARED_OBJ_FILE = $(LIBDIR)/eliot_udp.dll
CLIB_FLAGS =
LN=cp
else
ifeq ($(findstring vxworks,$(TARGET)),vxworks)
SHARED_OBJ_FILE = $(LIBDIR)/eliot_udp.eld
CLIB_FLAGS =
else
SHARED_OBJ_FILE = $(LIBDIR)/eliot_udp.so
CLIB_FLAGS = -lc
endif
LN= ln -s
endif

# ----------------------------------------------------
# Targets
# ----------------------------------------------------

_create_dirs := $(shell mkdir -p $(OBJDIR) $(LIBDIR))

opt: $(SHARED_OBJ_FILE)

debug: opt

clean:
	rm -f core *~ 
	rm -f $(LIBDIR)/*
	rm -f $(OBJDIR)/*

docs:

# ----------------------------------------------------
# Special Build Targets
# ----------------------------------------------------


$(OBJDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) -O3 -o $@ $<

$(SHARED_OBJ_FILE): $(OBJ_FILES)
	$(LD) $(LDFLAGS) -o $(SHARED_OBJ_FILE) $(OBJ_FILES) $(CLIB_FLAGS) $(LIBS)

# ----------------------------------------------------
# Release Target
# ----------------------------------------------------
include $(ERL_TOP)/make/otp_release_targets.mk

release_spec: opt
	$(INSTALL_DIR) "$(RELSYSDIR)/priv/lib"
	$(INSTALL_PROGRAM) $(NIF_SHARED_OBJ_FILE) "$(RELSYSDIR)/priv/lib"
	$(INSTALL_DIR) "$(RELSYSDIR)/c_src"
	$(INSTALL_DATA) *.c "$(RELSYSDIR)/c_src"

release_docs_spec:

