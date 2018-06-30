--
-- Copyright 2010-2017 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
--

project "texturec"
	kind "ConsoleApp"

	includedirs {
		path.join(BX_DIR,   "include"),
		path.join(BIMG_DIR, "include"),
		path.join(BIMG_DIR, "3rdparty"),
		path.join(BIMG_DIR, "3rdparty/nvtt"),
		path.join(BIMG_DIR, "3rdparty/iqa/include"),
	}

	files {
		path.join(BIMG_DIR, "3rdparty/libsquish/**.cpp"),
		path.join(BIMG_DIR, "3rdparty/libsquish/**.h"),
		path.join(BIMG_DIR, "3rdparty/edtaa3/**.cpp"),
		path.join(BIMG_DIR, "3rdparty/edtaa3/**.h"),
		path.join(BIMG_DIR, "3rdparty/etc1/**.cpp"),
		path.join(BIMG_DIR, "3rdparty/etc1/**.h"),
		path.join(BIMG_DIR, "3rdparty/etc2/**.cpp"),
		path.join(BIMG_DIR, "3rdparty/etc2/**.hpp"),
		path.join(BIMG_DIR, "3rdparty/nvtt/**.cpp"),
		path.join(BIMG_DIR, "3rdparty/nvtt/**.h"),
		path.join(BIMG_DIR, "3rdparty/pvrtc/**.cpp"),
		path.join(BIMG_DIR, "3rdparty/pvrtc/**.h"),
		path.join(BIMG_DIR, "3rdparty/tinyexr/**.h"),
		path.join(BIMG_DIR, "3rdparty/iqa/include/**.h"),
		path.join(BIMG_DIR, "3rdparty/iqa/source/**.c"),
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
