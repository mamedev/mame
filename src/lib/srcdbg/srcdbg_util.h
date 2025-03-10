// license:BSD-3-Clause
// copyright-holders:David Broman
/*********************************************************************

    srcdbg_util.h

    Internal helpers for srcdbg library.  Since srcdbg cannot rely
    on other MAME libraries, this file may include functionality
    found elsewhere in MAME

    WARNING: Tools external to MAME should only use functionality
    declared in srcdg_format.h and srcdbg_api.h.

***************************************************************************/

#ifndef MAME_SRCDBG_UTIL_H
#define MAME_SRCDBG_UTIL_H

#pragma once

#include "osdcomm.h"

#include <string>

using osd::u8;
using osd::u16;
using osd::u32;
using osd::u64;
using osd::s8;
using osd::s16;
using osd::s32;
using osd::s64;

// Like sprintf, but for std::string
void srcdbg_sprintf(std::string & out, const char * format, ...);

// osdcomm.h macros take arch-friendly numbers and convert
// them to big / little endian.  These convert a known
// endian (little) to arch-friendly numbers.
#ifdef LSB_FIRST
constexpr u16 from_little_endian16(u16 x) { return x; }
constexpr u32 from_little_endian32(u32 x) { return x; }
#else
constexpr u16 from_little_endian16(u16 x) { return swapendian_int16(x); }
constexpr u32 from_little_endian32(u32 x) { return swapendian_int32(x); }
#endif // LSB_FIRST


#endif // MAME_SRCDBG_UTIL_H
