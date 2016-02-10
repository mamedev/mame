--
-- Copyright 2010-2016 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
--

project "bin2c"
	uuid "60eaa654-7d06-11e4-be8e-880965202986"
	kind "ConsoleApp"

	includedirs {
		"../include",
	}

	files {
		"../tools/bin2c/**.cpp",
		"../tools/bin2c/**.h",
	}

	configuration {}
