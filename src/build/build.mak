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
MAKELIST_TARGET = $(BUILDOUT)/makelist$(BUILD_EXE)
PNG2BDC_TARGET = $(BUILDOUT)/png2bdc$(BUILD_EXE)
VERINFO_TARGET = $(BUILDOUT)/verinfo$(BUILD_EXE)

MAKEDEP = $(MAKEDEP_TARGET)
MAKEMAK = $(MAKEMAK_TARGET)
MAKELIST = $(MAKELIST_TARGET)
PNG2BDC = $(PNG2BDC_TARGET)
VERINFO = $(VERINFO_TARGET)

ifneq ($(TERM),cygwin)
ifeq ($(TARGETOS),win32)
MAKEDEP = $(subst /,\,$(MAKEDEP_TARGET))
MAKEMAK = $(subst /,\,$(MAKEMAK_TARGET))
MAKELIST = $(subst /,\,$(MAKELIST_TARGET))
PNG2BDC = $(subst /,\,$(PNG2BDC_TARGET))
VERINFO = $(subst /,\,$(VERINFO_TARGET))
endif
endif

ifneq ($(CROSS_BUILD),1)
BUILD += \
	$(MAKEDEP_TARGET) \
	$(MAKEMAK_TARGET) \
	$(MAKELIST_TARGET) \
	$(PNG2BDC_TARGET) \
	$(VERINFO_TARGET) \



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
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



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
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# makelist
#-------------------------------------------------

MAKELISTOBJS = \
	$(BUILDOBJ)/makelist.o \
	$(OBJ)/lib/util/astring.o \
	$(OBJ)/lib/util/corealloc.o \
	$(OBJ)/lib/util/cstrpool.o \
	$(OBJ)/lib/util/corefile.o \
	$(OBJ)/lib/util/unicode.o \
	$(OBJ)/lib/util/tagmap.o \

$(MAKELIST_TARGET): $(MAKELISTOBJS) $(LIBOCORE) $(ZLIB)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# png2bdc
#-------------------------------------------------

PNG2BDCOBJS = \
	$(BUILDOBJ)/png2bdc.o \
	$(OBJ)/lib/util/astring.o \
	$(OBJ)/lib/util/corefile.o \
	$(OBJ)/lib/util/corealloc.o \
	$(OBJ)/lib/util/bitmap.o \
	$(OBJ)/lib/util/png.o \
	$(OBJ)/lib/util/palette.o \
	$(OBJ)/lib/util/unicode.o \

$(PNG2BDC_TARGET): $(PNG2BDCOBJS) $(LIBOCORE) $(ZLIB)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# verinfo
#-------------------------------------------------

VERINFOOBJS = \
	$(BUILDOBJ)/verinfo.o

$(VERINFO_TARGET): $(VERINFOOBJS) $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

else
#-------------------------------------------------
# It's a CROSS_BUILD. Ensure the targets exist.
#-------------------------------------------------
$(MAKEDEP_TARGET):
	@echo $@ should be built natively. Nothing to do.

$(MAKELIST_TARGET):
	@echo $@ should be built natively. Nothing to do.

$(PNG2BDC_TARGET):
	@echo $@ should be built natively. Nothing to do.

$(VERINFO_TARGET):
	@echo $@ should be built natively. Nothing to do.

endif # CROSS_BUILD
