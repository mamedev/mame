--
-- Copyright 2010-2021 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
--

project "bin2c"
	kind "ConsoleApp"

	includedirs {
		"../include",
	}

	files {
		"../tools/bin2c/**.cpp",
		"../tools/bin2c/**.h",
	}

	links {
		"bx",
	}

	configuration { "mingw-*" }
		targetextension ".exe"

	configuration { "linux-*" }
		links {
			"pthread",
		}

	configuration { "vs20* or mingw*" }
		links {
			"psapi",
		}

	configuration {}

	strip()
