--
-- Copyright 2010-2024 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
--

project "bimg_decode"
	kind "StaticLib"

	includedirs {
		path.join(BIMG_DIR, "include"),
		path.join(BIMG_DIR, "3rdparty"),
		path.join(BIMG_DIR, "3rdparty/tinyexr/deps/miniz"),
	}

	files {
		path.join(BIMG_DIR, "include/**"),
		path.join(BIMG_DIR, "src/image_decode.*"),
	}

	if _OPTIONS["with-libheif"] then
		defines {
			"BIMG_DECODE_HEIF=1",
		}
	end

	using_bx()

	configuration { "linux-*" }
		buildoptions {
			"-fPIC",
		}

	configuration {}
