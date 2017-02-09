--
-- Copyright 2010-2017 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
--

project "bx"
	kind "StaticLib"

	includedirs {
		"../include",
	}

	files {
		"../include/**.h",
		"../include/**.inl",
		"../src/**.cpp",
	}

	configuration { "linux-*" }
		buildoptions {
			"-fPIC",
		}

	configuration {}
