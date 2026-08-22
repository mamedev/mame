// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    pkzipdefs.h

    Constants related to the PKZIP archive format

***************************************************************************/
#ifndef MAME_LIB_UTIL_PKZIPDEFS_H
#define MAME_LIB_UTIL_PKZIPDEFS_H

#pragma once

#include <cstdint>


namespace util {

struct pkzip_defs
{
	static constexpr std::uint32_t SIG_DATA_DESC = 0x08074b50;
	static constexpr std::uint32_t SIG_LCL_HDR   = 0x04034b50;
	static constexpr std::uint32_t SIG_CD_HDR    = 0x02014b50;
	static constexpr std::uint32_t SIG_ECD       = 0x06054b50;
	static constexpr std::uint32_t SIG_ECD64     = 0x06064b50;
	static constexpr std::uint32_t SIG_ECD64_LOC = 0x07064b50;

	static constexpr std::uint16_t GP_FLAG_ENCRYPTED        = 0x0001;
	static constexpr std::uint16_t GP_FLAG_IMPLODE_DICT_4K  = 0x0000;
	static constexpr std::uint16_t GP_FLAG_IMPLODE_DICT_8K  = 0x0002;
	static constexpr std::uint16_t GP_FLAG_IMPLODE_SFT_2    = 0x0000;
	static constexpr std::uint16_t GP_FLAG_IMPLODE_SFT_3    = 0x0004;
	static constexpr std::uint16_t GP_FLAG_DEFLATE_NORMAL   = 0x0000;
	static constexpr std::uint16_t GP_FLAG_DEFLATE_MAX      = 0x0002;
	static constexpr std::uint16_t GP_FLAG_DEFLATE_FAST     = 0x0004;
	static constexpr std::uint16_t GP_FLAG_DEFLATE_FASTEST  = 0x0006;
	static constexpr std::uint16_t GP_FLAG_LZMA_EOS         = 0x0002;
	static constexpr std::uint16_t GP_FLAG_DATA_DESC        = 0x0008;
	static constexpr std::uint16_t GP_FLAG_PATCHED_DATA     = 0x0020;
	static constexpr std::uint16_t GP_FLAG_ENCRYPTED_STRONG = 0x0040;
	static constexpr std::uint16_t GP_FLAG_UTF8             = 0x0800;
	static constexpr std::uint16_t GP_FLAG_CD_ENCRYPTED     = 0x2000;

	static constexpr std::uint16_t METHOD_STORE   = 0;
	static constexpr std::uint16_t METHOD_DEFLATE = 8;
	static constexpr std::uint16_t METHOD_BZIP2   = 12;
	static constexpr std::uint16_t METHOD_LZMA    = 14;
	static constexpr std::uint16_t METHOD_ZSTD    = 93;
	static constexpr std::uint16_t METHOD_MP3     = 94;
	static constexpr std::uint16_t METHOD_XZ      = 95;

	static constexpr std::uint16_t EXTRA_ID_ZIP64            = 0x0001;
	static constexpr std::uint16_t EXTRA_ID_OS2              = 0x0009;
	static constexpr std::uint16_t EXTRA_ID_NTFS             = 0x000a;
	static constexpr std::uint16_t EXTRA_ID_OPENVMS          = 0x000c;
	static constexpr std::uint16_t EXTRA_ID_UNIX             = 0x000d;
	static constexpr std::uint16_t EXTRA_ID_INFO_ZIP_UC_CMT  = 0x6375;
	static constexpr std::uint16_t EXTRA_ID_INFO_ZIP_UC_PATH = 0x7075;

	static constexpr std::uint16_t EXTRA_NTFS_TAG_TIMES = 0x0001;

	enum : unsigned
	{
		OFFS_LCL_HDR_SIG          = 0,
		OFFS_LCL_HDR_VER_REQ      = 4,
		OFFS_LCL_HDR_GP_FLAG      = 6,
		OFFS_LCL_HDR_METHOD       = 8,
		OFFS_LCL_HDR_MOD_TIME     = 10,
		OFFS_LCL_HDR_MOD_DATE     = 12,
		OFFS_LCL_HDR_CRC32        = 14,
		OFFS_LCL_HDR_CMP_SIZE     = 18,
		OFFS_LCL_HDR_RAW_SIZE     = 22,
		OFFS_LCL_HDR_NAME_LEN     = 26,
		OFFS_LCL_HDR_EXTRA_LEN    = 28,
		OFFS_LCL_HDR_NAME         = 30,

		OFFS_CD_HDR_SIG           = 0,
		OFFS_CD_HDR_VER_CREATE    = 4,
		OFFS_CD_HDR_VER_REQ       = 6,
		OFFS_CD_HDR_GP_FLAG       = 8,
		OFFS_CD_HDR_METHOD        = 10,
		OFFS_CD_HDR_MOD_TIME      = 12,
		OFFS_CD_HDR_MOD_DATE      = 14,
		OFFS_CD_HDR_CRC32         = 16,
		OFFS_CD_HDR_CMP_SIZE      = 20,
		OFFS_CD_HDR_RAW_SIZE      = 24,
		OFFS_CD_HDR_NAME_LEN      = 28,
		OFFS_CD_HDR_EXTRA_LEN     = 30,
		OFFS_CD_HDR_CMT_LEN       = 32,
		OFFS_CD_HDR_DISK          = 34,
		OFFS_CD_HDR_INT_ATTR      = 36,
		OFFS_CD_HDR_EXT_ATTR      = 38,
		OFFS_CD_HDR_OFFS          = 42,
		OFFS_CD_HDR_NAME          = 46,

		OFFS_ECD_SIG              = 0,
		OFFS_ECD_THIS_DISK        = 4,
		OFFS_ECD_CD_START         = 6,
		OFFS_ECD_THIS_ENTRIES     = 8,
		OFFS_ECD_CD_ENTRIES       = 10,
		OFFS_ECD_CD_SIZE          = 12,
		OFFS_ECD_CD_OFFS          = 16,
		OFFS_ECD_CMT_LEN          = 20,
		OFFS_ECD_CMT              = 22,

		OFFS_DATA_DESC_SIG        = 0,
		OFFS_DATA_DESC_CRC32      = 4,
		OFFS_DATA_DESC_CMP_SIZE   = 8,
		OFFS_DATA_DESC_RAW_SIZE   = 12,

		OFFS_DATA_DESC64_SIG      = 0,
		OFFS_DATA_DESC64_CRC32    = 4,
		OFFS_DATA_DESC64_CMP_SIZE = 8,
		OFFS_DATA_DESC64_RAW_SIZE = 16,
	};
};

} // namespace util

#endif // MAME_LIB_UTIL_PKZIPDEFS_H
