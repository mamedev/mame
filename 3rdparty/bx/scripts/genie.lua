--
-- Copyright 2010-2015 Branimir Karadzic. All rights reserved.
-- License: http://www.opensource.org/licenses/BSD-2-Clause
--

solution "bx"
	configurations {
		"Debug",
		"Release",
	}

	platforms {
		"x32",
		"x64",
		"Native", -- for targets where bitness is not specified
	}

	language "C++"

BX_DIR = path.getabsolute("..")
local BX_BUILD_DIR = path.join(BX_DIR, ".build")
local BX_THIRD_PARTY_DIR = path.join(BX_DIR, "3rdparty")

defines {
	"BX_CONFIG_ENABLE_MSVC_LEVEL4_WARNINGS=1"
}

dofile "toolchain.lua"
toolchain(BX_BUILD_DIR, BX_THIRD_PARTY_DIR)

function copyLib()
end

dofile "bx.lua"
dofile "unittest++.lua"
dofile "bin2c.lua"

project "bx.test"
	kind "ConsoleApp"

	debugdir (path.join(BX_DIR, "tests"))

	removeflags {
		"NoExceptions",
	}

	includedirs {
		path.join(BX_DIR, "include"),
		path.join(BX_THIRD_PARTY_DIR, "UnitTest++/src"),
	}

	links {
		"UnitTest++",
	}

	files {
		path.join(BX_DIR, "tests/**.cpp"),
		path.join(BX_DIR, "tests/**.H"),
	}

	configuration { "vs*" }

	configuration { "android*" }
		kind "ConsoleApp"
		targetextension ".so"
		linkoptions {
			"-shared",
		}

	configuration { "nacl or nacl-arm" }
		kind "ConsoleApp"
		targetextension ".nexe"
		links {
			"ppapi",
			"pthread",
		}

	configuration { "pnacl" }
		kind "ConsoleApp"
		targetextension ".pexe"
		links {
			"ppapi",
			"pthread",
		}

	configuration { "linux-*" }
		links {
			"pthread",
		}

	configuration { "osx" }
		links {
			"Cocoa.framework",
		}

	configuration {}

	strip()
