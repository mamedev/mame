-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   tests.lua
--
--   Rules for building tests
--
---------------------------------------------------------------------------
--------------------------------------------------
-- GoogleTest library objects
--------------------------------------------------

project "gtest"
	uuid "fa306a8d-fb10-4d4a-9d2e-fdb9076407b4"
	kind "StaticLib"

	configuration { "gmake" }
		buildoptions {
			"-Wno-undef",
			"-Wno-unused-variable",
		}

	configuration { "vs*" }
if _OPTIONS["vs"]=="intel-15" then
		buildoptions {
			"/Qwd1195",             -- error #1195: conversion from integer to smaller pointer
		}
end

	configuration { }

	includedirs {
		MAME_DIR .. "3rdparty/googletest/googletest/include",
		MAME_DIR .. "3rdparty/googletest/googletest",
	}
	files {
		MAME_DIR .. "3rdparty/googletest/googletest/src/gtest-all.cc",
	}


project("mametests")
	uuid ("66d4c639-196b-4065-a411-7ee9266564f5")
	kind "ConsoleApp"

	flags {
		"Symbols", -- always include minimum symbols for executables
	}

	if _OPTIONS["SEPARATE_BIN"]~="1" then
		targetdir(MAME_DIR)
	end

	configuration { "gmake" }
		buildoptions {
			"-Wno-undef",
		}

	configuration { }

	links {
		"gtest",
		"utils",
		ext_lib("expat"),
		ext_lib("zlib"),
		"ocore_" .. _OPTIONS["osd"],
	}

	includedirs {
		MAME_DIR .. "3rdparty/googletest/googletest/include",
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/lib/util",
		ext_includedir("expat"),
		ext_includedir("zlib"),
	}

	files {
		MAME_DIR .. "tests/main.cpp",
		MAME_DIR .. "tests/lib/util/corestr.cpp",
		MAME_DIR .. "tests/emu/attotime.cpp",
	}

