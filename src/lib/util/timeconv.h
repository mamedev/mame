// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Nathan Woods
/***************************************************************************

    timeconv.h

    Time conversion utility code

***************************************************************************/

#ifndef MAME_LIB_UTIL_TIMECONV_H
#define MAME_LIB_UTIL_TIMECONV_H

#pragma once

#include "osdcore.h"

#include <chrono>


namespace util {
/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef std::chrono::duration<std::uint64_t, std::ratio<1, 10000000> > ntfs_duration;


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

// -------------------------------------------------
// ntfs_duration_from_filetime
// -------------------------------------------------

inline constexpr ntfs_duration ntfs_duration_from_filetime(std::uint32_t high, std::uint32_t low)
{
	return ntfs_duration((std::uint64_t(high) << 32) | std::uint64_t(low));
}


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

std::chrono::system_clock::time_point system_clock_time_point_from_ntfs_duration(ntfs_duration d);


} // namespace util

#endif  // MAME_LIB_UTIL_TIMECONV_H
