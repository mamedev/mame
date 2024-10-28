--
-- Copyright 2010-2024 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx/blob/master/LICENSE
--

newoption {
	trigger = "with-amalgamated",
	description = "Enable amalgamated build.",
}

newoption {
	trigger = "with-crtnone",
	description = "Enable build without CRT.",
}

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
BX_BUILD_DIR = path.join(BX_DIR, ".build")
BX_THIRD_PARTY_DIR = path.join(BX_DIR, "3rdparty")

dofile "toolchain.lua"
toolchain(BX_BUILD_DIR, BX_THIRD_PARTY_DIR)

function copyLib()
end

dofile "bx.lua"
dofile "bin2c.lua"

project "bx.test"
	kind "ConsoleApp"

	debugdir (path.join(BX_DIR, "tests"))

	removeflags {
		"NoExceptions",
	}

	includedirs {
		BX_THIRD_PARTY_DIR,
	}

	defines {
		"CATCH_AMALGAMATED_CUSTOM_MAIN",
	}

	files {
		path.join(BX_DIR, "3rdparty/catch/catch_amalgamated.cpp"),
		path.join(BX_DIR, "tests/*_test.cpp"),
		path.join(BX_DIR, "tests/*.h"),
		path.join(BX_DIR, "tests/dbg.*"),
	}

	using_bx()

	configuration { "vs* or mingw*" }
		links {
			"psapi",
		}

	configuration { "android*" }
		targetextension ".so"
		linkoptions {
			"-shared",
		}

	configuration { "linux-*" }
		links {
			"pthread",
		}

	configuration { "osx*" }
		links {
			"Cocoa.framework",
		}

	configuration { "wasm" }
		buildoptions {
			"-fwasm-exceptions",
		}
		linkoptions {
			"-fwasm-exceptions",
			"-s STACK_SIZE=262144",
		}

	configuration {}

	strip()

project "bx.bench"
	kind "ConsoleApp"

	debugdir (path.join(BX_DIR, "tests"))

	includedirs {
		path.join(BX_DIR, "include"),
		BX_THIRD_PARTY_DIR,
	}

	files {
		path.join(BX_DIR, "tests/*_bench.cpp"),
		path.join(BX_DIR, "tests/*_bench.h"),
		path.join(BX_DIR, "tests/dbg.*"),
	}

	links {
		"bx",
	}

	configuration { "vs* or mingw*" }
		links {
			"psapi",
		}

	configuration { "android*" }
		targetextension ".so"
		linkoptions {
			"-shared",
		}

	configuration { "linux-*" }
		links {
			"pthread",
		}

	configuration { "osx*" }
		links {
			"Cocoa.framework",
		}

	configuration { "Debug" }
		defines {
			"BX_CONFIG_DEBUG=1",
		}

	configuration { "Release" }
		defines {
			"BX_CONFIG_DEBUG=0",
		}

	configuration {}

	strip()
