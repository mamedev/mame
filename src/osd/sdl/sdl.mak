###########################################################################
#
#   sdl.mak
#
#   SDL-specific makefile
#
#   Copyright (c) 1996-2013, Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
#   SDLMAME by Olivier Galibert and R. Belmont
#
###########################################################################

###########################################################################
#################   BEGIN USER-CONFIGURABLE OPTIONS   #####################
###########################################################################


#-------------------------------------------------
# specify build options; see each option below
# for details
#-------------------------------------------------

# uncomment and edit next line to specify a distribution
# supported debian-stable, ubuntu-intrepid

# DISTRO = debian-stable
# DISTRO = ubuntu-intrepid
# DISTRO = gcc44-generic

# uncomment next line to build without OpenGL support

# NO_OPENGL = 1

# uncomment next line to build without X11 support (TARGETOS=unix only)
# this also implies, that no debugger will be builtin.

# NO_X11 = 1

# uncomment next line to disable XInput support for e.g. multiple lightguns and mice on X11 systems
# using Wiimote driver (see http://spritesmods.com/?art=wiimote-mamegun for more info)
# enabling NO_X11 also implies no XInput support, of course.
# (currently defaults disabled due to causing issues with mouse capture, esp. in MESS)

NO_USE_XINPUT = 1

# uncomment and adapt next line to link against specific GL-Library
# this will also add a rpath to the executable
# MESA_INSTALL_ROOT = /usr/local/dfb_GL

# uncomment the next line to build a binary using GL-dispatching.
# This option takes precedence over MESA_INSTALL_ROOT

USE_DISPATCH_GL = 1

# The following settings are currently supported for unix only.
# There is no need to play with this option unless you are doing
# active development on sdlmame or SDL.

# uncomment the next line to use couriersud's multi-keyboard patch for SDL 2.1? (this API was removed prior to the 2.0 release)
# SDL2_MULTIAPI = 1

# uncomment the next line to specify where you have installed
# SDL. Equivalent to the ./configure --prefix=<path>
# SDL_INSTALL_ROOT = /usr/local/sdl20

# uncomment to disable the Qt debugger (on non-OSX this disables all debugging)
# NO_USE_QTDEBUG = 1

# uncomment to disable MIDI
# NO_USE_MIDI = 1

# uncomment to disable implementations based on assembler code
# NOASM = 1

# change for custom OS X installations
SDL_FRAMEWORK_PATH = /Library/Frameworks/

# uncomment to use SDL1.2 (depracated)
# SDL_LIBVER = sdl

# uncomment to use BGFX

# USE_BGFX = 1

###########################################################################
##################   END USER-CONFIGURABLE OPTIONS   ######################
###########################################################################
OSDSRC = $(SRC)/osd
OSDOBJ = $(OBJ)/osd

# add a define identifying the target osd
DEFS += -DOSD_SDL

# default to SDL2 for non-OS/2 builds now
ifndef SDL_LIBVER
ifneq ($(TARGETOS),os2)
SDL_LIBVER = sdl2
else
SDL_LIBVER = sdl
endif
endif

ifndef NO_USE_QTDEBUG
OBJDIRS += $(OSDOBJ)/modules/debugger/qt
endif

ifdef SDL_INSTALL_ROOT
SDL_CONFIG = $(SDL_INSTALL_ROOT)/bin/$(SDL_LIBVER)-config
else
SDL_CONFIG = $(SDL_LIBVER)-config
endif

ifeq ($(SDL_LIBVER),sdl2)
DEFS += -DSDLMAME_SDL2=1
	ifeq ($(SDL2_MULTIAPI),1)
	DEFS += -DSDL2_MULTIAPI
	endif
else
DEFS += -DSDLMAME_SDL2=0
endif

# minimal preliminary ARM support
ifeq ($(findstring arm,$(UNAME)),arm)
	NOASM = 1
	DEFS += -DSDLMAME_ARM
endif

ifdef NOASM
DEFS += -DSDLMAME_NOASM
endif

# patch up problems with new zlib
DEFS += -D_LFS64_LARGEFILE=0

# bring in external flags for RPM build
CCOMFLAGS += $(OPT_FLAGS)

#-------------------------------------------------
# distribution may change things
#-------------------------------------------------

ifeq ($(DISTRO),)
DISTRO = generic
else
ifeq ($(DISTRO),debian-stable)
DEFS += -DNO_AFFINITY_NP
else
ifeq ($(DISTRO),ubuntu-intrepid)
# Force gcc-4.2 on ubuntu-intrepid
CC = @gcc -V 4.2
LD = @g++-4.2
else
ifeq ($(DISTRO),gcc44-generic)
CC = @gcc-4.4
LD = @g++-4.4
else
ifeq ($(DISTRO),gcc45-generic)
CC = @gcc-4.5
LD = @g++-4.5
else
ifeq ($(DISTRO),gcc46-generic)
CC = @gcc-4.6
LD = @g++-4.6
else
ifeq ($(DISTRO),gcc47-generic)
CC = @gcc-4.7
LD = @g++-4.7
else
$(error DISTRO $(DISTRO) unknown)
endif
endif
endif
endif
endif
endif
endif

DEFS += -DDISTRO=$(DISTRO)

#-------------------------------------------------
# sanity check the configuration
#-------------------------------------------------

ifdef BIGENDIAN
X86_MIPS3_DRC =
X86_PPC_DRC =
FORCE_DRC_C_BACKEND = 1
endif

ifdef NOASM
X86_MIPS3_DRC =
X86_PPC_DRC =
FORCE_DRC_C_BACKEND = 1
endif

#-------------------------------------------------
# compile and linking flags
#-------------------------------------------------

# add SDLMAME BASE_TARGETOS definitions

ifeq ($(TARGETOS),unix)
BASE_TARGETOS = unix
SYNC_IMPLEMENTATION = tc
endif

ifeq ($(TARGETOS),linux)
BASE_TARGETOS = unix
SYNC_IMPLEMENTATION = tc
SDL_NETWORK = taptun

ifndef NO_USE_MIDI
ALSACFLAGS := $(shell pkg-config --cflags alsa)
ALSALIBS := $(shell pkg-config --libs alsa)

INCPATH += $(ALSACFLAGS)
LIBS += $(ALSALIBS)
endif

endif

ifeq ($(TARGETOS),freebsd)
BASE_TARGETOS = unix
SYNC_IMPLEMENTATION = tc
DEFS += -DNO_AFFINITY_NP
LIBS += -lutil
# /usr/local/include is not considered a system include directory
# on FreeBSD. GL.h resides there and throws warnings
CCOMFLAGS += -isystem /usr/local/include
NO_USE_MIDI = 1
endif

ifeq ($(TARGETOS),openbsd)
BASE_TARGETOS = unix
SYNC_IMPLEMENTATION = ntc
LIBS += -lutil
NO_USE_MIDI = 1
endif

ifeq ($(TARGETOS),netbsd)
BASE_TARGETOS = unix
SYNC_IMPLEMENTATION = ntc
LIBS += -lutil
NO_USE_MIDI = 1
SDL_NETWORK = pcap
endif

ifeq ($(TARGETOS),solaris)
BASE_TARGETOS = unix
#DEFS += -DNO_AFFINITY_NP -UHAVE_VSNPRINTF -DNO_vsnprintf
DEFS += -DNO_AFFINITY_NP
SYNC_IMPLEMENTATION = tc
NO_USE_MIDI = 1
NO_USE_QTDEBUG = 1
endif

ifeq ($(TARGETOS),haiku)
BASE_TARGETOS = unix
SYNC_IMPLEMENTATION = ntc
NO_X11 = 1
NO_USE_XINPUT = 1
NO_USE_MIDI = 1
NO_USE_QTDEBUG = 1
LIBS += -lnetwork -lbsd
endif

ifeq ($(TARGETOS),emscripten)
BASE_TARGETOS = unix
SYNC_IMPLEMENTATION = mini
NO_DEBUGGER = 1
NO_X11 = 1
NO_USE_XINPUT = 1
NO_USE_MIDI = 1
NO_USE_QTDEBUG = 1
DONT_USE_NETWORK = 1
endif

ifeq ($(TARGETOS),macosx)
NO_USE_QTDEBUG = 1
BASE_TARGETOS = unix
DEFS += -DSDLMAME_UNIX -DSDLMAME_MACOSX -DSDLMAME_DARWIN

ifndef NO_USE_MIDI
LIBS += -framework CoreAudio -framework CoreMIDI
endif

ifdef NO_USE_QTDEBUG

OBJDIRS += $(OSDOBJ)/modules/debugger/osx

DEBUGOBJS = \
	$(OSDOBJ)/modules/debugger/debugosx.o \
	$(OSDOBJ)/modules/debugger/osx/breakpointsview.o \
	$(OSDOBJ)/modules/debugger/osx/consoleview.o \
	$(OSDOBJ)/modules/debugger/osx/debugcommandhistory.o \
	$(OSDOBJ)/modules/debugger/osx/debugconsole.o \
	$(OSDOBJ)/modules/debugger/osx/debugview.o \
	$(OSDOBJ)/modules/debugger/osx/debugwindowhandler.o \
	$(OSDOBJ)/modules/debugger/osx/deviceinfoviewer.o \
	$(OSDOBJ)/modules/debugger/osx/devicesviewer.o \
	$(OSDOBJ)/modules/debugger/osx/disassemblyview.o \
	$(OSDOBJ)/modules/debugger/osx/disassemblyviewer.o \
	$(OSDOBJ)/modules/debugger/osx/errorlogview.o \
	$(OSDOBJ)/modules/debugger/osx/errorlogviewer.o \
	$(OSDOBJ)/modules/debugger/osx/memoryview.o \
	$(OSDOBJ)/modules/debugger/osx/memoryviewer.o \
	$(OSDOBJ)/modules/debugger/osx/pointsviewer.o \
	$(OSDOBJ)/modules/debugger/osx/registersview.o \
	$(OSDOBJ)/modules/debugger/osx/watchpointsview.o

endif

SYNC_IMPLEMENTATION = ntc

# SDLMain_tmpl isn't necessary for SDL2
ifneq ($(SDL_LIBVER),sdl2)
SDLMAIN = $(SDLOBJ)/SDLMain_tmpl.o
SDLUTILMAIN = $(SDLOBJ)/SDLMain_tmpl.o
endif

SDL_NETWORK = pcap
MAINLDFLAGS = -Xlinker -all_load
NO_X11 = 1
NO_USE_XINPUT = 1

ifdef BIGENDIAN
DEFS += -DOSX_PPC=1
CCOMFLAGS += -Wno-unused-label
ifdef SYMBOLS
CCOMFLAGS += -mlong-branch
endif   # SYMBOLS
ifeq ($(PTR64),1)
CCOMFLAGS += -arch ppc64
LDFLAGS += -arch ppc64
else
CCOMFLAGS += -arch ppc
LDFLAGS += -arch ppc
endif
$(OBJ)/emu/cpu/tms57002/tms57002.o : CCOMFLAGS += -O0
else    # BIGENDIAN
ifeq ($(PTR64),1)
CCOMFLAGS += -arch x86_64
LDFLAGS += -arch x86_64
else
CCOMFLAGS += -m32 -arch i386
LDFLAGS += -m32 -arch i386
endif
endif   # BIGENDIAN

endif

ifeq ($(TARGETOS),win32)
BASE_TARGETOS = win32
SYNC_IMPLEMENTATION = windows
NO_X11 = 1
NO_USE_XINPUT = 1
DEFS += -DSDLMAME_WIN32 -DX64_WINDOWS_ABI
LIBGL = -lopengl32
SDLMAIN = $(SDLOBJ)/main.o
# needed for unidasm
LDFLAGS += -Wl,--allow-multiple-definition
SDL_NETWORK = pcap
INCPATH += -I$(3RDPARTY)/winpcap/Include

# enable UNICODE
DEFS += -Dmain=utf8_main -DUNICODE -D_UNICODE
LDFLAGS += -municode

# Qt
ifndef NO_USE_QTDEBUG
QT_INSTALL_HEADERS = $(shell qmake -query QT_INSTALL_HEADERS)
INCPATH += -I$(QT_INSTALL_HEADERS)/QtCore -I$(QT_INSTALL_HEADERS)/QtGui -I$(QT_INSTALL_HEADERS)
BASELIBS += -lcomdlg32 -loleaut32 -limm32 -lwinspool -lmsimg32 -lole32 -luuid -lws2_32 -lshell32 -lkernel32
LIBS += -L$(shell qmake -query QT_INSTALL_LIBS) -lqtmain -lQtGui4 -lQtCore4 -lcomdlg32 -loleaut32 -limm32 -lwinspool -lmsimg32 -lole32 -luuid -lws2_32 -lshell32 -lkernel32
endif
endif

ifeq ($(TARGETOS),macosx)
ifndef NO_USE_QTDEBUG
MOC = @moc

QT_INSTALL_LIBS = $(shell qmake -query QT_INSTALL_LIBS)
INCPATH += -I$(QT_INSTALL_LIBS)/QtGui.framework/Versions/4/Headers -I$(QT_INSTALL_LIBS)/QtCore.framework/Versions/4/Headers -F$(QT_INSTALL_LIBS)
LIBS += -L$(QT_INSTALL_LIBS) -F$(QT_INSTALL_LIBS) -framework QtCore -framework QtGui
endif
endif

ifeq ($(TARGETOS),os2)
BASE_TARGETOS = os2
DEFS += -DSDLMAME_OS2
SYNC_IMPLEMENTATION = os2
NO_DEBUGGER = 1
NO_X11 = 1
NO_USE_XINPUT = 1
NO_USE_MIDI = 1
NO_USE_QTDEBUG = 1
# OS/2 can't have OpenGL (aww)
NO_OPENGL = 1
endif

#-------------------------------------------------
# Sanity checks
#-------------------------------------------------

ifeq ($(BASE_TARGETOS),)
$(error $(TARGETOS) not supported !)
endif

# if no Qt and not OS X, no debugger
ifneq ($(TARGETOS),macosx)
ifdef NO_USE_QTDEBUG
NO_DEBUGGER = 1
endif
endif

#-------------------------------------------------
# object and source roots
#-------------------------------------------------

SDLSRC = $(SRC)/osd/$(OSD)
SDLOBJ = $(OBJ)/osd/$(OSD)

OBJDIRS += $(SDLOBJ) \
	$(OSDOBJ)/modules/sync \
	$(OSDOBJ)/modules/lib \
	$(OSDOBJ)/modules/midi \
	$(OSDOBJ)/modules/font \
	$(OSDOBJ)/modules/netdev \
	$(OSDOBJ)/modules/opengl \
	$(OSDOBJ)/modules/render
	
#-------------------------------------------------
# OSD core library
#-------------------------------------------------

OSDCOREOBJS = \
	$(SDLOBJ)/strconv.o \
	$(SDLOBJ)/sdldir.o  \
	$(SDLOBJ)/sdlfile.o     \
	$(SDLOBJ)/sdlptty_$(BASE_TARGETOS).o    \
	$(SDLOBJ)/sdlsocket.o   \
	$(SDLOBJ)/sdlos_$(SDLOS_TARGETOS).o \
	$(OSDOBJ)/modules/lib/osdlib_$(SDLOS_TARGETOS).o \
	$(OSDOBJ)/modules/sync/sync_$(SYNC_IMPLEMENTATION).o \
	$(OSDOBJ)/modules/osdmodule.o \

ifdef NOASM
OSDCOREOBJS += $(OSDOBJ)/modules/sync/work_mini.o
else
OSDCOREOBJS += $(OSDOBJ)/modules/sync/work_osd.o
endif

# any "main" must be in LIBOSD or else the build will fail!
# for the windows build, we just add it to libocore as well.
OSDOBJS = \
	$(SDLMAIN) \
	$(SDLOBJ)/sdlmain.o \
	$(SDLOBJ)/input.o \
	$(OSDOBJ)/modules/sound/js_sound.o  \
	$(OSDOBJ)/modules/sound/direct_sound.o  \
	$(OSDOBJ)/modules/sound/sdl_sound.o  \
	$(OSDOBJ)/modules/sound/none.o  \
	$(SDLOBJ)/video.o \
	$(SDLOBJ)/window.o \
	$(SDLOBJ)/output.o \
	$(SDLOBJ)/watchdog.o \
	$(OSDOBJ)/modules/lib/osdobj_common.o  \
	$(OSDOBJ)/modules/font/font_sdl.o \
	$(OSDOBJ)/modules/font/font_windows.o \
	$(OSDOBJ)/modules/font/font_osx.o \
	$(OSDOBJ)/modules/font/font_none.o \
	$(OSDOBJ)/modules/netdev/taptun.o \
	$(OSDOBJ)/modules/netdev/pcap.o \
	$(OSDOBJ)/modules/netdev/none.o \
	$(OSDOBJ)/modules/midi/portmidi.o \
	$(OSDOBJ)/modules/midi/none.o \
	$(OSDOBJ)/modules/render/drawsdl.o \

ifdef NO_USE_MIDI
	DEFS += -DNO_USE_MIDI
else
endif

# Add SDL2.0 support

ifeq ($(SDL_LIBVER),sdl2)
OSDOBJS += $(OSDOBJ)/modules/render/draw13.o
endif

# add an ARCH define
DEFS += -DSDLMAME_ARCH="$(ARCHOPTS)" -DSYNC_IMPLEMENTATION=$(SYNC_IMPLEMENTATION)

#-------------------------------------------------
# Generic defines and additions
#-------------------------------------------------

OSDCLEAN = sdlclean

# copy off the include paths before the sdlprefix & sdl-config stuff shows up
MOCINCPATH := $(INCPATH)

# add the prefix file
INCPATH += -include $(SDLSRC)/sdlprefix.h


#-------------------------------------------------
# BASE_TARGETOS specific configurations
#-------------------------------------------------

SDLOS_TARGETOS = $(BASE_TARGETOS)

#-------------------------------------------------
# TEST_GCC for GCC version-specific stuff
#-------------------------------------------------

ifneq ($(TARGETOS),emscripten)
TEST_GCC = $(shell gcc --version)

# Ubuntu 12.10 GCC 4.7.2 autodetect
ifeq ($(findstring 4.7.2-2ubuntu1,$(TEST_GCC)),4.7.2-2ubuntu1)
GCC46TST = $(shell which g++-4.6 2>/dev/null)
ifeq '$(GCC46TST)' ''
$(error Ubuntu 12.10 detected.  Please install the gcc-4.6 and g++-4.6 packages)
endif
CC = @gcc-4.6
LD = @g++-4.6
endif
endif

include $(SRC)/build/cc_detection.mak

#-------------------------------------------------
# Unix
#-------------------------------------------------
ifeq ($(BASE_TARGETOS),unix)

#-------------------------------------------------
# Mac OS X
#-------------------------------------------------

ifeq ($(TARGETOS),macosx)
OSDCOREOBJS += $(SDLOBJ)/osxutils.o
SDLOS_TARGETOS = macosx

ifeq ($(TARGET),mame)
MACOSX_EMBED_INFO_PLIST = 1
endif
ifeq ($(TARGET),mess)
MACOSX_EMBED_INFO_PLIST = 1
endif
ifeq ($(TARGET),ume)
MACOSX_EMBED_INFO_PLIST = 1
endif
ifdef MACOSX_EMBED_INFO_PLIST
INFOPLIST = $(SDLOBJ)/$(TARGET)-Info.plist
LDFLAGSEMULATOR += -sectcreate __TEXT __info_plist $(INFOPLIST)
$(EMULATOR): $(INFOPLIST)
$(INFOPLIST): $(SRC)/build/verinfo.py $(SRC)/version.c
	@echo Emitting $@...
	$(PYTHON) $(SRC)/build/verinfo.py -b $(TARGET) -p -o $@ $(SRC)/version.c
endif

ifndef MACOSX_USE_LIBSDL
# Compile using framework (compile using libSDL is the exception)
ifeq ($(SDL_LIBVER),sdl2)
LIBS += -F$(SDL_FRAMEWORK_PATH) -framework SDL2 -framework Cocoa -framework OpenGL -lpthread
BASELIBS += -F$(SDL_FRAMEWORK_PATH) -framework SDL2 -framework Cocoa -framework OpenGL -lpthread
else
LIBS += -F$(SDL_FRAMEWORK_PATH) -framework SDL -framework Cocoa -framework OpenGL -lpthread
BASELIBS += -F$(SDL_FRAMEWORK_PATH) -framework SDL -framework Cocoa -framework OpenGL -lpthread
endif
INCPATH += -F$(SDL_FRAMEWORK_PATH)
else
# Compile using installed libSDL (Fink or MacPorts):
#
# Remove the "/SDL" component from the include path so that we can compile

# files (header files are #include "SDL/something.h", so the extra "/SDL"
# causes a significant problem)
SDLCFLAGS := $(shell $(SDL_CONFIG) --cflags | sed 's:/SDL::')
# Remove libSDLmain, as its symbols conflict with SDLMain_tmpl.m
SDLLIBS := $(shell $(SDL_CONFIG) --libs | sed 's/-lSDLmain//')

INCPATH += $(SDLCFLAGS)
CCOMFLAGS += -DNO_SDL_GLEXT
LIBS += $(SDLLIBS) -lpthread -framework Cocoa -framework OpenGL
BASELIBS += $(SDLLIBS) -lpthread -framework Cocoa -framework OpenGL
DEFS += -DMACOSX_USE_LIBSDL
endif   # MACOSX_USE_LIBSDL

else   # ifeq ($(TARGETOS),macosx)

DEFS += -DSDLMAME_UNIX

ifndef NO_USE_QTDEBUG
MOCTST = $(shell which moc-qt4 2>/dev/null)
ifeq '$(MOCTST)' ''
MOCTST = $(shell which moc 2>/dev/null)
ifeq '$(MOCTST)' ''
$(error Qt's Meta Object Compiler (moc) wasn't found!)
else
MOC = @$(MOCTST)
endif
else
MOC = @$(MOCTST)
endif
# Qt on Linux/UNIX
QMAKE = $(shell which qmake-qt4 2>/dev/null)
ifeq '$(QMAKE)' ''
QMAKE = $(shell which qmake 2>/dev/null)
ifeq '$(QMAKE)' ''
$(error qmake wasn't found!)
endif
endif
QT_INSTALL_HEADERS = $(shell $(QMAKE) -query QT_INSTALL_HEADERS)
INCPATH += -I$(QT_INSTALL_HEADERS)/QtCore -I$(QT_INSTALL_HEADERS)/QtGui -I$(QT_INSTALL_HEADERS)
LIBS += -L$(shell $(QMAKE) -query QT_INSTALL_LIBS) -lQtGui -lQtCore
endif

LIBGL = -lGL

ifeq ($(NO_X11),1)
NO_DEBUGGER = 1
endif

SDLINCLUDES := $(shell $(SDL_CONFIG) --cflags  | sed -e 's:/SDL[2]*::' -e 's:\(-D[^ ]*\)::g')
SDLDEFINES := $(shell $(SDL_CONFIG) --cflags  | sed -e 's:/SDL[2]*::' -e 's:\(-I[^ ]*\)::g')
SDLLIBS := $(shell $(SDL_CONFIG) --libs)

INCPATH += $(SDLINCLUDES)
CCOMFLAGS += $(SDLDEFINES)
BASELIBS += $(SDLLIBS)
LIBS += $(SDLLIBS)

ifeq ($(SDL_LIBVER),sdl2)
ifdef SDL_INSTALL_ROOT
# FIXME: remove the directfb ref. later. This is just there for now to work around an issue with SDL1.3 and SDL2.0
INCPATH += -I$(SDL_INSTALL_ROOT)/include/directfb
endif
endif

FONTCONFIGCFLAGS := $(shell pkg-config --cflags fontconfig)
FONTCONFIGLIBS := $(shell pkg-config --libs fontconfig)

ifneq ($(TARGETOS),emscripten)
INCPATH += $(FONTCONFIGCFLAGS)
endif
LIBS += $(FONTCONFIGLIBS)

ifeq ($(SDL_LIBVER),sdl2)
LIBS += -lSDL2_ttf
else
LIBS += -lSDL_ttf
endif

# FIXME: should be dealt with elsewhere
# libs that Haiku doesn't want but are mandatory on *IX
ifneq ($(TARGETOS),haiku)
BASELIBS += -lm -lpthread
LIBS += -lm -lpthread
ifneq ($(TARGETOS),solaris)
BASELIBS += -lutil
LIBS += -lutil
else
SUPPORTSM32M64 = 1
BASELIBS += -lsocket -lnsl
LIBS += -lsocket -lnsl
endif
endif


endif # not Mac OS X

ifneq (,$(findstring ppc,$(UNAME)))
# override for preprocessor weirdness on PPC Linux
CFLAGS += -Upowerpc
SUPPORTSM32M64 = 1
endif

ifneq (,$(findstring amd64,$(UNAME)))
SUPPORTSM32M64 = 1
endif
ifneq (,$(findstring x86_64,$(UNAME)))
SUPPORTSM32M64 = 1
endif
ifneq (,$(findstring i386,$(UNAME)))
SUPPORTSM32M64 = 1
endif

ifeq ($(SUPPORTSM32M64),1)
ifeq ($(PTR64),1)
CCOMFLAGS += -m64
LDFLAGS += -m64
else
CCOMFLAGS += -m32
LDFLAGS += -m32
endif
endif

endif # Unix

#-------------------------------------------------
# Windows
#-------------------------------------------------

# Win32: add the necessary libraries
ifeq ($(BASE_TARGETOS),win32)

# Add to osdcoreobjs so tools will build
OSDCOREOBJS += $(SDLMAIN)

ifdef SDL_INSTALL_ROOT
INCPATH += -I$(SDL_INSTALL_ROOT)/include
LIBS += -L$(SDL_INSTALL_ROOT)/lib
#-Wl,-rpath,$(SDL_INSTALL_ROOT)/lib
endif

# LIBS += -lmingw32 -lSDL
# Static linking

LDFLAGS += -static-libgcc
ifeq (,$(findstring clang,$(CC)))
ifeq ($(findstring 4.4,$(TEST_GCC)),)
	#if we use new tools
	LDFLAGS += -static-libstdc++
endif
endif

ifndef NO_USE_QTDEBUG
MOC = @moc
endif

ifeq ($(SDL_LIBVER),sdl2)
LIBS += -lSDL2 -limm32 -lversion -lole32 -loleaut32 -lws2_32 -static
BASELIBS += -lSDL2 -limm32 -lversion -lole32 -loleaut32 -lws2_32 -static
else
LIBS += -lSDL -lws2_32 -static
BASELIBS += -lSDL -lws2_32 -static
endif
LIBS += -luser32 -lgdi32 -lddraw -ldsound -ldxguid -lwinmm -ladvapi32 -lcomctl32 -lshlwapi
BASELIBS += -luser32 -lgdi32 -lddraw -ldsound -ldxguid -lwinmm -ladvapi32 -lcomctl32 -lshlwapi
endif   # Win32

#-------------------------------------------------
# OS/2
#-------------------------------------------------

ifeq ($(BASE_TARGETOS),os2)

SDLCFLAGS := $(shell sdl-config --cflags)
SDLLIBS := $(shell sdl-config --libs)

INCPATH += $(SDLCFLAGS)
LIBS += $(SDLLIBS) -lpthread
BASELIBS += $(SDLLIBS) -lpthread

endif # OS2

#-------------------------------------------------
# Debugging
#-------------------------------------------------

ifndef NO_USE_QTDEBUG
$(OSDOBJ)/%.moc.c: $(OSDSRC)/%.h
	$(MOC) $(MOCINCPATH) $< -o $@

DEBUGOBJS = \
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

DEFS += -DUSE_QTDEBUG=1

else
DEFS += -DUSE_QTDEBUG=0
endif

ifeq ($(NO_DEBUGGER),1)
DEFS += -DNO_DEBUGGER
else
OSDOBJS += $(DEBUGOBJS)
endif # NO_DEBUGGER

# Always add these
OSDOBJS += \
	$(OSDOBJ)/modules/debugger/none.o \
	$(OSDOBJ)/modules/debugger/debugint.o \
	$(OSDOBJ)/modules/debugger/debugwin.o \
	$(OSDOBJ)/modules/debugger/debugqt.o

#-------------------------------------------------
# BGFX
#-------------------------------------------------

ifdef USE_BGFX
DEFS += -DUSE_BGFX
OSDOBJS += $(OSDOBJ)/modules/render/drawbgfx.o
INCPATH += -I$(3RDPARTY)/bgfx/include -I$(3RDPARTY)/bx/include
USE_DISPATCH_GL = 0
BGFX_LIB = $(OBJ)/libbgfx.a
endif

#-------------------------------------------------
# OPENGL
#-------------------------------------------------

ifeq ($(NO_OPENGL),1)
DEFS += -DUSE_OPENGL=0
else
OSDOBJS += \
	$(OSDOBJ)/modules/render/drawogl.o \
	$(OSDOBJ)/modules/opengl/gl_shader_tool.o \
	$(OSDOBJ)/modules/opengl/gl_shader_mgr.o
	
DEFS += -DUSE_OPENGL=1
ifeq ($(USE_DISPATCH_GL),1)
DEFS += -DUSE_DISPATCH_GL=1
else
LIBS += $(LIBGL)
endif
endif

ifneq ($(USE_DISPATCH_GL),1)
ifdef MESA_INSTALL_ROOT
LIBS += -L$(MESA_INSTALL_ROOT)/lib
LDFLAGS += -Wl,-rpath=$(MESA_INSTALL_ROOT)/lib
INCPATH += -I$(MESA_INSTALL_ROOT)/include
endif
endif

#-------------------------------------------------
# X11
#-------------------------------------------------

ifeq ($(NO_X11),1)
DEFS += -DSDLMAME_NO_X11
else
# Default libs
DEFS += -DSDLMAME_X11
LIBS += -lX11 -lXinerama
ifneq ($(SDL_LIBVER),sdl2)
BASELIBS += -lX11
endif

# The newer debugger uses QT
ifndef NO_USE_QTDEBUG
QTCFLAGS := $(shell pkg-config --cflags QtGui)
QTLIBS := $(shell pkg-config --libs QtGui)

INCPATH += $(QTCFLAGS)
LIBS += $(QTLIBS)
endif

# some systems still put important things in a different prefix
LIBS += -L/usr/X11/lib -L/usr/X11R6/lib -L/usr/openwin/lib
# make sure we can find X headers
INCPATH += -I/usr/X11/include -I/usr/X11R6/include -I/usr/openwin/include
endif # NO_X11

# can't use native libs with emscripten
ifeq ($(TARGETOS),emscripten)
LIBS =
endif

#-------------------------------------------------
# XInput
#-------------------------------------------------

ifeq ($(NO_USE_XINPUT),1)
DEFS += -DUSE_XINPUT=0
else
DEFS += -DUSE_XINPUT=1 -DUSE_XINPUT_DEBUG=0
LIBS += -lXext -lXi
endif # USE_XINPUT

#-------------------------------------------------
# Network (TAP/TUN)
#-------------------------------------------------

ifndef DONT_USE_NETWORK

ifeq ($(SDL_NETWORK),taptun)

DEFS += -DSDLMAME_NET_TAPTUN
endif

ifeq ($(SDL_NETWORK),pcap)

DEFS += -DSDLMAME_NET_PCAP

# dynamically linked ...
#ifneq ($(TARGETOS),win32)
#LIBS += -lpcap
#endif

endif # ifeq ($(SDL_NETWORK),pcap)

endif # ifndef DONT_USE_NETWORK

#-------------------------------------------------
# Dependencies
#-------------------------------------------------

# due to quirks of using /bin/sh, we need to explicitly specify the current path
CURPATH = ./

ifeq ($(BASE_TARGETOS),os2)
# to avoid name clash of '_brk'
$(OBJ)/emu/cpu/h6280/6280dasm.o : CDEFS += -D__STRICT_ANSI__
endif # OS2

ifeq ($(TARGETOS),solaris)
# solaris only has gcc-4.3 by default and is reporting a false positive
$(OBJ)/emu/video/tms9927.o : CCOMFLAGS += -Wno-error
endif # solaris

# drawSDL depends on the core software renderer, so make sure it exists
$(OSDOBJ)/modules/render/drawsdl.o : $(SRC)/emu/rendersw.inc $(OSDSRC)/modules/render/drawogl.c

# draw13 depends on blit13.h
$(OSDOBJ)/modules/render/draw13.o : $(OSDSRC)/modules/render/blit13.h

#$(OSDCOREOBJS): $(SDLSRC)/sdl.mak

#$(OSDOBJS): $(SDLSRC)/sdl.mak


$(LIBOCORE): $(OSDCOREOBJS)

$(LIBOSD): $(OSDOBJS)


#-------------------------------------------------
# Tools
#-------------------------------------------------

TOOLS += \
	$(BIN)testkeys$(EXE)

$(SDLOBJ)/testkeys.o: $(SDLSRC)/testkeys.c
	@echo Compiling $<...
	$(CC)  $(CFLAGS) $(DEFS) -c $< -o $@

TESTKEYSOBJS = \
	$(SDLOBJ)/testkeys.o \

$(BIN)testkeys$(EXE): $(TESTKEYSOBJS) $(LIBUTIL) $(LIBOCORE) $(SDLUTILMAIN)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(BASELIBS) -o $@

#-------------------------------------------------
# clean up
#-------------------------------------------------

$(OSDCLEAN):
	@rm -f .depend_*

#-------------------------------------------------
# various support targets
#-------------------------------------------------

testlib:
	@echo LIBS: $(LIBS)
	@echo INCPATH: $(INCPATH)
	@echo DEFS: $(DEFS)
	@echo CORE: $(OSDCOREOBJS)

ifneq ($(TARGETOS),win32)
BUILD_VERSION = $(shell grep 'build_version\[\] =' src/version.c | sed -e "s/.*= \"//g" -e "s/ .*//g")
DISTFILES = test_dist.sh whatsnew.txt whatsnew_$(BUILD_VERSION).txt makefile  docs/ src/
EXCLUDES = -x "*/.svn/*"

zip:
	zip -rq ../mame_$(BUILD_VERSION).zip $(DISTFILES) $(EXCLUDES)

endif

