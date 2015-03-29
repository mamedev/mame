###########################################################################
#
#   makefile
#
#   Core makefile for building MAME and derivatives
#
#   Copyright (c) Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################



###########################################################################
#################   BEGIN USER-CONFIGURABLE OPTIONS   #####################
###########################################################################

#
# Determine running OS
#

ifeq ($(OS),Windows_NT)
OS=windows
GENIEOS=windows
else
UNAME = $(shell uname -mps)
GENIEOS=linux
ifeq ($(firstword $(filter Linux,$(UNAME))),Linux)
OS = linux
endif
ifeq ($(firstword $(filter Solaris,$(UNAME))),Solaris)
OS = solaris
endif
ifeq ($(firstword $(filter FreeBSD,$(UNAME))),FreeBSD)
OS = freebsd
endif
ifeq ($(firstword $(filter GNU/kFreeBSD,$(UNAME))),GNU/kFreeBSD)
OS = freebsd
endif
ifeq ($(firstword $(filter NetBSD,$(UNAME))),NetBSD)
OS = netbsd
endif
ifeq ($(firstword $(filter OpenBSD,$(UNAME))),OpenBSD)
OS = openbsd
endif
ifeq ($(firstword $(filter Darwin,$(UNAME))),Darwin)
OS=macosx
GENIEOS=darwin
DARWIN_VERSION = $(shell sw_vers -productVersion)
endif
ifeq ($(firstword $(filter Haiku,$(UNAME))),Haiku)
OS = haiku
endif
ifndef OS
$(error Unable to detect OS from uname -a: $(UNAME))
endif
endif

#-------------------------------------------------
# specify core target: mame, mess, etc.
# specify subtarget: mame, mess, tiny, etc.
# build rules will be included from
# src/$(TARGET)/$(SUBTARGET).mak
#-------------------------------------------------

ifndef TARGET
TARGET = mame
endif

ifndef SUBTARGET
SUBTARGET = $(TARGET)
endif

CONFIG = release
ifdef DEBUG
CONFIG = debug
endif

ifndef verbose
  SILENT = @
endif

#-------------------------------------------------
# specify OS target, which further differentiates
# the underlying OS; supported values are:
# win32, unix, macosx, os2
#-------------------------------------------------

ifndef TARGETOS

ifeq ($(OS),windows)
TARGETOS = windows
WINDRES = windres
ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
ARCHITECTURE =_x64
endif
ifeq ($(PROCESSOR_ARCHITECTURE),x86)
ARCHITECTURE =_x64
ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
else
ARCHITECTURE =_x86
endif
endif
else
WINDRES  = x86_64-w64-mingw32-windres
UNAME    = $(shell uname -mps)
TARGETOS = $(OS)

ARCHITECTURE =_x86

ifeq ($(firstword $(filter x86_64,$(UNAME))),x86_64)
ARCHITECTURE =_x64
endif
ifeq ($(firstword $(filter amd64,$(UNAME))),amd64)
ARCHITECTURE =_x64
endif
ifeq ($(firstword $(filter ppc64,$(UNAME))),ppc64)
ARCHITECTURE =_x64
endif
endif

else
CROSS_BUILD = 1
endif # TARGET_OS

ifdef PTR64
ifeq ($(PTR64),1)
ARCHITECTURE =_x64
else
ARCHITECTURE =_x86
endif
endif


PYTHON = @python
CC = @gcc
LD = @g++

#-------------------------------------------------
# specify OSD layer: windows, sdl, etc.
# build rules will be included from
# src/osd/$(OSD)/$(OSD).mak
#-------------------------------------------------

ifndef OSD

OSD = osdmini

ifeq ($(TARGETOS),windows)
OSD = windows
endif

ifeq ($(TARGETOS),linux)
OSD = sdl
endif

ifeq ($(TARGETOS),macosx)
OSD = sdl
endif
endif


#-------------------------------------------------
# distribution may change things
#-------------------------------------------------

ifeq ($(DISTRO),)
DISTRO = generic
else
ifeq ($(DISTRO),debian-stable)
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

PARAMS+= --distro=$(DISTRO)

ifdef OVERRIDE_CC
PARAMS+= --CC='$(OVERRIDE_CC)'
ifndef CROSS_BUILD
CC = $(OVERRIDE_CC)
endif
endif
ifdef OVERRIDE_CXX
PARAMS+= --CXX='$(OVERRIDE_CXX)'
ifndef CROSS_BUILD
CXX = $(OVERRIDE_CXX)
endif
endif
ifdef OVERRIDE_LD
PARAMS+= --LD='$(OVERRIDE_LD)'
ifndef CROSS_BUILD
LD = $(OVERRIDE_LD)
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
ifndef PROFILER
PROFILER = 1
endif
endif

# allow gprof profiling as well, which overrides the internal PROFILER
# also enable symbols as it is useless without them
ifdef PROFILE
PROFILER =
SYMBOLS = 1
OPTIMIZE = 3
ifndef SYMLEVEL
SYMLEVEL = 1
endif
endif

# specify a default optimization level if none explicitly stated
ifndef OPTIMIZE
ifndef SYMBOLS
OPTIMIZE = 3
else
OPTIMIZE = 0
endif
endif

# set the symbols level
ifdef SYMBOLS
ifndef SYMLEVEL
SYMLEVEL = 2
endif
endif

ifdef TOOLS
PARAMS+= --with-tools
endif

ifdef SYMBOLS
PARAMS+= --SYMBOLS=$(SYMBOLS)
endif

ifdef SYMLEVEL
PARAMS+= --SYMLEVEL=$(SYMLEVEL)
endif

ifdef PROFILER
PARAMS+= --PROFILER=$(PROFILER)
endif

ifdef PROFILE
PARAMS+= --PROFILE=$(PROFILE)
endif

ifdef OPTIMIZE
PARAMS+= --OPTIMIZE=$(OPTIMIZE)
endif

ifdef ARCHOPTS
PARAMS+= --ARCHOPTS='$(ARCHOPTS)'
endif

ifdef MAP
PARAMS+= --MAP=$(MAP)
endif

ifdef USE_BGFX
PARAMS+= --USE_BGFX=$(USE_BGFX)
endif

ifdef NOWERROR
PARAMS+= --NOWERROR=$(NOWERROR)
endif

ifdef TARGET
PARAMS+= --target=$(TARGET)
endif

ifdef SUBTARGET
PARAMS+= --subtarget=$(SUBTARGET)
endif

ifdef OSD
PARAMS+= --osd=$(OSD)
endif

ifdef TARGETOS
PARAMS+= --targetos=$(TARGETOS)
endif

# extension for executables
EXE =

ifeq ($(OS),windows)
EXE = .exe
endif
ifeq ($(OS),os2)
EXE = .exe
endif

SHELLTYPE := msdos
ifeq (,$(ComSpec)$(COMSPEC))
  SHELLTYPE := posix
endif
ifeq (/bin,$(findstring /bin,$(SHELL)))
  SHELLTYPE := posix
endif

ifeq (posix,$(SHELLTYPE))
  MKDIR = $(SILENT) mkdir -p "$(1)"
  COPY  = $(SILENT) cp -fR "$(1)" "$(2)"
else
  MKDIR = $(SILENT) mkdir "$(subst /,\\,$(1))" 2> nul || exit 0
  COPY  = $(SILENT) copy /Y "$(subst /,\\,$(1))" "$(subst /,\\,$(2))"
endif

GENDIR = build/generated

# all sources are under the src/ directory
SRC = src

# all 3rd party sources are under the 3rdparty/ directory
3RDPARTY = 3rdparty

ifeq ($(OS),windows)
GCC_VERSION:=$(shell gcc -dumpversion 2> NUL)
CLANG_VERSION:=$(shell %CLANG%\bin\clang --version 2> NUL| head -n 1 | sed "s/[^0-9,.]//g")
PYTHON_AVAILABLE:=$(shell python --version > NUL 2>&1 && echo python)
CHECK_CLANG:=
else
GCC_VERSION:=$(shell $(subst @,,$(CC)) -dumpversion 2> /dev/null)
CLANG_VERSION:=$(shell clang --version  2> /dev/null | grep 'LLVM [0-9]\.[0-9]' -o | grep '[0-9]\.[0-9]' -o | head -n 1)
PYTHON_AVAILABLE:=$(shell python --version > /dev/null 2>&1 && echo python)
CHECK_CLANG:=$(shell gcc --version  2> /dev/null | grep 'clang' | head -n 1)
endif

ifeq ($(TARGETOS),macosx)
ifneq (,$(findstring 3.,$(CLANG_VERSION)))
ifeq ($(ARCHITECTURE),_x64)
ARCHITECTURE=_x64_clang
else
ARCHITECTURE=_x86_clang
endif
endif
endif

ifneq ($(PYTHON_AVAILABLE),python)
$(error Python is not available in path)
endif

GENIE=3rdparty/genie/bin/$(GENIEOS)/genie

SILENT?=@

ifeq ($(TARGET),$(SUBTARGET))
SUBDIR = $(OSD)/$(TARGET)
else
SUBDIR = $(OSD)/$(TARGET)$(SUBTARGET)
endif

all: $(GENIE) $(TARGETOS)$(ARCHITECTURE)

windows_x64: generate
ifndef MINGW64
	$(error MINGW64 is not set)
endif
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=mingw64-gcc --gcc_version=$(GCC_VERSION) gmake 
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-mingw64-gcc config=$(CONFIG)64 WINDRES=$(WINDRES)

windows: windows_x86

windows_x86: generate
ifndef MINGW32
	$(error MINGW32 is not set)
endif
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=mingw32-gcc --gcc_version=$(GCC_VERSION) gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-mingw32-gcc config=$(CONFIG)32 WINDRES=$(WINDRES)

windows_x64_clang: generate
ifndef CLANG
	$(error CLANG is not set)
endif
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=mingw-clang --gcc_version=$(CLANG_VERSION) gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-mingw-clang config=$(CONFIG)64 WINDRES=$(WINDRES)
	
windows_x86_clang: generate
ifndef CLANG
	$(error CLANG is not set)
endif
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=mingw-clang --gcc_version=$(CLANG_VERSION) gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-mingw-clang config=$(CONFIG)32 WINDRES=$(WINDRES)

vs2010: generate
	$(SILENT) $(GENIE) $(PARAMS) vs2010

vs2012: generate
	$(SILENT) $(GENIE) $(PARAMS) vs2012

vs2012_intel: generate
	$(SILENT) $(GENIE) $(PARAMS) --vs=intel-15 vs2012

vs2012_xp: generate
	$(SILENT) $(GENIE) $(PARAMS) --vs=vs2012-xp vs2012

vs2013: generate
	$(SILENT) $(GENIE) $(PARAMS) vs2013

vs2013_intel: generate
	$(SILENT) $(GENIE) $(PARAMS) --vs=intel-15 vs2013

vs2013_xp: generate
	$(SILENT) $(GENIE) $(PARAMS) --vs=vs2013-xp vs2013

vs2015: generate
	$(SILENT) $(GENIE) $(PARAMS) vs2015

android-arm: generate
ifndef ANDROID_NDK_ARM
	$(error ANDROID_NDK_ARM is not set)
endif
ifndef ANDROID_NDK_ROOT
	$(error ANDROID_NDK_ROOT is not set)
endif
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=android-arm --gcc_version=4.8 gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-android-arm config=$(CONFIG)

android-mips: generate
ifndef ANDROID_NDK_MIPS
	$(error ANDROID_NDK_MIPS is not set)
endif
ifndef ANDROID_NDK_ROOT
	$(error ANDROID_NDK_ROOT is not set)
endif
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=android-mips --gcc_version=4.8 gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-android-mips config=$(CONFIG)

android-x86: generate
ifndef ANDROID_NDK_X86
	$(error ANDROID_NDK_X86 is not set)
endif
ifndef ANDROID_NDK_ROOT
	$(error ANDROID_NDK_ROOT is not set)
endif
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=android-x86 --gcc_version=4.8 gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-android-x86 config=$(CONFIG)

asmjs: generate
ifndef EMSCRIPTEN
	$(error EMSCRIPTEN is not set)
endif
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=asmjs --gcc_version=4.8 gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-asmjs config=$(CONFIG)


nacl: nacl_x86

nacl_x64: generate
ifndef NACL_SDK_ROOT
	$(error NACL_SDK_ROOT is not set)
endif
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=nacl --gcc_version=4.8 gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-nacl config=$(CONFIG)64

nacl_x86: generate
ifndef NACL_SDK_ROOT
	$(error NACL_SDK_ROOT is not set)
endif
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=nacl --gcc_version=4.8 gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-nacl config=$(CONFIG)32

nacl-arm: generate
ifndef NACL_SDK_ROOT
	$(error NACL_SDK_ROOT is not set)
endif
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=nacl-arm --gcc_version=4.8 gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-nacl-arm config=$(CONFIG)

pnacl: generate
ifndef NACL_SDK_ROOT
	$(error NACL_SDK_ROOT is not set)
endif
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=pnacl --gcc_version=4.8 gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-pnacl config=$(CONFIG)

linux_x64: generate
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=linux-gcc --gcc_version=$(GCC_VERSION) gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-linux config=$(CONFIG)64

linux: linux_x86

linux_x86: generate
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=linux-gcc --gcc_version=$(GCC_VERSION) gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-linux config=$(CONFIG)32

linux_x64_clang: generate
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=linux-clang --gcc_version=$(CLANG_VERSION) gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-linux-clang config=$(CONFIG)64

linux_x86_clang: generate
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=linux-clang --gcc_version=$(CLANG_VERSION) gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-linux-clang config=$(CONFIG)32

macosx_x64: generate
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=osx --gcc_version=$(GCC_VERSION) gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-osx config=$(CONFIG)64

macosx: macosx_x86

macosx_x86: generate
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=osx --os_version=$(DARWIN_VERSION) --gcc_version=$(GCC_VERSION) gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-osx config=$(CONFIG)32

macosx_x64_clang: generate
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=osx-clang --gcc_version=$(CLANG_VERSION) gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-osx-clang config=$(CONFIG)64

macosx_x86_clang: generate
ifndef COMPILE
	$(SILENT) $(GENIE) $(PARAMS) --gcc=osx-clang --os_version=$(DARWIN_VERSION) --gcc_version=$(CLANG_VERSION) gmake
endif
	$(SILENT) $(MAKE) --no-print-directory -R -C build/projects/$(SUBDIR)/gmake-osx-clang config=$(CONFIG)32

$(GENIE):
	$(SILENT) $(MAKE) --no-print-directory -R -C 3rdparty/genie/build/gmake.$(GENIEOS) -f genie.make

clean:
	@echo Cleaning...
	-@rm -rf build
	$(SILENT) $(MAKE) --no-print-directory -R -C 3rdparty/genie/build/gmake.$(GENIEOS) -f genie.make clean

GEN_FOLDERS :=  \
	$(GENDIR) \
	$(GENDIR)/$(TARGET)/$(SUBTARGET) \
	$(GENDIR)/emu/layout/ \
	$(GENDIR)/mame/layout/ \
	$(GENDIR)/mess/layout/ \
	$(GENDIR)/mess/drivers/ \
	$(GENDIR)/ldplayer/layout/ \
	$(GENDIR)/emu/cpu/arcompact/ \
	$(GENDIR)/emu/cpu/h8/ \
	$(GENDIR)/emu/cpu/mcs96/ \
	$(GENDIR)/emu/cpu/m6502/ \
	$(GENDIR)/emu/cpu/m6809/ \
	$(GENDIR)/emu/cpu/m68000/ \
	$(GENDIR)/emu/cpu/tms57002/ \
	$(GENDIR)/osd/modules/debugger/qt/ \
	$(GENDIR)/resource/

LAYOUTS=$(wildcard $(SRC)/emu/layout/*.lay) $(wildcard $(SRC)/mame/layout/*.lay) $(wildcard $(SRC)/mess/layout/*.lay) $(wildcard $(SRC)/ldplayer/layout/*.lay)

ifeq ($(TARGETOS),macosx)
MOC_FILES=
else
MOC_FILES=$(wildcard $(SRC)/osd/modules/debugger/qt/*.h)

ifeq ($(OS),windows)
MOC = moc
ifneq ($(OSD),sdl)
MOC_FILES=
endif
else
MOCTST = $(shell which moc-qt4 2>/dev/null)
ifeq '$(MOCTST)' ''
MOCTST = $(shell which moc 2>/dev/null)
ifeq '$(MOCTST)' ''
$(error Qt's Meta Object Compiler (moc) wasn't found!)
else
MOC = $(MOCTST)
endif
else
MOC = $(MOCTST)
endif
endif

endif

ifneq (,$(wildcard src/osd/$(OSD)/$(OSD).mak))
include src/osd/$(OSD)/$(OSD).mak
endif

$(GEN_FOLDERS):
	-$(call MKDIR,$@)

generate: \
		$(GENIE) \
		$(GEN_FOLDERS) \
		$(patsubst $(SRC)/%.lay,$(GENDIR)/%.lh,$(LAYOUTS)) \
		$(patsubst $(SRC)/%.h,$(GENDIR)/%.moc.c,$(MOC_FILES)) \
		$(GENDIR)/emu/uismall.fh \
		$(GENDIR)/resource/$(TARGET)vers.rc \
		$(GENDIR)/resource/$(TARGET)-Info.plist \
		$(GENDIR)/$(TARGET)/$(SUBTARGET)/drivlist.c \
		$(GENDIR)/mess/drivers/ymmu100.inc \
		$(GENDIR)/emu/cpu/arcompact/arcompact.inc \
		$(GENDIR)/emu/cpu/h8/h8.inc $(GENDIR)/emu/cpu/h8/h8h.inc $(GENDIR)/emu/cpu/h8/h8s2000.inc $(GENDIR)/emu/cpu/h8/h8s2600.inc \
		$(GENDIR)/emu/cpu/mcs96/mcs96.inc $(GENDIR)/emu/cpu/mcs96/i8x9x.inc $(GENDIR)/emu/cpu/mcs96/i8xc196.inc \
		$(GENDIR)/emu/cpu/m6502/deco16.inc $(GENDIR)/emu/cpu/m6502/m4510.inc $(GENDIR)/emu/cpu/m6502/m6502.inc $(GENDIR)/emu/cpu/m6502/m65c02.inc $(GENDIR)/emu/cpu/m6502/m65ce02.inc $(GENDIR)/emu/cpu/m6502/m6509.inc $(GENDIR)/emu/cpu/m6502/m6510.inc $(GENDIR)/emu/cpu/m6502/n2a03.inc $(GENDIR)/emu/cpu/m6502/r65c02.inc $(GENDIR)/emu/cpu/m6502/m740.inc \
		$(GENDIR)/emu/cpu/m6809/m6809.inc $(GENDIR)/emu/cpu/m6809/hd6309.inc $(GENDIR)/emu/cpu/m6809/konami.inc \
		$(GENDIR)/emu/cpu/tms57002/tms57002.inc \
		$(GENDIR)/m68kmake$(EXE) $(GENDIR)/emu/cpu/m68000/m68kops.c

$(GENDIR)/%.lh: $(SRC)/%.lay $(SRC)/build/file2str.py
	@echo Converting $<...
	$(PYTHON) $(SRC)/build/file2str.py $< $@ layout_$(basename $(notdir $<))

$(GENDIR)/%.fh: $(SRC)/%.png $(SRC)/build/png2bdc.py $(SRC)/build/file2str.py
	@echo Converting $<...
	$(PYTHON) $(SRC)/build/png2bdc.py $< $(GENDIR)/temp.bdc
	$(PYTHON) $(SRC)/build/file2str.py $(GENDIR)/temp.bdc $@ font_$(basename $(notdir $<)) UINT8

$(GENDIR)/resource/$(TARGET)vers.rc: $(SRC)/build/verinfo.py $(SRC)/version.c
	@echo Emitting $@...
	$(PYTHON) $(SRC)/build/verinfo.py -r -b $(TARGET) $(SRC)/version.c > $@

$(GENDIR)/resource/$(TARGET)-Info.plist: $(SRC)/build/verinfo.py $(SRC)/version.c
	@echo Emitting $@...
	$(PYTHON) $(SRC)/build/verinfo.py -p -b $(TARGET) $(SRC)/version.c > $@

$(GENDIR)/$(TARGET)/$(SUBTARGET)/drivlist.c: $(SRC)/$(TARGET)/$(SUBTARGET).lst $(SRC)/build/makelist.py
	@echo Building driver list $<...
	$(PYTHON) $(SRC)/build/makelist.py $< >$@

# rule to generate the C files
$(GENDIR)/emu/cpu/arcompact/arcompact.inc: $(SRC)/emu/cpu/arcompact/arcompact_make.py
	@echo Generating arcompact source .inc files...
	$(PYTHON) $(SRC)/emu/cpu/arcompact/arcompact_make.py $@

$(GENDIR)/emu/cpu/h8/h8.inc: $(SRC)/emu/cpu/h8/h8make.py $(SRC)/emu/cpu/h8/h8.lst
	@echo Generating H8-300 source file...
	$(PYTHON) $(SRC)/emu/cpu/h8/h8make.py $(SRC)/emu/cpu/h8/h8.lst o $@

$(GENDIR)/emu/cpu/h8/h8h.inc: $(SRC)/emu/cpu/h8/h8make.py $(SRC)/emu/cpu/h8/h8.lst
	@echo Generating H8-300H source file...
	$(PYTHON) $(SRC)/emu/cpu/h8/h8make.py $(SRC)/emu/cpu/h8/h8.lst h $@

$(GENDIR)/emu/cpu/h8/h8s2000.inc: $(SRC)/emu/cpu/h8/h8make.py $(SRC)/emu/cpu/h8/h8.lst
	@echo Generating H8S/2000 source file...
	$(PYTHON) $(SRC)/emu/cpu/h8/h8make.py $(SRC)/emu/cpu/h8/h8.lst s20 $@

$(GENDIR)/emu/cpu/h8/h8s2600.inc: $(SRC)/emu/cpu/h8/h8make.py $(SRC)/emu/cpu/h8/h8.lst
	@echo Generating H8S/2600 source file...
	$(PYTHON) $(SRC)/emu/cpu/h8/h8make.py $(SRC)/emu/cpu/h8/h8.lst s26 $@

$(GENDIR)/emu/cpu/mcs96/mcs96.inc:   $(SRC)/emu/cpu/mcs96/mcs96make.py $(SRC)/emu/cpu/mcs96/mcs96ops.lst
	@echo Generating mcs96 source file...
	$(PYTHON) $(SRC)/emu/cpu/mcs96/mcs96make.py mcs96 $(SRC)/emu/cpu/mcs96/mcs96ops.lst $@

$(GENDIR)/emu/cpu/mcs96/i8x9x.inc:   $(SRC)/emu/cpu/mcs96/mcs96make.py $(SRC)/emu/cpu/mcs96/mcs96ops.lst
	@echo Generating i8x9x source file...
	$(PYTHON) $(SRC)/emu/cpu/mcs96/mcs96make.py i8x9x $(SRC)/emu/cpu/mcs96/mcs96ops.lst $@

$(GENDIR)/emu/cpu/mcs96/i8xc196.inc: $(SRC)/emu/cpu/mcs96/mcs96make.py $(SRC)/emu/cpu/mcs96/mcs96ops.lst
	@echo Generating i8xc196 source file...
	$(PYTHON) $(SRC)/emu/cpu/mcs96/mcs96make.py i8xc196 $(SRC)/emu/cpu/mcs96/mcs96ops.lst $@

$(GENDIR)/emu/cpu/m6502/deco16.inc: $(SRC)/emu/cpu/m6502/m6502make.py $(SRC)/emu/cpu/m6502/odeco16.lst $(SRC)/emu/cpu/m6502/ddeco16.lst
	@echo Generating deco16 source file...
	$(PYTHON) $(SRC)/emu/cpu/m6502/m6502make.py deco16_device $(SRC)/emu/cpu/m6502/odeco16.lst $(SRC)/emu/cpu/m6502/ddeco16.lst $@

$(GENDIR)/emu/cpu/m6502/m4510.inc: $(SRC)/emu/cpu/m6502/m6502make.py $(SRC)/emu/cpu/m6502/om4510.lst $(SRC)/emu/cpu/m6502/dm4510.lst
	@echo Generating m4510 source file...
	$(PYTHON) $(SRC)/emu/cpu/m6502/m6502make.py m4510_device $(SRC)/emu/cpu/m6502/om4510.lst $(SRC)/emu/cpu/m6502/dm4510.lst $@

$(GENDIR)/emu/cpu/m6502/m6502.inc: $(SRC)/emu/cpu/m6502/m6502make.py $(SRC)/emu/cpu/m6502/om6502.lst $(SRC)/emu/cpu/m6502/dm6502.lst
	@echo Generating m6502 source file...
	$(PYTHON) $(SRC)/emu/cpu/m6502/m6502make.py m6502_device $(SRC)/emu/cpu/m6502/om6502.lst $(SRC)/emu/cpu/m6502/dm6502.lst $@

$(GENDIR)/emu/cpu/m6502/m65c02.inc: $(SRC)/emu/cpu/m6502/m6502make.py $(SRC)/emu/cpu/m6502/om65c02.lst $(SRC)/emu/cpu/m6502/dm65c02.lst
	@echo Generating m65c02 source file...
	$(PYTHON) $(SRC)/emu/cpu/m6502/m6502make.py m65c02_device $(SRC)/emu/cpu/m6502/om65c02.lst $(SRC)/emu/cpu/m6502/dm65c02.lst $@

$(GENDIR)/emu/cpu/m6502/m65ce02.inc: $(SRC)/emu/cpu/m6502/m6502make.py $(SRC)/emu/cpu/m6502/om65ce02.lst $(SRC)/emu/cpu/m6502/dm65ce02.lst
	@echo Generating m65ce02 source file...
	$(PYTHON) $(SRC)/emu/cpu/m6502/m6502make.py m65ce02_device $(SRC)/emu/cpu/m6502/om65ce02.lst $(SRC)/emu/cpu/m6502/dm65ce02.lst $@

$(GENDIR)/emu/cpu/m6502/m6509.inc: $(SRC)/emu/cpu/m6502/m6502make.py $(SRC)/emu/cpu/m6502/om6509.lst $(SRC)/emu/cpu/m6502/dm6509.lst
	@echo Generating m6509 source file...
	$(PYTHON) $(SRC)/emu/cpu/m6502/m6502make.py m6509_device $(SRC)/emu/cpu/m6502/om6509.lst $(SRC)/emu/cpu/m6502/dm6509.lst $@

$(GENDIR)/emu/cpu/m6502/m6510.inc: $(SRC)/emu/cpu/m6502/m6502make.py $(SRC)/emu/cpu/m6502/om6510.lst $(SRC)/emu/cpu/m6502/dm6510.lst
	@echo Generating m6510 source file...
	$(PYTHON) $(SRC)/emu/cpu/m6502/m6502make.py m6510_device $(SRC)/emu/cpu/m6502/om6510.lst $(SRC)/emu/cpu/m6502/dm6510.lst $@

$(GENDIR)/emu/cpu/m6502/n2a03.inc: $(SRC)/emu/cpu/m6502/m6502make.py $(SRC)/emu/cpu/m6502/on2a03.lst $(SRC)/emu/cpu/m6502/dn2a03.lst
	@echo Generating n2a03 source file...
	$(PYTHON) $(SRC)/emu/cpu/m6502/m6502make.py n2a03_device $(SRC)/emu/cpu/m6502/on2a03.lst $(SRC)/emu/cpu/m6502/dn2a03.lst $@

$(GENDIR)/emu/cpu/m6502/r65c02.inc: $(SRC)/emu/cpu/m6502/m6502make.py $(SRC)/emu/cpu/m6502/dr65c02.lst
	@echo Generating r65c02 source file...
	$(PYTHON) $(SRC)/emu/cpu/m6502/m6502make.py r65c02_device - $(SRC)/emu/cpu/m6502/dr65c02.lst $@

$(GENDIR)/emu/cpu/m6502/m740.inc: $(SRC)/emu/cpu/m6502/m6502make.py $(SRC)/emu/cpu/m6502/om740.lst $(SRC)/emu/cpu/m6502/dm740.lst
	@echo Generating m740 source file...
	$(PYTHON) $(SRC)/emu/cpu/m6502/m6502make.py m740_device $(SRC)/emu/cpu/m6502/om740.lst $(SRC)/emu/cpu/m6502/dm740.lst $@

$(GENDIR)/emu/cpu/m6809/m6809.inc:  $(SRC)/emu/cpu/m6809/m6809make.py $(SRC)/emu/cpu/m6809/m6809.ops $(SRC)/emu/cpu/m6809/base6x09.ops
	@echo Generating m6809 source file...
	$(PYTHON) $(SRC)/emu/cpu/m6809/m6809make.py $(SRC)/emu/cpu/m6809/m6809.ops > $@

$(GENDIR)/emu/cpu/m6809/hd6309.inc: $(SRC)/emu/cpu/m6809/m6809make.py $(SRC)/emu/cpu/m6809/hd6309.ops $(SRC)/emu/cpu/m6809/base6x09.ops
	@echo Generating hd6309 source file...
	$(PYTHON) $(SRC)/emu/cpu/m6809/m6809make.py $(SRC)/emu/cpu/m6809/hd6309.ops > $@

$(GENDIR)/emu/cpu/m6809/konami.inc: $(SRC)/emu/cpu/m6809/m6809make.py $(SRC)/emu/cpu/m6809/konami.ops $(SRC)/emu/cpu/m6809/base6x09.ops
	@echo Generating konami source file...
	$(PYTHON) $(SRC)/emu/cpu/m6809/m6809make.py $(SRC)/emu/cpu/m6809/konami.ops > $@

$(GENDIR)/emu/cpu/tms57002/tms57002.inc: $(SRC)/emu/cpu/tms57002/tmsmake.py $(SRC)/emu/cpu/tms57002/tmsinstr.lst
	@echo Generating TMS57002 source file...
	$(PYTHON) $(SRC)/emu/cpu/tms57002/tmsmake.py $(SRC)/emu/cpu/tms57002/tmsinstr.lst $@

$(GENDIR)/m68kmake.o: src/emu/cpu/m68000/m68kmake.c
	@echo $(notdir $<)
	$(SILENT) $(CC) -x c++ -std=gnu++98 -o "$@" -c "$<"

$(GENDIR)/m68kmake$(EXE) : $(GENDIR)/m68kmake.o
	@echo Linking $@...
	$(LD) -lstdc++ $^ -o $@

$(GENDIR)/emu/cpu/m68000/m68kops.c: $(GENDIR)/m68kmake$(EXE) $(SRC)/emu/cpu/m68000/m68k_in.c
	@echo Generating M68K source files...
	$(SILENT) $(GENDIR)/m68kmake $(GENDIR)/emu/cpu/m68000 $(SRC)/emu/cpu/m68000/m68k_in.c

$(GENDIR)/mess/drivers/ymmu100.inc: $(SRC)/mess/drivers/ymmu100.ppm $(SRC)/build/file2str.py
	@echo Converting $<...
	@$(PYTHON) $(SRC)/build/file2str.py $(SRC)/mess/drivers/ymmu100.ppm $@ ymmu100_bkg UINT8

$(GENDIR)/%.moc.c: $(SRC)/%.h
	$(SILENT) $(MOC) $(MOCINCPATH) $< -o $@
	