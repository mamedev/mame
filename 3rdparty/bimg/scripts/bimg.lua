--
-- Copyright 2010-2024 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
--

project "bimg"
	kind "StaticLib"

	includedirs {
		path.join(BIMG_DIR, "include"),
		path.join(BIMG_DIR, "3rdparty/astc-encoder/include"),
	}

	files {
		path.join(BIMG_DIR, "include/**"),
		path.join(BIMG_DIR, "src/image.*"),
		path.join(BIMG_DIR, "src/image_gnf.cpp"),

		path.join(BIMG_DIR, "3rdparty/astc-encoder/source/**.cpp"),
		path.join(BIMG_DIR, "3rdparty/astc-encoder/source/**.h"),
	}

	using_bx()

	configuration {}

	removeflags {
		"FloatFast", -- astc-encoder doesn't work with it.
	}

	configuration { "linux-*" }
		buildoptions {
			"-fPIC",
		}

	configuration {}
