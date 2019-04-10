--
-- Copyright 2010-2018 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
--

project "bimg_decode"
	kind "StaticLib"

	includedirs {
		path.join(BX_DIR, "include"),
		path.join(BIMG_DIR, "include"),
		path.join(BIMG_DIR, "3rdparty"),
		path.join(BIMG_DIR, "3rdparty/nvtt"),
		path.join(BIMG_DIR, "3rdparty/iqa/include"),
	}

	files {
		path.join(BIMG_DIR, "include/**"),
		path.join(BIMG_DIR, "src/image_decode.*"),
	}

	configuration { "linux-*" }
		buildoptions {
			"-fPIC",
		}

	configuration {}
