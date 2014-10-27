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
MAKEMAK_TARGET = $(BUILDOUT)/makemak$(BUILD_EXE)
MAKELIST_TARGET = $(BUILDOUT)/makelist$(BUILD_EXE)
PNG2BDC_TARGET = $(BUILDOUT)/png2bdc$(BUILD_EXE)
VERINFO_TARGET = $(BUILDOUT)/verinfo$(BUILD_EXE)

FILE2STR = $(FILE2STR_TARGET)
MAKEDEP = $(MAKEDEP_TARGET)
MAKEMAK = $(MAKEMAK_TARGET)
MAKELIST = $(MAKELIST_TARGET)
PNG2BDC = $(PNG2BDC_TARGET)
VERINFO = $(VERINFO_TARGET)

ifneq ($(CROSS_BUILD),1)
BUILD += \
	$(FILE2STR_TARGET) \
	$(MAKEDEP_TARGET) \
	$(MAKEMAK_TARGET) \
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
	$(NATIVELD) $(NATIVELDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# makedep
#-------------------------------------------------

MAKEDEPOBJS = \
	$(BUILDOBJ)/makedep.o \

$(MAKEDEP_TARGET): $(MAKEDEPOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(FLAC_LIB)
	@echo Linking $@...
	$(NATIVELD) $(NATIVELDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# makemak
#-------------------------------------------------

MAKEMAKOBJS = \
	$(BUILDOBJ)/makemak.o \

# TODO: 7z and flac - really?
$(MAKEMAK_TARGET): $(MAKEMAKOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(FLAC_LIB) $(7Z_LIB)
	@echo Linking $@...
	$(NATIVELD) $(NATIVELDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# makelist
#-------------------------------------------------

MAKELISTOBJS = \
	$(BUILDOBJ)/makelist.o \

# TODO: 7z and flac - really?
$(MAKELIST_TARGET): $(MAKELISTOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(FLAC_LIB) $(7Z_LIB)
	@echo Linking $@...
	$(NATIVELD) $(NATIVELDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# png2bdc
#-------------------------------------------------

PNG2BDCOBJS = \
	$(BUILDOBJ)/png2bdc.o \

$(PNG2BDC_TARGET): $(PNG2BDCOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(FLAC_LIB) $(7Z_LIB)
	@echo Linking $@...
	$(NATIVELD) $(NATIVELDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# verinfo
#-------------------------------------------------

VERINFOOBJS = \
	$(BUILDOBJ)/verinfo.o

$(VERINFO_TARGET): $(VERINFOOBJS) $(LIBOCORE) $(FLAC_LIB) $(7Z_LIB)
	@echo Linking $@...
	$(NATIVELD) $(NATIVELDFLAGS) $^ $(LIBS) -o $@

endif # CROSS_BUILD
