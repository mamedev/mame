--
-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   3rdparty.lua
--
--   Library objects for all 3rdparty sources
--
---------------------------------------------------------------------------

--------------------------------------------------
-- expat library objects
--------------------------------------------------

if not _OPTIONS["with-system-expat"] then
project "expat"
	uuid "f4cd40b1-c37c-452d-9785-640f26f0bf54"
	kind "StaticLib"

	-- fake out the enough of expat_config.h to get by
	-- could possibly add more defines here for specific targets
	defines {
		"HAVE_MEMMOVE",
		"HAVE_STDINT_H",
		"HAVE_STDLIB_H",
		"HAVE_STRING_H",
		"PACKAGE=\"expat\"",
		"PACKAGE_BUGREPORT=\"expat-bugs@libexpat.org\"",
		"PACKAGE_NAME=\"expat\"",
		"PACKAGE_STRING=\"expat-2.2.10\"",
		"PACKAGE_TARNAME=\"expat\"",
		"PACKAGE_URL=\"\"",
		"PACKAGE_VERSION=\"2.2.10\"",
		"STDC_HEADERS",
		"VERSION=\"2.2.10\"",
		"XML_CONTEXT_BYTES=1024",
		"XML_DTD",
		"XML_NS",
	}
if _OPTIONS["BIGENDIAN"]=="1" then
	defines {
		"BYTEORDER=4321",
		"WORDS_BIGENDIAN",
	}
else
	defines {
		"BYTEORDER=1234",
	}
end
if _OPTIONS["targetos"]=="macosx" or _OPTIONS["targetos"]=="freebsd" then
	defines {
		"HAVE_ARC4RANDOM",
	}
end
if BASE_TARGETOS=="unix" then
	defines {
		"HAVE_DLFCN_H",
		"HAVE_FCNTL_H",
		"HAVE_MMAP",
		"HAVE_SYS_STAT_H",
		"HAVE_SYS_TYPES_H",
		"HAVE_UNISTD_H",
		"XML_DEV_URANDOM",
	}
end

	configuration { "vs*" }
		buildoptions {
			"/wd4100", -- warning C4100: 'xxx' : unreferenced formal parameter
			"/wd4127", -- warning C4127: conditional expression is constant
			"/wd4244", -- warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
			"/wd4456", -- warning C4456: declaration of 'xxx' hides previous local declaration
		}
if _OPTIONS["vs"]=="intel-15" then
		buildoptions {
			"/Qwd111",              -- remark #111: statement is unreachable
			"/Qwd1879",             -- warning #1879: unimplemented pragma ignored
			"/Qwd2557",             -- remark #2557: comparison between signed and unsigned operands
			"/Qwd869",              -- remark #869: parameter "xxx" was never referenced
		}
end

	configuration { "gmake or ninja" }
if _OPTIONS["gcc"]~=nil then
	if string.find(_OPTIONS["gcc"], "clang") or string.find(_OPTIONS["gcc"], "asmjs") or string.find(_OPTIONS["gcc"], "android") then

	else
		buildoptions_c {
			"-Wno-maybe-uninitialized", -- expat in GCC 11.1
		}
	end
end

	configuration { }

	files {
		MAME_DIR .. "3rdparty/expat/lib/xmlparse.c",
		MAME_DIR .. "3rdparty/expat/lib/xmlrole.c",
		MAME_DIR .. "3rdparty/expat/lib/xmltok.c",
	}
else
links {
	ext_lib("expat"),
}
end


--------------------------------------------------
-- zlib library objects
--------------------------------------------------

if not _OPTIONS["with-system-zlib"] then
project "zlib"
	uuid "3d78bd2a-2bd0-4449-8087-42ddfaef7ec9"
	kind "StaticLib"

	local version = str_to_version(_OPTIONS["gcc_version"])
	if _OPTIONS["gcc"]~=nil and (string.find(_OPTIONS["gcc"], "clang") or string.find(_OPTIONS["gcc"], "asmjs") or string.find(_OPTIONS["gcc"], "android")) then
		configuration { "gmake or ninja" }
		if (version >= 30700) then
			buildoptions {
				"-Wno-shift-negative-value",
			}
		end
	end

	configuration { "vs*" }
		buildoptions {
			"/wd4127", -- warning C4127: conditional expression is constant
			"/wd4244", -- warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
		}
if _OPTIONS["vs"]=="intel-15" then
		buildoptions {
			"/Qwd111",              -- remark #111: statement is unreachable
			"/Qwd280",              -- remark #280: selector expression is constant
		}
end
	configuration "Debug"
		defines {
			"verbose=-1",
		}

	configuration { }
		defines {
			"ZLIB_CONST",
		}

	files {
		MAME_DIR .. "3rdparty/zlib/adler32.c",
		MAME_DIR .. "3rdparty/zlib/compress.c",
		MAME_DIR .. "3rdparty/zlib/crc32.c",
		MAME_DIR .. "3rdparty/zlib/deflate.c",
		MAME_DIR .. "3rdparty/zlib/inffast.c",
		MAME_DIR .. "3rdparty/zlib/inflate.c",
		MAME_DIR .. "3rdparty/zlib/infback.c",
		MAME_DIR .. "3rdparty/zlib/inftrees.c",
		MAME_DIR .. "3rdparty/zlib/trees.c",
		MAME_DIR .. "3rdparty/zlib/uncompr.c",
		MAME_DIR .. "3rdparty/zlib/zutil.c",
	}
else
links {
	ext_lib("zlib"),
}
end


--------------------------------------------------
-- zstd library objects
--------------------------------------------------

if not _OPTIONS["with-system-zstd"] then
project "zstd"
	uuid "5edd8713-bc60-456d-9c95-b928a913c84b"
	kind "StaticLib"

	configuration { "Release" }
		defines {
			"NDEBUG",
		}

	configuration { }

	defines {
		"XXH_NAMESPACE=ZSTD_",
		"DEBUGLEVEL=0",
		"ZSTD_DISABLE_ASM",
	}

	files {
		MAME_DIR .. "3rdparty/zstd/lib/common/debug.c",
		MAME_DIR .. "3rdparty/zstd/lib/common/entropy_common.c",
		MAME_DIR .. "3rdparty/zstd/lib/common/error_private.c",
		MAME_DIR .. "3rdparty/zstd/lib/common/fse_decompress.c",
		MAME_DIR .. "3rdparty/zstd/lib/common/pool.c",
		MAME_DIR .. "3rdparty/zstd/lib/common/threading.c",
		MAME_DIR .. "3rdparty/zstd/lib/common/xxhash.c",
		MAME_DIR .. "3rdparty/zstd/lib/common/zstd_common.c",
		MAME_DIR .. "3rdparty/zstd/lib/compress/fse_compress.c",
		MAME_DIR .. "3rdparty/zstd/lib/compress/hist.c",
		MAME_DIR .. "3rdparty/zstd/lib/compress/huf_compress.c",
		MAME_DIR .. "3rdparty/zstd/lib/compress/zstd_compress.c",
		MAME_DIR .. "3rdparty/zstd/lib/compress/zstd_compress_literals.c",
		MAME_DIR .. "3rdparty/zstd/lib/compress/zstd_compress_sequences.c",
		MAME_DIR .. "3rdparty/zstd/lib/compress/zstd_compress_superblock.c",
		MAME_DIR .. "3rdparty/zstd/lib/compress/zstd_double_fast.c",
		MAME_DIR .. "3rdparty/zstd/lib/compress/zstd_fast.c",
		MAME_DIR .. "3rdparty/zstd/lib/compress/zstd_lazy.c",
		MAME_DIR .. "3rdparty/zstd/lib/compress/zstd_ldm.c",
		MAME_DIR .. "3rdparty/zstd/lib/compress/zstdmt_compress.c",
		MAME_DIR .. "3rdparty/zstd/lib/compress/zstd_opt.c",
		--MAME_DIR .. "3rdparty/zstd/lib/decompress/huf_decompress_amd64.S", only supports GCC-like assemblers and SysV calling convention
		MAME_DIR .. "3rdparty/zstd/lib/decompress/huf_decompress.c",
		MAME_DIR .. "3rdparty/zstd/lib/decompress/zstd_ddict.c",
		MAME_DIR .. "3rdparty/zstd/lib/decompress/zstd_decompress_block.c",
		MAME_DIR .. "3rdparty/zstd/lib/decompress/zstd_decompress.c",
	}
else
links {
	ext_lib("zstd"),
}
end


--------------------------------------------------
-- SoftFloat library objects
--------------------------------------------------

project "softfloat"
	uuid "04fbf89e-4761-4cf2-8a12-64500cf0c5c5"
	kind "StaticLib"

	options {
		"ForceCPP",
	}

	includedirs {
		MAME_DIR .. "src/osd",
	}

	configuration { "gmake or ninja" }
		buildoptions_cpp {
			"-x c++",
		}

	configuration { "vs*" }
		buildoptions {
			"/wd4244", -- warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
			"/wd4146", -- warning C4146: unary minus operator applied to unsigned type, result still unsigned
			"/wd4018", -- warning C4018: 'x' : signed/unsigned mismatch
		}
if _OPTIONS["vs"]=="intel-15" then
		buildoptions {
			"/Qwd2557",             -- remark #2557: comparison between signed and unsigned operands
		}
end
	configuration { }

	files {
		MAME_DIR .. "3rdparty/softfloat/softfloat.c",
		MAME_DIR .. "3rdparty/softfloat/fsincos.c",
		MAME_DIR .. "3rdparty/softfloat/fpatan.c",
		MAME_DIR .. "3rdparty/softfloat/fyl2x.c",
		MAME_DIR .. "3rdparty/softfloat/f2xm1.c",
	}


--------------------------------------------------
-- SoftFloat 3 library objects
--------------------------------------------------

project "softfloat3"
	uuid "9c22fc90-53fd-11e8-b566-0800200c9a66"
	kind "StaticLib"

	options {
		"ForceCPP",
	}

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "3rdparty/softfloat3/build/MAME",
		MAME_DIR .. "3rdparty/softfloat3/source",
		MAME_DIR .. "3rdparty/softfloat3/source/include",
		MAME_DIR .. "3rdparty/softfloat3/source/8086",
		MAME_DIR .. "3rdparty/softfloat3/bochs_ext"
	}

	configuration { "gmake or ninja" }
		buildoptions_cpp {
			"-x c++",
		}
if _OPTIONS["gcc"]~=nil and not string.find(_OPTIONS["gcc"], "clang") then
		buildoptions_cpp {
			"-Wno-error=implicit-fallthrough",
		}
end

	configuration { "vs*" }
		buildoptions {
			"/wd4701", -- warning C4701: potentially uninitialized local variable 'xxx' used
			"/wd4703", -- warning C4703: potentially uninitialized local pointer variable 'xxx' used
		}

	configuration { }

	defines {
		"SOFTFLOAT_ROUND_ODD",
		"INLINE_LEVEL=5",
		"SOFTFLOAT_FAST_DIV32TO16",
		"SOFTFLOAT_FAST_DIV64TO32",
		"SOFTFLOAT_FAST_INT64"
	}

if _OPTIONS["BIGENDIAN"]~="1" then
	defines {
		"LITTLEENDIAN=1"
	}
end
	files {
		MAME_DIR .. "3rdparty/softfloat3/source/s_eq128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_le128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_lt128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_shortShiftLeft128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_shortShiftRight128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_shortShiftRightJam64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_shortShiftRightJam64Extra.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_shortShiftRightJam128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_shortShiftRightJam128Extra.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_shiftRightJam32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_shiftRightJam64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_shiftRightJam64Extra.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_shiftRightJam128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_shiftRightJam128Extra.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_shiftRightJam256M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_countLeadingZeros8.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_countLeadingZeros16.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_countLeadingZeros32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_countLeadingZeros64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_add128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_add256M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_sub128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_sub256M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_mul64ByShifted32To128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_mul64To128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_mul128By32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_mul128To256M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_approxRecip_1Ks.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_approxRecip32_1.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_approxRecipSqrt_1Ks.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_approxRecipSqrt32_1.c",
		MAME_DIR .. "3rdparty/softfloat3/source/8086/softfloat_raiseFlags.c",
		MAME_DIR .. "3rdparty/softfloat3/source/8086/s_commonNaNToF16UI.c",
		MAME_DIR .. "3rdparty/softfloat3/source/8086/s_propagateNaNF16UI.c",
		MAME_DIR .. "3rdparty/softfloat3/source/8086/s_f32UIToCommonNaN.c",
		MAME_DIR .. "3rdparty/softfloat3/source/8086/s_commonNaNToF32UI.c",
		MAME_DIR .. "3rdparty/softfloat3/source/8086/s_propagateNaNF32UI.c",
		MAME_DIR .. "3rdparty/softfloat3/source/8086/s_f64UIToCommonNaN.c",
		MAME_DIR .. "3rdparty/softfloat3/source/8086/s_commonNaNToF64UI.c",
		MAME_DIR .. "3rdparty/softfloat3/source/8086/s_propagateNaNF64UI.c",
		MAME_DIR .. "3rdparty/softfloat3/source/8086/extF80M_isSignalingNaN.c",
		MAME_DIR .. "3rdparty/softfloat3/source/8086/s_extF80UIToCommonNaN.c",
		MAME_DIR .. "3rdparty/softfloat3/source/8086/s_commonNaNToExtF80UI.c",
		MAME_DIR .. "3rdparty/softfloat3/source/8086/s_propagateNaNExtF80UI.c",
		MAME_DIR .. "3rdparty/softfloat3/source/8086/f128M_isSignalingNaN.c",
		MAME_DIR .. "3rdparty/softfloat3/source/8086/s_f128UIToCommonNaN.c",
		MAME_DIR .. "3rdparty/softfloat3/source/8086/s_commonNaNToF128UI.c",
		MAME_DIR .. "3rdparty/softfloat3/source/8086/s_propagateNaNF128UI.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_roundToUI32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_roundToUI64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_roundToI32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_roundToI64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_normSubnormalF16Sig.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_roundPackToF16.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_normRoundPackToF16.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_addMagsF16.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_subMagsF16.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_mulAddF16.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_normSubnormalF32Sig.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_roundPackToF32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_normRoundPackToF32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_addMagsF32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_subMagsF32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_mulAddF32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_normSubnormalF64Sig.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_roundPackToF64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_normRoundPackToF64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_addMagsF64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_subMagsF64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_mulAddF64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_normSubnormalExtF80Sig.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_roundPackToExtF80.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_normRoundPackToExtF80.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_addMagsExtF80.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_subMagsExtF80.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_normSubnormalF128Sig.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_roundPackToF128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_normRoundPackToF128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_addMagsF128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_subMagsF128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/s_mulAddF128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/softfloat_state.c",
		MAME_DIR .. "3rdparty/softfloat3/source/ui32_to_f16.c",
		MAME_DIR .. "3rdparty/softfloat3/source/ui32_to_f32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/ui32_to_f64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/ui32_to_extF80.c",
		MAME_DIR .. "3rdparty/softfloat3/source/ui32_to_extF80M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/ui32_to_f128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/ui32_to_f128M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/ui64_to_f16.c",
		MAME_DIR .. "3rdparty/softfloat3/source/ui64_to_f32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/ui64_to_f64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/ui64_to_extF80.c",
		MAME_DIR .. "3rdparty/softfloat3/source/ui64_to_extF80M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/ui64_to_f128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/ui64_to_f128M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/i32_to_f16.c",
		MAME_DIR .. "3rdparty/softfloat3/source/i32_to_f32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/i32_to_f64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/i32_to_extF80.c",
		MAME_DIR .. "3rdparty/softfloat3/source/i32_to_extF80M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/i32_to_f128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/i32_to_f128M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/i64_to_f16.c",
		MAME_DIR .. "3rdparty/softfloat3/source/i64_to_f32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/i64_to_f64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/i64_to_extF80.c",
		MAME_DIR .. "3rdparty/softfloat3/source/i64_to_extF80M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/i64_to_f128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/i64_to_f128M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_to_ui32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_to_ui64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_to_i32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_to_i64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_to_ui32_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_to_ui64_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_to_i32_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_to_i64_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_to_f32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_to_f64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_to_extF80.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_to_extF80M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_to_f128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_to_f128M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_roundToInt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_add.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_sub.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_mul.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_mulAdd.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_div.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_rem.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_sqrt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_eq.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_le.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_lt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_eq_signaling.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_le_quiet.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_lt_quiet.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f16_isSignalingNaN.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_to_ui32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_to_ui64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_to_i32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_to_i64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_to_ui32_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_to_ui64_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_to_i32_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_to_i64_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_to_f16.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_to_f64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_to_extF80.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_to_extF80M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_to_f128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_to_f128M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_roundToInt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_add.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_sub.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_mul.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_mulAdd.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_div.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_rem.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_sqrt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_eq.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_le.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_lt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_eq_signaling.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_le_quiet.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_lt_quiet.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f32_isSignalingNaN.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_to_ui32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_to_ui64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_to_i32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_to_i64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_to_ui32_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_to_ui64_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_to_i32_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_to_i64_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_to_f16.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_to_f32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_to_extF80.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_to_extF80M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_to_f128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_to_f128M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_roundToInt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_add.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_sub.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_mul.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_mulAdd.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_div.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_rem.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_sqrt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_eq.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_le.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_lt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_eq_signaling.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_le_quiet.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_lt_quiet.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f64_isSignalingNaN.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_to_ui32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_to_ui64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_to_i32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_to_i64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_to_ui32_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_to_ui64_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_to_i32_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_to_i64_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_to_f16.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_to_f32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_to_f64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_to_f128.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_roundToInt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_add.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_sub.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_mul.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_div.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_rem.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_sqrt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_eq.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_le.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_lt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_eq_signaling.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_le_quiet.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_lt_quiet.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80_isSignalingNaN.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_to_ui32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_to_ui64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_to_i32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_to_i64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_to_ui32_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_to_ui64_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_to_i32_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_to_i64_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_to_f16.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_to_f32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_to_f64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_to_f128M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_roundToInt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_add.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_sub.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_mul.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_div.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_rem.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_sqrt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_eq.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_le.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_lt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_eq_signaling.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_le_quiet.c",
		MAME_DIR .. "3rdparty/softfloat3/source/extF80M_lt_quiet.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_to_ui32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_to_ui64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_to_i32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_to_i64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_to_ui32_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_to_ui64_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_to_i32_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_to_i64_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_to_f16.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_to_f32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_to_extF80.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_to_f64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_roundToInt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_add.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_sub.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_mul.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_mulAdd.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_div.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_rem.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_sqrt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_eq.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_le.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_lt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_eq_signaling.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_le_quiet.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_lt_quiet.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128_isSignalingNaN.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_to_ui32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_to_ui64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_to_i32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_to_i64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_to_ui32_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_to_ui64_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_to_i32_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_to_i64_r_minMag.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_to_f16.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_to_f32.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_to_extF80M.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_to_f64.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_roundToInt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_add.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_sub.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_mul.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_mulAdd.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_div.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_rem.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_sqrt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_eq.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_le.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_lt.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_eq_signaling.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_le_quiet.c",
		MAME_DIR .. "3rdparty/softfloat3/source/f128M_lt_quiet.c",
		MAME_DIR .. "3rdparty/softfloat3/bochs_ext/f2xm1.c",
		MAME_DIR .. "3rdparty/softfloat3/bochs_ext/fpatan.c",
		MAME_DIR .. "3rdparty/softfloat3/bochs_ext/fprem.c",
		MAME_DIR .. "3rdparty/softfloat3/bochs_ext/fsincos.c",
		MAME_DIR .. "3rdparty/softfloat3/bochs_ext/fyl2x.c",
		MAME_DIR .. "3rdparty/softfloat3/bochs_ext/poly.c",
		MAME_DIR .. "3rdparty/softfloat3/bochs_ext/extF80_scale.c",
	}


-------------------------------------------------
-- libJPEG library objects
--------------------------------------------------

if not _OPTIONS["with-system-jpeg"] then
project "jpeg"
	uuid "447c6800-dcfd-4c48-b72a-a8223bb409ca"
	kind "StaticLib"

	configuration { "vs*" }
		buildoptions {
			"/wd4100", -- warning C4100: 'xxx' : unreferenced formal parameter
			"/wd4127", -- warning C4127: conditional expression is constant
			"/wd4244", -- warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
		}
if _OPTIONS["vs"]=="intel-15" then
		buildoptions {
			"/Qwd869",              -- remark #869: parameter "xxx" was never referenced
		}
end

	configuration { }

	files {
		MAME_DIR .. "3rdparty/libjpeg/jaricom.c",
		MAME_DIR .. "3rdparty/libjpeg/jcapimin.c",
		MAME_DIR .. "3rdparty/libjpeg/jcapistd.c",
		MAME_DIR .. "3rdparty/libjpeg/jcarith.c",
		MAME_DIR .. "3rdparty/libjpeg/jccoefct.c",
		MAME_DIR .. "3rdparty/libjpeg/jccolor.c",
		MAME_DIR .. "3rdparty/libjpeg/jcdctmgr.c",
		MAME_DIR .. "3rdparty/libjpeg/jchuff.c",
		MAME_DIR .. "3rdparty/libjpeg/jcinit.c",
		MAME_DIR .. "3rdparty/libjpeg/jcmainct.c",
		MAME_DIR .. "3rdparty/libjpeg/jcmarker.c",
		MAME_DIR .. "3rdparty/libjpeg/jcmaster.c",
		MAME_DIR .. "3rdparty/libjpeg/jcomapi.c",
		MAME_DIR .. "3rdparty/libjpeg/jcparam.c",
		MAME_DIR .. "3rdparty/libjpeg/jcprepct.c",
		MAME_DIR .. "3rdparty/libjpeg/jcsample.c",
		MAME_DIR .. "3rdparty/libjpeg/jctrans.c",
		MAME_DIR .. "3rdparty/libjpeg/jdapimin.c",
		MAME_DIR .. "3rdparty/libjpeg/jdapistd.c",
		MAME_DIR .. "3rdparty/libjpeg/jdarith.c",
		MAME_DIR .. "3rdparty/libjpeg/jdatadst.c",
		MAME_DIR .. "3rdparty/libjpeg/jdatasrc.c",
		MAME_DIR .. "3rdparty/libjpeg/jdcoefct.c",
		MAME_DIR .. "3rdparty/libjpeg/jdcolor.c",
		MAME_DIR .. "3rdparty/libjpeg/jddctmgr.c",
		MAME_DIR .. "3rdparty/libjpeg/jdhuff.c",
		MAME_DIR .. "3rdparty/libjpeg/jdinput.c",
		MAME_DIR .. "3rdparty/libjpeg/jdmainct.c",
		MAME_DIR .. "3rdparty/libjpeg/jdmarker.c",
		MAME_DIR .. "3rdparty/libjpeg/jdmaster.c",
		MAME_DIR .. "3rdparty/libjpeg/jdmerge.c",
		MAME_DIR .. "3rdparty/libjpeg/jdpostct.c",
		MAME_DIR .. "3rdparty/libjpeg/jdsample.c",
		MAME_DIR .. "3rdparty/libjpeg/jdtrans.c",
		MAME_DIR .. "3rdparty/libjpeg/jerror.c",
		MAME_DIR .. "3rdparty/libjpeg/jfdctflt.c",
		MAME_DIR .. "3rdparty/libjpeg/jfdctfst.c",
		MAME_DIR .. "3rdparty/libjpeg/jfdctint.c",
		MAME_DIR .. "3rdparty/libjpeg/jidctflt.c",
		MAME_DIR .. "3rdparty/libjpeg/jidctfst.c",
		MAME_DIR .. "3rdparty/libjpeg/jidctint.c",
		MAME_DIR .. "3rdparty/libjpeg/jquant1.c",
		MAME_DIR .. "3rdparty/libjpeg/jquant2.c",
		MAME_DIR .. "3rdparty/libjpeg/jutils.c",
		MAME_DIR .. "3rdparty/libjpeg/jmemmgr.c",
		MAME_DIR .. "3rdparty/libjpeg/jmemansi.c",
	}
else
links {
	ext_lib("jpeg"),
}
end


--------------------------------------------------
-- libflac library objects
--------------------------------------------------

if not _OPTIONS["with-system-flac"] then
project "flac"
	uuid "b6fc19e8-073a-4541-bb7b-d24b548d424a"
	kind "StaticLib"

	configuration { "vs*" }
		buildoptions {
			"/wd4127", -- warning C4127: conditional expression is constant
			"/wd4244", -- warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
			"/wd4100", -- warning C4100: 'xxx' : unreferenced formal parameter
			"/wd4456", -- warning C4456: declaration of 'xxx' hides previous local declaration
			"/wd4702", -- warning C4702: unreachable code
		}
if _OPTIONS["vs"]=="intel-15" then
		buildoptions {
			"/Qwd111",              -- remark #111: statement is unreachable
			"/Qwd177",              -- remark #177: function "xxx" was declared but never referenced
			"/Qwd181",              -- remark #181: argument of type "UINT32={unsigned int}" is incompatible with format "%d", expecting argument of type "int"
			"/Qwd188",              -- error #188: enumerated type mixed with another type
			"/Qwd869",              -- remark #869: parameter "xxx" was never referenced
		}
end

	configuration { "mingw*" }
		defines {
			"HAVE_FSEEKO",
		}

	configuration { }
		defines {
			"NDEBUG", -- always enable this to avoid debug log spam on compression
			"HAVE_CONFIG_H", -- mostly because PACKAGE_VERSION is a pain to do otherwise
			"ENABLE_64_BIT_WORDS=1",
			"OGG_FOUND=0",
			"FLAC__HAS_OGG=0",
			"HAVE_LROUND=1",
			"HAVE_INTTYPES_H",
			"HAVE_STDBOOL_H",
			"HAVE_STDINT_H",
			"HAVE_STDIO_H",
			"HAVE_STDLIB_H",
			"HAVE_STRING_H",
			"_FILE_OFFSET_BITS=64",
			"_LARGEFILE_SOURCE",
		}

		if _OPTIONS["gcc"]~=nil then
			defines {
				"HAVE_BSWAP16",
				"HAVE_BSWAP32",
			}
		end

		if _OPTIONS["BIGENDIAN"]=="1" then
			defines {
				"CPU_IS_BIG_ENDIAN=1",
				"CPU_IS_LITTLE_ENDIAN=0",
				"WORDS_BIGENDIAN=1",
			}
		else
			defines {
				"CPU_IS_BIG_ENDIAN=0",
				"CPU_IS_LITTLE_ENDIAN=1",
				"WORDS_BIGENDIAN=0",
			}
		end

		if _OPTIONS["targetos"]=="macosx" then
			defines {
				"FLAC__SYS_DARWIN",
			}
		elseif _OPTIONS["targetos"]=="linux" then
			defines {
				"FLAC__SYS_LINUX",
			}
		end


	configuration { "gmake or ninja" }
		buildoptions_c {
			"-Wno-error=bad-function-cast",
			"-Wno-error=unused-function",
		}
	if _OPTIONS["gcc"]~=nil and (string.find(_OPTIONS["gcc"], "clang") or string.find(_OPTIONS["gcc"], "android")) then
		buildoptions {
			"-Wno-enum-conversion",
		}
		if _OPTIONS["targetos"]=="macosx" then
			buildoptions_c {
				"-Wno-unknown-attributes",
			}
		end
	end
	configuration { }

	includedirs {
		MAME_DIR .. "3rdparty/flac/src/libFLAC/include",
		MAME_DIR .. "3rdparty/flac/include",
	}

	files {
		MAME_DIR .. "3rdparty/flac/src/libFLAC/bitmath.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/bitreader.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/bitwriter.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/cpu.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/crc.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/fixed.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/fixed_intrin_avx2.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/fixed_intrin_sse2.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/fixed_intrin_sse42.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/fixed_intrin_ssse3.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/float.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/format.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/lpc.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/lpc_intrin_avx2.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/lpc_intrin_fma.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/lpc_intrin_neon.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/lpc_intrin_sse2.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/lpc_intrin_sse41.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/md5.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/memory.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/stream_decoder.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/stream_encoder.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/stream_encoder_framing.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/stream_encoder_intrin_avx2.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/stream_encoder_intrin_sse2.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/stream_encoder_intrin_ssse3.c",
		MAME_DIR .. "3rdparty/flac/src/libFLAC/window.c",
	}

	if _OPTIONS["targetos"]=="windows" then
		files {
			MAME_DIR .. "3rdparty/flac/src/share/win_utf8_io/win_utf8_io.c",
		}
	end
else
links {
	ext_lib("flac"),
}
end


--------------------------------------------------
-- lib7z library objects
--------------------------------------------------

project "7z"
	uuid "ad573d62-e76a-4b11-ae34-5110a6789a42"
	kind "StaticLib"

	configuration { "gmake or ninja" }
		buildoptions_c {
			"-Wno-error=undef",
		}
if _OPTIONS["gcc"]~=nil then
	if string.find(_OPTIONS["gcc"], "clang") then

	else
		if str_to_version(_OPTIONS["gcc_version"]) >= 130000 then
			buildoptions_c {
				"-Wno-error=dangling-pointer",
			}
		end
	end
end

	configuration { "vs*" }
		buildoptions {
			"/wd4100", -- warning C4100: 'xxx' : unreferenced formal parameter
			"/wd4456", -- warning C4456: declaration of 'xxx' hides previous local declaration
			"/wd4457", -- warning C4457: declaration of 'xxx' hides function parameter
		}
if _OPTIONS["vs"]=="intel-15" then
		buildoptions {
			"/Qwd869",              -- remark #869: parameter "xxx" was never referenced
		}
end
	configuration { }
		defines {
			"Z7_PPMD_SUPPPORT",
			"Z7_ST",
		}

	files {
			MAME_DIR .. "3rdparty/lzma/C/7zAlloc.c",
			MAME_DIR .. "3rdparty/lzma/C/7zArcIn.c",
			MAME_DIR .. "3rdparty/lzma/C/7zBuf.c",
			MAME_DIR .. "3rdparty/lzma/C/7zBuf2.c",
			MAME_DIR .. "3rdparty/lzma/C/7zCrc.c",
			MAME_DIR .. "3rdparty/lzma/C/7zCrcOpt.c",
			MAME_DIR .. "3rdparty/lzma/C/7zDec.c",
			MAME_DIR .. "3rdparty/lzma/C/7zFile.c",
			MAME_DIR .. "3rdparty/lzma/C/7zStream.c",
			MAME_DIR .. "3rdparty/lzma/C/Aes.c",
			MAME_DIR .. "3rdparty/lzma/C/AesOpt.c",
			MAME_DIR .. "3rdparty/lzma/C/Alloc.c",
			MAME_DIR .. "3rdparty/lzma/C/Bcj2.c",
			-- MAME_DIR .. "3rdparty/lzma/C/Bcj2Enc.c",
			MAME_DIR .. "3rdparty/lzma/C/Bra.c",
			MAME_DIR .. "3rdparty/lzma/C/Bra86.c",
			MAME_DIR .. "3rdparty/lzma/C/BraIA64.c",
			MAME_DIR .. "3rdparty/lzma/C/CpuArch.c",
			MAME_DIR .. "3rdparty/lzma/C/Delta.c",
			-- MAME_DIR .. "3rdparty/lzma/C/DllSecur.c",
			MAME_DIR .. "3rdparty/lzma/C/LzFind.c",
			-- MAME_DIR .. "3rdparty/lzma/C/LzFindMt.c",
			MAME_DIR .. "3rdparty/lzma/C/LzFindOpt.c",
			MAME_DIR .. "3rdparty/lzma/C/Lzma2Dec.c",
			-- MAME_DIR .. "3rdparty/lzma/C/Lzma2DecMt.c",
			-- MAME_DIR .. "3rdparty/lzma/C/Lzma2Enc.c",
			-- MAME_DIR .. "3rdparty/lzma/C/Lzma86Dec.c",
			-- MAME_DIR .. "3rdparty/lzma/C/Lzma86Enc.c",
			MAME_DIR .. "3rdparty/lzma/C/LzmaDec.c",
			MAME_DIR .. "3rdparty/lzma/C/LzmaEnc.c",
			-- MAME_DIR .. "3rdparty/lzma/C/LzmaLib.c",
			-- MAME_DIR .. "3rdparty/lzma/C/MtCoder.c",
			-- MAME_DIR .. "3rdparty/lzma/C/MtDec.c",
			MAME_DIR .. "3rdparty/lzma/C/Ppmd7.c",
			MAME_DIR .. "3rdparty/lzma/C/Ppmd7Dec.c",
			-- MAME_DIR .. "3rdparty/lzma/C/Ppmd7Enc.c",
			MAME_DIR .. "3rdparty/lzma/C/Sha256.c",
			MAME_DIR .. "3rdparty/lzma/C/Sha256Opt.c",
			MAME_DIR .. "3rdparty/lzma/C/Sort.c",
			-- MAME_DIR .. "3rdparty/lzma/C/SwapBytes.c",
			-- MAME_DIR .. "3rdparty/lzma/C/Threads.c",
			-- MAME_DIR .. "3rdparty/lzma/C/Xz.c",
			-- MAME_DIR .. "3rdparty/lzma/C/XzCrc64.c",
			-- MAME_DIR .. "3rdparty/lzma/C/XzCrc64Opt.c",
			-- MAME_DIR .. "3rdparty/lzma/C/XzDec.c",
			-- MAME_DIR .. "3rdparty/lzma/C/XzEnc.c",
			-- MAME_DIR .. "3rdparty/lzma/C/XzIn.c",
		}


--------------------------------------------------
-- LUA library objects
--------------------------------------------------
if (STANDALONE~=true) then

if not _OPTIONS["with-system-lua"] then
project "lua"
	uuid "d9e2eed1-f1ab-4737-a6ac-863700b1a5a9"
	kind "StaticLib"

	options {
		"ForceCPP",
	}

	configuration { "gmake or ninja" }
		buildoptions_cpp {
			"-x c++",
		}

	configuration { "vs*" }
		buildoptions {
			"/wd4244", -- warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
			"/wd4702", -- warning C4702: unreachable code
			"/wd4310", -- warning C4310: cast truncates constant value
		}
if _OPTIONS["vs"]=="intel-15" then
		buildoptions {
			"/Qwd592", -- error #592: variable "xxx" is used before its value is set
		}
end

	configuration { }
		defines {
			"LUA_COMPAT_ALL",
			"LUA_COMPAT_5_1",
			"LUA_COMPAT_5_2",
		}
	if not (_OPTIONS["targetos"]=="windows") and not (_OPTIONS["targetos"]=="asmjs") then
		defines {
			"LUA_USE_POSIX",
		}
	end

	configuration { }

	includedirs {
		MAME_DIR .. "3rdparty",
	}

	files {
		MAME_DIR .. "3rdparty/lua/src/lapi.c",
		MAME_DIR .. "3rdparty/lua/src/lcode.c",
		MAME_DIR .. "3rdparty/lua/src/lctype.c",
		MAME_DIR .. "3rdparty/lua/src/ldebug.c",
		MAME_DIR .. "3rdparty/lua/src/ldo.c",
		MAME_DIR .. "3rdparty/lua/src/ldump.c",
		MAME_DIR .. "3rdparty/lua/src/lfunc.c",
		MAME_DIR .. "3rdparty/lua/src/lgc.c",
		MAME_DIR .. "3rdparty/lua/src/llex.c",
		MAME_DIR .. "3rdparty/lua/src/lmem.c",
		MAME_DIR .. "3rdparty/lua/src/lobject.c",
		MAME_DIR .. "3rdparty/lua/src/lopcodes.c",
		MAME_DIR .. "3rdparty/lua/src/lparser.c",
		MAME_DIR .. "3rdparty/lua/src/lstate.c",
		MAME_DIR .. "3rdparty/lua/src/lstring.c",
		MAME_DIR .. "3rdparty/lua/src/ltable.c",
		MAME_DIR .. "3rdparty/lua/src/ltm.c",
		MAME_DIR .. "3rdparty/lua/src/lundump.c",
		MAME_DIR .. "3rdparty/lua/src/lvm.c",
		MAME_DIR .. "3rdparty/lua/src/lzio.c",
		MAME_DIR .. "3rdparty/lua/src/lauxlib.c",
		MAME_DIR .. "3rdparty/lua/src/lbaselib.c",
		MAME_DIR .. "3rdparty/lua/src/lcorolib.c",
		MAME_DIR .. "3rdparty/lua/src/ldblib.c",
		MAME_DIR .. "3rdparty/lua/src/liolib.c",
		MAME_DIR .. "3rdparty/lua/src/lmathlib.c",
		MAME_DIR .. "3rdparty/lua/src/loslib.c",
		MAME_DIR .. "3rdparty/lua/src/lstrlib.c",
		MAME_DIR .. "3rdparty/lua/src/ltablib.c",
		MAME_DIR .. "3rdparty/lua/src/loadlib.c",
		MAME_DIR .. "3rdparty/lua/src/linit.c",
		MAME_DIR .. "3rdparty/lua/src/lutf8lib.c",
	}
else
links {
	ext_lib("lua"),
}
end


--------------------------------------------------
-- small lua library objects
--------------------------------------------------

project "lualibs"
	uuid "1d84edab-94cf-48fb-83ee-b75bc697660e"
	kind "StaticLib"

	options {
		"ForceCPP",
	}

	configuration { "gmake or ninja" }
		buildoptions {
			"-Wno-error=unused-variable",
		}
		buildoptions_cpp {
			"-x c++",
		}

	configuration { "vs*" }
		buildoptions {
			"/wd4244", -- warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
			"/wd4055", -- warning C4055: 'type cast': from data pointer 'void *' to function pointer 'xxx'
			"/wd4152", -- warning C4152: nonstandard extension, function/data pointer conversion in expression
			"/wd4130", -- warning C4130: '==': logical operation on address of string constant
		}

	configuration { }
		defines {
			"LUA_COMPAT_ALL",
		}

	includedirs {
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/linenoise",
	}

	includedirs {
		ext_includedir("lua"),
		ext_includedir("zlib"),
		ext_includedir("sqlite3"),
	}

	configuration {}

	files {
		MAME_DIR .. "3rdparty/lsqlite3/lsqlite3.c",
		MAME_DIR .. "3rdparty/lua-zlib/lua_zlib.c",
		MAME_DIR .. "3rdparty/luafilesystem/src/lfs.c",
		MAME_DIR .. "3rdparty/lua-linenoise/linenoise.c",
	}

--------------------------------------------------
-- SQLite3 library objects
--------------------------------------------------

if not _OPTIONS["with-system-sqlite3"] then
project "sqlite3"
	uuid "5cb3d495-57ed-461c-81e5-80dc0857517d"
	kind "StaticLib"

	configuration { "gmake or ninja" }
		buildoptions_c {
			"-Wno-bad-function-cast",
			"-Wno-discarded-qualifiers",
			"-Wno-undef",
			"-Wno-unused-but-set-variable",
			"-Wno-unused-variable",
		}
if _OPTIONS["gcc"]~=nil then
	if string.find(_OPTIONS["gcc"], "clang") or string.find(_OPTIONS["gcc"], "asmjs") or string.find(_OPTIONS["gcc"], "android") then
		buildoptions_c {
			"-Wno-incompatible-pointer-types-discards-qualifiers",
		}
	else
		buildoptions_c {
			"-Wno-return-local-addr", -- sqlite3.c in GCC 10
			"-Wno-misleading-indentation",  -- sqlite3.c in GCC 11.1
		}
	end
end
	configuration { "vs*" }
if _OPTIONS["vs"]=="clangcl" then
		buildoptions {
			"-Wno-implicit-int-float-conversion",
		}
end

	configuration { }

	files {
		MAME_DIR .. "3rdparty/sqlite3/sqlite3.c",
	}
else
links {
	ext_lib("sqlite3"),
}
end

end


--------------------------------------------------
-- portmidi library objects
--------------------------------------------------

if _OPTIONS["NO_USE_MIDI"]~="1" then
if not _OPTIONS["with-system-portmidi"] then
project "portmidi"
	uuid "587f2da6-3274-4a65-86a2-f13ea315bb98"
	kind "StaticLib"

	includedirs {
		MAME_DIR .. "3rdparty/portmidi/pm_common",
		MAME_DIR .. "3rdparty/portmidi/porttime",
	}

	configuration { "vs*" }
		buildoptions {
			"/wd4100", -- warning C4100: 'xxx' : unreferenced formal parameter
			"/wd4127", -- warning C4127: conditional expression is constant
			"/wd4244", -- warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
			"/wd4456", -- warning C4456: declaration of 'xxx' hides previous local declaration
			"/wd4706", -- warning C4706: assignment within conditional expression
		}
if _OPTIONS["vs"]=="intel-15" then
		buildoptions {
			"/Qwd188",              -- error #188: enumerated type mixed with another type
			"/Qwd344",              -- remark #344: typedef name has already been declared (with same type)
			"/Qwd869",              -- remark #869: parameter "xxx" was never referenced
			"/Qwd2557",             -- remark #2557: comparison between signed and unsigned operands
		}
end

	configuration { "linux*" }
		defines {
			"PMALSA=1",
		}

	configuration { }

	if _OPTIONS["gcc"]~=nil and string.find(_OPTIONS["gcc"], "clang") and str_to_version(_OPTIONS["gcc_version"]) >= 150000 then
		buildoptions_c {
			"-Wno-strict-prototypes",
		}
	end

	files {
		MAME_DIR .. "3rdparty/portmidi/pm_common/portmidi.c",
		MAME_DIR .. "3rdparty/portmidi/pm_common/pmutil.c",
	}

	if _OPTIONS["targetos"]=="windows" then
		files {
			MAME_DIR .. "3rdparty/portmidi/porttime/ptwinmm.c",
			MAME_DIR .. "3rdparty/portmidi/pm_win/pmwin.c",
			MAME_DIR .. "3rdparty/portmidi/pm_win/pmwinmm.c",
			MAME_DIR .. "3rdparty/portmidi/porttime/ptwinmm.c",
		}
	end

	if _OPTIONS["targetos"]=="linux" then
		files {
			MAME_DIR .. "3rdparty/portmidi/pm_linux/pmlinux.c",
			MAME_DIR .. "3rdparty/portmidi/pm_linux/pmlinuxalsa.c",
			MAME_DIR .. "3rdparty/portmidi/pm_linux/finddefault.c",
			MAME_DIR .. "3rdparty/portmidi/porttime/ptlinux.c",
		}
	end
	if _OPTIONS["targetos"]=="netbsd" then
		files {
			MAME_DIR .. "3rdparty/portmidi/pm_linux/pmlinux.c",
			MAME_DIR .. "3rdparty/portmidi/pm_linux/finddefault.c",
			MAME_DIR .. "3rdparty/portmidi/porttime/ptlinux.c",
		}
	end
	if _OPTIONS["targetos"]=="macosx" then
		files {
			MAME_DIR .. "3rdparty/portmidi/pm_mac/pmmac.c",
			MAME_DIR .. "3rdparty/portmidi/pm_mac/pmmacosxcm.c",
			MAME_DIR .. "3rdparty/portmidi/pm_mac/finddefault.c",
			MAME_DIR .. "3rdparty/portmidi/pm_mac/readbinaryplist.c",
			MAME_DIR .. "3rdparty/portmidi/pm_mac/osxsupport.m",
			MAME_DIR .. "3rdparty/portmidi/porttime/ptmacosx_mach.c",
		}
	end
else
links {
	ext_lib("portmidi"),
}
end
end


--------------------------------------------------
-- BX library objects
--------------------------------------------------

project "bx"
	uuid "238318fe-49f5-4eb4-88be-0618900f5eac"
	kind "StaticLib"

	defines {
		"__STDC_LIMIT_MACROS",
		"__STDC_FORMAT_MACROS",
		"__STDC_CONSTANT_MACROS",
		"BX_CONFIG_DEBUG=0",
	}

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

	configuration { "freebsd" }
		includedirs {
			MAME_DIR .. "3rdparty/bx/include/compat/freebsd",
		}

	configuration { "netbsd" }
		includedirs {
			MAME_DIR .. "3rdparty/bx/include/compat/freebsd",
		}

	configuration { "linux*" }
		includedirs {
			MAME_DIR .. "3rdparty/bx/include/compat/linux",
		}

	configuration { }

	if _OPTIONS["targetos"]=="macosx" or _OPTIONS["targetos"]=="linux" or _OPTIONS["targetos"]=="windows" or _OPTIONS["targetos"]=="asmjs" then
		if _OPTIONS["gcc"]~=nil and (string.find(_OPTIONS["gcc"], "clang") or string.find(_OPTIONS["gcc"], "asmjs")) then
			buildoptions_cpp {
				"-Wno-unused-private-field",
			}
		end
	end

	includedirs {
		MAME_DIR .. "3rdparty/bx/include",
		MAME_DIR .. "3rdparty/bx/3rdparty",
	}

	files {
		MAME_DIR .. "3rdparty/bx/src/allocator.cpp",
		MAME_DIR .. "3rdparty/bx/src/bx.cpp",
		MAME_DIR .. "3rdparty/bx/src/commandline.cpp",
		MAME_DIR .. "3rdparty/bx/src/crtnone.cpp",
		MAME_DIR .. "3rdparty/bx/src/debug.cpp",
		MAME_DIR .. "3rdparty/bx/src/dtoa.cpp",
		MAME_DIR .. "3rdparty/bx/src/easing.cpp",
		MAME_DIR .. "3rdparty/bx/src/file.cpp",
		MAME_DIR .. "3rdparty/bx/src/filepath.cpp",
		MAME_DIR .. "3rdparty/bx/src/hash.cpp",
		MAME_DIR .. "3rdparty/bx/src/math.cpp",
		MAME_DIR .. "3rdparty/bx/src/mutex.cpp",
		MAME_DIR .. "3rdparty/bx/src/os.cpp",
		MAME_DIR .. "3rdparty/bx/src/process.cpp",
		MAME_DIR .. "3rdparty/bx/src/semaphore.cpp",
		MAME_DIR .. "3rdparty/bx/src/settings.cpp",
		MAME_DIR .. "3rdparty/bx/src/sort.cpp",
		MAME_DIR .. "3rdparty/bx/src/string.cpp",
		MAME_DIR .. "3rdparty/bx/src/thread.cpp",
		MAME_DIR .. "3rdparty/bx/src/timer.cpp",
		MAME_DIR .. "3rdparty/bx/src/url.cpp",
	}


--------------------------------------------------
-- BIMG library objects
--------------------------------------------------

project "bimg"
	uuid "5603611b-8bf8-4ffd-85bc-76858cd7df39"
	kind "StaticLib"

	includedirs {
		MAME_DIR .. "3rdparty/bx/include",
	}

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

	configuration { "freebsd" }
		includedirs {
			MAME_DIR .. "3rdparty/bx/include/compat/freebsd",
		}

	configuration { "netbsd" }
		includedirs {
			MAME_DIR .. "3rdparty/bx/include/compat/freebsd",
		}

	configuration { "linux*" }
		includedirs {
			MAME_DIR .. "3rdparty/bx/include/compat/linux",
		}

	configuration { "gmake or ninja" }
		buildoptions {
			"-Wno-unused-but-set-variable",
			"-Wno-undef",
		}
		buildoptions_c {
			"-Wno-strict-prototypes",
		}

	configuration { }

	if _OPTIONS["targetos"]=="macosx" or _OPTIONS["targetos"]=="linux" or _OPTIONS["targetos"]=="windows" or _OPTIONS["targetos"]=="asmjs" then
		if _OPTIONS["gcc"]~=nil and (string.find(_OPTIONS["gcc"], "clang") or string.find(_OPTIONS["gcc"], "asmjs")) then
			buildoptions_cpp {
				"-Wno-unused-const-variable",
			}
		end
	end

	defines {
		"__STDC_LIMIT_MACROS",
		"__STDC_FORMAT_MACROS",
		"__STDC_CONSTANT_MACROS",
		"BX_CONFIG_DEBUG=0",
	}

	configuration { "x64", "mingw*", "not arm64" }
		defines {
			"ASTCENC_AVX=0",
			"ASTCENC_SSE=20",
		}
	configuration { }

	configuration { "x32" }
		defines {
			"ASTCENC_AVX=0",
			"ASTCENC_SSE=0", -- assumes x86-64 ALU is present when using SSE
		}
	configuration { }

	includedirs {
		MAME_DIR .. "3rdparty/bimg/include",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/include",
		MAME_DIR .. "3rdparty/bimg/3rdparty/tinyexr/deps/miniz",
	}

	files {
		MAME_DIR .. "3rdparty/bimg/src/image.cpp",
		MAME_DIR .. "3rdparty/bimg/src/image_gnf.cpp",

		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_averages_and_directions.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_block_sizes.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_color_quantize.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_color_unquantize.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_compress_symbolic.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_compute_variance.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_decompress_symbolic.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_diagnostic_trace.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_entry.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_find_best_partitioning.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_ideal_endpoints_and_weights.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_image.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_integer_sequence.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_mathlib.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_mathlib_softfloat.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_partition_tables.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_percentile_tables.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_pick_best_endpoint_format.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_platform_isa_detection.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_quantization.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_symbolic_physical.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_weight_align.cpp",
		MAME_DIR .. "3rdparty/bimg/3rdparty/astc-encoder/source/astcenc_weight_quant_xfer_tables.cpp",

		MAME_DIR .. "3rdparty/bimg/3rdparty/tinyexr/deps/miniz/miniz.c",
	}


--------------------------------------------------
-- BGFX library objects
--------------------------------------------------

project "bgfx"
	uuid "d3e7e119-35cf-4f4f-aba0-d3bdcd1b879a"
	kind "StaticLib"

	configuration { "vs*" }
		buildoptions {
			"/wd4324", -- warning C4324: 'xxx' : structure was padded due to __declspec(align())
			"/wd4244", -- warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
			"/wd4611", -- warning C4611: interaction between '_setjmp' and C++ object destruction is non-portable
			"/wd4310", -- warning C4310: cast truncates constant value
			"/wd4701", -- warning C4701: potentially uninitialized local variable 'xxx' used
		}

if _OPTIONS["vs"]=="intel-15" then
		buildoptions {
			"/Qwd906",              -- message #906: effect of this "#pragma pack" directive is local to function "xxx"
			"/Qwd1879",             -- warning #1879: unimplemented pragma ignored
			"/Qwd82",               -- remark #82: storage class is not first
		}
end
	configuration { }

	includedirs {
		MAME_DIR .. "3rdparty/bgfx/include",
		MAME_DIR .. "3rdparty/bgfx/3rdparty",
		MAME_DIR .. "3rdparty/bx/include",
		MAME_DIR .. "3rdparty/bimg/include",
		MAME_DIR .. "3rdparty/bgfx/3rdparty/directx-headers/include/directx",
		MAME_DIR .. "3rdparty/bgfx/3rdparty/khronos",
	}

	configuration { "android-*"}
		buildoptions {
			"-Wno-macro-redefined",
		}

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

	configuration { "freebsd" }
		includedirs {
			MAME_DIR .. "3rdparty/bx/include/compat/freebsd",
		}

	configuration { "netbsd" }
		includedirs {
			MAME_DIR .. "3rdparty/bx/include/compat/freebsd",
		}

	configuration { "linux*", "not mingw*" }
		includedirs {
			MAME_DIR .. "3rdparty/bgfx/3rdparty/directx-headers/include/wsl/stubs",
			MAME_DIR .. "3rdparty/bx/include/compat/linux",
		}

	configuration { "gmake or ninja" }
		buildoptions {
			"-Wno-uninitialized",
			"-Wno-unused-but-set-variable",
			"-Wno-unused-function",
			"-Wno-unused-variable",
			"-Wno-tautological-compare",
		}

	configuration { }

	local version = str_to_version(_OPTIONS["gcc_version"])
	if _OPTIONS["gcc"]~=nil and string.find(_OPTIONS["gcc"], "gcc") then
		if version >= 60000 then
			buildoptions_cpp {
				"-Wno-misleading-indentation",
			}
		end
	end

	if _OPTIONS["targetos"]=="windows" then
		if _OPTIONS["gcc"]~=nil and string.find(_OPTIONS["gcc"], "clang") then
			buildoptions {
				"-Wno-unknown-attributes",
				"-Wno-missing-braces",
				"-Wno-int-to-pointer-cast",
				"-Wno-ignored-attributes", -- many instances in ImGui
			}
		end
	end

	if _OPTIONS["targetos"]=="macosx" or  _OPTIONS["targetos"]=="linux" then
		if _OPTIONS["gcc"]~=nil and string.find(_OPTIONS["gcc"], "clang") then
			buildoptions {
				"-Wno-switch",
			}
			buildoptions_cpp {
				"-Wno-unknown-pragmas",
			}
		end
	end

	if _OPTIONS["targetos"]=="freebsd" then
		buildoptions {
			backtick(pkgconfigcmd() .. " --cflags gl")
		}
	end

	defines {
		"__STDC_LIMIT_MACROS",
		"__STDC_FORMAT_MACROS",
		"__STDC_CONSTANT_MACROS",
		"BX_CONFIG_DEBUG=0",
		"BGFX_CONFIG_MAX_FRAME_BUFFERS=128",
	}

	if _OPTIONS["targetos"]=="linux" or _OPTIONS["targetos"]=="netbsd" or _OPTIONS["targetos"]=="openbsd" then
		if _OPTIONS["NO_X11"]=="1" then
			defines {
				"BGFX_CONFIG_RENDERER_OPENGLES=1",
				"BGFX_CONFIG_RENDERER_OPENGL=0",
			}
		end
		if _OPTIONS["USE_WAYLAND"]=="1" then
			defines {
				"WL_EGL_PLATFORM=1",
			}
			buildoptions {
				backtick(pkgconfigcmd() .. " --cflags wayland-egl-backend"),
			}
		end
	end

	if _OPTIONS["targetos"]=="macosx" and _OPTIONS["gcc"]~=nil then
		if string.find(_OPTIONS["gcc"], "clang") and (version < 80000) then
			defines {
				"TARGET_OS_OSX=1",
			}
		end
	end

	files {
		MAME_DIR .. "3rdparty/bgfx/src/bgfx.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/debug_renderdoc.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/dxgi.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/glcontext_egl.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/glcontext_html5.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/glcontext_wgl.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/nvapi.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/renderer_agc.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/renderer_d3d11.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/renderer_d3d12.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/renderer_d3d9.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/renderer_gl.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/renderer_gnm.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/renderer_noop.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/renderer_nvn.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/renderer_vk.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/renderer_webgpu.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/shader.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/shader_dx9bc.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/shader_dxbc.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/shader_spirv.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/topology.cpp",
		MAME_DIR .. "3rdparty/bgfx/src/vertexlayout.cpp",
		MAME_DIR .. "3rdparty/bgfx/examples/common/imgui/imgui.cpp",
		MAME_DIR .. "3rdparty/bgfx/examples/common/nanovg/nanovg.cpp",
		MAME_DIR .. "3rdparty/bgfx/examples/common/nanovg/nanovg_bgfx.cpp",
		MAME_DIR .. "3rdparty/bgfx/3rdparty/dear-imgui/imgui.cpp",
		MAME_DIR .. "3rdparty/bgfx/3rdparty/dear-imgui/imgui_draw.cpp",
		MAME_DIR .. "3rdparty/bgfx/3rdparty/dear-imgui/imgui_tables.cpp",
		MAME_DIR .. "3rdparty/bgfx/3rdparty/dear-imgui/imgui_widgets.cpp",
	}
	if _OPTIONS["targetos"]=="macosx" then
		files {
			MAME_DIR .. "3rdparty/bgfx/src/glcontext_eagl.mm",
			MAME_DIR .. "3rdparty/bgfx/src/glcontext_nsgl.mm",
			MAME_DIR .. "3rdparty/bgfx/src/renderer_mtl.mm",
		}
		buildoptions {
			"-x objective-c++",
			"-D BGFX_CONFIG_MULTITHREADED=0",
		}
	end


--------------------------------------------------
-- PortAudio library objects
--------------------------------------------------

if _OPTIONS["NO_USE_PORTAUDIO"]~="1" then
if not _OPTIONS["with-system-portaudio"] then
project "portaudio"
	uuid "0755c5f5-eccf-47f3-98a9-df67018a94d4"
	kind "StaticLib"

	configuration { "vs*" }
		buildoptions {
			"/wd4245", -- warning C4245: 'conversion' : conversion from 'type1' to 'type2', signed/unsigned mismatch
			"/wd4244", -- warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
			"/wd4100", -- warning C4100: 'xxx' : unreferenced formal parameter
			"/wd4389", -- warning C4389: 'operator' : signed/unsigned mismatch
			"/wd4189", -- warning C4189: 'xxx' : local variable is initialized but not referenced
			"/wd4127", -- warning C4127: conditional expression is constant
			"/wd4456", -- warning C4456: declaration of 'xxx' hides previous local declaration
			"/wd4312", -- warning C4312: 'type cast': conversion from 'UINT' to 'HWAVEIN' of greater size
		}
	if _OPTIONS["vs"]=="clangcl" then
		buildoptions {
			"-Wno-implicit-const-int-float-conversion",
			"-Wno-sometimes-uninitialized",
			"-Wno-unused-but-set-variable",
		}
	end
	if _OPTIONS["vs"]=="intel-15" then
		buildoptions {
			"/Qwd869",              -- remark #869: parameter "xxx" was never referenced
			"/Qwd1478",             -- warning #1478: function "xxx" (declared at line yyy of "zzz") was declared deprecated
			"/Qwd2544",             -- message #2544: empty dependent statement in if-statement
			"/Qwd1879",             -- warning #1879: unimplemented pragma ignored
		}
	end

	configuration { "gmake or ninja" }
		buildoptions_c {
			"-Wno-bad-function-cast",
			"-Wno-missing-braces",
			"-Wno-strict-prototypes",
			"-Wno-undef",
			"-Wno-unknown-pragmas",
			"-Wno-unused-function",
			"-Wno-unused-value",
			"-Wno-unused-variable",
		}

	local version = str_to_version(_OPTIONS["gcc_version"])
	if (_OPTIONS["gcc"]~=nil) then
		if string.find(_OPTIONS["gcc"], "clang") or string.find(_OPTIONS["gcc"], "android") then
			buildoptions_c {
				"-Wno-unknown-warning-option",
				"-Wno-absolute-value",
				"-Wno-unused-but-set-variable",
				"-Wno-maybe-uninitialized",
				"-Wno-sometimes-uninitialized",
				"-Wno-incompatible-pointer-types-discards-qualifiers",
				"-Wno-pointer-sign",
				"-Wno-switch",
				"-Wno-macro-redefined",
				"-Wno-unused-label",
			}
		else
			buildoptions_c {
				"-Wno-maybe-uninitialized",
				"-Wno-sometimes-uninitialized",
				"-Wno-unused-but-set-variable",
				"-Wno-incompatible-pointer-types-discards-qualifiers",
				"-w",
			}
		end
		if string.find(_OPTIONS["gcc"], "clang") and version >= 100000 then
			buildoptions_c {
				"-Wno-misleading-indentation",
			}
		end
	end
	configuration { "vs*" }
		buildoptions {
			"/wd4204", -- warning C4204: nonstandard extension used : non-constant aggregate initializer
			"/wd4701", -- warning C4701: potentially uninitialized local variable 'xxx' used
			"/wd4057", -- warning C4057: 'function': 'xxx' differs in indirection to slightly different base types from 'xxx'
		}

	configuration { }

	includedirs {
		MAME_DIR .. "3rdparty/portaudio/include",
		MAME_DIR .. "3rdparty/portaudio/src/common",
	}

	files {
		MAME_DIR .. "3rdparty/portaudio/src/common/pa_allocation.c",
		MAME_DIR .. "3rdparty/portaudio/src/common/pa_converters.c",
		MAME_DIR .. "3rdparty/portaudio/src/common/pa_cpuload.c",
		MAME_DIR .. "3rdparty/portaudio/src/common/pa_dither.c",
		MAME_DIR .. "3rdparty/portaudio/src/common/pa_debugprint.c",
		MAME_DIR .. "3rdparty/portaudio/src/common/pa_front.c",
		MAME_DIR .. "3rdparty/portaudio/src/common/pa_process.c",
		MAME_DIR .. "3rdparty/portaudio/src/common/pa_stream.c",
		MAME_DIR .. "3rdparty/portaudio/src/common/pa_trace.c",
		MAME_DIR .. "3rdparty/portaudio/src/hostapi/skeleton/pa_hostapi_skeleton.c",
	}

	if _OPTIONS["targetos"]=="windows" then
		defines {
			"PA_USE_DS=1",
			"PA_USE_WASAPI=1",
			"PA_USE_WDMKS=1",
			"PA_USE_WMME=1",
		}
		includedirs {
			MAME_DIR .. "3rdparty/portaudio/src/os/win",
		}

		configuration { }
		files {
			MAME_DIR .. "3rdparty/portaudio/src/os/win/pa_win_util.c",
			MAME_DIR .. "3rdparty/portaudio/src/os/win/pa_win_version.c",
			MAME_DIR .. "3rdparty/portaudio/src/os/win/pa_win_waveformat.c",
			MAME_DIR .. "3rdparty/portaudio/src/os/win/pa_win_hostapis.c",
			MAME_DIR .. "3rdparty/portaudio/src/os/win/pa_win_coinitialize.c",
			MAME_DIR .. "3rdparty/portaudio/src/hostapi/dsound/pa_win_ds.c",
			MAME_DIR .. "3rdparty/portaudio/src/hostapi/dsound/pa_win_ds_dynlink.c",
			MAME_DIR .. "3rdparty/portaudio/src/os/win/pa_win_hostapis.c",
			MAME_DIR .. "3rdparty/portaudio/src/hostapi/wasapi/pa_win_wasapi.c",
			MAME_DIR .. "3rdparty/portaudio/src/hostapi/wdmks/pa_win_wdmks.c",
			MAME_DIR .. "3rdparty/portaudio/src/hostapi/wmme/pa_win_wmme.c",
			MAME_DIR .. "3rdparty/portaudio/src/common/pa_ringbuffer.c",
		}

	end
	if _OPTIONS["targetos"]=="linux" then
		defines {
			"PA_USE_ALSA=1",
			"PA_USE_OSS=1",
			"HAVE_LINUX_SOUNDCARD_H",
		}
		includedirs {
			MAME_DIR .. "3rdparty/portaudio/src/os/unix",
		}
		files {
			MAME_DIR .. "3rdparty/portaudio/src/os/unix/pa_unix_hostapis.c",
			MAME_DIR .. "3rdparty/portaudio/src/os/unix/pa_unix_util.c",
			MAME_DIR .. "3rdparty/portaudio/src/hostapi/alsa/pa_linux_alsa.c",
			MAME_DIR .. "3rdparty/portaudio/src/hostapi/oss/pa_unix_oss.c",
		}
	end
	if _OPTIONS["targetos"]=="macosx" then
		defines {
			"PA_USE_COREAUDIO=1",
		}
		includedirs {
			MAME_DIR .. "3rdparty/portaudio/src/os/unix",
		}
		files {
			MAME_DIR .. "3rdparty/portaudio/src/os/unix/pa_unix_hostapis.c",
			MAME_DIR .. "3rdparty/portaudio/src/os/unix/pa_unix_util.c",
			MAME_DIR .. "3rdparty/portaudio/src/hostapi/coreaudio/pa_mac_core.c",
			MAME_DIR .. "3rdparty/portaudio/src/hostapi/coreaudio/pa_mac_core_utilities.c",
			MAME_DIR .. "3rdparty/portaudio/src/hostapi/coreaudio/pa_mac_core_blocking.c",
			MAME_DIR .. "3rdparty/portaudio/src/common/pa_ringbuffer.c",
		}
	end

else
links {
	ext_lib("portaudio"),
}
end
end

--------------------------------------------------
-- linenoise library
--------------------------------------------------

project "linenoise"
	uuid "7320ffc8-2748-4add-8864-ae29b72a8511"
	kind (LIBTYPE)

	addprojectflags()

	configuration { "vs*" }
		buildoptions {
			"/wd4701", -- warning C4701: potentially uninitialized local variable 'xxx' used
		}

	configuration { }

	defines {
		"USE_UTF8",
	}

	includedirs {
		MAME_DIR .. "3rdparty/linenoise",
	}

	files {
		MAME_DIR .. "3rdparty/linenoise/linenoise.c",
		MAME_DIR .. "3rdparty/linenoise/stringbuf.c",
		MAME_DIR .. "3rdparty/linenoise/utf8.c",
	}


--------------------------------------------------
-- utf8proc library objects
--------------------------------------------------

if not _OPTIONS["with-system-utf8proc"] then
project "utf8proc"
	uuid "1f881f09-0395-4483-ac37-2935fb092187"
	kind "StaticLib"

	defines {
		"UTF8PROC_STATIC",
	}

	configuration "Debug"
		defines {
			"verbose=-1",
		}

	configuration { }

	files {
		MAME_DIR .. "3rdparty/utf8proc/utf8proc.c"
	}
else
links {
	ext_lib("utf8proc"),
}
end


--------------------------------------------------
-- wdlfft library objects (from Cockos WDL)
--------------------------------------------------

project "wdlfft"
	uuid "74ca017e-fa0d-48b8-81d6-8081a37be14c"
	kind "StaticLib"

	configuration { "gmake or ninja" }
		buildoptions_c {
			"-Wno-strict-prototypes",
		}

	configuration { }

	files {
		MAME_DIR .. "3rdparty/wdlfft/fft.c",
		MAME_DIR .. "3rdparty/wdlfft/fft.h"
	}


--------------------------------------------------
-- ymfm library objects
--------------------------------------------------

project "ymfm"
	uuid "2403a536-cb0a-4b50-b41f-10c17917689b"
	kind "StaticLib"

	configuration { }
		defines {
			"YMFM_MAME",
		}

	files {
		MAME_DIR .. "3rdparty/ymfm/src/ymfm.h",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_adpcm.cpp",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_adpcm.h",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_fm.h",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_fm.ipp",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_misc.cpp",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_misc.h",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_opl.cpp",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_opl.h",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_opm.cpp",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_opm.h",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_opn.cpp",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_opn.h",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_opq.cpp",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_opq.h",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_opz.cpp",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_opz.h",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_pcm.cpp",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_pcm.h",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_ssg.cpp",
		MAME_DIR .. "3rdparty/ymfm/src/ymfm_ssg.h",
	}


--------------------------------------------------
-- asmjit library
--------------------------------------------------

if not _OPTIONS["FORCE_DRC_C_BACKEND"] then
project "asmjit"
	uuid "4539757c-6e99-4bae-b3d0-b342a7c49539"
	kind "StaticLib"

	configuration { "gmake or ninja" }
	if (_OPTIONS["gcc"] ~= nil) and (not string.find(_OPTIONS["gcc"], "clang")) and (str_to_version(_OPTIONS["gcc_version"]) < 80000) then
		buildoptions {
			"-Wno-maybe-uninitialized",
		}
	end

	configuration { }

	if _OPTIONS["targetos"]=="macosx" and _OPTIONS["gcc"]~=nil then
		if string.find(_OPTIONS["gcc"], "clang") and (version < 80000) then
			defines {
				"TARGET_OS_OSX=1",
			}
		end
	end

	files {
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/a64.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/asmjit-scope-begin.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/asmjit-scope-end.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/asmjit.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64archtraits_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64assembler.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64assembler.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64builder.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64builder.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64compiler.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64compiler.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64emithelper.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64emithelper_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64emitter.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64formatter.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64formatter_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64func.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64func_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64globals.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64instapi.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64instapi_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64instdb.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64instdb.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64instdb_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64operand.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64operand.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64rapass.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/a64rapass_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/armformatter.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/armformatter_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/armglobals.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/armoperand.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/arm/armutils.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/api-build_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/api-config.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/archcommons.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/archtraits.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/archtraits.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/assembler.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/assembler.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/builder.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/builder.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/builder_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/codebuffer.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/codeholder.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/codeholder.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/codewriter.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/codewriter_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/compiler.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/compiler.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/compilerdefs.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/constpool.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/constpool.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/cpuinfo.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/cpuinfo.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/emithelper.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/emithelper_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/emitter.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/emitter.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/emitterutils.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/emitterutils_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/environment.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/environment.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/errorhandler.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/errorhandler.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/formatter.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/formatter.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/formatter_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/func.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/func.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/funcargscontext.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/funcargscontext_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/globals.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/globals.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/inst.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/inst.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/instdb.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/instdb_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/jitallocator.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/jitallocator.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/jitruntime.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/jitruntime.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/logger.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/logger.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/misc_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/operand.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/operand.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/osutils.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/osutils.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/osutils_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/raassignment_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/rabuilders_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/radefs_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/ralocal.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/ralocal_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/rapass.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/rapass_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/rastack.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/rastack_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/string.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/string.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/support.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/support.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/support_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/target.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/target.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/type.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/type.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/virtmem.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/virtmem.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/zone.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/zone.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/zonehash.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/zonehash.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/zonelist.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/zonelist.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/zonestack.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/zonestack.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/zonestring.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/zonetree.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/zonetree.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/zonevector.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/core/zonevector.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86archtraits_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86assembler.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86assembler.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86builder.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86builder.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86compiler.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86compiler.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86emithelper.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86emithelper_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86emitter.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86formatter.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86formatter_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86func.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86func_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86globals.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86instapi.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86instapi_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86instdb.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86instdb.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86instdb_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86opcode_p.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86operand.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86operand.h",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86rapass.cpp",
		MAME_DIR .. "3rdparty/asmjit/src/asmjit/x86/x86rapass_p.h",
	}
end
