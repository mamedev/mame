<p align="center">
<img src="https://repository-images.githubusercontent.com/23779242/961ad700-8d21-11ea-85d3-1d64eccc4531" width="1280"> 
</p>

[GENie](https://github.com/bkaradzic/genie#what-is-it) - Project generator tool
===============================================================================

[![Build and Upload Artifact](https://github.com/bkaradzic/GENie/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/bkaradzic/GENie/actions/workflows/build.yml)

What is it?
-----------

**GENie** (pronounced as Jenny) is project generator tool. It automagically
generates project from Lua script, making applying the same settings for
multiple projects easy.

Supported project generators:
 * GNU Makefile
 * [JSON Compilation Database][jcdb]
 * Ninja (experimental)
 * Visual Studio 2010, 2012, 2013, 2015, 2017, 2019, 2022
 * XCode

Download (stable)
-----------------

	version 1181 (commit 29e6832fdf3b106c0906d288c8ced6c0761b8985)

Linux:  
https://github.com/bkaradzic/bx/raw/master/tools/bin/linux/genie

OSX:  
https://github.com/bkaradzic/bx/raw/master/tools/bin/darwin/genie

Windows:  
https://github.com/bkaradzic/bx/raw/master/tools/bin/windows/genie.exe

Building (dev)
--------------

	$ git clone https://github.com/bkaradzic/genie
	$ cd genie
	$ make

Documentation
-------------

[Scripting Reference](https://github.com/bkaradzic/genie/blob/master/docs/scripting-reference.md#scripting-reference)  

Introduction to GENie - CppCon 2016  
<a href="http://www.youtube.com/watch?feature=player_embedded&v=_vArtdDTrTM" 
target="_blank"><img src="http://img.youtube.com/vi/_vArtdDTrTM/0.jpg" 
alt="Introduction to GENie - CppCon 2016" width="640" height="480" border="0" /></a>

History
-------

Initial version of **GENie** is [fork](https://github.com/bkaradzic/GENie/blob/c7e7da4aafe4210aa014a8ae8f6b01ce1d6802f0/README.md#why-fork)
of Premake 4.4 beta 5, and there is no intention to keep it compatible with it.

## Changelog (since fork)

 - Added vs2013 support.
 - Added hash UUID support. `os.uuid(<string>)` should produce consistent UUID.
 - Added search for default script. Default script name is changed to genie.lua
   (solution.lua and premake4.lua are also allowed), and it can be located in
   `scripts` directory.
 - Updated Lua from 5.1.4 to 5.3.0.
 - Disabled `SmallerTypeCheck` VS option when `ExtraWarnings` is set (need to
   move it into separate option).
 - New versioning scheme based on revision number from git.
 - Added `startproject "<project name>"` to set default project in VS.
 - Removed `NoMinimalRebuild` and added reversed logic to `EnableMinimalRebuild`.
 - Added `NoMultiProcessorCompilation` flag to disable multiprocessor
   compilation in MSVC.
 - Added ability to configure Visual Studio toolset from GENie script.
 - Added `UnsignedChar` flag to force char to be unsigned.
 - Removed vs2002, vs2003, vs2005, Solaris, and Haiku support.
 - Allow source files in the same project to have the same name. Added 
   `SingleOutputDir` flag to use single output directory (original behaviour).
 - Added WinRT support (Windows Phone 8.1, Windows Store, Universal Apps).
 - Added `removeflags`, `removelinks`.
 - Added vs2015 support.
 - Added `targetsubdir`.
 - Added support for solution folders `group`.
 - Added `options` section (and `ForceCpp` to enforce C++ compile even if
   extension is for C files)
 - Added `msgcompile`, `msgresource`, `msglinking` and `msgarchiving` as
   overrides for make messages.
 - Added `messageskip` list to disable some of compiler messages.
 - Added `buildoptions_c`, `buildoptions_cpp`, `buildoptions_objc`,
   `buildoptions_objcpp`, `buildoptions_asm`, `buildoptions_swift` for
   configuring language specific build options.
 - Split functionality of `excludes` in `removefiles` and `excludes`. With VS
   `excludes` will exclude files from build but files will be added to project
   file. `removefiles` removes files completely from project.
 - Added support for generating PS4/Orbis projects.
 - Fixed PCH race when using concurrent Makefile build.
 - Added Green Hills Software compiler support.
 - Added edit & continue support for 64-bit builds in vs2013 upwards.
 - Added `windowstargetplatformversion` to specify VS Windows target version.
 - Added `NoWinRT` flag to disable WinRT CX builds.
 - Added `NoBufferSecurityCheck` flag to disable security checks in VS.
 - Added `nopch` file list to exclude files from using PCH.
 - Added `EnableAVX` and `EnableAVX2` flags to enable enhanced instruction set.
 - Added FASTBuild (.bff) project generator.
 - Added Vala language support.
 - Added MASM support for Visual Studio projects.
 - Added `userincludedirs` for include header with angle brackets and quotes
   search path control.
 - Detect when generated project files are not changing, and skip writing over
   existing project files.
 - Added Ninja project generator.
 - Added ability to specify MSVC "Old Style" debug info format with
   `C7DebugInfo`.
 - Added some support for per-configuration `files` lists.
 - Removed `clean` action.
 - Added support for QtCreator via Qbs build tool.
 - Added .natvis file type support for Visual Studio.
 - Added Swift language support for make and ninja build generators.
 - Removed CodeBlocks and CodeLite support.
 - Added vs2017 support.
 - Removed vs2008 support.
 - Added `removeplatforms` that removes VS build target platforms.
 - Added `PedanticWarnings` flag.
 - Added `ObjcARC` flag to enable automatic reference counting for Objective-C(++).
 - Added `iostargetplatformversion`, `macostargetplatformversion`, and
   `tvostargetplatformversion` to specify XCode OS target version.
 - Removed the `xcode3`, and `xcode4` actions.
 - Added the `xcode8`, `xcode9`, `xcode10`, `xcode11` and `xcode14` actions.
 - Added `systemincludedirs` that are always searched after directories added
   using `includedirs`.
 - Added `NoRuntimeChecks` flag to disable Basic Runtime Checks in non-optimized
   Visual Studio builds.
 - Added support for Nintendo Switch projects.
 - Added flags for selecting C++ standard: `Cpp11`, `Cpp14`, `Cpp17`, `Cpp20`
   and `CppLatest`.
 - Added `xcodeprojectopts` and `xcodetargetopts`.
 - Added vs2019 support.
 - Added `UnitySupport` flag to enable Unity (Jumbo) builds in vs2019
 - Added the `jcdb` action for generating a [JSON compilation database][jcdb].
 - Added support for generating Switch/NX32 Switch/NX64 projects.
 - Removed FASTBuild.
 - Removed Qbs support.
 - Added vs2022 support.
 - Added xcode15 action with visionOS support.

build - GENie build system scripts
----------------------------------

build is a set of build system scripts and related tools, built around
GENie project generator tool.

https://milostosic.github.io/build/

Debugging GENie scripts
-----------------------

It is possible to debug build scripts using [ZeroBrane Studio][zbs]. You must
compile GENie in debug mode

    $ make config=debug

This ensures the core lua scripts are loaded from disk rather than compiled
into the GENie binary. Create a file named `debug.lua` as a sibling to your
main `genie.lua` script with the following content:

    local zb_path = <path to ZeroBraneStudio>
    local cpaths = {
        string.format("%s/bin/lib?.dylib;%s/bin/clibs53/?.dylib;", zb_path, zb_path),
        package.cpath,
    }
    package.cpath = table.concat(cpaths, ';')

    local paths = {
        string.format('%s/lualibs/?.lua;%s/lualibs/?/?.lua', zb_path, zb_path),
        string.format('%s/lualibs/?/init.lua;%s/lualibs/?/?/?.lua', zb_path, zb_path),
        string.format('%s/lualibs/?/?/init.lua', zb),
        package.path,
    }
    package.path = table.concat(paths, ';')

    require('mobdebug').start()

**NOTE:** update `zb_path` to refer to the root of your ZeroBrane Studio
install. For reference, you should find `lualibs` in you `zb_path` folder

To debug, make sure ZBS is listening for debug connections and add
`dofile("debug.lua")` to `genie.lua`

Who is using it?
----------------

https://github.com/bkaradzic/bgfx bgfx - Cross-platform, graphics API
agnostic, "Bring Your Own Engine/Framework" style rendering library.

https://github.com/Psybrus/Psybrus Psybrus Engine & Toolchain

https://github.com/dariomanesku/cmftstudio cmftStudio - cubemap filtering tool

https://github.com/mamedev/mame MAME - Multiple Arcade Machine Emulator

http://sol.gfxile.net/soloud SoLoud is an easy to use, free, 
portable c/c++ audio engine for games.

https://github.com/andr3wmac/Torque6 Torque 6 is an MIT licensed 3D engine
loosely based on Torque2D. Being neither Torque2D or Torque3D it is the 6th
derivative of the original Torque Engine.

https://milostosic.github.io/MTuner MTuner is a memory profiler and memory leak finder for Windows, PS4,
PS3.

Developer Crackshell used GENie for development of games
[Heroes of Hammerwatch](http://store.steampowered.com/app/677120/Heroes_of_Hammerwatch/), and
[Serious Sam's Bogus Detour](http://store.steampowered.com/app/272620/Serious_Sams_Bogus_Detour/).

[License](https://github.com/bkaradzic/genie/blob/master/LICENSE)
-----------------------------------------------------------------

	GENie
	Copyright (c) 2014-2018 Branimir Karadžić, Neil Richardson, Mike Popoloski,
	Drew Solomon, Ted de Munnik, Miodrag Milanović, Brett Vickers, Bill Freist,
	Terry Hendrix II, Ryan Juckett, Andrew Johnson, Johan Sköld,
	Alastair Murray, Patrick Munns, Jan-Eric Duden, Phil Stevens, Stuart Carnie,
	Nikolay Aleksiev, Jon Olson, Mike Fitzgerald, Anders Stenberg, Violets,
	Hugo Amnov, Christian Helmich.
	All rights reserved.

	https://github.com/bkaradzic/genie
	
	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:
	
	1. Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.
	
	2. Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the documentation
		and/or other materials provided with the distribution.
	
	3. Neither the name of the GENie nor the names of its contributors may be 
		used to endorse or promote products derived from this software without
		specific prior written permission.
	
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
	FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  [jcdb]: https://clang.llvm.org/docs/JSONCompilationDatabase.html
  [zbs]: https://studio.zerobrane.com
