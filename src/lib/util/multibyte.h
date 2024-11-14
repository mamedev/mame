// license:BSD-3-Clause
// copyright-holders:Mietek Bak
#ifndef MAME_UTIL_MULTIBYTE_H
#define MAME_UTIL_MULTIBYTE_H

#pragma once

#include "coretmpl.h"
#include "osdcomm.h"

//////////////////////////////////////////////////////////////////////////////

// unsigned big-endian

constexpr osd::u16 get_u16be(osd::u8 const *buf) noexcept
{
	return (osd::u16(buf[0]) << 8)
		 | (osd::u16(buf[1]) << 0);
}

constexpr osd::u32 get_u24be(osd::u8 const *buf) noexcept
{
	return (osd::u32(buf[0]) << 16)
		 | (osd::u32(buf[1]) << 8)
		 | (osd::u32(buf[2]) << 0);
}

constexpr osd::u32 get_u32be(osd::u8 const *buf) noexcept
{
	return (osd::u32(buf[0]) << 24)
		 | (osd::u32(buf[1]) << 16)
		 | (osd::u32(buf[2]) << 8)
		 | (osd::u32(buf[3]) << 0);
}

constexpr osd::u64 get_u48be(osd::u8 const *buf) noexcept
{
	return (osd::u64(buf[0]) << 40)
		 | (osd::u64(buf[1]) << 32)
		 | (osd::u64(buf[2]) << 24)
		 | (osd::u64(buf[3]) << 16)
		 | (osd::u64(buf[4]) << 8)
		 | (osd::u64(buf[5]) << 0);
}

constexpr osd::u64 get_u64be(osd::u8 const *buf) noexcept
{
	return (osd::u64(buf[0]) << 56)
		 | (osd::u64(buf[1]) << 48)
		 | (osd::u64(buf[2]) << 40)
		 | (osd::u64(buf[3]) << 32)
		 | (osd::u64(buf[4]) << 24)
		 | (osd::u64(buf[5]) << 16)
		 | (osd::u64(buf[6]) << 8)
		 | (osd::u64(buf[7]) << 0);
}

inline void put_u16be(osd::u8 *buf, osd::u16 data) noexcept
{
	buf[0] = osd::u8(data >> 8);
	buf[1] = osd::u8(data >> 0);
}

inline void put_u24be(osd::u8 *buf, osd::u32 data) noexcept
{
	buf[0] = osd::u8(data >> 16);
	buf[1] = osd::u8(data >> 8);
	buf[2] = osd::u8(data >> 0);
}

inline void put_u32be(osd::u8 *buf, osd::u32 data) noexcept
{
	buf[0] = osd::u8(data >> 24);
	buf[1] = osd::u8(data >> 16);
	buf[2] = osd::u8(data >> 8);
	buf[3] = osd::u8(data >> 0);
}

inline void put_u48be(osd::u8 *buf, osd::u64 data) noexcept
{
	buf[0] = osd::u8(data >> 40);
	buf[1] = osd::u8(data >> 32);
	buf[2] = osd::u8(data >> 24);
	buf[3] = osd::u8(data >> 16);
	buf[4] = osd::u8(data >> 8);
	buf[5] = osd::u8(data >> 0);
}

inline void put_u64be(osd::u8 *buf, osd::u64 data) noexcept
{
	buf[0] = osd::u8(data >> 56);
	buf[1] = osd::u8(data >> 48);
	buf[2] = osd::u8(data >> 40);
	buf[3] = osd::u8(data >> 32);
	buf[4] = osd::u8(data >> 24);
	buf[5] = osd::u8(data >> 16);
	buf[6] = osd::u8(data >> 8);
	buf[7] = osd::u8(data >> 0);
}

//////////////////////////////////////////////////////////////////////////////

// signed big-endian

constexpr osd::s16 get_s16be(osd::u8 const *buf) noexcept
{
	return get_u16be(buf);
}

constexpr osd::s32 get_s24be(osd::u8 const *buf) noexcept
{
	return util::sext(get_u24be(buf), 24);
}

constexpr osd::s32 get_s32be(osd::u8 const *buf) noexcept
{
	return get_u32be(buf);
}

constexpr osd::s64 get_s48be(osd::u8 const *buf) noexcept
{
	return util::sext(get_u48be(buf), 48);
}

constexpr osd::s64 get_s64be(osd::u8 const *buf) noexcept
{
	return get_u64be(buf);
}

inline void put_s16be(osd::u8 *buf, osd::s16 data) noexcept
{
	put_u16be(buf, data);
}

inline void put_s24be(osd::u8 *buf, osd::s32 data) noexcept
{
	put_u24be(buf, data);
}

inline void put_s32be(osd::u8 *buf, osd::s32 data) noexcept
{
	put_u32be(buf, data);
}

inline void put_s48be(osd::u8 *buf, osd::s64 data) noexcept
{
	put_u48be(buf, data);
}

inline void put_s64be(osd::u8 *buf, osd::s64 data) noexcept
{
	put_u64be(buf, data);
}

//////////////////////////////////////////////////////////////////////////////

// unsigned little-endian

constexpr osd::u16 get_u16le(osd::u8 const *buf) noexcept
{
	return (osd::u16(buf[0]) << 0)
		 | (osd::u16(buf[1]) << 8);
}

constexpr osd::u32 get_u24le(osd::u8 const *buf) noexcept
{
	return (osd::u32(buf[0]) << 0)
		 | (osd::u32(buf[1]) << 8)
		 | (osd::u32(buf[2]) << 16);
}

constexpr osd::u32 get_u32le(osd::u8 const *buf) noexcept
{
	return (osd::u32(buf[0]) << 0)
		 | (osd::u32(buf[1]) << 8)
		 | (osd::u32(buf[2]) << 16)
		 | (osd::u32(buf[3]) << 24);
}

constexpr osd::u64 get_u48le(osd::u8 const *buf) noexcept
{
	return (osd::u64(buf[0]) << 0)
		 | (osd::u64(buf[1]) << 8)
		 | (osd::u64(buf[2]) << 16)
		 | (osd::u64(buf[3]) << 24)
		 | (osd::u64(buf[4]) << 32)
		 | (osd::u64(buf[5]) << 40);
}

constexpr osd::u64 get_u64le(osd::u8 const *buf) noexcept
{
	return (osd::u64(buf[0]) << 0)
		 | (osd::u64(buf[1]) << 8)
		 | (osd::u64(buf[2]) << 16)
		 | (osd::u64(buf[3]) << 24)
		 | (osd::u64(buf[4]) << 32)
		 | (osd::u64(buf[5]) << 40)
		 | (osd::u64(buf[6]) << 48)
		 | (osd::u64(buf[7]) << 56);
}

inline void put_u16le(osd::u8 *buf, osd::u16 data) noexcept
{
	buf[0] = osd::u8(data >> 0);
	buf[1] = osd::u8(data >> 8);
}

inline void put_u24le(osd::u8 *buf, osd::u32 data) noexcept
{
	buf[0] = osd::u8(data >> 0);
	buf[1] = osd::u8(data >> 8);
	buf[2] = osd::u8(data >> 16);
}

inline void put_u32le(osd::u8 *buf, osd::u32 data) noexcept
{
	buf[0] = osd::u8(data >> 0);
	buf[1] = osd::u8(data >> 8);
	buf[2] = osd::u8(data >> 16);
	buf[3] = osd::u8(data >> 24);
}

inline void put_u48le(osd::u8 *buf, osd::u64 data) noexcept
{
	buf[0] = osd::u8(data >> 0);
	buf[1] = osd::u8(data >> 8);
	buf[2] = osd::u8(data >> 16);
	buf[3] = osd::u8(data >> 24);
	buf[4] = osd::u8(data >> 32);
	buf[5] = osd::u8(data >> 40);
}

inline void put_u64le(osd::u8 *buf, osd::u64 data) noexcept
{
	buf[0] = osd::u8(data >> 0);
	buf[1] = osd::u8(data >> 8);
	buf[2] = osd::u8(data >> 16);
	buf[3] = osd::u8(data >> 24);
	buf[4] = osd::u8(data >> 32);
	buf[5] = osd::u8(data >> 40);
	buf[6] = osd::u8(data >> 48);
	buf[7] = osd::u8(data >> 56);
}

//////////////////////////////////////////////////////////////////////////////

// signed little-endian

constexpr osd::s16 get_s16le(osd::u8 const *buf) noexcept
{
	return get_u16le(buf);
}

constexpr osd::s32 get_s24le(osd::u8 const *buf) noexcept
{
	return util::sext(get_u24le(buf), 24);
}

constexpr osd::s32 get_s32le(osd::u8 const *buf) noexcept
{
	return get_u32le(buf);
}

constexpr osd::s64 get_s48le(osd::u8 const *buf) noexcept
{
	return util::sext(get_u48le(buf), 48);
}

constexpr osd::s64 get_s64le(osd::u8 const *buf) noexcept
{
	return get_u64le(buf);
}

inline void put_s16le(osd::u8 *buf, osd::s16 data) noexcept
{
	put_u16le(buf, data);
}

inline void put_s24le(osd::u8 *buf, osd::s32 data) noexcept
{
	put_u24le(buf, data);
}

inline void put_s32le(osd::u8 *buf, osd::s32 data) noexcept
{
	put_u32le(buf, data);
}

inline void put_s48le(osd::u8 *buf, osd::s64 data) noexcept
{
	put_u48le(buf, data);
}

inline void put_s64le(osd::u8 *buf, osd::s64 data) noexcept
{
	put_u64le(buf, data);
}

//////////////////////////////////////////////////////////////////////////////

#endif // MAME_UTIL_MULTIBYTE_H
