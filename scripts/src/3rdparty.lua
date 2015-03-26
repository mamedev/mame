project "expat"
	uuid "f4cd40b1-c37c-452d-9785-640f26f0bf54"
	kind "StaticLib"

	options {
		"ForceCPP",
	}

	files {
		MAME_DIR .. "3rdparty/expat/lib/**.c",
		MAME_DIR .. "3rdparty/expat/lib/**.h",
	}

project "zlib"
	uuid "3d78bd2a-2bd0-4449-8087-42ddfaef7ec9"
	kind "StaticLib"

	configuration "Debug"
		defines {
			"verbose=-1",
		}

	configuration { "gmake" }
		buildoptions_c {
			"-Wno-strict-prototypes",
		}

	configuration { }
		defines {
			"ZLIB_CONST",
		}

	files {
		MAME_DIR .. "3rdparty/zlib/adler32.*",
		MAME_DIR .. "3rdparty/zlib/compress.*",
		MAME_DIR .. "3rdparty/zlib/crc32.*",
		MAME_DIR .. "3rdparty/zlib/deflate.*",
		MAME_DIR .. "3rdparty/zlib/inffast.*",
		MAME_DIR .. "3rdparty/zlib/inflate.*",
		MAME_DIR .. "3rdparty/zlib/infback.*",
		MAME_DIR .. "3rdparty/zlib/inftrees.*",
		MAME_DIR .. "3rdparty/zlib/trees.*",
		MAME_DIR .. "3rdparty/zlib/uncompr.*",
		MAME_DIR .. "3rdparty/zlib/zutil.*",
	}

project "softfloat"
	uuid "04fbf89e-4761-4cf2-8a12-64500cf0c5c5"
	kind "StaticLib"

	options {
		"ForceCPP",
	}

	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/expat/lib/",
	}
	includeosd()
	
	files {
		MAME_DIR .. "3rdparty/softfloat/**.c",
		MAME_DIR .. "3rdparty/softfloat/**.h",
	}

project "jpeg"
	uuid "447c6800-dcfd-4c48-b72a-a8223bb409ca"
	kind "StaticLib"

	files {
		MAME_DIR .. "3rdparty/libjpeg/jaricom.*",
		MAME_DIR .. "3rdparty/libjpeg/jcapimin.*",
		MAME_DIR .. "3rdparty/libjpeg/jcapistd.*",
		MAME_DIR .. "3rdparty/libjpeg/jcarith.*",
		MAME_DIR .. "3rdparty/libjpeg/jccoefct.*",
		MAME_DIR .. "3rdparty/libjpeg/jccolor.*",
		MAME_DIR .. "3rdparty/libjpeg/jcdctmgr.*",
		MAME_DIR .. "3rdparty/libjpeg/jchuff.*",
		MAME_DIR .. "3rdparty/libjpeg/jcinit.*",
		MAME_DIR .. "3rdparty/libjpeg/jcmainct.*",
		MAME_DIR .. "3rdparty/libjpeg/jcmarker.*",
		MAME_DIR .. "3rdparty/libjpeg/jcmaster.*",
		MAME_DIR .. "3rdparty/libjpeg/jcomapi.*",
		MAME_DIR .. "3rdparty/libjpeg/jcparam.*",
		MAME_DIR .. "3rdparty/libjpeg/jcprepct.*",
		MAME_DIR .. "3rdparty/libjpeg/jcsample.*",
		MAME_DIR .. "3rdparty/libjpeg/jctrans.*",
		MAME_DIR .. "3rdparty/libjpeg/jdapimin.*",
		MAME_DIR .. "3rdparty/libjpeg/jdapistd.*",
		MAME_DIR .. "3rdparty/libjpeg/jdarith.*",
		MAME_DIR .. "3rdparty/libjpeg/jdatadst.*",
		MAME_DIR .. "3rdparty/libjpeg/jdatasrc.*",
		MAME_DIR .. "3rdparty/libjpeg/jdcoefct.*",
		MAME_DIR .. "3rdparty/libjpeg/jdcolor.*",
		MAME_DIR .. "3rdparty/libjpeg/jddctmgr.*",
		MAME_DIR .. "3rdparty/libjpeg/jdhuff.*",
		MAME_DIR .. "3rdparty/libjpeg/jdinput.*",
		MAME_DIR .. "3rdparty/libjpeg/jdmainct.*",
		MAME_DIR .. "3rdparty/libjpeg/jdmarker.*",
		MAME_DIR .. "3rdparty/libjpeg/jdmaster.*",
		MAME_DIR .. "3rdparty/libjpeg/jdmerge.*",
		MAME_DIR .. "3rdparty/libjpeg/jdpostct.*",
		MAME_DIR .. "3rdparty/libjpeg/jdsample.*",
		MAME_DIR .. "3rdparty/libjpeg/jdtrans.*",
		MAME_DIR .. "3rdparty/libjpeg/jerror.*",
		MAME_DIR .. "3rdparty/libjpeg/jfdctflt.*",
		MAME_DIR .. "3rdparty/libjpeg/jfdctfst.*",
		MAME_DIR .. "3rdparty/libjpeg/jfdctint.*",
		MAME_DIR .. "3rdparty/libjpeg/jidctflt.*",
		MAME_DIR .. "3rdparty/libjpeg/jidctfst.*",
		MAME_DIR .. "3rdparty/libjpeg/jidctint.*",
		MAME_DIR .. "3rdparty/libjpeg/jquant1.*",
		MAME_DIR .. "3rdparty/libjpeg/jquant2.*",
		MAME_DIR .. "3rdparty/libjpeg/jutils.*",
		MAME_DIR .. "3rdparty/libjpeg/jmemmgr.*",
		MAME_DIR .. "3rdparty/libjpeg/jmemansi.*",
	}

project "flac"
	uuid "b6fc19e8-073a-4541-bb7b-d24b548d424a"
	kind "StaticLib"

	configuration { }
		defines {
			"WORDS_BIGENDIAN=0",
			"FLAC__NO_ASM",
			"_LARGEFILE_SOURCE",
			"_FILE_OFFSET_BITS=64",
			"FLAC__HAS_OGG=0",
		}
	configuration { "vs*" }
		defines {
			"VERSION=\"1.2.1\""
		}
	configuration { "gmake" }
		defines {
			"VERSION=\\\"1.2.1\\\""
		}
		buildoptions_c {
			"-Wno-unused-function",
			"-O0",
		}

	configuration { }

	includedirs {
		MAME_DIR .. "3rdparty/libflac/src/libFLAC/include",
		MAME_DIR .. "3rdparty/libflac/include",
	}

	files {
		MAME_DIR .. "3rdparty/libflac/src/libFLAC/bitmath.*",
		MAME_DIR .. "3rdparty/libflac/src/libFLAC/bitreader.*",
		MAME_DIR .. "3rdparty/libflac/src/libFLAC/bitwriter.*",
		MAME_DIR .. "3rdparty/libflac/src/libFLAC/cpu.*",
		MAME_DIR .. "3rdparty/libflac/src/libFLAC/crc.*",
		MAME_DIR .. "3rdparty/libflac/src/libFLAC/fixed.*",
		MAME_DIR .. "3rdparty/libflac/src/libFLAC/float.*",
		MAME_DIR .. "3rdparty/libflac/src/libFLAC/format.*",
		MAME_DIR .. "3rdparty/libflac/src/libFLAC/lpc.*",
		MAME_DIR .. "3rdparty/libflac/src/libFLAC/md5.*",
		MAME_DIR .. "3rdparty/libflac/src/libFLAC/memory.*",
		MAME_DIR .. "3rdparty/libflac/src/libFLAC/stream_decoder.*",
		MAME_DIR .. "3rdparty/libflac/src/libFLAC/stream_encoder.*",
		MAME_DIR .. "3rdparty/libflac/src/libFLAC/stream_encoder_framing.*",
		MAME_DIR .. "3rdparty/libflac/src/libFLAC/window.*",
	}


project "7z"
	uuid "ad573d62-e76a-4b11-ae34-5110a6789a42"
	kind "StaticLib"

	configuration { }
		defines {
			"_7ZIP_PPMD_SUPPPORT",
			"_7ZIP_ST",
		}

	files {
			MAME_DIR .. "3rdparty/lzma/C/7zBuf.*",
			MAME_DIR .. "3rdparty/lzma/C/7zBuf2.*",
			MAME_DIR .. "3rdparty/lzma/C/7zCrc.*",
			MAME_DIR .. "3rdparty/lzma/C/7zCrcOpt.*",
			MAME_DIR .. "3rdparty/lzma/C/7zDec.*",
			MAME_DIR .. "3rdparty/lzma/C/7zIn.*",
			MAME_DIR .. "3rdparty/lzma/C/CpuArch.*",
			MAME_DIR .. "3rdparty/lzma/C/LzmaDec.*",
			MAME_DIR .. "3rdparty/lzma/C/Lzma2Dec.*",
			MAME_DIR .. "3rdparty/lzma/C/LzmaEnc.*",
			MAME_DIR .. "3rdparty/lzma/C/Lzma2Enc.*",
			MAME_DIR .. "3rdparty/lzma/C/LzFind.*",
			MAME_DIR .. "3rdparty/lzma/C/Bra.*",
			MAME_DIR .. "3rdparty/lzma/C/Bra86.*",
			MAME_DIR .. "3rdparty/lzma/C/Bcj2.*",
			MAME_DIR .. "3rdparty/lzma/C/Ppmd7.*",
			MAME_DIR .. "3rdparty/lzma/C/Ppmd7Dec.*",
			MAME_DIR .. "3rdparty/lzma/C/7zStream.*",
		}


project "lua"
	uuid "d9e2eed1-f1ab-4737-a6ac-863700b1a5a9"
	kind "StaticLib"

	configuration { }
		defines {
			"LUA_COMPAT_ALL",
		}
	if not (_OPTIONS["targetos"]=="windows") then
		defines {
			"LUA_USE_POSIX",
		}
	end
	if ("pnacl" == _OPTIONS["gcc"]) then
		defines {
			"LUA_32BITS",
		}
	end
	
	configuration { }

	includedirs {
		MAME_DIR .. "3rdparty",
	}

	files {
		MAME_DIR .. "3rdparty/lua/**.c",
		MAME_DIR .. "3rdparty/lua/**.h",
	}
	
	removefiles {
		MAME_DIR .. "3rdparty/lua/src/lua.c",
		MAME_DIR .. "3rdparty/lua/src/luac.c",
	}

project "lsqlite3"
	uuid "1d84edab-94cf-48fb-83ee-b75bc697660e"
	kind "StaticLib"

	configuration { }
		defines {
			"LUA_COMPAT_ALL",
		}

	includedirs {
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/lua/src",
	}

	files {
		MAME_DIR .. "3rdparty/lsqlite3/lsqlite3.c",
	}

project "mongoose"
	uuid "ff05b529-2b6f-4166-9dff-5fe2aef89c40"
	kind "StaticLib"

	options {
		"ForceCPP",
	}
	defines {
		"MONGOOSE_ENABLE_THREADS",
		"NS_STACK_SIZE=0"
	}

	includedirs {
		MAME_DIR .. "3rdparty/mongoose",
	}

	files {
		MAME_DIR .. "3rdparty/mongoose/*.c",
		MAME_DIR .. "3rdparty/mongoose/*.h",
	}

project "jsoncpp"
	uuid "ae023ff3-d712-4e54-adc5-3b56a148650f"
	kind "StaticLib"

	options {
		"ForceCPP",
	}

	includedirs {
		MAME_DIR .. "3rdparty/jsoncpp/include",
	}

	files {
		MAME_DIR .. "3rdparty/jsoncpp/src/lib_json/*.cpp",
		MAME_DIR .. "3rdparty/jsoncpp/src/lib_json/*.h",
	}

project "sqllite3"
	uuid "5cb3d495-57ed-461c-81e5-80dc0857517d"
	kind "StaticLib"

	configuration { "gmake" }
		buildoptions_c {
			"-Wno-bad-function-cast",
			"-Wno-undef",
		}

	configuration { }

	files {
		MAME_DIR .. "3rdparty/sqlite3/**.c",
		MAME_DIR .. "3rdparty/sqlite3/**.h",
	}

	removefiles {
		MAME_DIR .. "3rdparty/sqlite3/shell.c",
	}



project "portmidi"
	uuid "587f2da6-3274-4a65-86a2-f13ea315bb98"
	kind "StaticLib"

	includedirs {
		MAME_DIR .. "3rdparty/portmidi/pm_common",
		MAME_DIR .. "3rdparty/portmidi/porttime",
	}
	
	includeosd()

	configuration { "linux*" }
		defines {
			"PMALSA=1",
		}

	configuration { }

	files {
		MAME_DIR .. "3rdparty/portmidi/pm_common/*.c",
		MAME_DIR .. "3rdparty/portmidi/pm_common/*.h",
		MAME_DIR .. "3rdparty/portmidi/porttime/porttime.*",
	}

	if _OPTIONS["targetos"]=="windows" then
		files {
			MAME_DIR .. "3rdparty/portmidi/pm_win/*.c",
			MAME_DIR .. "3rdparty/portmidi/pm_win/*.h",
			MAME_DIR .. "3rdparty/portmidi/porttime/ptwinmm.*",
		}
	end

	if _OPTIONS["targetos"]=="linux" then
		files {
			MAME_DIR .. "3rdparty/portmidi/pm_linux/*.c",
			MAME_DIR .. "3rdparty/portmidi/pm_linux/*.h",
			MAME_DIR .. "3rdparty/portmidi/porttime/ptlinux.*",
		}
	end
	if _OPTIONS["targetos"]=="macosx" then
		files {
			MAME_DIR .. "3rdparty/portmidi/pm_mac/*.c",
			MAME_DIR .. "3rdparty/portmidi/pm_mac/*.h",
			MAME_DIR .. "3rdparty/portmidi/pm_mac/*.m",
			MAME_DIR .. "3rdparty/portmidi/porttime/ptmacosx_mach.*",
		}
	end
if (USE_BGFX == 1) then
project "bgfx"
	uuid "d3e7e119-35cf-4f4f-aba0-d3bdcd1b879a"
	kind "StaticLib"

	includedirs {		
		MAME_DIR .. "3rdparty/bgfx/include",
		MAME_DIR .. "3rdparty/bgfx/3rdparty",
		MAME_DIR .. "3rdparty/bx/include",
		MAME_DIR .. "3rdparty/bgfx/3rdparty/khronos",
		MAME_DIR .. "3rdparty/dxsdk/Include",
	}

	includeosd()

	configuration { "vs*" }
		includedirs {
			MAME_DIR .. "3rdparty/bx/include/compat/msvc",
		}
	configuration { "mingw*" }
		includedirs {
			MAME_DIR .. "3rdparty/bx/include/compat/mingw",
		}

	configuration { "osx*" }
		includedirs {
			MAME_DIR .. "3rdparty/bx/include/compat/osx",
		}
	
	configuration { }

	defines {
		"__STDC_LIMIT_MACROS",
		"__STDC_FORMAT_MACROS",
		"__STDC_CONSTANT_MACROS",
	}
	files {
		MAME_DIR .. "3rdparty/bgfx/src/*.h",
		MAME_DIR .. "3rdparty/bgfx/src/*.cpp",
		MAME_DIR .. "3rdparty/bgfx/examples/common/bgfx_utils.*",
		MAME_DIR .. "3rdparty/bgfx/examples/common/bounds.*",
		MAME_DIR .. "3rdparty/bgfx/examples/common/camera.*",
		MAME_DIR .. "3rdparty/bgfx/examples/common/cube_atlas.*",
		MAME_DIR .. "3rdparty/bgfx/examples/common/font/font_manager.*",
		MAME_DIR .. "3rdparty/bgfx/examples/common/font/text_buffer_manager.*",
		MAME_DIR .. "3rdparty/bgfx/examples/common/font/text_metrics.*",
		MAME_DIR .. "3rdparty/bgfx/examples/common/font/utf8.*",
		MAME_DIR .. "3rdparty/bgfx/examples/common/imgui/imgui.*",
		MAME_DIR .. "3rdparty/bgfx/examples/common/nanovg/nanovg.*",
		MAME_DIR .. "3rdparty/bgfx/examples/common/nanovg/nanovg_bgfx.*",
	}
	if _OPTIONS["targetos"]=="macosx" then
		files {
			MAME_DIR .. "3rdparty/bgfx/src/*.mm",
		}
	end
	removefiles {
		MAME_DIR .. "3rdparty/bgfx/src/amalgamated.cpp",
	}
end
