--
-- Copyright 2010-2023 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx/blob/master/LICENSE
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
