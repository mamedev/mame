libretro
========

Compilation Instructions
------------------------


### Linux x86-64

```sh
make OSD="retro" verbose=1 RETRO=1 NOWERROR=1  OS="linux" TARGETOS="linux" CONFIG="libretro"  NO_USE_MIDI="1" PTR64=1 TARGET=mame  -j10
```


### crosswin64

```sh
export MINGW64=/usr
make OSD="retro" verbose=1 RETRO=1 NOWERROR=1  OS="linux" TARGETOS="windows" CONFIG="libretro"  NO_USE_MIDI="1" PTR64=1 TARGET=mame GCC="mingw64-gcc"  -j10
```


### Win64

```sh
export MINGW64=/mingw64
make OSD="retro" verbose=1 RETRO=1 NOWERROR=1 TARGETOS="windows" CONFIG="libretro" NO_USE_MIDI="1" PTR64=1 TARGET=mame GCC="mingw64-gcc" -j4
```


### macOS

```sh
make macosx_x64_clang OSD="retro" DONT_USE_NETWORK=1 verbose=1 RETRO=1 NOWERROR=1 TARGETOS="macosx" CONFIG="libretro" NO_USE_MIDI="1" PTR64=1 TARGET=mame SUBTARGET=arcade -j8
```


### android-arm32

export `ANDROID_NDK_ARM`, `ANDROID_NDK_ROOT` and `ANDROID_NDK_LLVM`
to match your NDK root and toolchain.


```sh
export ANDROID_NDK_LLVM="/opt/ndk/toolchains/llvm-3.5/prebuilt/linux-x86_64"
export ANDROID_NDK_ARM="/opt/ndk/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64"
export ANDROID_NDK_ROOT="/opt/ndk"

make OSD="retro" verbose=1 RETRO=1 NOWERROR=1  NOASM="1" gcc="android-arm" OS="linux" TARGETOS="android-arm" CONFIG=libretro NO_USE_MIDI="1" TARGET=mame -j4
```


### emscripten

first setup your emsdk (source emsdk_env.sh) 

for a tiny build :

```sh
emmake make  OSD="retro" verbose=1 RETRO=1 NOWERROR=1  NOASM="1"  CONFIG=libretro NO_USE_MIDI="1" SUBTARGET=tiny 
```


for only one driver build :

```sh
emmake make  OSD="retro" verbose=1 RETRO=1 NOWERROR=1  NOASM="1"  CONFIG=libretro NO_USE_MIDI="1" SUBTARGET=pacman SOURCES=src/mame/drivers/pacman.cpp
```

to build only drivers atetris add :

```
SUBTARGET=atetris SOURCES=src/mame/drivers/atetris.cpp
```
