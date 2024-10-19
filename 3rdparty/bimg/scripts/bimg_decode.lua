--
-- Copyright 2010-2024 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
--

if _OPTIONS["with-libheif"] then
	print('\n')
	print('\tWARNING!')
	print('')
	print('\t*** LICENSE INCOMPATIBILITY WARNING!')
	print('\t*** LibHeif is licensed under LGPL! See:')
	print('\t*** https://github.com/strukturag/libheif/blob/master/COPYING')
	print('\n')
end

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
		path.join(BIMG_DIR, "3rdparty/tinyexr/deps/miniz/miniz.*"),
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
