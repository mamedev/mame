###########################################################################
#
#   windows.mak
#
#   Windows-specific makefile
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


###########################################################################
#################   BEGIN USER-CONFIGURABLE OPTIONS   #####################
###########################################################################


#-------------------------------------------------
# specify build options; see each option below
# for details
#-------------------------------------------------

# uncomment next line to enable a build using Microsoft tools
# MSVC_BUILD = 1

# uncomment next line to enable code analysis using Microsoft tools
# MSVC_ANALYSIS = 1

# uncomment next line to use cygwin compiler
# CYGWIN_BUILD = 1

# set this to the minimum Direct3D version to support (8 or 9)
# DIRECT3D = 9

# set this to the minimum DirectInput version to support (7 or 8)
# DIRECTINPUT = 8


###########################################################################
##################   END USER-CONFIGURABLE OPTIONS   ######################
###########################################################################


#-------------------------------------------------
# object and source roots
#-------------------------------------------------

RENDSRC = $(SRC)/render
RENDOBJ = $(OBJ)/render

OBJDIRS += $(RENDOBJ) $(RENDOBJ)/windows



#-------------------------------------------------
# overrides for the CYGWIN compiler
#-------------------------------------------------

# ifdef CYGWIN_BUILD
# CCOMFLAGS += -mno-cygwin
# LDFLAGS += -mno-cygwin
# endif



#-------------------------------------------------
# due to quirks of using /bin/sh, we need to
# explicitly specify the current path
#-------------------------------------------------

# CURPATH = ./



#-------------------------------------------------
# Windows-specific debug objects and flags
#-------------------------------------------------

# define the x64 ABI to be Windows
# DEFS += -DX64_WINDOWS_ABI

# enable UNICODE flags
# DEFS += -DUNICODE -D_UNICODE
# LDFLAGS += -municode

# debug build: enable guard pages on all memory allocations
# ifdef DEBUG
# DEFS += -DMALLOC_DEBUG
# endif



#-------------------------------------------------
# Windows-specific flags and libraries
#-------------------------------------------------

# ensure we statically link the gcc runtime lib
# LDFLAGS += -static-libgcc
# TODO: needs to use $(CC)
# TEST_GCC = $(shell gcc --version)
# ifeq ($(findstring 4.4.,$(TEST_GCC)),)
	#if we use new tools
# 	LDFLAGS += -static-libstdc++
# endif
# ifeq ($(findstring 4.7.,$(TEST_GCC)),4.7.)
# 	CCOMFLAGS += -Wno-narrowing -Wno-attributes
# endif
# add the windows libraries
# LIBS += -luser32 -lgdi32 -ldsound -ldxguid -lwinmm -ladvapi32 -lcomctl32 -lshlwapi -lwsock32

# ifeq ($(DIRECTINPUT),8)
# LIBS += -ldinput8
# CCOMFLAGS += -DDIRECTINPUT_VERSION=0x0800
# else
# LIBS += -ldinput
# CCOMFLAGS += -DDIRECTINPUT_VERSION=0x0700
# endif

# LIBS += -lcomdlg32

#-------------------------------------------------
# Render core library
#-------------------------------------------------

RENDOBJS = \
	$(RENDOBJ)/drawhal.o    \
	$(RENDOBJ)/video.o \
	$(RENDOBJ)/window.o \
	$(RENDOBJ)/windows/monitor.o \
	$(RENDOBJ)/windows/video.o \
	$(RENDOBJ)/windows/window.o


# CCOMFLAGS += -DDIRECT3D_VERSION=0x0900

# extra dependencies
# $(WINOBJ)/drawdd.o :    $(SRC)/emu/rendersw.c
# $(WINOBJ)/drawgdi.o :   $(SRC)/emu/rendersw.c
# $(WINOBJ)/winmidi.o:    $(SRC)/osd/portmedia/pmmidi.c

# add debug-specific files
# OSDOBJS += \
# 	$(WINOBJ)/debugwin.o



#-------------------------------------------------
# rules for building the libaries
#-------------------------------------------------

$(LIBRENDER): $(RENDOBJS)
