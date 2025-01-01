--
-- Copyright 2010-2024 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
--

project "bimg_encode"
	kind "StaticLib"

	includedirs {
		path.join(BIMG_DIR, "include"),
		path.join(BIMG_DIR, "3rdparty"),
		path.join(BIMG_DIR, "3rdparty/astc-encoder/include"),
		path.join(BIMG_DIR, "3rdparty/iqa/include"),
		path.join(BIMG_DIR, "3rdparty/nvtt"),
		path.join(BIMG_DIR, "3rdparty/tinyexr/deps/miniz"),
	}

	files {
		path.join(BIMG_DIR, "include/**"),
		path.join(BIMG_DIR, "src/image_encode.*"),
		path.join(BIMG_DIR, "src/image_cubemap_filter.*"),
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

	configuration { "mingw* or linux* or osx*" }
		buildoptions {
			"-Wno-implicit-fallthrough",
			"-Wno-shadow",
			"-Wno-shift-negative-value",
			"-Wno-undef",
		}

		buildoptions_cpp {
			"-Wno-class-memaccess",
			"-Wno-deprecated-copy",
		}

	configuration {}
