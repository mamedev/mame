#--------------------------------------------
# Linux 64bit to 32bit sdl cross compile 
#--------------------------------------------

include configs/linux64-win32-windows.mak

TARGET_OSD = sdl

MAME_TOOLS += testkeys

BUILD_SUFFIX = s32

