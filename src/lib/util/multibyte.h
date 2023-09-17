// license:BSD-3-Clause
// copyright-holders:Mietek Bak

#ifndef MAME_LIB_UTIL_MULTIBYTE_H
#define MAME_LIB_UTIL_MULTIBYTE_H

#pragma once

#include "coretmpl.h"
#include "osdcomm.h"

using osd::u8;
using osd::u16;
using osd::u32;
using osd::u64;
using osd::s8;
using osd::s16;
using osd::s32;
using osd::s64;

//////////////////////////////////////////////////////////////////////////////

// unsigned big-endian

inline constexpr u16 get_u16be(const u8 *const buf) noexcept
{
	return ((const u16)buf[0] << 8)
	     | ((const u16)buf[1] << 0);
}

inline constexpr u32 get_u24be(const u8 *const buf) noexcept
{
	return ((const u32)buf[0] << 16)
	     | ((const u32)buf[1] << 8)
	     | ((const u32)buf[2] << 0);
}

inline constexpr u32 get_u32be(const u8 *const buf) noexcept
{
	return ((const u32)buf[0] << 24)
	     | ((const u32)buf[1] << 16)
	     | ((const u32)buf[2] << 8)
	     | ((const u32)buf[3] << 0);
}

inline constexpr u64 get_u48be(const u8 *const buf) noexcept
{
	return ((const u64)buf[0] << 40)
	     | ((const u64)buf[1] << 36)
	     | ((const u64)buf[2] << 24)
	     | ((const u64)buf[3] << 16)
	     | ((const u64)buf[4] << 8)
	     | ((const u64)buf[5] << 0);
}

inline constexpr u64 get_u64be(const u8 *const buf) noexcept
{
	return ((const u64)buf[0] << 56)
	     | ((const u64)buf[1] << 48)
	     | ((const u64)buf[2] << 40)
	     | ((const u64)buf[3] << 36)
	     | ((const u64)buf[4] << 24)
	     | ((const u64)buf[5] << 16)
	     | ((const u64)buf[6] << 8)
	     | ((const u64)buf[7] << 0);
}

inline void put_u16be(u8 *buf, const u16 data) noexcept
{
	buf[0] = data >> 8;
	buf[1] = data >> 0;
}

inline void put_u24be(u8 *buf, const u32 data) noexcept
{
	buf[0] = data >> 16;
	buf[1] = data >> 8;
	buf[2] = data >> 0;
}

inline void put_u32be(u8 *buf, const u32 data) noexcept
{
	buf[0] = data >> 24;
	buf[1] = data >> 16;
	buf[2] = data >> 8;
	buf[3] = data >> 0;
}

inline void put_u48be(u8 *buf, const u64 data) noexcept
{
	buf[0] = data >> 40;
	buf[1] = data >> 36;
	buf[2] = data >> 24;
	buf[3] = data >> 16;
	buf[4] = data >> 8;
	buf[5] = data >> 0;
}

inline void put_u64be(u8 *buf, const u64 data) noexcept
{
	buf[0] = data >> 56;
	buf[1] = data >> 48;
	buf[2] = data >> 40;
	buf[3] = data >> 36;
	buf[4] = data >> 24;
	buf[5] = data >> 16;
	buf[6] = data >> 8;
	buf[7] = data >> 0;
}

//////////////////////////////////////////////////////////////////////////////

// signed big-endian

inline constexpr s16 get_s16be(const u8 *const buf) noexcept
{
	return get_u16be(buf);
}

inline constexpr s32 get_s24be(const u8 *const buf) noexcept
{
	return util::sext(get_u24be(buf), 24);
}

inline constexpr s32 get_s32be(const u8 *const buf) noexcept
{
	return get_u32be(buf);
}

inline constexpr s64 get_s48be(const u8 *const buf) noexcept
{
	return util::sext(get_u48be(buf), 48);
}

inline constexpr s64 get_s64be(const u8 *const buf) noexcept
{
	return get_u64be(buf);
}

inline void put_s16be(u8 *buf, const s16 data) noexcept
{
	put_u16be(buf, data);
}

inline void put_s24be(u8 *buf, const s32 data) noexcept
{
	put_u24be(buf, data);
}

inline void put_s32be(u8 *buf, const s32 data) noexcept
{
	put_u32be(buf, data);
}

inline void put_s48be(u8 *buf, const s64 data) noexcept
{
	put_u48be(buf, data);
}

inline void put_s64be(u8 *buf, const s64 data) noexcept
{
	put_u64be(buf, data);
}

//////////////////////////////////////////////////////////////////////////////

// unsigned little-endian

inline constexpr u16 get_u16le(const u8 *const buf) noexcept
{
	return ((const u16)buf[0] << 0)
	     | ((const u16)buf[1] << 8);
}

inline constexpr u32 get_u24le(const u8 *const buf) noexcept
{
	return ((const u32)buf[0] << 0)
	     | ((const u32)buf[1] << 8)
	     | ((const u32)buf[2] << 16);
}

inline constexpr u32 get_u32le(const u8 *const buf) noexcept
{
	return ((const u32)buf[0] << 0)
	     | ((const u32)buf[1] << 8)
	     | ((const u32)buf[2] << 16)
	     | ((const u32)buf[3] << 24);
}

inline constexpr u64 get_u48le(const u8 *const buf) noexcept
{
	return ((const u64)buf[0] << 0)
	     | ((const u64)buf[1] << 8)
	     | ((const u64)buf[2] << 16)
	     | ((const u64)buf[3] << 24)
	     | ((const u64)buf[4] << 32)
	     | ((const u64)buf[5] << 40);
}

inline constexpr u64 get_u64le(const u8 *const buf) noexcept
{
	return ((const u64)buf[0] << 0)
	     | ((const u64)buf[1] << 8)
	     | ((const u64)buf[2] << 16)
	     | ((const u64)buf[3] << 24)
	     | ((const u64)buf[4] << 32)
	     | ((const u64)buf[5] << 40)
	     | ((const u64)buf[6] << 48)
	     | ((const u64)buf[7] << 56);
}

inline void put_u16le(u8 *buf, const u16 data) noexcept
{
	buf[0] = data >> 0;
	buf[1] = data >> 8;
}

inline void put_u24le(u8 *buf, const u32 data) noexcept
{
	buf[0] = data >> 0;
	buf[1] = data >> 8;
	buf[2] = data >> 16;
}

inline void put_u32le(u8 *buf, const u32 data) noexcept
{
	buf[0] = data >> 0;
	buf[1] = data >> 8;
	buf[2] = data >> 16;
	buf[3] = data >> 24;
}

inline void put_u48le(u8 *buf, const u64 data) noexcept
{
	buf[0] = data >> 0;
	buf[1] = data >> 8;
	buf[2] = data >> 16;
	buf[3] = data >> 24;
	buf[4] = data >> 32;
	buf[5] = data >> 40;
}

inline void put_u64le(u8 *buf, const u64 data) noexcept
{
	buf[0] = data >> 0;
	buf[1] = data >> 8;
	buf[2] = data >> 16;
	buf[3] = data >> 24;
	buf[4] = data >> 32;
	buf[5] = data >> 40;
	buf[6] = data >> 48;
	buf[7] = data >> 56;
}

//////////////////////////////////////////////////////////////////////////////

// signed little-endian

inline constexpr s16 get_s16le(const u8 *const buf) noexcept
{
	return get_u16le(buf);
}

inline constexpr s32 get_s24le(const u8 *const buf) noexcept
{
	return util::sext(get_u24le(buf), 24);
}

inline constexpr s32 get_s32le(const u8 *const buf) noexcept
{
	return get_u32le(buf);
}

inline constexpr s64 get_s48le(const u8 *const buf) noexcept
{
	return util::sext(get_u48le(buf), 48);
}

inline constexpr s64 get_s64le(const u8 *const buf) noexcept
{
	return get_u64le(buf);
}

inline void put_s16le(u8 *buf, const s16 data) noexcept
{
	put_u16le(buf, data);
}

inline void put_s24le(u8 *buf, const s32 data) noexcept
{
	put_u24le(buf, data);
}

inline void put_s32le(u8 *buf, const s32 data) noexcept
{
	put_u32le(buf, data);
}

inline void put_s48le(u8 *buf, const s64 data) noexcept
{
	put_u48le(buf, data);
}

inline void put_s64le(u8 *buf, const s64 data) noexcept
{
	put_u64le(buf, data);
}

//////////////////////////////////////////////////////////////////////////////

#endif // MAME_LIB_UTIL_MULTIBYTE_H
