--
-- Copyright 2010-2021 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
--

project "bimg"
	kind "StaticLib"

	includedirs {
		path.join(BX_DIR, "include"),
		path.join(BIMG_DIR, "include"),
		path.join(BIMG_DIR, "3rdparty/astc-codec"),
		path.join(BIMG_DIR, "3rdparty/astc-codec/include"),
		path.join(BIMG_DIR, "3rdparty/tinyexr/deps/miniz"),
	}

	local ASTC_CODEC_DIR = path.join(BIMG_DIR, "3rdparty/astc-codec")

	files {
		path.join(BIMG_DIR, "include/**"),
		path.join(BIMG_DIR, "src/image.*"),
		path.join(BIMG_DIR, "src/image_gnf.cpp"),

		path.join(ASTC_CODEC_DIR, "src/decoder/astc_file.*"),
		path.join(ASTC_CODEC_DIR, "src/decoder/codec.*"),
		path.join(ASTC_CODEC_DIR, "src/decoder/endpoint_codec.*"),
		path.join(ASTC_CODEC_DIR, "src/decoder/footprint.*"),
		path.join(ASTC_CODEC_DIR, "src/decoder/integer_sequence_codec.*"),
		path.join(ASTC_CODEC_DIR, "src/decoder/intermediate_astc_block.*"),
		path.join(ASTC_CODEC_DIR, "src/decoder/logical_astc_block.*"),
		path.join(ASTC_CODEC_DIR, "src/decoder/partition.*"),
		path.join(ASTC_CODEC_DIR, "src/decoder/physical_astc_block.*"),
		path.join(ASTC_CODEC_DIR, "src/decoder/quantization.*"),
		path.join(ASTC_CODEC_DIR, "src/decoder/weight_infill.*"),

		path.join(BIMG_DIR, "3rdparty/tinyexr/deps/miniz/miniz.*"),
	}

	configuration { "linux-*" }
		buildoptions {
			"-fPIC",
		}

	configuration {}
