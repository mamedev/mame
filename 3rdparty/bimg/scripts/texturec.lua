--
-- Copyright 2010-2024 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bimg/blob/master/LICENSE
--

project "texturec"
	kind "ConsoleApp"

	includedirs {
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
	}

	if _OPTIONS["with-libheif"] then
		links {
			"heif",
		}

		configuration {}
	end

	using_bx()

	configuration { "mingw-*" }
		targetextension ".exe"

	configuration { "osx*" }
		links {
			"Cocoa.framework",
		}

	configuration { "vs20* or mingw*" }
		links {
			"psapi",
		}

	configuration {}
