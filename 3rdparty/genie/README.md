[GENie](https://github.com/bkaradzic/genie#what-is-it) - Project generator tool
===============================================================================

What is it?
-----------

**GENie** (pronounced as Jenny) is project generator tool. It automagically
generates project from Lua script, making applying the same settings for
multiple projects easy.

Supported project generators:
 * Visual Studio 2008, 2010, 2012, 2013, 2015
 * GNU Makefile
 * XCode

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

Download (stable)
-----------------

[![Build Status](https://travis-ci.org/bkaradzic/GENie.svg?branch=master)](https://travis-ci.org/bkaradzic/GENie)

	version 415 (commit 53635be7264271a6d6c95b059c420251b7eb3280)

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

History
-------

Initial version of **GENie** is fork of Premake 4.4 beta 5, and there is no
intention to keep it compatible with it.

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
 - Added `buildoptions_c`, `buildoptions_cpp`, `buildoptions_objc` for
   configuring language specific build options.
 - Split functionality of `excludes` in `removefiles` and `excludes`. With VS
   `excludes` will exclude files from build but files will be added to project
   file. `removefiles` removes files completely from project.
 - Added support for generating PS4/Orbis projects.
 - Fixed PCH race when using concurrent Makefile build.
 - Added Green Hills Software compiler support.
 - Added edit & continue support for 64-bit builds in VS2013 upwards.
 - Added `windowstargetplatformversion` to specify VS Windows target version.

[License](https://github.com/bkaradzic/genie/blob/master/LICENSE)
-----------------------------------------------------------------

	GENie
	Copyright (c) 2014-2016 Branimir Karadžić, Neil Richardson, Mike Popoloski,
	Drew Solomon, Ted de Munnik, Miodrag Milanović, Brett Vickers, Bill Freist,
	Terry Hendrix II, Ryan Juckett, Andrew Johnson
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
