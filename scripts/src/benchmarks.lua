-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   benchmarks.lua
--
--   Rules for building benchmarks
--
---------------------------------------------------------------------------

--------------------------------------------------
-- Google Benchmark library objects
--------------------------------------------------

project "benchmark"
	uuid "60a7e05c-8b4f-497c-bfda-2949a009ba0d"
	kind "StaticLib"

	configuration { }
		defines {
			"HAVE_STD_REGEX",
		}

	includedirs {
		MAME_DIR .. "3rdparty/benchmark/include",
	}
	files {
		MAME_DIR .. "3rdparty/benchmark/src/benchmark.cc",
		MAME_DIR .. "3rdparty/benchmark/src/colorprint.cc",
		MAME_DIR .. "3rdparty/benchmark/src/commandlineflags.cc",
		MAME_DIR .. "3rdparty/benchmark/src/console_reporter.cc",
		MAME_DIR .. "3rdparty/benchmark/src/csv_reporter.cc",
		MAME_DIR .. "3rdparty/benchmark/src/json_reporter.cc",
		MAME_DIR .. "3rdparty/benchmark/src/log.cc",
		MAME_DIR .. "3rdparty/benchmark/src/reporter.cc",
		MAME_DIR .. "3rdparty/benchmark/src/sleep.cc",
		MAME_DIR .. "3rdparty/benchmark/src/string_util.cc",
		MAME_DIR .. "3rdparty/benchmark/src/sysinfo.cc",
		MAME_DIR .. "3rdparty/benchmark/src/walltime.cc",
		MAME_DIR .. "3rdparty/benchmark/src/re_std.cc",
	}



project("benchmarks")
	uuid ("a9750a48-d283-4a6d-b126-31c7ce049af1")
	kind "ConsoleApp"

	flags {
		"Symbols", -- always include minimum symbols for executables
	}

	if _OPTIONS["SEPARATE_BIN"]~="1" then
		targetdir(MAME_DIR)
	end

	configuration { }

	links {
		"benchmark",
	}

	includedirs {
		MAME_DIR .. "3rdparty/benchmark/include",
		MAME_DIR .. "src/osd",
	}

	files {
		MAME_DIR .. "benchmarks/main.cpp",
		MAME_DIR .. "benchmarks/eminline_native.cpp",
		MAME_DIR .. "benchmarks/eminline_noasm.cpp",
	}

