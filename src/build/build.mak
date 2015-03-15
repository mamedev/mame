###########################################################################
#
#   build.mak
#
#   MAME build tools makefile
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################

OBJDIRS += \
	$(BUILDOBJ) \



#-------------------------------------------------
# set of build targets
#-------------------------------------------------

MAKEDEP_TARGET = $(BUILDOUT)/makedep$(BUILD_EXE)
MAKEMAK_TARGET = $(BUILDOUT)/makemak$(BUILD_EXE)

MAKEDEP = $(MAKEDEP_TARGET)
MAKEMAK = $(MAKEMAK_TARGET)

ifneq ($(TERM),cygwin)
ifeq ($(OS),Windows_NT)
MAKEDEP = $(subst /,\,$(MAKEDEP_TARGET))
MAKEMAK = $(subst /,\,$(MAKEMAK_TARGET))
endif
endif

ifneq ($(CROSS_BUILD),1)
BUILD += \
	$(MAKEDEP_TARGET) \
	$(MAKEMAK_TARGET) \



#-------------------------------------------------
# makedep
#-------------------------------------------------

MAKEDEPOBJS = \
	$(BUILDOBJ)/makedep.o \
	$(OBJ)/lib/util/astring.o \
	$(OBJ)/lib/util/corealloc.o \
	$(OBJ)/lib/util/corefile.o \
	$(OBJ)/lib/util/unicode.o \
	$(OBJ)/lib/util/tagmap.o \

$(MAKEDEP_TARGET): $(MAKEDEPOBJS) $(LIBOCORE) $(ZLIB)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(BASELIBS) -o $@



#-------------------------------------------------
# makemak
#-------------------------------------------------

MAKEMAKOBJS = \
	$(BUILDOBJ)/makemak.o \
	$(OBJ)/lib/util/astring.o \
	$(OBJ)/lib/util/corealloc.o \
	$(OBJ)/lib/util/corefile.o \
	$(OBJ)/lib/util/corestr.o \
	$(OBJ)/lib/util/unicode.o \
	$(OBJ)/lib/util/tagmap.o \

$(MAKEMAK_TARGET): $(MAKEMAKOBJS) $(LIBOCORE) $(ZLIB)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(BASELIBS) -o $@


else
#-------------------------------------------------
# It's a CROSS_BUILD. Ensure the targets exist.
#-------------------------------------------------
$(MAKEDEP_TARGET):
	@echo $@ should be built natively. Nothing to do.

endif # CROSS_BUILD
