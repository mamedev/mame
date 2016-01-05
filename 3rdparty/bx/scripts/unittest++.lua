--
-- Copyright 2010-2016 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
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
