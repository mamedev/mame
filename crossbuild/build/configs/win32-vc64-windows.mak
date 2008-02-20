
include default.mak
include configs/win32.inc

HOST_OS = win32
TARGET_OS = win64
HOST_OSD = windows
TARGET_OSD = windows

#MAME_TOOLS += 
MAKE_PROCESSES = 

BUILD_PARAM = CROSS_BUILD=1 OSD=windows MSVC_BUILD=1 PTR64=1 TARGETOS=win32 VCONV=$(BUILDOUT)/vconf.exe
BUILD_TARGETS = tools emulator
BUILD_SUFFIX = v64
EXE_SUFFIX = .exe

HAS_PREBUILD = 1
PREBUILD_PARAM = OSD=windows CROSS_BUILD_OSD=windows
PREBUILD_TARGETS = buildtools
PREBUILD_LOCAL = buildvconv

buildvconv:
	@$(MAKE) -C $(MAMESVN) maketree
	gcc -mconsole $(MAMESVN)/src/osd/windows/vconv.c -lversion -o $(BUILDOUT)/vconf.exe  

help:
	$(PRINTHELP) 64bit Windows build on 32bit platform

