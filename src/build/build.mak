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

FILE2STR_TARGET = $(BUILDOUT)/file2str$(BUILD_EXE)
MAKEDEP_TARGET = $(BUILDOUT)/makedep$(BUILD_EXE)
MAKELIST_TARGET = $(BUILDOUT)/makelist$(BUILD_EXE)
PNG2BDC_TARGET = $(BUILDOUT)/png2bdc$(BUILD_EXE)
VERINFO_TARGET = $(BUILDOUT)/verinfo$(BUILD_EXE)

ifeq ($(TARGETOS),win32)
FILE2STR = $(subst /,\,$(FILE2STR_TARGET))
MAKEDEP = $(subst /,\,$(MAKEDEP_TARGET))
MAKELIST = $(subst /,\,$(MAKELIST_TARGET))
PNG2BDC = $(subst /,\,$(PNG2BDC_TARGET))
VERINFO = $(subst /,\,$(VERINFO_TARGET))
else
FILE2STR = $(FILE2STR_TARGET)
MAKEDEP = $(MAKEDEP_TARGET)
MAKELIST = $(MAKELIST_TARGET)
PNG2BDC = $(PNG2BDC_TARGET)
VERINFO = $(VERINFO_TARGET)
endif

ifneq ($(CROSS_BUILD),1)
BUILD += \
	$(FILE2STR_TARGET) \
	$(MAKEDEP_TARGET) \
	$(MAKELIST_TARGET) \
	$(PNG2BDC_TARGET) \
	$(VERINFO_TARGET) \



#-------------------------------------------------
# file2str
#-------------------------------------------------

FILE2STROBJS = \
	$(BUILDOBJ)/file2str.o \

$(FILE2STR_TARGET): $(FILE2STROBJS) $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# makedep
#-------------------------------------------------

MAKEDEPOBJS = \
	$(BUILDOBJ)/makedep.o \

$(MAKEDEP_TARGET): $(MAKEDEPOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# makelist
#-------------------------------------------------

MAKELISTOBJS = \
	$(BUILDOBJ)/makelist.o \

$(MAKELIST_TARGET): $(MAKELISTOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# png2bdc
#-------------------------------------------------

PNG2BDCOBJS = \
	$(BUILDOBJ)/png2bdc.o \

$(PNG2BDC_TARGET): $(PNG2BDCOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB)
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

endif
