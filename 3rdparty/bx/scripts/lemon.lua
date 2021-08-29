--
-- Copyright 2010-2019 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
--

project "lemon"
	kind "ConsoleApp"

	files {
		path.join(BX_DIR, "tools/lemon/lemon.c")
	}

	configuration { "not vs*" }
		buildoptions {
			"-Wno-implicit-fallthrough",
			"-Wno-sign-compare",
			"-Wno-unused-parameter",
		}

	configuration {}

	strip()
