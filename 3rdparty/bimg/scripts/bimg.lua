--
-- Copyright 2010-2018 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
--

function filesexist(_srcPath, _dstPath, _files)
	for _, file in ipairs(_files) do
		file = path.getrelative(_srcPath, file)
		local filePath = path.join(_dstPath, file)
		if not os.isfile(filePath) then return false end
	end

	return true
end

project "bimg"
	kind "StaticLib"

	includedirs {
		path.join(BX_DIR, "include"),
		path.join(BIMG_DIR, "include"),
	}

	files {
		path.join(BIMG_DIR, "include/**"),
		path.join(BIMG_DIR, "src/image.*"),
		path.join(BIMG_DIR, "src/image_gnf.cpp"),
	}

	configuration { "linux-*" }
		buildoptions {
			"-fPIC",
		}

	configuration {}
