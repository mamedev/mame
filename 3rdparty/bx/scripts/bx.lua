--
-- Copyright 2010-2015 Branimir Karadzic. All rights reserved.
-- License: http://www.opensource.org/licenses/BSD-2-Clause
--
 
project "bx"
	uuid "4db0b09e-d6df-11e1-a0ec-65ccdd6a022f"
	kind "StaticLib"

	configuration { "osx or ios" }
		-- OSX ar doesn't like creating archive without object files
		-- here is object file...
		prebuildcommands {
			"@echo \"void dummy() {}\" > /tmp/dummy.cpp",
		}
		files {
			"/tmp/dummy.cpp",
		}

	configuration {}

	files {
		"../include/**.h",
	}
