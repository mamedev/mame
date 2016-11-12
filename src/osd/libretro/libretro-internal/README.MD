WIP don't use , use libretro/mame git instead.

compilation linux64 :

make OSD="retro" verbose=1 RETRO=1 NOWERROR=1  OS="linux" TARGETOS="linux" CONFIG="libretro"  NO_USE_MIDI="1" PTR64=1 TARGET=mame  -j10

compilation crosswin64 :
make OSD="retro" verbose=1 RETRO=1 NOWERROR=1  OS="linux" TARGETOS="windows" CONFIG="libretro"  NO_USE_MIDI="1" PTR64=1 TARGET=mame GCC="mingw64-gcc"  -j10

drivers atetris:

SUBTARGET=atetris SOURCES=src/mame/drivers/atetris.cpp
