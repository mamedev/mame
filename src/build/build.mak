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

FILE2STR = $(BUILDOUT)/file2str$(BUILD_EXE)
PNG2BDC = $(BUILDOUT)/png2bdc$(BUILD_EXE)
VERINFO = $(BUILDOUT)/verinfo$(BUILD_EXE)

VERINFO = $(BUILDOUT)/verinfo$(EXE)

ifneq ($(CROSS_BUILD),1)
BUILD += \
	$(FILE2STR) \
	$(PNG2BDC) \
	$(VERINFO) \



#-------------------------------------------------
# file2str
#-------------------------------------------------

FILE2STROBJS = \
	$(BUILDOBJ)/file2str.o \

$(FILE2STR): $(FILE2STROBJS) $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ -o $@



#-------------------------------------------------
# png2bdc
#-------------------------------------------------

PNG2BDCOBJS = \
	$(BUILDOBJ)/png2bdc.o \

$(PNG2BDC): $(PNG2BDCOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ -o $@



#-------------------------------------------------
# verinfo
#-------------------------------------------------

VERINFOOBJS = \
	$(BUILDOBJ)/verinfo.o

$(VERINFO): $(VERINFOOBJS) $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ -o $@

endif
