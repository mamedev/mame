#--------------------------------------------
# Linux 64bit to 32bit windowscross compile 
#--------------------------------------------

include default.mak

HOST_OS = linux64
TARGET_OS = win32
HOST_OSD = sdl
TARGET_OSD = windows

#---------------------------------------------------------------
PFX = /usr/bin/i586-mingw32msvc
#---------------------------------------------------------------

PACKAGES += MANUAL mingw32-binutils mingw32-runtime

BUILD_PARAM = CROSS_BUILD=1 \
		AR=@${PFX}-ar CC=@${PFX}-gcc LD=@${PFX}-gcc MD=-/bin/mkdir \
		"RM=@/bin/rm -f" RC=@${PFX}-windres BUILD_EXE= TARGETOS=win32 \
		OSD=$(TARGET_OSD)  
BUILD_TARGETS = tools emulator
BUILD_SUFFIX = w32
EXE_SUFFIX = .exe

HAS_PREBUILD = 1
PREBUILD_PARAM = PTR64=1 OSD=$(HOST_OSD) CROSS_BUILD_OSD=$(TARGET_OSD)
PREBUILD_TARGETS = buildtools


