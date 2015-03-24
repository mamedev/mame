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
	$(BIN)romcmp$(EXE) \
	$(BIN)chdman$(EXE) \
	$(BIN)jedutil$(EXE) \
	$(BIN)unidasm$(EXE) \
	$(BIN)ldresample$(EXE) \
	$(BIN)ldverify$(EXE) \
	$(BIN)regrep$(EXE) \
	$(BIN)srcclean$(EXE) \
	$(BIN)src2html$(EXE) \
	$(BIN)split$(EXE) \
	$(BIN)pngcmp$(EXE) \
	$(BIN)nltool$(EXE) \


#-------------------------------------------------
# romcmp
#-------------------------------------------------

ROMCMPOBJS = \
	$(TOOLSOBJ)/romcmp.o \

$(BIN)romcmp$(EXE): $(ROMCMPOBJS) $(LIBUTIL) $(ZLIB) $(EXPAT) $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(BASELIBS) -o $@



#-------------------------------------------------
# chdman
#-------------------------------------------------

CHDMANOBJS = \
	$(TOOLSOBJ)/chdman.o \

$(BIN)chdman$(EXE): $(CHDMANOBJS) $(LIBUTIL) $(ZLIB) $(EXPAT) $(FLAC_LIB) $(7Z_LIB) $(LIBOCORE)
	$(CC) $(CDEFS) $(CFLAGS) -c $(SRC)/version.c -o $(VERSIONOBJ)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(VERSIONOBJ) $^ $(BASELIBS) $(FLAC_LIB) -o $@



#-------------------------------------------------
# jedutil
#-------------------------------------------------

JEDUTILOBJS = \
	$(TOOLSOBJ)/jedutil.o \

$(BIN)jedutil$(EXE): $(JEDUTILOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(EXPAT)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(BASELIBS) -o $@



#-------------------------------------------------
# unidasm
#-------------------------------------------------

UNIDASMOBJS = \
	$(TOOLSOBJ)/unidasm.o \

# TODO: Visual Studio wants $(FLAC_LIB) and $(7Z_LIB) during linking...
$(BIN)unidasm$(EXE): $(UNIDASMOBJS) $(LIBDASM) $(LIBEMU) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(EXPAT) $(FLAC_LIB) $(7Z_LIB)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(BASELIBS) -o $@



#-------------------------------------------------
# ldresample
#-------------------------------------------------

LDRESAMPLEOBJS = \
	$(TOOLSOBJ)/ldresample.o \

$(BIN)ldresample$(EXE): $(LDRESAMPLEOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(FLAC_LIB) $(7Z_LIB) $(EXPAT)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(BASELIBS) $(FLAC_LIB) -o $@



#-------------------------------------------------
# ldverify
#-------------------------------------------------

LDVERIFYOBJS = \
	$(TOOLSOBJ)/ldverify.o \

$(BIN)ldverify$(EXE): $(LDVERIFYOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(FLAC_LIB) $(7Z_LIB) $(EXPAT)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(BASELIBS) $(FLAC_LIB) -o $@



#-------------------------------------------------
# regrep
#-------------------------------------------------

REGREPOBJS = \
	$(TOOLSOBJ)/regrep.o \

$(BIN)regrep$(EXE): $(REGREPOBJS) $(LIBUTIL) $(FLAC_LIB) $(LIBOCORE) $(ZLIB) $(EXPAT)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(BASELIBS) -o $@



#-------------------------------------------------
# srcclean
#-------------------------------------------------

SRCCLEANOBJS = \
	$(TOOLSOBJ)/srcclean.o \

$(BIN)srcclean$(EXE): $(SRCCLEANOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(EXPAT)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(BASELIBS) -o $@



#-------------------------------------------------
# src2html
#-------------------------------------------------

SRC2HTMLOBJS = \
	$(TOOLSOBJ)/src2html.o \

$(BIN)src2html$(EXE): $(SRC2HTMLOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(EXPAT)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(BASELIBS) -o $@



#-------------------------------------------------
# split
#-------------------------------------------------

SPLITOBJS = \
	$(TOOLSOBJ)/split.o \

# TODO: Visual Studio wants $(FLAC_LIB) and $(7Z_LIB) during linking...
$(BIN)split$(EXE): $(SPLITOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(EXPAT) $(FLAC_LIB) $(7Z_LIB)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(BASELIBS) -o $@



#-------------------------------------------------
# pngcmp
#-------------------------------------------------

PNGCMPOBJS = \
	$(TOOLSOBJ)/pngcmp.o \

$(BIN)pngcmp$(EXE): $(PNGCMPOBJS) $(LIBUTIL) $(FLAC_LIB) $(LIBOCORE) $(ZLIB)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(BASELIBS) -o $@

#-------------------------------------------------
# nltool
#-------------------------------------------------

NLTOOLOBJS = \
	$(TOOLSOBJ)/nltool.o \
	$(NETLISTOBJS) \

# TODO: Visual Studio wants $(FLAC_LIB) and $(7Z_LIB) during linking...
$(BIN)nltool$(EXE): $(NLTOOLOBJS) $(LIBUTIL) $(LIBOCORE) $(ZLIB) $(EXPAT) $(FLAC_LIB) $(7Z_LIB)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(BASELIBS) -o $@

