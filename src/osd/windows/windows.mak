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

# uncomment next line to use ICL with MSVC
# USE_ICL = 1

# uncomment next line to enable code analysis using Microsoft tools
# MSVC_ANALYSIS = 1

# uncomment next line to use cygwin compiler
# CYGWIN_BUILD = 1

# set this to the minimum DirectInput version to support (7 or 8)
DIRECTINPUT = 8

# uncomment next line to use SDL library for sound and video output
# USE_SDL = 1

# uncomment next line to compile OpenGL video renderer
USE_OPENGL = 1

# uncomment the next line to build a binary using GL-dispatching.
USE_DISPATCH_GL = 1

# uncomment next line to use QT debugger
# USE_QTDEBUG = 1

###########################################################################
##################   END USER-CONFIGURABLE OPTIONS   ######################
###########################################################################

# add a define identifying the target osd
DEFS += -DOSD_WINDOWS


#-------------------------------------------------
# object and source roots
#-------------------------------------------------

WINSRC = $(SRC)/osd/$(OSD)
WINOBJ = $(OBJ)/osd/$(OSD)

OSDSRC = $(SRC)/osd
OSDOBJ = $(OBJ)/osd

OBJDIRS += $(WINOBJ) \
	$(OSDOBJ)/modules/sync \
	$(OSDOBJ)/modules/lib \
	$(OSDOBJ)/modules/midi \
	$(OSDOBJ)/modules/font \
	$(OSDOBJ)/modules/netdev \
	$(OSDOBJ)/modules/render \
	$(OSDOBJ)/modules/render/d3d \
	$(OSDOBJ)/modules/debugger/win

ifdef USE_QTDEBUG
OBJDIRS += $(OSDOBJ)/modules/debugger/qt
DEFS += -DUSE_QTDEBUG=1
else
DEFS += -DUSE_QTDEBUG=0
endif

ifdef USE_SDL
DEFS += -DSDLMAME_SDL2=0
DEFS += -DUSE_XINPUT=0
DEFS += -DUSE_OPENGL=0
DEFS += -DUSE_SDL=1
else
DEFS += -DUSE_SDL=0
endif

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
ifdef USE_ICL
PREFIX = vi
else
PREFIX = v
endif
endif

# replace the various compilers with vconv.exe prefixes
ifdef USE_ICL
CC = @$(VCONV) gcc -icl -I.
LD = @$(VCONV) ld -icl /profile
AR = @$(VCONV) ar -icl
else
CC = @$(VCONV) gcc -I.
LD = @$(VCONV) ld /profile
AR = @$(VCONV) ar
endif
RC = @$(VCONV) windres

# make sure we use the multithreaded runtime
ifdef DEBUG
CCOMFLAGS += /MTd
else
CCOMFLAGS += /MT
endif

# use link-time optimizations when enabled
ifneq ($(OPTIMIZE),0)
ifdef LTO
AR += /LTCG
endif
endif

# disable warnings and link against bufferoverflowu for 64-bit targets
ifeq ($(PTR64),1)
CCOMFLAGS += /wd4267
#LIBS += -lbufferoverflowu
endif

# enable basic run-time checks in non-optimized build
ifeq ($(OPTIMIZE),0)
ifndef FASTDEBUG
CCOMFLAGS += /RTC1
else
# disable the stack check since it has quite a speed impact
CCOMFLAGS += /RTCu
endif
endif

ifdef MSVC_ANALYSIS
CCOMFLAGS += /analyze /wd6011 /wd6328 /wd6204 /wd6244 /wd6385 /wd6308 /wd6246 /wd6031 /wd6326 /wd6255 /wd6330 /wd28251 /wd6054 /wd6340 /wd28125 /wd6053 /wd6001 /wd6386 /wd28278 /wd6297 /wd28183 /wd28159 /wd28182 /wd6237 /wd6239 /wd6240 /wd6323 /wd28199 /wd6235 /wd6285 /wd6286 /wd6384 /wd6293 /analyze:stacksize1070232
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

# only show deprecation warnings when enabled
ifndef DEPRECATED
CCOMFLAGS += /wd4996
endif

# explicitly set the entry point for UNICODE builds
LDFLAGS += /ENTRY:wmainCRTStartup

ifdef DEBUG
LDFLAGS += /NODEFAULTLIB:LIBCMT
endif

# add some VC++-specific defines
DEFS += -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -DXML_STATIC -DWIN32

OSDCLEAN = msvcclean

msvcclean:
	@echo Deleting Visual Studio specific files...
	$(RM) *.pdb
	$(RM) *.lib
	$(RM) *.exp

endif # MSVC_BUILD


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

include $(SRC)/build/cc_detection.mak

# ensure we statically link the gcc runtime lib
LDFLAGS += -static-libgcc

ifeq ($(CROSS_BUILD),1)
	LDFLAGS += -static
endif

# TODO: needs to use $(CC)
TEST_GCC := $(shell gcc --version)
ifeq ($(findstring 4.4.,$(TEST_GCC)),)
	#if we use new tools
	LDFLAGS += -static-libstdc++
endif

# add the windows libraries
BASELIBS += -luser32 -lgdi32 -ldsound -ldxguid -lwinmm -ladvapi32 -lcomctl32 -lshlwapi -lwsock32
LIBS += -luser32 -lgdi32 -ldsound -ldxguid -lwinmm -ladvapi32 -lcomctl32 -lshlwapi -lwsock32

ifdef USE_SDL
LIBS += -lSDL.dll
endif

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
	$(OSDOBJ)/modules/sync/sync_windows.o \
	$(WINOBJ)/winutf8.o \
	$(WINOBJ)/winutil.o \
	$(WINOBJ)/winclip.o \
	$(WINOBJ)/winsocket.o \
	$(OSDOBJ)/modules/sync/work_osd.o \
	$(OSDOBJ)/modules/lib/osdlib_win32.o \
	$(OSDOBJ)/modules/osdmodule.o \
	$(WINOBJ)/winptty.o \


#-------------------------------------------------
# OSD Windows library
#-------------------------------------------------

OSDOBJS = \
	$(OSDOBJ)/modules/render/drawd3d.o \
	$(OSDOBJ)/modules/render/d3d/d3d9intf.o \
	$(OSDOBJ)/modules/render/d3d/d3dhlsl.o \
	$(OSDOBJ)/modules/render/drawdd.o \
	$(OSDOBJ)/modules/render/drawgdi.o \
	$(OSDOBJ)/modules/render/drawbgfx.o \
	$(OSDOBJ)/modules/render/drawnone.o \
	$(WINOBJ)/input.o \
	$(WINOBJ)/output.o \
	$(OSDOBJ)/modules/sound/js_sound.o  \
	$(OSDOBJ)/modules/sound/direct_sound.o  \
	$(OSDOBJ)/modules/sound/sdl_sound.o  \
	$(OSDOBJ)/modules/sound/none.o  \
	$(WINOBJ)/video.o \
	$(WINOBJ)/window.o \
	$(WINOBJ)/winmenu.o \
	$(WINOBJ)/winmain.o \
	$(OSDOBJ)/modules/midi/portmidi.o \
	$(OSDOBJ)/modules/midi/none.o \
	$(OSDOBJ)/modules/lib/osdobj_common.o  \
	$(OSDOBJ)/modules/font/font_sdl.o \
	$(OSDOBJ)/modules/font/font_windows.o \
	$(OSDOBJ)/modules/font/font_osx.o \
	$(OSDOBJ)/modules/font/font_none.o \
	$(OSDOBJ)/modules/netdev/pcap.o \
	$(OSDOBJ)/modules/netdev/taptun.o \
	$(OSDOBJ)/modules/netdev/none.o \

ifdef USE_OPENGL
OSDOBJS += \
	$(OSDOBJ)/modules/render/drawogl.o \
	$(OSDOBJ)/modules/opengl/gl_shader_tool.o \
	$(OSDOBJ)/modules/opengl/gl_shader_mgr.o

OBJDIRS += \
	$(OSDOBJ)/modules/opengl

DEFS += -DUSE_OPENGL=1

ifdef USE_DISPATCH_GL
DEFS += -DUSE_DISPATCH_GL=1
else
LIBS += -lopengl32
endif

else
DEFS += -DUSE_OPENGL=0
endif

ifdef USE_SDL
DEFS += -DUSE_SDL_SOUND
endif

ifndef DONT_USE_NETWORK
DEFS += -DSDLMAME_NET_PCAP
endif

CCOMFLAGS += -DDIRECT3D_VERSION=0x0900

# extra dependencies
$(WINOBJ)/drawdd.o :    $(SRC)/emu/rendersw.inc
$(WINOBJ)/drawgdi.o :   $(SRC)/emu/rendersw.inc

# add debug-specific files
OSDOBJS += \
	$(OSDOBJ)/modules/debugger/debugwin.o \
	$(OSDOBJ)/modules/debugger/win/consolewininfo.o \
	$(OSDOBJ)/modules/debugger/win/debugbaseinfo.o \
	$(OSDOBJ)/modules/debugger/win/debugviewinfo.o \
	$(OSDOBJ)/modules/debugger/win/debugwininfo.o \
	$(OSDOBJ)/modules/debugger/win/disasmbasewininfo.o \
	$(OSDOBJ)/modules/debugger/win/disasmviewinfo.o \
	$(OSDOBJ)/modules/debugger/win/disasmwininfo.o \
	$(OSDOBJ)/modules/debugger/win/editwininfo.o \
	$(OSDOBJ)/modules/debugger/win/logwininfo.o \
	$(OSDOBJ)/modules/debugger/win/memoryviewinfo.o \
	$(OSDOBJ)/modules/debugger/win/memorywininfo.o \
	$(OSDOBJ)/modules/debugger/win/pointswininfo.o \
	$(OSDOBJ)/modules/debugger/win/uimetrics.o \
	$(OSDOBJ)/modules/debugger/debugint.o \
	$(OSDOBJ)/modules/debugger/debugqt.o \
	$(OSDOBJ)/modules/debugger/none.o

# add a stub resource file
RESFILE = $(WINOBJ)/mame.res

BGFX_LIB = $(OBJ)/libbgfx.a
INCPATH += -I$(3RDPARTY)/bgfx/include -I$(3RDPARTY)/bx/include

#-------------------------------------------------
# QT Debug library
#-------------------------------------------------
ifdef USE_QTDEBUG
QT_INSTALL_HEADERS := $(shell qmake -query QT_INSTALL_HEADERS)
QT_LIBS := -L$(shell qmake -query QT_INSTALL_LIBS)
LIBS += $(QT_LIBS) -lqtmain -lQtGui4 -lQtCore4
INCPATH += -I$(QT_INSTALL_HEADERS)/QtCore -I$(QT_INSTALL_HEADERS)/QtGui -I$(QT_INSTALL_HEADERS)

MOC = @moc
$(OSDOBJ)/%.moc.c: $(OSDSRC)/%.h
	$(MOC) $(INCPATH) $< -o $@

OSDOBJS += \
	$(OSDOBJ)/modules/debugger/qt/debuggerview.o \
	$(OSDOBJ)/modules/debugger/qt/windowqt.o \
	$(OSDOBJ)/modules/debugger/qt/logwindow.o \
	$(OSDOBJ)/modules/debugger/qt/dasmwindow.o \
	$(OSDOBJ)/modules/debugger/qt/mainwindow.o \
	$(OSDOBJ)/modules/debugger/qt/memorywindow.o \
	$(OSDOBJ)/modules/debugger/qt/breakpointswindow.o \
	$(OSDOBJ)/modules/debugger/qt/deviceswindow.o \
	$(OSDOBJ)/modules/debugger/qt/deviceinformationwindow.o \
	$(OSDOBJ)/modules/debugger/qt/debuggerview.moc.o \
	$(OSDOBJ)/modules/debugger/qt/windowqt.moc.o \
	$(OSDOBJ)/modules/debugger/qt/logwindow.moc.o \
	$(OSDOBJ)/modules/debugger/qt/dasmwindow.moc.o \
	$(OSDOBJ)/modules/debugger/qt/mainwindow.moc.o \
	$(OSDOBJ)/modules/debugger/qt/memorywindow.moc.o \
	$(OSDOBJ)/modules/debugger/qt/breakpointswindow.moc.o \
	$(OSDOBJ)/modules/debugger/qt/deviceswindow.moc.o \
	$(OSDOBJ)/modules/debugger/qt/deviceinformationwindow.moc.o
endif

#-------------------------------------------------
# WinPCap
#-------------------------------------------------
INCPATH += -I$(3RDPARTY)/winpcap/Include

#-------------------------------------------------
# rules for building the libaries
#-------------------------------------------------

$(LIBOCORE): $(OSDCOREOBJS)

$(LIBOSD): $(OSDOBJS)



#-------------------------------------------------
# rule for making the ledutil sample
#-------------------------------------------------

LEDUTIL = $(BIN)ledutil$(EXE)
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

$(WINOBJ)/mamevers.rc: $(SRC)/build/verinfo.py $(SRC)/version.c
	@echo Emitting $@...
	$(PYTHON) $(SRC)/build/verinfo.py -b mame -o $@ $(SRC)/version.c
