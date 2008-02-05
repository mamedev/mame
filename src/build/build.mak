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


BUILDSRC = $(SRC)/build
BUILDOBJ = $(OBJ)/build
BUILDOUT = $(BUILDOBJ)

OBJDIRS += \
	$(BUILDOBJ) \



#-------------------------------------------------
# set of build targets
#-------------------------------------------------

FILE2STR = $(BUILDOUT)/file2str$(EXE)
PNG2BDC = $(BUILDOUT)/png2bdc$(EXE)

ifneq ($(CROSS_BUILD),1)
BUILD += \
	$(FILE2STR) \
	$(PNG2BDC) \



#-------------------------------------------------
# file2str
#-------------------------------------------------

FILE2STROBJS = \
	$(BUILDOBJ)/file2str.o \

$(FILE2STR): $(FILE2STROBJS)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ -o $@



#-------------------------------------------------
# png2bdc
#-------------------------------------------------

PNG2BDCOBJS = \
	$(BUILDOBJ)/png2bdc.o \

$(PNG2BDC): $(PNG2BDCOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBUTIL) $(LIBOCORE) $(ZLIB) -o $@

#-------------------------------------------------
# rule for making the verinfo tool
#-------------------------------------------------

VERINFO = $(BUILDOBJ)/verinfo$(EXE)

VERINFOOBJS = \
	$(BUILDOBJ)/verinfo.o

$(VERINFO): $(VERINFOOBJS)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ -o $@

endif
