--
-- Copyright 2010-2018 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bimg#license-bsd-2-clause
--

project "texturec"
	kind "ConsoleApp"

	includedirs {
		path.join(BX_DIR,   "include"),
		path.join(BIMG_DIR, "include"),
	}

	files {
		path.join(BIMG_DIR, "tools/texturec/**.cpp"),
		path.join(BIMG_DIR, "tools/texturec/**.h"),
	}

	links {
		"bimg_decode",
		"bimg_encode",
		"bimg",
		"bx",
	}

	configuration { "mingw-*" }
		targetextension ".exe"

	configuration { "osx" }
		links {
			"Cocoa.framework",
		}

	configuration { "vs20* or mingw*" }
		links {
			"psapi",
		}

	configuration {}
