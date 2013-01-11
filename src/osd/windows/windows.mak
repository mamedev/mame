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

WINSRC = $(SRC)/osd/$(OSD)
WINOBJ = $(OBJ)/osd/$(OSD)

OBJDIRS += $(WINOBJ)



#-------------------------------------------------
# configure the resource compiler
#-------------------------------------------------

RC = @windres --use-temp-file

RCDEFS = -DNDEBUG -D_WIN32_IE=0x0501

RCFLAGS = -O coff -I $(WINSRC) -I $(WINOBJ)



#-------------------------------------------------
# overrides for the CYGWIN compiler
#-------------------------------------------------

ifdef CYGWIN_BUILD
CCOMFLAGS += -mno-cygwin
LDFLAGS += -mno-cygwin
endif



#-------------------------------------------------
# overrides for the MSVC compiler
#-------------------------------------------------

ifdef MSVC_BUILD

OSPREBUILD = $(VCONV_TARGET)

# append a 'v' prefix if nothing specified
ifndef PREFIX
PREFIX = v
endif

# replace the various compilers with vconv.exe prefixes
CC = @$(VCONV) gcc -I.
LD = @$(VCONV) ld /profile
AR = @$(VCONV) ar
RC = @$(VCONV) windres

# make sure we use the multithreaded runtime
ifdef DEBUG
CCOMFLAGS += /MTd
else
CCOMFLAGS += /MT
endif

# turn on link-time codegen if the MAXOPT flag is also set
ifdef MAXOPT
CCOMFLAGS += /GL
LDFLAGS += /LTCG
AR += /LTCG
endif

# disable warnings and link against bufferoverflowu for 64-bit targets
ifeq ($(PTR64),1)
CCOMFLAGS += /wd4267
#LIBS += -lbufferoverflowu
endif

# enable basic run-time checks in non-optimized build
ifeq ($(OPTIMIZE),0)
CCOMFLAGS += /RTC1
endif

ifdef MSVC_ANALYSIS
CCOMFLAGS += /analyze /wd6011 /wd6328 /wd6204 /wd6244 /wd6385 /wd6308 /wd6246 /wd6031 /wd6326 /analyze:stacksize384112
endif

# enable exception handling for C++
CPPONLYFLAGS += /EHsc

# disable function pointer warnings in C++ which are evil to work around
CPPONLYFLAGS += /wd4191 /wd4060 /wd4065 /wd4640

# disable warning about exception specifications and using this in constructors
CPPONLYFLAGS += /wd4290 /wd4355

# disable performance warnings about casting ints to bools
CPPONLYFLAGS += /wd4800

# disable better packing warning
CPPONLYFLAGS += /wd4371

# disable side effects warning in STL headers
CPPONLYFLAGS += /wd4548

# disable macro redefinition warning
CCOMFLAGS += /wd4005

# disable behavior change: 'member1' called instead of 'member2' warning
CCOMFLAGS += /wd4350

# explicitly set the entry point for UNICODE builds
LDFLAGS += /ENTRY:wmainCRTStartup

# add some VC++-specific defines
DEFS += -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -DXML_STATIC -Dsnprintf=_snprintf -DWIN32

OSDCLEAN = msvcclean

msvcclean:
	@echo Deleting Visual Studio specific files...
	$(RM) *.pdb
	$(RM) *.lib
	$(RM) *.exp

endif


#-------------------------------------------------
# build VCONV
#-------------------------------------------------

VCONV_TARGET = $(BUILDOUT)/vconv$(BUILD_EXE)
VCONV = $(subst /,\,$(VCONV_TARGET))

ifneq ($(CROSS_BUILD),1)
BUILD += \
	$(VCONV_TARGET)
endif

$(VCONV_TARGET): $(WINOBJ)/vconv.o
	@echo Linking $@...
	@gcc.exe -static-libgcc $^ $(LIBS) -lversion -o $@

$(WINOBJ)/vconv.o: $(WINSRC)/vconv.c
	@echo Compiling $<...
	@gcc.exe -O3 -c $< -o $@



#-------------------------------------------------
# due to quirks of using /bin/sh, we need to
# explicitly specify the current path
#-------------------------------------------------

CURPATH = ./



#-------------------------------------------------
# Windows-specific debug objects and flags
#-------------------------------------------------

# define the x64 ABI to be Windows
DEFS += -DX64_WINDOWS_ABI

# enable UNICODE flags
DEFS += -DUNICODE -D_UNICODE
LDFLAGS += -municode

# map all instances of "main" to "utf8_main"
DEFS += -Dmain=utf8_main

# debug build: enable guard pages on all memory allocations
ifdef DEBUG
DEFS += -DMALLOC_DEBUG
endif



#-------------------------------------------------
# Windows-specific flags and libraries
#-------------------------------------------------

# add our prefix files to the mix
CCOMFLAGS += -include $(WINSRC)/winprefix.h

# ensure we statically link the gcc runtime lib
LDFLAGS += -static-libgcc
TEST_GCC = $(shell gcc --version)
ifeq ($(findstring 4.4,$(TEST_GCC)),)
	#if we use new tools
	LDFLAGS += -static-libstdc++
endif
ifeq ($(findstring 4.7.,$(TEST_GCC)),4.7.)
	CCOMFLAGS += -Wno-narrowing -Wno-attributes
endif
# add the windows libraries
LIBS += -luser32 -lgdi32 -ldsound -ldxguid -lwinmm -ladvapi32 -lcomctl32 -lshlwapi -lwsock32

ifeq ($(DIRECTINPUT),8)
LIBS += -ldinput8
CCOMFLAGS += -DDIRECTINPUT_VERSION=0x0800
else
LIBS += -ldinput
CCOMFLAGS += -DDIRECTINPUT_VERSION=0x0700
endif

LIBS += -lcomdlg32

#-------------------------------------------------
# OSD core library
#-------------------------------------------------

OSDCOREOBJS = \
	$(WINOBJ)/main.o    \
	$(WINOBJ)/strconv.o \
	$(WINOBJ)/windir.o \
	$(WINOBJ)/winfile.o \
	$(WINOBJ)/winmisc.o \
	$(WINOBJ)/winsync.o \
	$(WINOBJ)/wintime.o \
	$(WINOBJ)/winutf8.o \
	$(WINOBJ)/winutil.o \
	$(WINOBJ)/winclip.o \
	$(WINOBJ)/winsocket.o \
	$(WINOBJ)/winwork.o \
	$(WINOBJ)/winptty.o \
	$(WINOBJ)/winmidi.o


#-------------------------------------------------
# OSD Windows library
#-------------------------------------------------

OSDOBJS = \
	$(WINOBJ)/d3d9intf.o \
	$(WINOBJ)/drawd3d.o \
	$(WINOBJ)/d3dhlsl.o \
	$(WINOBJ)/drawdd.o \
	$(WINOBJ)/drawgdi.o \
	$(WINOBJ)/drawnone.o \
	$(WINOBJ)/input.o \
	$(WINOBJ)/output.o \
	$(WINOBJ)/sound.o \
	$(WINOBJ)/video.o \
	$(WINOBJ)/window.o \
	$(WINOBJ)/winmenu.o \
	$(WINOBJ)/winmain.o


ifdef USE_NETWORK
OSDOBJS += \
	$(WINOBJ)/netdev.o \
	$(WINOBJ)/netdev_pcap.o
endif

ifeq ($(DIRECT3D),9)
CCOMFLAGS += -DDIRECT3D_VERSION=0x0900
else
OSDOBJS += $(WINOBJ)/d3d8intf.o
endif

# extra dependencies
$(WINOBJ)/drawdd.o :    $(SRC)/emu/rendersw.c
$(WINOBJ)/drawgdi.o :   $(SRC)/emu/rendersw.c
$(WINOBJ)/winmidi.o:    $(SRC)/osd/portmedia/pmmidi.c

# add debug-specific files
OSDOBJS += \
	$(WINOBJ)/debugwin.o

# add a stub resource file
RESFILE = $(WINOBJ)/mame.res



#-------------------------------------------------
# rules for building the libaries
#-------------------------------------------------

$(LIBOCORE): $(OSDCOREOBJS)

$(LIBOSD): $(OSDOBJS)



#-------------------------------------------------
# rule for making the ledutil sample
#-------------------------------------------------

LEDUTIL = ledutil$(EXE)
TOOLS += $(LEDUTIL)

LEDUTILOBJS = \
	$(WINOBJ)/ledutil.o

$(LEDUTIL): $(LEDUTILOBJS) $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# generic rule for the resource compiler
#-------------------------------------------------

$(WINOBJ)/%.res: $(WINSRC)/%.rc | $(OSPREBUILD)
	@echo Compiling resources $<...
	$(RC) $(RCDEFS) $(RCFLAGS) -o $@ -i $<



#-------------------------------------------------
# rules for resource file
#-------------------------------------------------

$(RESFILE): $(WINSRC)/mame.rc $(WINOBJ)/mamevers.rc

$(WINOBJ)/mamevers.rc: $(BUILDOUT)/verinfo$(BUILD_EXE) $(SRC)/version.c
	@echo Emitting $@...
	@"$(BUILDOUT)/verinfo$(BUILD_EXE)" -b mame $(SRC)/version.c > $@
