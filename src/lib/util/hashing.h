// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    hashing.h

    Hashing helper classes.

***************************************************************************/
#ifndef MAME_UTIL_HASHING_H
#define MAME_UTIL_HASHING_H

#pragma once

#include "md5.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <string_view>


namespace util {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> SHA-1

// final digest
struct sha1_t
{
	bool operator==(const sha1_t &rhs) const { return memcmp(m_raw, rhs.m_raw, sizeof(m_raw)) == 0; }
	bool operator!=(const sha1_t &rhs) const { return memcmp(m_raw, rhs.m_raw, sizeof(m_raw)) != 0; }
	operator uint8_t *() { return m_raw; }
	bool from_string(std::string_view string);
	std::string as_string() const;
	uint8_t m_raw[20];
	static const sha1_t null;
};

// creation helper
class sha1_creator
{
public:
	// construction/destruction
	sha1_creator() { reset(); }

	// reset
	void reset();

	// append data
	void append(const void *data, uint32_t length);

	// finalize and compute the final digest
	sha1_t finish();

	// static wrapper to just get the digest from a block
	static sha1_t simple(const void *data, uint32_t length)
	{
		sha1_creator creator;
		creator.append(data, length);
		return creator.finish();
	}

protected:
	uint64_t m_cnt;
	std::array<uint32_t, 5> m_st;
	uint32_t m_buf[16];
};



// ======================> MD5

// final digest
struct md5_t
{
	bool operator==(const md5_t &rhs) const { return memcmp(m_raw, rhs.m_raw, sizeof(m_raw)) == 0; }
	bool operator!=(const md5_t &rhs) const { return memcmp(m_raw, rhs.m_raw, sizeof(m_raw)) != 0; }
	operator uint8_t *() { return m_raw; }
	bool from_string(std::string_view string);
	std::string as_string() const;
	uint8_t m_raw[16];
	static const md5_t null;
};

// creation helper
class md5_creator
{
public:
	// construction/destruction
	md5_creator() { reset(); }

	// reset
	void reset() { MD5Init(&m_context); }

	// append data
	void append(const void *data, uint32_t length) { MD5Update(&m_context, reinterpret_cast<const unsigned char *>(data), length); }

	// finalize and compute the final digest
	md5_t finish()
	{
		md5_t result;
		MD5Final(result.m_raw, &m_context);
		return result;
	}

	// static wrapper to just get the digest from a block
	static md5_t simple(const void *data, uint32_t length)
	{
		md5_creator creator;
		creator.append(data, length);
		return creator.finish();
	}

protected:
	// internal state
	struct MD5Context   m_context;      // internal context
};



// ======================> CRC-32

// final digest
struct crc32_t
{
	crc32_t() { }
	constexpr crc32_t(const crc32_t &rhs) = default;
	constexpr crc32_t(const uint32_t crc) : m_raw(crc) { }

	constexpr bool operator==(const crc32_t &rhs) const { return m_raw == rhs.m_raw; }
	constexpr bool operator!=(const crc32_t &rhs) const { return m_raw != rhs.m_raw; }

	crc32_t &operator=(const crc32_t &rhs) = default;
	crc32_t &operator=(const uint32_t crc) { m_raw = crc; return *this; }

	constexpr operator uint32_t() const { return m_raw; }

	bool from_string(std::string_view string);
	std::string as_string() const;

	uint32_t m_raw;

	static const crc32_t null;
};

// creation helper
class crc32_creator
{
public:
	// construction/destruction
	crc32_creator() { reset(); }

	// reset
	void reset() { m_accum.m_raw = 0; }

	// append data
	void append(const void *data, uint32_t length);

	// finalize and compute the final digest
	crc32_t finish() { return m_accum; }

	// static wrapper to just get the digest from a block
	static crc32_t simple(const void *data, uint32_t length)
	{
		crc32_creator creator;
		creator.append(data, length);
		return creator.finish();
	}

protected:
	// internal state
	crc32_t             m_accum;        // internal accumulator
};



// ======================> CRC-16

// final digest
struct crc16_t
{
	crc16_t() { }
	constexpr crc16_t(const crc16_t &rhs) = default;
	constexpr crc16_t(const uint16_t crc) : m_raw(crc) { }

	constexpr bool operator==(const crc16_t &rhs) const { return m_raw == rhs.m_raw; }
	constexpr bool operator!=(const crc16_t &rhs) const { return m_raw != rhs.m_raw; }

	crc16_t &operator=(const crc16_t &rhs) = default;
	crc16_t &operator=(const uint16_t crc) { m_raw = crc; return *this; }

	constexpr operator uint16_t() const { return m_raw; }

	bool from_string(std::string_view string);
	std::string as_string() const;

	uint16_t m_raw;

	static const crc16_t null;
};

// creation helper
class crc16_creator
{
public:
	// construction/destruction
	crc16_creator() { reset(); }

	// reset
	void reset() { m_accum.m_raw = 0xffff; }

	// append data
	void append(const void *data, uint32_t length);

	// finalize and compute the final digest
	crc16_t finish() { return m_accum; }

	// static wrapper to just get the digest from a block
	static crc16_t simple(const void *data, uint32_t length)
	{
		crc16_creator creator;
		creator.append(data, length);
		return creator.finish();
	}

protected:
	// internal state
	crc16_t             m_accum;        // internal accumulator
};



// ======================> SUM-16

// final digest
struct sum16_t
{
	sum16_t() { }
	constexpr sum16_t(const sum16_t &rhs) = default;
	constexpr sum16_t(const uint16_t sum) : m_raw(sum) { }

	constexpr bool operator==(const sum16_t &rhs) const { return m_raw == rhs.m_raw; }
	constexpr bool operator!=(const sum16_t &rhs) const { return m_raw != rhs.m_raw; }

	sum16_t &operator=(const sum16_t &rhs) = default;
	sum16_t &operator=(const uint16_t sum) { m_raw = sum; return *this; }

	constexpr operator uint16_t() const { return m_raw; }

	bool from_string(std::string_view string);
	std::string as_string() const;

	uint16_t m_raw;

	static const sum16_t null;
};

// creation helper
class sum16_creator
{
public:
	// construction/destruction
	sum16_creator() { reset(); }

	// reset
	void reset() { m_accum.m_raw = 0; }

	// append data
	void append(const void *data, uint32_t length);

	// finalize and compute the final digest
	sum16_t finish() { return m_accum; }

	// static wrapper to just get the digest from a block
	static sum16_t simple(const void *data, uint32_t length)
	{
		sum16_creator creator;
		creator.append(data, length);
		return creator.finish();
	}

protected:
	// internal state
	sum16_t             m_accum;        // internal accumulator
};

} // namespace util

namespace std {

template <> struct hash<::util::crc32_t>
{
	typedef ::util::crc32_t argument_type;
	typedef std::size_t result_type;
	result_type operator()(argument_type const & s) const { return std::hash<std::uint32_t>()(s); }
};

template <> struct hash<::util::crc16_t>
{
	typedef ::util::crc16_t argument_type;
	typedef std::size_t result_type;
	result_type operator()(argument_type const & s) const { return std::hash<std::uint16_t>()(s); }
};

} // namespace std

#endif // MAME_UTIL_HASHING_H
