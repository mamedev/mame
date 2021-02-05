// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    msdib.h

    Microsoft Device-Independent Bitmap file loading.

***************************************************************************/
#ifndef MAME_LIB_UTIL_MSDIB_H
#define MAME_LIB_UTIL_MSDIB_H

#pragma once

#include "bitmap.h"
#include "corefile.h"

#include <cstdint>


namespace util {

/***************************************************************************
    CONSTANTS
***************************************************************************/

// Error types
enum class msdib_error
{
	NONE,
	OUT_OF_MEMORY,
	FILE_ERROR,
	BAD_SIGNATURE,
	FILE_TRUNCATED,
	FILE_CORRUPT,
	UNSUPPORTED_FORMAT
};

msdib_error msdib_verify_header(core_file &fp);
msdib_error msdib_read_bitmap(core_file &fp, bitmap_argb32 &bitmap);
msdib_error msdib_read_bitmap_data(core_file &fp, bitmap_argb32 &bitmap, std::uint32_t length, std::uint32_t dirheight = 0U);

} // namespace util

#endif // MAME_LIB_UTIL_MSDIB_H
