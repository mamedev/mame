###########################################################################
#
#   makefile
#
#   Core makefile for building MAME and derivatives
#
###########################################################################



###########################################################################
#################   BEGIN USER-CONFIGURABLE OPTIONS   #####################
###########################################################################

# REGENIE = 1
# VERBOSE = 1
# NOWERROR = 1
# IGNORE_GIT = 1

# TARGET = mame
# SUBTARGET = tiny
# TOOLS = 1
# EMULATOR = 1
# TESTS = 1
# BENCHMARKS = 1
# OSD = sdl

# NO_OPENGL = 0
# USE_DISPATCH_GL = 0
# MODERN_WIN_API = 0
# USE_SDL = 1
# SDL_INI_PATH = .;$HOME/.mame/;ini;
# SDL2_MULTIAPI = 1
# NO_USE_MIDI = 1
# NO_USE_PORTAUDIO = 1
# NO_USE_PULSEAUDIO = 1
# USE_TAPTUN = 1
# USE_PCAP = 1
# USE_QTDEBUG = 1
# NO_X11 = 1
# NO_USE_XINPUT = 1
# NO_USE_XINPUT_WII_LIGHTGUN_HACK = 1
# FORCE_DRC_C_BACKEND = 1

# DEBUG = 1
# PROFILER = 1
# SANITIZE =

# PTR64 = 1
# BIGENDIAN = 1
# NOASM = 1

# OPTIMIZE = 3
# SYMBOLS = 1
# SYMLEVEL = 2
# MAP = 1
# PROFILE = 1
# ARCHOPTS =
# ARCHOPTS_C =
# ARCHOPTS_CXX =
# ARCHOPTS_OBJC =
# ARCHOPTS_OBJCXX =
# OPT_FLAGS =
# LDOPTS =

# USE_SYSTEM_LIB_ASIO = 1
# USE_SYSTEM_LIB_EXPAT = 1
# USE_SYSTEM_LIB_ZLIB = 1
# USE_SYSTEM_LIB_ZSTD = 1
# USE_SYSTEM_LIB_JPEG = 1
# USE_SYSTEM_LIB_FLAC = 1
# USE_SYSTEM_LIB_LUA = 1
# USE_SYSTEM_LIB_SQLITE3 = 1
# USE_SYSTEM_LIB_PORTMIDI = 1
# USE_SYSTEM_LIB_PORTAUDIO = 1
# USE_SYSTEM_LIB_UTF8PROC = 1
# USE_SYSTEM_LIB_GLM = 1
# USE_SYSTEM_LIB_RAPIDJSON = 1
# USE_SYSTEM_LIB_PUGIXML = 1

# MESA_INSTALL_ROOT = /opt/mesa
# SDL_INSTALL_ROOT = /opt/sdl2
# SDL_FRAMEWORK_PATH = $(HOME)/Library/Frameworks
# USE_LIBSDL = 1
# CYGWIN_BUILD = 1

# BUILDDIR = build
# TARGETOS = windows
# CROSS_BUILD = 1
# TOOLCHAIN =
# OVERRIDE_CC = cc
# OVERRIDE_CXX = c++
# OVERRIDE_LD = ld
# OVERRIDE_AR = ar

# DEPRECATED = 0
# LTO = 1
# SSE2 = 1
# OPENMP = 1

# SEPARATE_BIN = 1
# PYTHON_EXECUTABLE = python3
# SHADOW_CHECK = 1
# STRIP_SYMBOLS = 0

# QT_HOME = /usr/lib64/qt48/

# SOURCES = src/mame/atari/asteroid.cpp,src/mame/cinematronics/cchasm.cpp

# SOURCEFILTER = mydrivers.flt

# FORCE_VERSION_COMPILE = 1

# MSBUILD = 1
# IGNORE_BAD_LOCALISATION = 1
# PRECOMPILE = 0

# DEBUG_DIR=c:\test\location
# DEBUG_ARGS= -window -video bgfx

ifdef PREFIX_MAKEFILE
include $(PREFIX_MAKEFILE)
else
-include useroptions.mak
endif

###########################################################################
##################   END USER-CONFIGURABLE OPTIONS   ######################
###########################################################################


ifndef MAKETYPE
MAKETYPE := gmake
endif

MAKEPARAMS := -R

#
# Determine running OS
#

ifeq ($(OS),Windows_NT)
OS := windows
GENIEOS := windows
PLATFORM := x86
else
UNAME := $(shell uname -mps)
UNAME_M := $(shell uname -m)
UNAME_P := $(shell uname -p)
GENIEOS := linux
PLATFORM := unknown
ifneq ($(filter x86_64,$(UNAME_P)),)
PLATFORM := x86
endif
ifneq ($(filter %86,$(UNAME_P)),)
PLATFORM := x86
endif
ifneq ($(filter alpha,$(UNAME_M)),)
PLATFORM := alpha
endif
ifneq ($(filter alpha,$(UNAME_P)),)
PLATFORM := alpha
endif
ifneq ($(filter arm%,$(UNAME_M)),)
PLATFORM := arm
endif
ifneq ($(filter arm%,$(UNAME_P)),)
PLATFORM := arm
endif
ifneq ($(filter aarch64%,$(UNAME_M)),)
PLATFORM := arm64
endif
ifneq ($(filter arm64%,$(UNAME_M)),)
PLATFORM := arm64
endif
ifneq ($(filter aarch64%,$(UNAME_P)),)
PLATFORM := arm64
endif
ifneq ($(filter powerpc,$(UNAME_P)),)
PLATFORM := powerpc
endif
ifneq ($(filter riscv64%,$(UNAME_M)),)
PLATFORM := riscv64
endif
ifneq ($(filter riscv64%,$(UNAME_P)),)
PLATFORM := riscv64
endif
ifneq ($(filter mips64%,$(UNAME_M)),)
ifeq ($(shell getconf LONG_BIT),64)
PLATFORM := mips64
endif
endif
ifeq ($(firstword $(filter Linux,$(UNAME))),Linux)
OS := linux
endif
ifeq ($(firstword $(filter Solaris,$(UNAME))),Solaris)
OS := solaris
GENIEOS := solaris
endif
ifeq ($(firstword $(filter SunOS,$(UNAME))),SunOS)
OS := solaris
GENIEOS := solaris
endif
ifeq ($(firstword $(filter FreeBSD,$(UNAME))),FreeBSD)
OS := freebsd
GENIEOS := bsd
endif
ifeq ($(firstword $(filter GNU/kFreeBSD,$(UNAME))),GNU/kFreeBSD)
OS := freebsd
GENIEOS := freebsd
endif
ifeq ($(firstword $(filter NetBSD,$(UNAME))),NetBSD)
OS := netbsd
GENIEOS := freebsd
endif
ifeq ($(firstword $(filter OpenBSD,$(UNAME))),OpenBSD)
OS := openbsd
GENIEOS := freebsd
endif
ifeq ($(firstword $(filter Darwin,$(UNAME))),Darwin)
OS := macosx
GENIEOS := darwin
endif
ifeq ($(firstword $(filter Haiku,$(UNAME))),Haiku)
OS := haiku
endif
ifndef OS
$(error Unable to detect OS from uname -a: $(UNAME))
endif
endif

MINGW:=
ifdef MINGW64
	MINGW := $(MINGW64)
else
	MINGW := $(MINGW32)
endif

#-------------------------------------------------
# specify core target: mame, ldplayer
# specify subtarget: mame, tiny, etc.
# build scripts will be run from
# scripts/target/$(TARGET)/$(SUBTARGET).lua
#-------------------------------------------------

ifdef PROJECT
PARAMS += --PROJECT='$(PROJECT)'
TARGET := $(PROJECT)
endif

ifndef TARGET
TARGET := mame
endif

ifndef SUBTARGET
SUBTARGET := $(TARGET)
endif

SUBTARGET_FULL := $(subst -,_,$(SUBTARGET))


CONFIG = release
ifdef DEBUG
ifneq '$(DEBUG)' '0'
CONFIG := debug
endif
endif

ifdef VERBOSE
MAKEPARAMS += verbose=1
else
SILENT := @
MAKEPARAMS += --no-print-directory
endif

ifndef BUILDDIR
BUILDDIR := build
endif

#-------------------------------------------------
# specify OS target, which further differentiates
# the underlying OS; supported values are:
# win32, unix, macosx
#-------------------------------------------------

ifndef TARGETOS

ifeq ($(OS),windows)
TARGETOS := windows
ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
ARCHITECTURE := _x64
endif
ifeq ($(PROCESSOR_ARCHITECTURE),x86)
ARCHITECTURE := _x64
ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
else
ARCHITECTURE := _x86
endif
endif
else
UNAME    := $(shell uname -mps)
TARGETOS := $(OS)

#-------------------------------------------------
# determine the whether -m32, -m64 or nothing
# should be passed to gcc when building genie
#-------------------------------------------------

ifeq ($(ARCHITECTURE),_x86)
MPARAM := -m32
else
ifeq ($(ARCHITECTURE),_x64)
MPARAM := -m64
else
MPARAM :=
endif
endif

ARCHITECTURE := _x86

ifeq ($(firstword $(filter x86_64,$(UNAME))),x86_64)
ARCHITECTURE := _x64
endif
ifeq ($(firstword $(filter amd64,$(UNAME))),amd64)
ARCHITECTURE := _x64
endif
ifeq ($(firstword $(filter ppc64,$(UNAME))),ppc64)
ARCHITECTURE := _x64
endif
ifeq ($(firstword $(filter powerpc64,$(UNAME))),powerpc64)
ARCHITECTURE := _x64
endif
ifeq ($(firstword $(filter ppc64le,$(UNAME))),ppc64le)
ARCHITECTURE := _x64
endif
ifeq ($(firstword $(filter s390x,$(UNAME))),s390x)
ARCHITECTURE := _x64
endif
endif

else
CROSS_BUILD := 1
endif # TARGET_OS

ifdef PTR64
ifeq ($(PTR64),1)
ARCHITECTURE := _x64
else
ARCHITECTURE := _x86
endif
endif

ifeq ($(OS),windows)
ifndef MINGW64
ARCHITECTURE := _x86
endif
ifeq ($(ARCHITECTURE),_x64)
WINDRES  := $(MINGW64)/bin/windres
else
WINDRES  := $(MINGW32)/bin/windres
endif
else
ifeq ($(ARCHITECTURE),_x64)
WINDRES  := $(word 1,$(TOOLCHAIN) x86_64-w64-mingw32-)windres
else
WINDRES  := $(word 1,$(TOOLCHAIN) i686-w64-mingw32-)windres
endif
endif

# Emscripten
ifeq ($(findstring emcc,$(CC)),emcc)
TARGETOS := asmjs
ARCHITECTURE :=
ifndef NOASM
	NOASM := 1
endif
endif

ifeq ($(findstring ppc,$(UNAME)),ppc)
ifndef FORCE_DRC_C_BACKEND
	FORCE_DRC_C_BACKEND := 1
endif
endif

ifeq ($(findstring powerpc,$(UNAME)),powerpc)
ifndef FORCE_DRC_C_BACKEND
	FORCE_DRC_C_BACKEND := 1
endif
endif

ifeq ($(findstring arm,$(UNAME)),arm)
ARCHITECTURE :=
ifneq ($(PLATFORM),arm64)
	ifndef FORCE_DRC_C_BACKEND
		FORCE_DRC_C_BACKEND := 1
	endif
endif
endif

ifeq ($(findstring aarch64,$(UNAME)),aarch64)
ARCHITECTURE :=
endif

ifeq ($(findstring s390x,$(UNAME)),s390x)
ifndef FORCE_DRC_C_BACKEND
	FORCE_DRC_C_BACKEND := 1
endif
endif

ifeq ($(findstring riscv64,$(UNAME)),riscv64)
ARCHITECTURE :=
ifndef FORCE_DRC_C_BACKEND
	FORCE_DRC_C_BACKEND := 1
endif
endif

# Autodetect BIGENDIAN
# MacOSX
ifndef BIGENDIAN
ifneq (,$(findstring Power,$(UNAME)))
BIGENDIAN := 1
endif
# Linux
ifneq (,$(findstring ppc,$(UNAME)))
ifneq (,$(findstring ppc64le,$(UNAME)))
BIGENDIAN := 0
else
BIGENDIAN := 1
endif
endif
ifneq (,$(findstring s390x,$(UNAME)))
BIGENDIAN := 1
endif
# FreeBSD
ifneq (,$(findstring powerpc,$(UNAME)))
ifneq (,$(findstring powerpc64le,$(UNAME)))
BIGENDIAN := 0
else
BIGENDIAN := 1
endif
endif
endif # BIGENDIAN

ifndef PYTHON_EXECUTABLE
PYTHON := python3
else
PYTHON := $(PYTHON_EXECUTABLE)
endif

ifneq ($(TARGETOS),asmjs)
CC := $(SILENT)gcc
LD := $(SILENT)g++
CXX:= $(SILENT)g++
endif

#-------------------------------------------------
# specify OSD layer: windows, sdl, etc.
# build scripts will be run from
# scripts/src/osd/$(OSD).lua
#-------------------------------------------------

ifndef OSD

OSD := sdl

ifeq ($(TARGETOS),windows)
OSD := windows
endif

ifeq ($(TARGETOS),linux)
OSD := sdl
endif

ifeq ($(TARGETOS),freebsd)
OSD := sdl
endif

ifeq ($(TARGETOS),netbsd)
OSD := sdl
endif

ifeq ($(TARGETOS),openbsd)
OSD := sdl
endif

ifeq ($(TARGETOS),solaris)
OSD := sdl
endif

ifeq ($(TARGETOS),macosx)
OSD := sdl
endif

ifeq ($(TARGETOS),asmjs)
OSD := sdl
endif
endif

#-------------------------------------------------
# which 3rdparty library to build;
#  link against system (common) library otherwise
#-------------------------------------------------
ifdef USE_SYSTEM_LIB_ASIO
PARAMS += --with-system-asio='$(USE_SYSTEM_LIB_ASIO)'
endif

ifdef USE_SYSTEM_LIB_EXPAT
PARAMS += --with-system-expat='$(USE_SYSTEM_LIB_EXPAT)'
endif

ifdef USE_SYSTEM_LIB_ZLIB
PARAMS += --with-system-zlib='$(USE_SYSTEM_LIB_ZLIB)'
endif

ifdef USE_SYSTEM_LIB_ZSTD
PARAMS += --with-system-zstd='$(USE_SYSTEM_LIB_ZSTD)'
endif

ifdef USE_SYSTEM_LIB_JPEG
PARAMS += --with-system-jpeg='$(USE_SYSTEM_LIB_JPEG)'
endif

ifdef USE_SYSTEM_LIB_FLAC
PARAMS += --with-system-flac='$(USE_SYSTEM_LIB_FLAC)'
endif

ifdef USE_SYSTEM_LIB_LUA
PARAMS += --with-system-lua='$(USE_SYSTEM_LIB_LUA)'
endif

ifdef USE_SYSTEM_LIB_SQLITE3
PARAMS += --with-system-sqlite3='$(USE_SYSTEM_LIB_SQLITE3)'
endif

ifdef USE_SYSTEM_LIB_PORTMIDI
PARAMS += --with-system-portmidi='$(USE_SYSTEM_LIB_PORTMIDI)'
endif

ifdef USE_SYSTEM_LIB_PORTAUDIO
PARAMS += --with-system-portaudio='$(USE_SYSTEM_LIB_PORTAUDIO)'
endif

ifdef USE_SYSTEM_LIB_UTF8PROC
PARAMS += --with-system-utf8proc='$(USE_SYSTEM_LIB_UTF8PROC)'
endif

ifdef USE_SYSTEM_LIB_GLM
PARAMS += --with-system-glm='$(USE_SYSTEM_LIB_GLM)'
endif

ifdef USE_SYSTEM_LIB_RAPIDJSON
PARAMS += --with-system-rapidjson='$(USE_SYSTEM_LIB_RAPIDJSON)'
endif

ifdef USE_SYSTEM_LIB_PUGIXML
PARAMS += --with-system-pugixml='$(USE_SYSTEM_LIB_PUGIXML)'
endif

#-------------------------------------------------
# distribution may change things
#-------------------------------------------------

ifeq ($(DISTRO),)
DISTRO := generic
else
ifeq ($(DISTRO),debian-stable)
else
$(error DISTRO $(DISTRO) unknown)
endif
endif

PARAMS+= --distro=$(DISTRO)

ifdef TOOLCHAIN
PARAMS += --TOOLCHAIN='$(TOOLCHAIN)'
endif
ifdef OVERRIDE_CC
PARAMS += --CC='$(OVERRIDE_CC)'
ifndef CROSS_BUILD
CC := $(OVERRIDE_CC)
endif
endif
ifdef OVERRIDE_CXX
PARAMS += --CXX='$(OVERRIDE_CXX)'
ifndef CROSS_BUILD
CXX := $(OVERRIDE_CXX)
endif
endif
ifdef OVERRIDE_LD
PARAMS += --LD='$(OVERRIDE_LD)'
ifndef CROSS_BUILD
LD := $(OVERRIDE_LD)
endif
endif
ifdef OVERRIDE_AR
PARAMS += --AR='$(OVERRIDE_AR)'
ifndef CROSS_BUILD
AR := $(OVERRIDE_AR)
endif
endif

#-------------------------------------------------
# sanity check the configuration
#-------------------------------------------------

# enable symbols as it is useless without them
ifdef SANITIZE
SYMBOLS = 1
endif

# profiler defaults to on for DEBUG builds
ifdef DEBUG
ifneq '$(DEBUG)' '0'
ifndef PROFILER
PROFILER = 1
endif
endif
endif

# allow gprof profiling as well, which overrides the internal PROFILER
# also enable symbols as it is useless without them
ifdef PROFILE
PROFILER =
SYMBOLS = 1
endif

# specify a default optimization level if none explicitly stated
ifndef OPTIMIZE
OPTIMIZE = 3
endif

# set the symbols level
ifdef SYMBOLS
PARAMS += --SYMBOLS='$(SYMBOLS)'
ifneq '$(SYMBOLS)' '0'
ifndef SYMLEVEL
ifdef SOURCES
SYMLEVEL = 2
else
SYMLEVEL = 1
endif
endif
endif
endif

ifdef TOOLS
ifneq '$(TOOLS)' '0'
PARAMS += --with-tools
endif
endif

ifndef EMULATOR
PARAMS += --with-emulator
else
ifeq '$(EMULATOR)' '1'
PARAMS += --with-emulator
endif
endif

ifdef TESTS
ifneq '$(TESTS)' '0'
PARAMS += --with-tests
endif
endif

ifdef BENCHMARKS
ifneq '$(BENCHMARKS)' '0'
PARAMS += --with-benchmarks
endif
endif

ifdef SYMLEVEL
PARAMS += --SYMLEVEL='$(SYMLEVEL)'
endif

ifdef PROFILER
PARAMS += --PROFILER='$(PROFILER)'
endif

ifdef PROFILE
PARAMS += --PROFILE='$(PROFILE)'
endif

ifdef OPTIMIZE
PARAMS += --OPTIMIZE=$(OPTIMIZE)
endif

ifdef SHLIB
PARAMS += --SHLIB=$(SHLIB)
endif

ifdef ARCHOPTS
PARAMS += --ARCHOPTS='$(ARCHOPTS)'
endif

ifdef ARCHOPTS_C
PARAMS += --ARCHOPTS_C='$(ARCHOPTS_C)'
endif

ifdef ARCHOPTS_CXX
PARAMS += --ARCHOPTS_CXX='$(ARCHOPTS_CXX)'
endif

ifdef ARCHOPTS_OBJC
PARAMS += --ARCHOPTS_OBJC='$(ARCHOPTS_OBJC)'
endif

ifdef ARCHOPTS_OBJCXX
PARAMS += --ARCHOPTS_OBJCXX='$(ARCHOPTS_OBJCXX)'
endif

ifdef OPT_FLAGS
PARAMS += --OPT_FLAGS='$(OPT_FLAGS)'
endif

ifdef MAP
PARAMS += --MAP='$(MAP)'
endif

ifdef NOASM
TARGET_PARAMS += --NOASM='$(NOASM)'
endif

ifdef BIGENDIAN
PARAMS += --BIGENDIAN='$(BIGENDIAN)'
endif

ifdef FORCE_DRC_C_BACKEND
PARAMS += --FORCE_DRC_C_BACKEND='$(FORCE_DRC_C_BACKEND)'
endif

ifdef NOWERROR
PARAMS += --NOWERROR='$(NOWERROR)'
endif

ifdef TARGET
PARAMS += --target='$(TARGET)'
endif

ifdef SUBTARGET_FULL
PARAMS += --subtarget='$(SUBTARGET_FULL)'
endif

ifdef OSD
TARGET_PARAMS += --osd='$(OSD)'
endif

ifdef BUILDDIR
PARAMS += --build-dir='$(BUILDDIR)'
endif

ifdef TARGETOS
TARGET_PARAMS += --targetos='$(TARGETOS)'
endif

ifdef USE_TAPTUN
PARAMS += --USE_TAPTUN='$(USE_TAPTUN)'
endif

ifdef USE_PCAP
PARAMS += --USE_PCAP='$(USE_PCAP)'
endif

ifdef NO_OPENGL
PARAMS += --NO_OPENGL='$(NO_OPENGL)'
endif

ifdef USE_DISPATCH_GL
PARAMS += --USE_DISPATCH_GL='$(USE_DISPATCH_GL)'
endif

ifdef NO_USE_MIDI
PARAMS += --NO_USE_MIDI='$(NO_USE_MIDI)'
endif

ifdef NO_USE_PORTAUDIO
PARAMS += --NO_USE_PORTAUDIO='$(NO_USE_PORTAUDIO)'
endif

ifdef NO_USE_PULSEAUDIO
PARAMS += --NO_USE_PULSEAUDIO='$(NO_USE_PULSEAUDIO)'
endif

ifdef USE_QTDEBUG
PARAMS += --USE_QTDEBUG='$(USE_QTDEBUG)'
endif

ifdef MODERN_WIN_API
PARAMS += --MODERN_WIN_API='$(MODERN_WIN_API)'
endif

ifdef USE_SDL
PARAMS += --USE_SDL='$(USE_SDL)'
endif

ifdef SDL_INI_PATH
PARAMS += --SDL_INI_PATH='$(SDL_INI_PATH)'
endif

ifdef CYGWIN_BUILD
PARAMS += --CYGWIN_BUILD='$(CYGWIN_BUILD)'
endif

ifdef MESA_INSTALL_ROOT
PARAMS += --MESA_INSTALL_ROOT='$(MESA_INSTALL_ROOT)'
endif

ifdef NO_X11
PARAMS += --NO_X11='$(NO_X11)'
endif

ifdef NO_USE_XINPUT
PARAMS += --NO_USE_XINPUT='$(NO_USE_XINPUT)'
endif

ifdef NO_USE_XINPUT_WII_LIGHTGUN_HACK
PARAMS += --NO_USE_XINPUT_WII_LIGHTGUN_HACK='$(NO_USE_XINPUT_WII_LIGHTGUN_HACK)'
endif

ifdef SDL_LIBVER
PARAMS += --SDL_LIBVER='$(SDL_LIBVER)'
endif

ifdef SDL2_MULTIAPI
PARAMS += --SDL2_MULTIAPI='$(SDL2_MULTIAPI)'
endif

ifdef SDL_INSTALL_ROOT
PARAMS += --SDL_INSTALL_ROOT='$(SDL_INSTALL_ROOT)'
endif

ifdef SDL_FRAMEWORK_PATH
PARAMS += --SDL_FRAMEWORK_PATH='$(SDL_FRAMEWORK_PATH)'
endif

ifdef USE_LIBSDL
PARAMS += --USE_LIBSDL='$(USE_LIBSDL)'
endif

ifdef LDOPTS
PARAMS += --LDOPTS='$(LDOPTS)'
endif

ifdef LTO
PARAMS += --LTO='$(LTO)'
endif

ifdef DEPRECATED
PARAMS += --DEPRECATED='$(DEPRECATED)'
endif

ifdef SSE2
PARAMS += --SSE2='$(SSE2)'
endif

ifdef OPENMP
PARAMS += --OPENMP='$(OPENMP)'
endif

ifdef SEPARATE_BIN
PARAMS += --SEPARATE_BIN='$(SEPARATE_BIN)'
endif

ifdef PYTHON_EXECUTABLE
PARAMS += --PYTHON_EXECUTABLE='$(PYTHON_EXECUTABLE)'
endif

ifdef SHADOW_CHECK
PARAMS += --SHADOW_CHECK='$(SHADOW_CHECK)'
endif

ifdef STRIP_SYMBOLS
PARAMS += --STRIP_SYMBOLS='$(STRIP_SYMBOLS)'
endif

ifdef QT_HOME
PARAMS += --QT_HOME='$(QT_HOME)'
endif

ifdef SOURCES
PARAMS += --SOURCES='$(SOURCES)'
endif

ifdef SOURCEFILTER
PARAMS += --SOURCEFILTER='$(SOURCEFILTER)'
endif

ifdef FORCE_VERSION_COMPILE
PARAMS += --FORCE_VERSION_COMPILE='$(FORCE_VERSION_COMPILE)'
endif

ifdef PLATFORM
TARGET_PARAMS += --PLATFORM='$(PLATFORM)'
endif

ifdef PRECOMPILE
PARAMS += --precompile='$(PRECOMPILE)'
endif

ifdef DEBUG_DIR
PARAMS += --DEBUG_DIR='$(DEBUG_DIR)'
endif

ifdef DEBUG_ARGS
PARAMS += --DEBUG_ARGS='$(DEBUG_ARGS)'
endif

ifdef WEBASSEMBLY
PARAMS += --WEBASSEMBLY='$(WEBASSEMBLY)'
endif

ifdef SANITIZE
PARAMS += --SANITIZE='$(SANITIZE)'
endif

#-------------------------------------------------
# All scripts
#-------------------------------------------------

SCRIPTS = scripts/genie.lua \
	scripts/src/lib.lua \
	scripts/src/emu.lua \
	scripts/src/machine.lua \
	scripts/src/main.lua \
	scripts/src/3rdparty.lua \
	scripts/src/cpu.lua \
	scripts/src/mame/frontend.lua \
	scripts/src/osd/modules.lua \
	$(wildcard scripts/src/osd/$(OSD)*.lua) \
	scripts/src/sound.lua \
	scripts/src/tools.lua \
	scripts/src/tests.lua \
	scripts/src/benchmarks.lua \
	scripts/src/video.lua \
	scripts/src/bus.lua \
	scripts/src/netlist.lua \
	scripts/src/formats.lua \
	scripts/toolchain.lua \
	scripts/src/osd/modules.lua \
	$(wildcard src/osd/$(OSD)/$(OSD).mak) \
	$(wildcard src/$(TARGET)/$(SUBTARGET_FULL).mak)

ifdef SOURCEFILTER
SCRIPTS += $(SOURCEFILTER)
else
ifndef SOURCES
ifdef PROJECT
SCRIPTS += projects/$(PROJECT)/scripts/target/$(TARGET)/$(SUBTARGET_FULL).lua
else
# A filter file can be used as an alternative
#SCRIPTS += scripts/target/$(TARGET)/$(SUBTARGET_FULL).lua
endif
endif
endif

ifdef REGENIE
SCRIPTS+= regenie
endif

#-------------------------------------------------
# Dependent stuff
#-------------------------------------------------

# extension for executables
EXE :=

ifeq ($(OS),windows)
EXE := .exe
endif

SHELLTYPE := msdos
ifeq (,$(ComSpec)$(COMSPEC))
  SHELLTYPE := posix
endif
ifeq (/bin,$(findstring /bin,$(SHELL)))
  SHELLTYPE := posix
endif
ifeq (/bin,$(findstring /bin,$(MAKESHELL)))
  SHELLTYPE := posix
endif

ifeq (posix,$(SHELLTYPE))
  MKDIR = $(SILENT) mkdir -p "$(1)"
  COPY  = $(SILENT) cp -fR "$(1)"/* "$(2)"
else
  MKDIR = $(SILENT) mkdir "$(subst /,\\,$(1))" 2> nul || exit 0
  COPY  = $(SILENT) copy /Y "$(subst /,\\,$(1))" "$(subst /,\\,$(2))" > nul || exit 0
endif

GENDIR = $(BUILDDIR)/generated

# all sources are under the src/ directory
SRC = src

# all 3rd party sources are under the 3rdparty/ directory
3RDPARTY = 3rdparty
ifeq ($(SUBTARGET_FULL),mame)
PROJECT_NAME := $(SUBTARGET_FULL)
else
PROJECT_NAME := $(TARGET)$(SUBTARGET_FULL)
endif


ifeq ($(OS),windows)
ifeq (posix,$(SHELLTYPE))
GCC_VERSION      := $(shell $(TOOLCHAIN)$(subst @,,$(CC)) -dumpfullversion 2> /dev/null)
CLANG_VERSION    := $(shell $(TOOLCHAIN)$(subst @,,$(CC)) --version 2> /dev/null| head -n 1 | grep clang | sed "s/^.*[^0-9]\([0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\).*$$/\1/" | head -n 1)
PYTHON_AVAILABLE := $(shell $(PYTHON) --version > /dev/null 2>&1 && echo python)
GIT_AVAILABLE    := $(shell git --version > /dev/null 2>&1 && echo git)
else
GCC_VERSION      := $(shell $(TOOLCHAIN)$(subst @,,$(CC)) -dumpfullversion 2> NUL)
CLANG_VERSION    := $(shell $(TOOLCHAIN)$(subst @,,$(CC)) --version 2> NUL| head -n 1 | grep clang | sed "s/^.*[^0-9]\([0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\).*$$/\1/" | head -n 1)
PYTHON_AVAILABLE := $(shell $(PYTHON) --version > NUL 2>&1 && echo python)
GIT_AVAILABLE    := $(shell git --version > NUL 2>&1 && echo git)
endif
ifdef MSBUILD
MSBUILD_PARAMS   := /v:minimal /m:$(NUMBER_OF_PROCESSORS)
ifeq ($(CONFIG),debug)
MSBUILD_PARAMS += /p:Configuration=Debug
else
MSBUILD_PARAMS += /p:Configuration=Release
endif
ifeq ($(ARCHITECTURE),_x64)
MSBUILD_PARAMS += /p:Platform=x64
else
MSBUILD_PARAMS += /p:Platform=win32
endif
ifeq (posix,$(SHELLTYPE))
MSBUILD_PARAMS := $(subst /,-,$(MSBUILD_PARAMS))
endif
endif
else
ifdef OVERRIDE_CC
GCC_VERSION      := $(shell $(TOOLCHAIN)$(subst @,,$(OVERRIDE_CC)) -dumpfullversion 2> /dev/null)
else
GCC_VERSION      := $(shell $(TOOLCHAIN)$(subst @,,$(CC)) -dumpfullversion 2> /dev/null)
endif
ifeq ($(findstring emcc,$(CC)),emcc)
CLANG_VERSION    := $(shell $(TOOLCHAIN)$(subst @,,$(CC))  -v  2>&1 >/dev/null | grep 'clang version' | head -n 1 | grep -e 'version [0-9]\+\.[0-9]\+\(\.[0-9]\+\)\?' -o | grep -e '[0-9]\+\.[0-9]\+\(\.[0-9]\+\)\?' -o | tail -n 1)
else
CLANG_VERSION    := $(shell $(TOOLCHAIN)$(subst @,,$(CC))  --version  2> /dev/null | head -n 1 | grep -e 'version [0-9]\+\.[0-9]\+\(\.[0-9]\+\)\?' -o | grep -e '[0-9]\+\.[0-9]\+\(\.[0-9]\+\)\?' -o | tail -n 1)
endif
PYTHON_AVAILABLE := $(shell $(PYTHON) --version > /dev/null 2>&1 && echo python)
GIT_AVAILABLE := $(shell git --version > /dev/null 2>&1 && echo git)
endif

ifeq ($(CLANG_VERSION),)
$(info GCC $(GCC_VERSION) detected)
else
$(info Clang $(CLANG_VERSION) detected)
ifneq ($(TARGETOS),asmjs)
ifeq ($(ARCHITECTURE),_x64)
ARCHITECTURE := _x64_clang
else
ifneq ($(filter arm64%,$(UNAME_M)),)
ARCHITECTURE := _arm64_clang
else
ARCHITECTURE := _x86_clang
endif
endif
endif
endif

ifneq ($(PYTHON_AVAILABLE),python)
$(error Python is not available in path)
endif

ifneq ($(GIT_AVAILABLE),git)
	IGNORE_GIT := 1
endif
ifeq ($(filter .git,$(wildcard .*)),)
	IGNORE_GIT := 1
endif

ifeq (posix,$(SHELLTYPE))
OLD_GIT_VERSION := $(shell cat $(GENDIR)/git_desc 2> /dev/null)
else
OLD_GIT_VERSION := $(shell cat $(GENDIR)/git_desc 2> NUL)
endif
ifneq ($(IGNORE_GIT),1)
NEW_GIT_VERSION := $(shell git describe --dirty)
else
NEW_GIT_VERSION := unknown
endif
ifeq ($(NEW_GIT_VERSION),)
NEW_GIT_VERSION := unknown
endif

GENIE := 3rdparty/genie/bin/$(GENIEOS)/genie$(EXE)

ifeq ($(TARGET),$(SUBTARGET_FULL))
FULLTARGET := $(TARGET)
else
FULLTARGET := $(TARGET)$(SUBTARGET_FULL)
endif
PROJECTDIR := $(BUILDDIR)/projects/$(OSD)/$(FULLTARGET)
PROJECTDIR_SDL := $(BUILDDIR)/projects/sdl/$(FULLTARGET)
PROJECTDIR_WIN := $(BUILDDIR)/projects/windows/$(FULLTARGET)

.PHONY: all clean regenie generate FORCE
all: $(GENIE) $(TARGETOS)$(ARCHITECTURE)
regenie:
FORCE:

#-------------------------------------------------
# gmake-mingw64-gcc
#-------------------------------------------------

$(PROJECTDIR)/$(MAKETYPE)-mingw64-gcc/Makefile: makefile $(SCRIPTS) $(GENIE)
ifndef MINGW64
	$(error MINGW64 is not set)
endif
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --gcc=mingw64-gcc --gcc_version=$(GCC_VERSION) $(MAKETYPE)

.PHONY: windows_x64
windows_x64: generate $(PROJECTDIR)/$(MAKETYPE)-mingw64-gcc/Makefile
ifndef MINGW64
	$(error MINGW64 is not set)
endif
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-mingw64-gcc config=$(CONFIG)64 WINDRES=$(WINDRES) precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-mingw64-gcc config=$(CONFIG)64 WINDRES=$(WINDRES)

#-------------------------------------------------
# gmake-mingw32-gcc
#-------------------------------------------------

.PHONY: windows
windows: windows_x86

$(PROJECTDIR)/$(MAKETYPE)-mingw32-gcc/Makefile: makefile $(SCRIPTS) $(GENIE)
ifndef MINGW32
	$(error MINGW32 is not set)
endif
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --gcc=mingw32-gcc --gcc_version=$(GCC_VERSION) $(MAKETYPE)

.PHONY: windows_x86
windows_x86: generate $(PROJECTDIR)/$(MAKETYPE)-mingw32-gcc/Makefile
ifndef MINGW32
	$(error MINGW32 is not set)
endif
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-mingw32-gcc config=$(CONFIG)32 WINDRES=$(WINDRES) precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-mingw32-gcc config=$(CONFIG)32 WINDRES=$(WINDRES)

#-------------------------------------------------
# gmake-mingw-clang
#-------------------------------------------------

$(PROJECTDIR)/$(MAKETYPE)-mingw-clang/Makefile: makefile $(SCRIPTS) $(GENIE)
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --gcc=mingw-clang --gcc_version=$(CLANG_VERSION) $(MAKETYPE)

.PHONY: windows_x64_clang
windows_x64_clang: generate $(PROJECTDIR)/$(MAKETYPE)-mingw-clang/Makefile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-mingw-clang config=$(CONFIG)64 WINDRES=$(WINDRES) precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-mingw-clang config=$(CONFIG)64 WINDRES=$(WINDRES)

.PHONY: windows_x86_clang
windows_x86_clang: generate $(PROJECTDIR)/$(MAKETYPE)-mingw-clang/Makefile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-mingw-clang config=$(CONFIG)32 WINDRES=$(WINDRES) precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-mingw-clang config=$(CONFIG)32 WINDRES=$(WINDRES)

#-------------------------------------------------
# Visual Studio 2022
#-------------------------------------------------

.PHONY: vs2022
vs2022: generate
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) vs2022
ifdef MSBUILD
	$(SILENT) msbuild.exe $(PROJECTDIR_WIN)/vs2022/$(PROJECT_NAME).sln $(MSBUILD_PARAMS)
endif

.PHONY: vs2022_clang
vs2022_clang: generate
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --vs=clangcl vs2022
ifdef MSBUILD
	$(SILENT) msbuild.exe $(PROJECTDIR_WIN)/vs2022-clang/$(PROJECT_NAME).sln $(MSBUILD_PARAMS)
endif

.PHONY: vs2022_intel
vs2022_intel: generate
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --vs=intel-15 vs2022
ifdef MSBUILD
	$(SILENT) msbuild.exe $(PROJECTDIR_WIN)/vs2022-intel/$(PROJECT_NAME).sln $(MSBUILD_PARAMS)
endif

#-------------------------------------------------
# android-ndk
#-------------------------------------------------

.PHONY: android-ndk
android-ndk:
ifndef ANDROID_NDK_HOME
	$(error ANDROID_NDK_HOME is not set)
endif
ifndef SDL_INSTALL_ROOT
	$(error SDL_INSTALL_ROOT is not set)
endif
ifeq ($(OS),windows)
	$(eval CLANG_VERSION := $(shell $(ANDROID_NDK_HOME)/toolchains/llvm/prebuilt/windows-x86_64/bin/clang -dumpversion 2> /dev/null))
else ifeq ($(OS),linux)
	$(eval CLANG_VERSION := $(shell $(ANDROID_NDK_HOME)/toolchains/llvm/prebuilt/linux-x86_64/bin/clang -dumpversion 2> /dev/null))
else ifeq ($(OS),macosx)
	$(eval CLANG_VERSION := $(shell $(ANDROID_NDK_HOME)/toolchains/llvm/prebuilt/darwin-x86_64/bin/clang -dumpversion 2> /dev/null))
else
	$(error Unsupported Android build platform)
endif

#-------------------------------------------------
# android-arm
#-------------------------------------------------

$(PROJECTDIR_SDL)/$(MAKETYPE)-android-arm/Makefile: makefile $(SCRIPTS) $(GENIE)
	$(SILENT) $(GENIE) $(PARAMS) --gcc=android-arm --gcc_version=$(CLANG_VERSION) --osd=sdl --targetos=android --PLATFORM=arm --NOASM=1 $(MAKETYPE)

.PHONY: android-arm
android-arm: android-ndk generate $(PROJECTDIR_SDL)/$(MAKETYPE)-android-arm/Makefile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR_SDL)/$(MAKETYPE)-android-arm config=$(CONFIG) precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR_SDL)/$(MAKETYPE)-android-arm config=$(CONFIG)

#-------------------------------------------------
# android-arm64
#-------------------------------------------------

$(PROJECTDIR_SDL)/$(MAKETYPE)-android-arm64/Makefile: makefile $(SCRIPTS) $(GENIE)
	$(SILENT) $(GENIE) $(PARAMS) --gcc=android-arm64 --gcc_version=$(CLANG_VERSION) --osd=sdl --targetos=android --PLATFORM=arm64 --NOASM=1 $(MAKETYPE)

.PHONY: android-arm64
android-arm64: android-ndk generate $(PROJECTDIR_SDL)/$(MAKETYPE)-android-arm64/Makefile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR_SDL)/$(MAKETYPE)-android-arm64 config=$(CONFIG) precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR_SDL)/$(MAKETYPE)-android-arm64 config=$(CONFIG)

#-------------------------------------------------
# android-x86
#-------------------------------------------------

$(PROJECTDIR_SDL)/$(MAKETYPE)-android-x86/Makefile: makefile $(SCRIPTS) $(GENIE)
	$(SILENT) $(GENIE) $(PARAMS) --gcc=android-x86 --gcc_version=$(CLANG_VERSION) --osd=sdl --targetos=android --PLATFORM=x86 $(MAKETYPE)

.PHONY: android-x86
android-x86: android-ndk generate $(PROJECTDIR_SDL)/$(MAKETYPE)-android-x86/Makefile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR_SDL)/$(MAKETYPE)-android-x86 config=$(CONFIG) precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR_SDL)/$(MAKETYPE)-android-x86 config=$(CONFIG)

#-------------------------------------------------
# android-x64
#-------------------------------------------------

$(PROJECTDIR_SDL)/$(MAKETYPE)-android-x64/Makefile: makefile $(SCRIPTS) $(GENIE)
	$(SILENT) $(GENIE) $(PARAMS) --gcc=android-x64 --gcc_version=$(CLANG_VERSION) --osd=sdl --targetos=android --PLATFORM=x64 $(MAKETYPE)

.PHONY: android-x64
android-x64: android-ndk generate $(PROJECTDIR_SDL)/$(MAKETYPE)-android-x64/Makefile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR_SDL)/$(MAKETYPE)-android-x64 config=$(CONFIG) precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR_SDL)/$(MAKETYPE)-android-x64 config=$(CONFIG)

#-------------------------------------------------
# asmjs / Emscripten
#-------------------------------------------------

$(PROJECTDIR)/$(MAKETYPE)-asmjs/Makefile: makefile $(SCRIPTS) $(GENIE)
ifndef EMSCRIPTEN
	$(error EMSCRIPTEN is not set)
endif
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --gcc=asmjs --gcc_version=$(CLANG_VERSION) $(MAKETYPE)

.PHONY: asmjs
asmjs: generate $(PROJECTDIR)/$(MAKETYPE)-asmjs/Makefile
ifndef EMSCRIPTEN
	$(error EMSCRIPTEN is not set)
endif
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-asmjs config=$(CONFIG) precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-asmjs config=$(CONFIG)

#-------------------------------------------------
# gmake-linux
#-------------------------------------------------

$(PROJECTDIR)/$(MAKETYPE)-linux/Makefile: makefile $(SCRIPTS) $(GENIE)
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --gcc=linux-gcc --gcc_version=$(GCC_VERSION) $(MAKETYPE)

.PHONY: linux_x64
linux_x64: generate $(PROJECTDIR)/$(MAKETYPE)-linux/Makefile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-linux config=$(CONFIG)64 precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-linux config=$(CONFIG)64

.PHONY: linux_x86
linux_x86: generate $(PROJECTDIR)/$(MAKETYPE)-linux/Makefile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-linux config=$(CONFIG)32 precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-linux config=$(CONFIG)32

.PHONY: linux
linux: generate $(PROJECTDIR)/$(MAKETYPE)-linux/Makefile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-linux config=$(CONFIG) precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-linux config=$(CONFIG)

#-------------------------------------------------
# gmake-linux-clang
#-------------------------------------------------

$(PROJECTDIR)/$(MAKETYPE)-linux-clang/Makefile: makefile $(SCRIPTS) $(GENIE)
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --gcc=linux-clang --gcc_version=$(CLANG_VERSION) $(MAKETYPE)

.PHONY: linux_x64_clang
linux_x64_clang: generate $(PROJECTDIR)/$(MAKETYPE)-linux-clang/Makefile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-linux-clang config=$(CONFIG)64 precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-linux-clang config=$(CONFIG)64

.PHONY: linux_x86_clang
linux_x86_clang: generate $(PROJECTDIR)/$(MAKETYPE)-linux-clang/Makefile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-linux-clang config=$(CONFIG)32 precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-linux-clang config=$(CONFIG)32

#-------------------------------------------------
# gmake-osx
#-------------------------------------------------

$(PROJECTDIR)/$(MAKETYPE)-osx/Makefile: makefile $(SCRIPTS) $(GENIE)
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --gcc=osx --gcc_version=$(GCC_VERSION) $(MAKETYPE)

.PHONY: macosx_x64
macosx_x64: generate $(PROJECTDIR)/$(MAKETYPE)-osx/Makefile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-osx config=$(CONFIG)64 precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-osx config=$(CONFIG)64

.PHONY: macosx
macosx: macosx_x86

.PHONY: macosx_x86
macosx_x86: generate $(PROJECTDIR)/$(MAKETYPE)-osx/Makefile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-osx config=$(CONFIG)32 precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-osx config=$(CONFIG)32

#-------------------------------------------------
# gmake-osx-clang
#-------------------------------------------------

$(PROJECTDIR)/$(MAKETYPE)-osx-clang/Makefile: makefile $(SCRIPTS) $(GENIE)
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --gcc=osx-clang --gcc_version=$(CLANG_VERSION) $(MAKETYPE)

.PHONY: macosx_x64_clang
macosx_x64_clang: generate $(PROJECTDIR)/$(MAKETYPE)-osx-clang/Makefile
	$(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-osx-clang config=$(CONFIG)64 precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-osx-clang config=$(CONFIG)64

.PHONY: macosx_arm64_clang
macosx_arm64_clang: generate $(PROJECTDIR)/$(MAKETYPE)-osx-clang/Makefile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-osx-clang config=$(CONFIG)64 precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-osx-clang config=$(CONFIG)64

.PHONY: macosx_x86_clang
macosx_x86_clang: generate $(PROJECTDIR)/$(MAKETYPE)-osx-clang/Makefile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-osx-clang config=$(CONFIG)32 precompile
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C $(PROJECTDIR)/$(MAKETYPE)-osx-clang config=$(CONFIG)32

#-------------------------------------------------
# gmake-solaris
#-------------------------------------------------

ifndef CLANG_VERSION
$(PROJECTDIR)/$(MAKETYPE)-solaris/Makefile: makefile $(SCRIPTS) $(GENIE)
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --gcc=solaris --gcc_version=$(GCC_VERSION) $(MAKETYPE)
endif
.PHONY: solaris_x64
solaris_x64: generate $(PROJECTDIR)/$(MAKETYPE)-solaris/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-solaris config=$(CONFIG)64 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-solaris config=$(CONFIG)64

.PHONY: solaris
solaris: solaris_x86

.PHONY: solaris_x86
solaris_x86: generate $(PROJECTDIR)/$(MAKETYPE)-solaris/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-solaris config=$(CONFIG)32 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-solaris config=$(CONFIG)32

#-------------------------------------------------
# gmake-solaris-clang
#-------------------------------------------------

ifdef CLANG_VERSION
$(PROJECTDIR)/$(MAKETYPE)-solaris/Makefile: makefile $(SCRIPTS) $(GENIE)
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --gcc=solaris --gcc_version=$(CLANG_VERSION) $(MAKETYPE)
endif
.PHONY: solaris_x64_clang
solaris_x64_clang: generate $(PROJECTDIR)/$(MAKETYPE)-solaris/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-solaris config=$(CONFIG)64 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-solaris config=$(CONFIG)64

.PHONY: solaris_clang
solaris_clang: solaris_x86_clang

.PHONY: solaris_x86_clang
solaris_x86_clang: generate $(PROJECTDIR)/$(MAKETYPE)-solaris/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-solaris config=$(CONFIG)32 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-solaris config=$(CONFIG)32

#-------------------------------------------------
# gmake-freebsd
#-------------------------------------------------

$(PROJECTDIR)/$(MAKETYPE)-freebsd/Makefile: makefile $(SCRIPTS) $(GENIE)
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --gcc=freebsd --gcc_version=$(GCC_VERSION) $(MAKETYPE)

.PHONY: freebsd_x64
freebsd_x64: generate $(PROJECTDIR)/$(MAKETYPE)-freebsd/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-freebsd config=$(CONFIG)64 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-freebsd config=$(CONFIG)64

.PHONY: freebsd
freebsd: freebsd_x86

.PHONY: freebsd_x86
freebsd_x86: generate $(PROJECTDIR)/$(MAKETYPE)-freebsd/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-freebsd config=$(CONFIG)32 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-freebsd config=$(CONFIG)32

#-------------------------------------------------
# gmake-freebsd-clang
#-------------------------------------------------

$(PROJECTDIR)/$(MAKETYPE)-freebsd-clang/Makefile: makefile $(SCRIPTS) $(GENIE)
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --gcc=freebsd-clang --gcc_version=$(CLANG_VERSION) $(MAKETYPE)

.PHONY: freebsd_x64_clang
freebsd_x64_clang: generate $(PROJECTDIR)/$(MAKETYPE)-freebsd-clang/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-freebsd-clang config=$(CONFIG)64 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-freebsd-clang config=$(CONFIG)64

.PHONY: freebsd_x86_clang
freebsd_x86_clang: generate $(PROJECTDIR)/$(MAKETYPE)-freebsd-clang/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-freebsd-clang config=$(CONFIG)32 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-freebsd-clang config=$(CONFIG)32

#-------------------------------------------------
# gmake-netbsd
#-------------------------------------------------

$(PROJECTDIR)/$(MAKETYPE)-netbsd/Makefile: makefile $(SCRIPTS) $(GENIE)
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --gcc=netbsd --gcc_version=$(GCC_VERSION) $(MAKETYPE)

.PHONY: netbsd_x64
netbsd_x64: generate $(PROJECTDIR)/$(MAKETYPE)-netbsd/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-netbsd config=$(CONFIG)64 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-netbsd config=$(CONFIG)64

.PHONY: netbsd
netbsd: netbsd_x86

.PHONY: netbsd_x86
netbsd_x86: generate $(PROJECTDIR)/$(MAKETYPE)-netbsd/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-netbsd config=$(CONFIG)32 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-netbsd config=$(CONFIG)32

#-------------------------------------------------
# gmake-netbsd-clang
#-------------------------------------------------

$(PROJECTDIR)/$(MAKETYPE)-netbsd-clang/Makefile: makefile $(SCRIPTS) $(GENIE)
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --gcc=netbsd-clang --gcc_version=$(CLANG_VERSION) $(MAKETYPE)

.PHONY: netbsd_x64_clang
netbsd_x64_clang: generate $(PROJECTDIR)/$(MAKETYPE)-netbsd-clang/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-netbsd-clang config=$(CONFIG)64 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-netbsd-clang config=$(CONFIG)64

.PHONY: netbsd_x86_clang
netbsd_x86_clang: generate $(PROJECTDIR)/$(MAKETYPE)-netbsd-clang/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-netbsd-clang config=$(CONFIG)32 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-netbsd-clang config=$(CONFIG)32

#-------------------------------------------------
# gmake-openbsd
#-------------------------------------------------

$(PROJECTDIR)/$(MAKETYPE)-openbsd/Makefile: makefile $(SCRIPTS) $(GENIE)
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --gcc=openbsd --gcc_version=$(GCC_VERSION) $(MAKETYPE)

.PHONY: openbsd_x64
openbsd_x64: generate $(PROJECTDIR)/$(MAKETYPE)-openbsd/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-openbsd config=$(CONFIG)64 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-openbsd config=$(CONFIG)64

.PHONY: openbsd_arm64
openbsd_arm64: generate $(PROJECTDIR)/$(MAKETYPE)-openbsd/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-openbsd config=$(CONFIG)64 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-openbsd config=$(CONFIG)64

.PHONY: openbsd
openbsd: openbsd_x86

.PHONY: openbsd_x86
openbsd_x86: generate $(PROJECTDIR)/$(MAKETYPE)-openbsd/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-openbsd config=$(CONFIG)32 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-openbsd config=$(CONFIG)32

#-------------------------------------------------
# gmake-openbsd-clang
#-------------------------------------------------

$(PROJECTDIR)/$(MAKETYPE)-openbsd-clang/Makefile: makefile $(SCRIPTS) $(GENIE)
	$(SILENT) $(GENIE) $(PARAMS) $(TARGET_PARAMS) --gcc=openbsd-clang --gcc_version=$(CLANG_VERSION) $(MAKETYPE)

.PHONY: openbsd_x64_clang
openbsd_x64_clang: generate $(PROJECTDIR)/$(MAKETYPE)-openbsd-clang/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-openbsd-clang config=$(CONFIG)64 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-openbsd-clang config=$(CONFIG)64

.PHONY: openbsd_arm64_clang
openbsd_arm64_clang: generate $(PROJECTDIR)/$(MAKETYPE)-openbsd-clang/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-openbsd-clang config=$(CONFIG)64 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-openbsd-clang config=$(CONFIG)64

.PHONY: openbsd_x86_clang
openbsd_x86_clang: generate $(PROJECTDIR)/$(MAKETYPE)-openbsd-clang/Makefile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-openbsd-clang config=$(CONFIG)32 precompile
	$(SILENT) $(MAKE) -C $(PROJECTDIR)/$(MAKETYPE)-openbsd-clang config=$(CONFIG)32

#-------------------------------------------------
# Clean/bootstrap
#-------------------------------------------------

GENIE_SRC=$(wildcard 3rdparty/genie/src/host/*.c)

$(GENIE): $(GENIE_SRC)
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C 3rdparty/genie/build/gmake.$(GENIEOS) -f genie.make MPARAM=$(MPARAM)

3rdparty/genie/src/hosts/%.c:

.PHONY: genieclean
genieclean:
	$(SILENT) $(MAKE) $(MAKEPARAMS) -C 3rdparty/genie/build/gmake.$(GENIEOS) -f genie.make MPARAM=$(MPARAM) clean

clean: genieclean
	@echo Cleaning...
	-$(SILENT)rm -f language/*/*.mo
	-$(SILENT)rm -rf $(BUILDDIR)
	-$(SILENT)rm -rf 3rdparty/bgfx/.build

GEN_FOLDERS := $(GENDIR)/$(TARGET)/layout/ $(GENDIR)/$(TARGET)/$(SUBTARGET_FULL)/ $(GENDIR)/mame/drivers/ $(GENDIR)/mame/machine/

rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
LAYOUTS=$(wildcard $(SRC)/$(TARGET)/layout/*.lay)

ifneq (,$(wildcard src/osd/$(OSD)/$(OSD).mak))
include src/osd/$(OSD)/$(OSD).mak
endif

ifneq (,$(wildcard src/$(TARGET)/$(TARGET).mak))
include src/$(TARGET)/$(TARGET).mak
endif

$(GEN_FOLDERS):
	-$(call MKDIR,$@)

genie: $(GENIE)

generate: \
		genie \
		$(GEN_FOLDERS) \
		$(GENDIR)/version.cpp \
		$(patsubst %.po,%.mo,$(call rwildcard, language/, *.po)) \
		$(patsubst $(SRC)/%.lay,$(GENDIR)/%.lh,$(LAYOUTS))

ifneq ($(NEW_GIT_VERSION),$(OLD_GIT_VERSION))
stale:

.PHONY: stale

$(GENDIR)/git_desc: stale | $(GEN_FOLDERS)
	@echo $(NEW_GIT_VERSION) > $@
endif

ifeq (posix,$(SHELLTYPE))
$(GENDIR)/version.cpp: makefile $(GENDIR)/git_desc | $(GEN_FOLDERS)
	@echo '#define BARE_BUILD_VERSION "0.275"' > $@
	@echo '#define BARE_VCS_REVISION "$(NEW_GIT_VERSION)"' >> $@
	@echo 'extern const char bare_build_version[];' >> $@
	@echo 'extern const char bare_vcs_revision[];' >> $@
	@echo 'extern const char build_version[];' >> $@
	@echo 'const char bare_build_version[] = BARE_BUILD_VERSION;' >> $@
	@echo 'const char bare_vcs_revision[] = BARE_VCS_REVISION;' >> $@
	@echo 'const char build_version[] = BARE_BUILD_VERSION " (" BARE_VCS_REVISION ")";' >> $@
else
$(GENDIR)/version.cpp: makefile $(GENDIR)/git_desc | $(GEN_FOLDERS)
	@echo #define BARE_BUILD_VERSION "0.275" > $@
	@echo #define BARE_VCS_REVISION "$(NEW_GIT_VERSION)" >> $@
	@echo extern const char bare_build_version[]; >> $@
	@echo extern const char bare_vcs_revision[]; >> $@
	@echo extern const char build_version[]; >> $@
	@echo const char bare_build_version[] = BARE_BUILD_VERSION; >> $@
	@echo const char bare_vcs_revision[] = BARE_VCS_REVISION; >> $@
	@echo const char build_version[] = BARE_BUILD_VERSION " (" BARE_VCS_REVISION ")"; >> $@
endif


$(GENDIR)/%.lh: $(SRC)/%.lay scripts/build/complay.py | $(GEN_FOLDERS)
	@echo Compressing $<...
	$(SILENT)$(PYTHON) scripts/build/complay.py $< $@ layout_$(basename $(notdir $<))

%.mo: %.po
	@echo Converting translation $<...
ifdef IGNORE_BAD_LOCALISATION
	$(SILENT)$(PYTHON) scripts/build/msgfmt.py --output-file $@ $< || exit 0
else
	$(SILENT)$(PYTHON) scripts/build/msgfmt.py --output-file $@ $<
endif

#-------------------------------------------------
# Regression tests
#-------------------------------------------------

include regtests/regtests.mak

.PHONY: tests

tests: $(REGTESTS)

#-------------------------------------------------
# Source cleanup
#-------------------------------------------------

.PHONY: cleansrc

# FIXME: make this work with SEPARATE_BIN
cleansrc:
	@echo Cleaning up tabs/spaces/end of lines....
ifeq (posix,$(SHELLTYPE))
	$(SILENT)- find src \( \
		-name \*.c -o -name \*.cpp -o \
		-name \*.h -o -name \*.hpp -o -name \*.hxx -o \
		-name \*.ipp -o \
		-name \*.mm -o \
		-name \*.lay -o \
		-name \*.lst \
		\) -print0 | xargs -0 -n 20 ./srcclean >&2
	$(SILENT)- find hash    \( -name \*.hsi -o -name \*.xml  \) -print0 | xargs -0 -n 20 ./srcclean >&2
	$(SILENT)- find bgfx    \( -name \*.json                 \) -print0 | xargs -0 -n 20 ./srcclean >&2
	$(SILENT)- find plugins \( -name \*.lua -o -name \*.json \) -print0 | xargs -0 -n 20 ./srcclean >&2
	$(SILENT)- find scripts \( -name \*.lua                  \) -print0 | xargs -0 -n 20 ./srcclean >&2
else
	$(shell for /r src     %%i in (*.c, *.cpp, *.h, *.hpp, *.hxx, *.ipp, *.mm, *.lay, *.lst) do srcclean %%i >&2 )
	$(shell for /r hash    %%i in (*.hsi, *.xml)  do srcclean %%i >&2 )
	$(shell for /r bgfx    %%i in (*.json)        do srcclean %%i >&2 )
	$(shell for /r plugins %%i in (*.lua, *.json) do srcclean %%i >&2 )
	$(shell for /r scripts %%i in (*.lua)         do srcclean %%i >&2 )
endif

#-------------------------------------------------
# Doxygen documentation
#-------------------------------------------------

.PHONY: doxygen

doxygen:
	@echo Generate Doxygen documentation
	doxygen doxygen/doxygen.config

#-------------------------------------------------
# CppCheck analysis
#-------------------------------------------------

.PHONY: cppcheck

CPPCHECK_PARAMS  = -Isrc/osd
CPPCHECK_PARAMS += -Isrc/emu
CPPCHECK_PARAMS += -Isrc/lib
CPPCHECK_PARAMS += -Isrc/lib/util
CPPCHECK_PARAMS += -Isrc/mame
CPPCHECK_PARAMS += -Isrc/osd/modules/render
CPPCHECK_PARAMS += -Isrc/osd/windows
CPPCHECK_PARAMS += -I3rdparty
ifndef USE_SYSTEM_LIB_LUA
CPPCHECK_PARAMS += -I3rdparty/lua/src
endif
ifndef USE_SYSTEM_LIB_ZLIB
CPPCHECK_PARAMS += -I3rdparty/zlib
endif
CPPCHECK_PARAMS += -I3rdparty/bgfx/include
CPPCHECK_PARAMS += -I3rdparty/bx/include
CPPCHECK_PARAMS += -I$(BUILDDIR)/generated/emu
CPPCHECK_PARAMS += -I$(BUILDDIR)/generated/emu/layout
CPPCHECK_PARAMS += -I$(BUILDDIR)/generated/mame/layout
CPPCHECK_PARAMS += -DPTR64=1
CPPCHECK_PARAMS += -DMAME_DEBUG
CPPCHECK_PARAMS += -DMAME_PROFILER
CPPCHECK_PARAMS += -DCRLF=3
CPPCHECK_PARAMS += -DLSB_FIRST
ifndef USE_SYSTEM_LIB_FLAC
CPPCHECK_PARAMS += -DFLAC__NO_DLL
endif
CPPCHECK_PARAMS += -DNATIVE_DRC=drcbe_x64
CPPCHECK_PARAMS += -DLUA_COMPAT_APIINTCASTS
CPPCHECK_PARAMS += -DWIN32
CPPCHECK_PARAMS += -D__GNUC__
CPPCHECK_PARAMS += -D__x86_64__
ifndef VERBOSE
CPPCHECK_PARAMS += --quiet
endif

cppcheck:
	@echo Generate CppCheck analysis report
	cppcheck --enable=all src/ $(CPPCHECK_PARAMS) -j9

#-------------------------------------------------
# BGFX shaders
#
# to build all just use : make shaders
#
# to build specific chain use for example : make shaders CHAIN=eagle
# data for chain is taken from src/osd/modules/render/bgfx/shaders/chains/
# subfolder named in CHAIN
# NOTE: shaders can be only built on Windows for now
# due to restrictions of way how hlsl shaders are compiled
#-------------------------------------------------

.PHONY: shaders bgfx-tools

bgfx-tools:
	$(SILENT) $(MAKE) -C 3rdparty/bgfx -f makefile shaderc CC="$(CC)" CXX="$(CXX)" MINGW="$(MINGW)" SILENT="$(SILENT)"

shaders: bgfx-tools
	-$(call MKDIR,build/shaders/dx11)
	-$(call MKDIR,build/shaders/dx9)
	-$(call MKDIR,build/shaders/pssl)
	-$(call MKDIR,build/shaders/metal)
	-$(call MKDIR,build/shaders/essl)
	-$(call MKDIR,build/shaders/glsl)
	-$(call MKDIR,build/shaders/spirv)
	$(SILENT) $(MAKE) -C $(SRC)/osd/modules/render/bgfx/shaders rebuild CHAIN="$(CHAIN)" SILENT="$(SILENT)"

#-------------------------------------------------
# Translation
#-------------------------------------------------

.PHONY: translation

$(GENDIR)/mame.pot: FORCE
	$(SILENT) echo Generating mame.pot
	$(SILENT) find src/frontend "(" -name "*.cpp" -o -name "*.ipp" ")" -print0 | xargs -0 \
		xgettext -o $@ --from-code=UTF-8 --language=C++ -k_:1,1t -k_:1c,2,2t -kN_ -kN_p:1c,2
	$(SILENT) find src/devices "(" -name "*.cpp" -o -name "*.ipp" ")" -print0 | xargs -0 \
		xgettext -o $@ --from-code=UTF-8 --language=C++ -k_:1,1t -k_:1c,2,2t -kN_ -kN_p:1c,2 -j
	$(SILENT) find src/emu "(" -name "*.cpp" -o -name "*.ipp" ")" -print0 | xargs -0 \
		xgettext -o $@ --from-code=UTF-8 --language=C++ -k_:1,1t -k_:1c,2,2t -kN_ -kN_p:1c,2 -j
	$(SILENT) find src/lib "(" -name "*.cpp" -o -name "*.ipp" ")" -print0 | xargs -0 \
		xgettext -o $@ --from-code=UTF-8 --language=C++ -k_:1,1t -k_:1c,2,2t -kN_ -kN_p:1c,2 -j
	$(SILENT) find src/mame "(" -name "*.cpp" -o -name "*.ipp" ")" -print0 | xargs -0 \
		xgettext -o $@ --from-code=UTF-8 --language=C++ -k_:1,1t -k_:1c,2,2t -kN_ -kN_p:1c,2 -j
	$(SILENT) find src/osd "(" -name "*.cpp" -o -name "*.ipp" ")" -print0 | xargs -0 \
		xgettext -o $@ --from-code=UTF-8 --language=C++ -k_:1,1t -k_:1c,2,2t -kN_ -kN_p:1c,2 -j
	$(SILENT) find src/tools "(" -name "*.cpp" -o -name "*.ipp" ")" -print0 | xargs -0 \
		xgettext -o $@ --from-code=UTF-8 --language=C++ -k_:1,1t -k_:1c,2,2t -kN_ -kN_p:1c,2 -j
	$(SILENT) find plugins -name "*.lua" -print0 | xargs -0 \
		xgettext -o $@ --from-code=UTF-8 --language=Lua -k_:1 -k_p:1c,2 -kN_ -kN_p:1c,2 -j

translation: $(GENDIR)/mame.pot
	$(SILENT) find language -name "*.po" -print0 | xargs -0 -n 1 -I %% msgmerge -U -N %% $<
	$(SILENT) find language -name "*.po" -print0 | xargs -0 -n 1 -I %% msgattrib --clear-fuzzy --empty %% -o %%
