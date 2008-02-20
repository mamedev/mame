
include default.mak
include configs/win32.inc

HOST_OS = win32
TARGET_OS = vc64
HOST_OSD = winui
TARGET_OSD = winui

#MAME_TOOLS += 
MAKE_PROCESSES = 

BUILD_PARAM = CROSS_BUILD=1 OSD=winui MSVC_BUILD=1 PTR64=1 TARGETOS=win32 VCONV=$(BUILDOUT)/vconf.exe
BUILD_TARGETS = tools emulator
BUILD_SUFFIX = uiv64
EXE_SUFFIX = .exe

HAS_PREBUILD = 1
PREBUILD_PARAM = OSD=winui CROSS_BUILD_OSD=winui
PREBUILD_TARGETS = buildtools
PREBUILD_LOCAL = buildvconv

buildvconv:
	@$(MAKE) -C $(MAMESVN) OBJ=$(DESTOBJ) maketree
	gcc -mconsole $(MAMESVN)/src/osd/windows/vconv.c -lversion -o $(BUILDOUT)/vconf.exe  

help:
	$(PRINTHELP) 64bit Windows build on 32bit platform

