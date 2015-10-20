--
-- Copyright 2010-2015 Branimir Karadzic. All rights reserved.
-- License: http://www.opensource.org/licenses/BSD-2-Clause
--

project "UnitTest++"
	kind "StaticLib"

	removeflags {
		"NoExceptions",
	}

	files {
		"../3rdparty/UnitTest++/src/*.cpp",
		"../3rdparty/UnitTest++/src/*.h",
	}

	configuration { "linux or osx or android-* or *nacl* or ps4" }
		files {
			"../3rdparty/UnitTest++/src/Posix/**.cpp",
			"../3rdparty/UnitTest++/src/Posix/**.h",
		}

	configuration { "mingw* or vs*" }
		files {
			"../3rdparty/UnitTest++/src/Win32/**.cpp",
			"../3rdparty/UnitTest++/src/Win32/**.h",
		}

	configuration {}
