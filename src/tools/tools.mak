###########################################################################
#
#   tools.mak
#
#   MAME tools makefile
#
#   Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################


TOOLSSRC = $(SRC)/tools
TOOLSOBJ = $(OBJ)/tools

OBJDIRS += \
	$(TOOLSOBJ) \



#-------------------------------------------------
# set of tool targets
#-------------------------------------------------

TOOLS += \
	romcmp$(EXE) \
	chdman$(EXE) \
	jedutil$(EXE) \
	makemeta$(EXE) \
	regrep$(EXE) \
	srcclean$(EXE) \
	src2html$(EXE) \



#-------------------------------------------------
# romcmp
#-------------------------------------------------

ROMCMPOBJS = \
	$(TOOLSOBJ)/romcmp.o \

romcmp$(EXE): $(ROMCMPOBJS) $(LIBUTIL) $(ZLIB) $(EXPAT) $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# chdman
#-------------------------------------------------

CHDMANOBJS = \
	$(TOOLSOBJ)/chdman.o \
	$(TOOLSOBJ)/chdcd.o \

chdman$(EXE): $(VERSIONOBJ) $(CHDMANOBJS) $(LIBUTIL) $(ZLIB) $(EXPAT) $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# jedutil
#-------------------------------------------------

JEDUTILOBJS = \
	$(TOOLSOBJ)/jedutil.o \

jedutil$(EXE): $(JEDUTILOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(EXPAT)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# makemeta
#-------------------------------------------------

MAKEMETAOBJS = \
	$(TOOLSOBJ)/makemeta.o \

makemeta$(EXE): $(MAKEMETAOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(EXPAT)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# regrep
#-------------------------------------------------

REGREPOBJS = \
	$(TOOLSOBJ)/regrep.o \

regrep$(EXE): $(REGREPOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(EXPAT)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# srcclean
#-------------------------------------------------

SRCCLEANOBJS = \
	$(TOOLSOBJ)/srcclean.o \

srcclean$(EXE): $(SRCCLEANOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(EXPAT)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# src2html
#-------------------------------------------------

SRC2HTMLOBJS = \
	$(TOOLSOBJ)/src2html.o \

src2html$(EXE): $(SRC2HTMLOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(EXPAT)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@
