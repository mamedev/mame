#--------------------------------------------
# Stock 32bit native sdl build 
#--------------------------------------------

include configs/win32-win32-windows.mak

HOST_OSD = sdl
TARGET_OSD = sdl

MAME_TOOLS += testkeys

BUILD_PARAM = OSD=sdl
BUILD_SUFFIX = s32

