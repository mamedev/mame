--
-- Copyright 2010-2024 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bimg/blob/master/LICENSE
--

newoption {
	trigger = "with-amalgamated",
	description = "Enable amalgamated build.",
}

newoption {
	trigger = "with-shared-lib",
	description = "Enable building shared library.",
}

newoption {
	trigger = "with-tools",
	description = "Enable building tools.",
}

newoption {
	trigger = "with-libheif",
	description = "Enable building with libheif HEIF and AVIF file format decoder.",
}

solution "bimg"
	configurations {
		"Debug",
		"Release",
	}

	if _ACTION == "xcode4" then
		platforms {
			"Universal",
		}
	else
		platforms {
			"x32",
			"x64",
			"Native", -- for targets where bitness is not specified
		}
	end

	language "C++"

MODULE_DIR = path.getabsolute("..")
BIMG_DIR   = path.getabsolute("..")
BX_DIR     = os.getenv("BX_DIR")

local BIMG_BUILD_DIR = path.join(BIMG_DIR, ".build")
local BIMG_THIRD_PARTY_DIR = path.join(BIMG_DIR, "3rdparty")
if not BX_DIR then
	BX_DIR = path.getabsolute(path.join(BIMG_DIR, "../bx"))
end

if not os.isdir(BX_DIR) then
	print("bx not found at " .. BX_DIR)
	print("For more info see: https://bkaradzic.github.io/bgfx/build.html")
	os.exit()
end

dofile (path.join(BX_DIR, "scripts/toolchain.lua"))
if not toolchain(BIMG_BUILD_DIR, BIMG_THIRD_PARTY_DIR) then
	return -- no action specified
end

function copyLib()
end

group "libs"
dofile(path.join(BX_DIR, "scripts/bx.lua"))
dofile "bimg.lua"
dofile "bimg_decode.lua"
dofile "bimg_encode.lua"

if _OPTIONS["with-tools"] then
	group "tools"
	dofile "texturec.lua"
end
