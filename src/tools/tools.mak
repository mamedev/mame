###########################################################################
#
#   tools.mak
#
#   MAME tools makefile
#
###########################################################################
#
#   Copyright Aaron Giles
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions are
#   met:
#
#       * Redistributions of source code must retain the above copyright
#         notice, this list of conditions and the following disclaimer.
#       * Redistributions in binary form must reproduce the above copyright
#         notice, this list of conditions and the following disclaimer in
#         the documentation and/or other materials provided with the
#         distribution.
#       * Neither the name 'MAME' nor the names of its contributors may be
#         used to endorse or promote products derived from this software
#         without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
#   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#   DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
#   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
#   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
#   IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#   POSSIBILITY OF SUCH DAMAGE.
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
	unidasm$(EXE) \
	ldresample$(EXE) \
	ldverify$(EXE) \
	regrep$(EXE) \
	srcclean$(EXE) \
	src2html$(EXE) \
	split$(EXE) \
	pngcmp$(EXE) \
	nltool$(EXE) \

ifdef USE_SQLITE
TOOLS += sqlite3$(EXE)
endif


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

chdman$(EXE): $(CHDMANOBJS) $(LIBUTIL) $(ZLIB) $(EXPAT) $(FLAC_LIB) $(7Z_LIB) $(LIBOCORE)
	$(CC) $(CDEFS) $(CFLAGS) -c $(SRC)/version.c -o $(VERSIONOBJ)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(VERSIONOBJ) $^ $(LIBS) $(FLAC_LIB) -o $@



#-------------------------------------------------
# jedutil
#-------------------------------------------------

JEDUTILOBJS = \
	$(TOOLSOBJ)/jedutil.o \

jedutil$(EXE): $(JEDUTILOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(EXPAT)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# unidasm
#-------------------------------------------------

UNIDASMOBJS = \
	$(TOOLSOBJ)/unidasm.o \

# TODO: Visual Studio wants $(FLAC_LIB) and $(7Z_LIB) during linking...
unidasm$(EXE): $(UNIDASMOBJS) $(LIBDASM) $(LIBEMU) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(EXPAT) $(FLAC_LIB) $(7Z_LIB)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# ldresample
#-------------------------------------------------

LDRESAMPLEOBJS = \
	$(TOOLSOBJ)/ldresample.o \

ldresample$(EXE): $(LDRESAMPLEOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(FLAC_LIB) $(7Z_LIB) $(EXPAT)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) $(FLAC_LIB) -o $@



#-------------------------------------------------
# ldverify
#-------------------------------------------------

LDVERIFYOBJS = \
	$(TOOLSOBJ)/ldverify.o \

ldverify$(EXE): $(LDVERIFYOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(FLAC_LIB) $(7Z_LIB) $(EXPAT)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) $(FLAC_LIB) -o $@



#-------------------------------------------------
# regrep
#-------------------------------------------------

REGREPOBJS = \
	$(TOOLSOBJ)/regrep.o \

regrep$(EXE): $(REGREPOBJS) $(LIBUTIL) $(FLAC_LIB) $(LIBOCORE) $(ZLIB) $(EXPAT)
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



#-------------------------------------------------
# split
#-------------------------------------------------

SPLITOBJS = \
	$(TOOLSOBJ)/split.o \

# TODO: Visual Studio wants $(FLAC_LIB) and $(7Z_LIB) during linking...
split$(EXE): $(SPLITOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(EXPAT) $(FLAC_LIB) $(7Z_LIB)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# pngcmp
#-------------------------------------------------

PNGCMPOBJS = \
	$(TOOLSOBJ)/pngcmp.o \

pngcmp$(EXE): $(PNGCMPOBJS) $(LIBUTIL) $(FLAC_LIB) $(LIBOCORE) $(ZLIB)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

#-------------------------------------------------
# nltool
#-------------------------------------------------

NLTOOLOBJS = \
	$(TOOLSOBJ)/nltool.o \
	$(NETLISTOBJS) \

nltool$(EXE): $(NLTOOLOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(EXPAT)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

#-------------------------------------------------
# SQLite3
#-------------------------------------------------

SQLITESHELLOBJS = \
	$(LIBOBJ)/sqlite3/shell.o

sqlite3$(EXE): $(SQLITESHELLOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(SQLITE3_LIB)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

$(LIBOBJ)/sqlite3/shell.o: $(LIBSRC)/sqlite3/shell.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@ -DUSE_SYSTEM_SQLITE=0 -Wno-write-strings
