
compilation linux64 :

make OSD="retro" verbose=1 RETRO=1 NOWERROR=1  OS="linux" TARGETOS="linux" CONFIG="libretro"  NO_USE_MIDI="1" PTR64=1 TARGET=mame  -j10

compilation crosswin64 :

export MINGW64=/usr
make OSD="retro" verbose=1 RETRO=1 NOWERROR=1  OS="linux" TARGETOS="windows" CONFIG="libretro"  NO_USE_MIDI="1" PTR64=1 TARGET=mame GCC="mingw64-gcc"  -j10

compilation win64 :

export MINGW64=/mingw64
make OSD="retro" verbose=1 RETRO=1 NOWERROR=1 TARGETOS="windows" CONFIG="libretro" NO_USE_MIDI="1" PTR64=1 TARGET=mame GCC="mingw64-gcc" -j4

to build only drivers atetris add :

SUBTARGET=atetris SOURCES=src/mame/drivers/atetris.cpp

to build without libco (experimental) add :

USELIBCO=0 
