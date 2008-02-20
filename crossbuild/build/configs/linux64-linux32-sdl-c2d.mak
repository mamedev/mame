#--------------------------------------------
# 32bit build optimized for C2D on 64bit host
#--------------------------------------------

include default.mak

HOST_OS = linux64
TARGET_OS = linux32
HOST_OSD = sdl
TARGET_OSD = sdl

PACKAGES += MANUAL ia32-libs ia32-libs-dev

BUILD_PARAM = OSD=sdl PTR64= "ARCHOPTS=-m32 -march=pentium3 -msse2" "LDFLAGS=-m32 -Wl,--warn-common,-L/usr/lib32,-L/lib32" 
BUILD_TARGETS = 
BUILD_SUFFIX = pm32

HAS_PREBUILD = 0
PREBUILD_PARAM =
PREBUILD_TARGETS = 

