--
-- Copyright 2010-2017 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
--

project "bimg"
	kind "StaticLib"

	includedirs {
		path.join(BX_DIR, "include"),
		path.join(BIMG_DIR, "include"),
	}

	files {
		path.join(BIMG_DIR, "include/**"),
		path.join(BIMG_DIR, "src/image.*"),
	}

	configuration { "linux-*" }
		buildoptions {
			"-fPIC",
		}

	configuration {}
