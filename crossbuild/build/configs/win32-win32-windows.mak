#--------------------------------------------
# Stock 32bit native windows build 
#--------------------------------------------

include default.mak
include configs/win32.inc

HOST_OS = win32
TARGET_OS = win32
HOST_OSD = windows
TARGET_OSD = windows

#MAME_TOOLS += 

BUILD_PARAM = OSD=windows
BUILD_TARGETS = 
BUILD_SUFFIX = w32
EXE_SUFFIX = .exe

HAS_PREBUILD = 0
PREBUILD_PARAM =
PREBUILD_TARGETS = 


