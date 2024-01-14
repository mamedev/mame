--
-- Copyright 2010-2024 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx/blob/master/LICENSE
--

project "bin2c"
	kind "ConsoleApp"

	files {
		"../tools/bin2c/**.cpp",
		"../tools/bin2c/**.h",
	}

	using_bx()

	configuration { "mingw-*" }
		targetextension ".exe"

	configuration { "linux-*" }
		links {
			"pthread",
		}
	configuration { "osx-*" }
		linkoptions {
			"-framework Foundation"
		}

	configuration { "vs20* or mingw*" }
		links {
			"psapi",
		}

	configuration {}

	strip()
