#--------------------------------------------
# Stock 64bit sdl build 
#--------------------------------------------

include default.mak

HOST_OS = linux64
TARGET_OS = linux64
HOST_OSD = sdl
TARGET_OSD = sdl

MAME_TOOLS += testkeys

BUILD_PARAM = OSD=sdl PTR64=1
BUILD_TARGETS = 
BUILD_SUFFIX = 64

HAS_PREBUILD = 0
PREBUILD_PARAM =
PREBUILD_TARGETS = 

