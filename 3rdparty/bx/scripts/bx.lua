--
-- Copyright 2010-2022 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx/blob/master/LICENSE
--

local function userdefines()
	local defines = {}
	local BX_CONFIG = os.getenv("BX_CONFIG")
	if BX_CONFIG then
		for def in BX_CONFIG:gmatch "[^%s:]+" do
			table.insert(defines, "BX_CONFIG_" .. def)
		end
	end

	return defines
end

function using_bx()
	includedirs {
		path.join(BX_DIR, "include"),
	}

	links {
		"bx",
	}

	configuration { "Debug" }
		defines {
			"BX_CONFIG_DEBUG=1",
		}

	configuration { "Release" }
		defines {
			"BX_CONFIG_DEBUG=0",
		}

	configuration {}
end

project "bx"
	kind "StaticLib"

	includedirs {
		path.join(BX_DIR, "include"),
		path.join(BX_DIR, "3rdparty"),
	}

	files {
		path.join(BX_DIR, "include/**.h"),
		path.join(BX_DIR, "include/**.inl"),
		path.join(BX_DIR, "src/**.cpp"),
		path.join(BX_DIR, "scripts/**.natvis"),
	}

	defines (userdefines())

	configuration { "linux-*" }
		buildoptions {
			"-fPIC",
		}

	configuration {}

	if _OPTIONS["with-amalgamated"] then
		excludes {
			path.join(BX_DIR, "src/allocator.cpp"),
			path.join(BX_DIR, "src/bounds.cpp"),
			path.join(BX_DIR, "src/bx.cpp"),
			path.join(BX_DIR, "src/commandline.cpp"),
			path.join(BX_DIR, "src/crtnone.cpp"),
			path.join(BX_DIR, "src/debug.cpp"),
			path.join(BX_DIR, "src/dtoa.cpp"),
			path.join(BX_DIR, "src/easing.cpp"),
			path.join(BX_DIR, "src/file.cpp"),
			path.join(BX_DIR, "src/filepath.cpp"),
			path.join(BX_DIR, "src/hash.cpp"),
			path.join(BX_DIR, "src/math.cpp"),
			path.join(BX_DIR, "src/mutex.cpp"),
			path.join(BX_DIR, "src/os.cpp"),
			path.join(BX_DIR, "src/process.cpp"),
			path.join(BX_DIR, "src/semaphore.cpp"),
			path.join(BX_DIR, "src/settings.cpp"),
			path.join(BX_DIR, "src/sort.cpp"),
			path.join(BX_DIR, "src/string.cpp"),
			path.join(BX_DIR, "src/thread.cpp"),
			path.join(BX_DIR, "src/timer.cpp"),
			path.join(BX_DIR, "src/url.cpp"),
		}
	else
		excludes {
			path.join(BX_DIR, "src/amalgamated.**"),
		}
	end

	configuration { "Debug" }
		defines {
			"BX_CONFIG_DEBUG=1",
		}

	configuration { "Release" }
		defines {
			"BX_CONFIG_DEBUG=0",
		}

	configuration {}
