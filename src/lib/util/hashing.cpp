// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    hashing.c

    Hashing helper classes.

***************************************************************************/

#include "hashing.h"

#include <zlib.h>

#include <iomanip>
#include <sstream>


namespace util {

//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

namespace {

//-------------------------------------------------
//  char_to_hex - return the hex value of a
//  character
//-------------------------------------------------

constexpr int char_to_hex(char c)
{
	return
			(c >= '0' && c <= '9') ? (c - '0') :
			(c >= 'a' && c <= 'f') ? (10 + c - 'a') :
			(c >= 'A' && c <= 'F') ? (10 + c - 'A') :
			-1;
}


constexpr uint32_t sha1_rol(uint32_t x, unsigned n)
{
	return (x << n) | (x >> (32 - n));
}

inline uint32_t sha1_b(uint32_t *data, unsigned i)
{
	uint32_t r = data[(i + 13) & 15U];
	r ^= data[(i + 8) & 15U];
	r ^= data[(i + 2) & 15U];
	r ^= data[i & 15U];
	r = sha1_rol(r, 1);
	data[i & 15U] = r;
	return r;
}

inline void sha1_r0(const uint32_t *data, std::array<uint32_t, 5> &d, unsigned i)
{
	d[i % 5] = d[i % 5] + ((d[(i + 3) % 5] & (d[(i + 2) % 5] ^ d[(i + 1) % 5])) ^ d[(i + 1) % 5]) + data[i] + 0x5a827999U + sha1_rol(d[(i + 4) % 5], 5);
	d[(i + 3) % 5] = sha1_rol(d[(i + 3) % 5], 30);
}

inline void sha1_r1(uint32_t *data, std::array<uint32_t, 5> &d, unsigned i)
{
	d[i % 5] = d[i % 5] + ((d[(i + 3) % 5] & (d[(i + 2) % 5] ^ d[(i + 1) % 5])) ^ d[(i + 1) % 5])+ sha1_b(data, i) + 0x5a827999U + sha1_rol(d[(i + 4) % 5], 5);
	d[(i + 3) % 5] = sha1_rol(d[(i + 3) % 5], 30);
}

inline void sha1_r2(uint32_t *data, std::array<uint32_t, 5> &d, unsigned i)
{
	d[i % 5] = d[i % 5] + (d[(i + 3) % 5] ^ d[(i + 2) % 5] ^ d[(i + 1) % 5]) + sha1_b(data, i) + 0x6ed9eba1U + sha1_rol(d[(i + 4) % 5], 5);
	d[(i + 3) % 5] = sha1_rol(d[(i + 3) % 5], 30);
}

inline void sha1_r3(uint32_t *data, std::array<uint32_t, 5> &d, unsigned i)
{
	d[i % 5] = d[i % 5] + (((d[(i + 3) % 5] | d[(i + 2) % 5]) & d[(i + 1) % 5]) | (d[(i + 3) % 5] & d[(i + 2) % 5])) + sha1_b(data, i) + 0x8f1bbcdcU + sha1_rol(d[(i + 4) % 5], 5);
	d[(i + 3) % 5] = sha1_rol(d[(i + 3) % 5], 30);
}

inline void sha1_r4(uint32_t *data, std::array<uint32_t, 5> &d, unsigned i)
{
	d[i % 5] = d[i % 5] + (d[(i + 3) % 5] ^ d[(i + 2) % 5] ^ d[(i + 1) % 5]) + sha1_b(data, i) + 0xca62c1d6U + sha1_rol(d[(i + 4) % 5], 5);
	d[(i + 3) % 5] = sha1_rol(d[(i + 3) % 5], 30);
}

inline void sha1_process(std::array<uint32_t, 5> &st, uint32_t *data)
{
	std::array<uint32_t, 5> d = st;
	unsigned i = 0U;
	while (i < 16U)
		sha1_r0(data, d, i++);
	while (i < 20U)
		sha1_r1(data, d, i++);
	while (i < 40U)
		sha1_r2(data, d, i++);
	while (i < 60U)
		sha1_r3(data, d, i++);
	while (i < 80U)
		sha1_r4(data, d, i++);
	for (i = 0U; i < 5U; i++)
		st[i] += d[i];
}

} // anonymous namespace



//**************************************************************************
//  CONSTANTS
//**************************************************************************

const crc16_t crc16_t::null = { 0 };
const crc32_t crc32_t::null = { 0 };
const md5_t md5_t::null = { { 0 } };
const sha1_t sha1_t::null = { { 0 } };



//**************************************************************************
//  SHA-1 HELPERS
//**************************************************************************

//-------------------------------------------------
//  from_string - convert from a string
//-------------------------------------------------

bool sha1_t::from_string(const char *string, int length)
{
	// must be at least long enough to hold everything
	memset(m_raw, 0, sizeof(m_raw));
	if (length == -1)
		length = strlen(string);
	if (length < 2 * sizeof(m_raw))
		return false;

	// iterate through our raw buffer
	for (auto & elem : m_raw)
	{
		int upper = char_to_hex(*string++);
		int lower = char_to_hex(*string++);
		if (upper == -1 || lower == -1)
			return false;
		elem = (upper << 4) | lower;
	}
	return true;
}


//-------------------------------------------------
//  as_string - convert to a string
//-------------------------------------------------

std::string sha1_t::as_string() const
{
	std::ostringstream buffer;
	buffer.fill('0');
	buffer << std::hex;
	for (auto & elem : m_raw)
		buffer << std::setw(2) << unsigned(elem);
	return buffer.str();
}


//-------------------------------------------------
//  reset - prepare to digest a block of data
//-------------------------------------------------

void sha1_creator::reset()
{
	m_cnt = 0U;
	m_st[0] = 0xc3d2e1f0U;
	m_st[1] = 0x10325476U;
	m_st[2] = 0x98badcfeU;
	m_st[3] = 0xefcdab89U;
	m_st[4] = 0x67452301U;
}


//-------------------------------------------------
//  append - digest a block of data
//-------------------------------------------------

void sha1_creator::append(const void *data, uint32_t length)
{
#ifdef LSB_FIRST
	constexpr unsigned swizzle = 3U;
#else
	constexpr unsigned swizzle = 0U;
#endif
	uint32_t residual = (uint32_t(m_cnt) >> 3) & 63U;
	m_cnt += uint64_t(length) << 3;
	uint32_t offset = 0U;
	if (length >= (64U - residual))
	{
		if (residual)
		{
			for (offset = 0U; (offset + residual) < 64U; offset++)
				reinterpret_cast<uint8_t *>(m_buf)[(offset + residual) ^ swizzle] = reinterpret_cast<const uint8_t *>(data)[offset];
			sha1_process(m_st, m_buf);
		}
		while ((length - offset) >= 64U)
		{
			for (residual = 0U; residual < 64U; residual++, offset++)
				reinterpret_cast<uint8_t *>(m_buf)[residual ^ swizzle] = reinterpret_cast<const uint8_t *>(data)[offset];
			sha1_process(m_st, m_buf);
		}
		residual = 0U;
	}
	for ( ; offset < length; residual++, offset++)
		reinterpret_cast<uint8_t *>(m_buf)[residual ^ swizzle] = reinterpret_cast<const uint8_t *>(data)[offset];
}


//-------------------------------------------------
//  finish - compute final hash
//-------------------------------------------------

sha1_t sha1_creator::finish()
{
	const unsigned padlen = 64U - (63U & ((unsigned(m_cnt) >> 3) + 8U));
	uint8_t padbuf[64];
	padbuf[0] = 0x80;
	for (unsigned i = 1U; i < padlen; i++)
		padbuf[i] = 0x00;
	uint8_t lenbuf[8];
	for (unsigned i = 0U; i < 8U; i++)
		lenbuf[i] = uint8_t(m_cnt >> ((7U - i) << 3));
	append(padbuf, padlen);
	append(lenbuf, sizeof(lenbuf));
	sha1_t result;
	for (unsigned i = 0U; i < 20U; i++)
		result.m_raw[i] = uint8_t(m_st[4U - (i >> 2)] >> ((3U - (i & 3)) << 3));
	return result;
}



//**************************************************************************
//  MD-5 HELPERS
//**************************************************************************

//-------------------------------------------------
//  from_string - convert from a string
//-------------------------------------------------

bool md5_t::from_string(const char *string, int length)
{
	// must be at least long enough to hold everything
	memset(m_raw, 0, sizeof(m_raw));
	if (length == -1)
		length = strlen(string);
	if (length < 2 * sizeof(m_raw))
		return false;

	// iterate through our raw buffer
	for (auto & elem : m_raw)
	{
		int upper = char_to_hex(*string++);
		int lower = char_to_hex(*string++);
		if (upper == -1 || lower == -1)
			return false;
		elem = (upper << 4) | lower;
	}
	return true;
}


//-------------------------------------------------
//  as_string - convert to a string
//-------------------------------------------------

std::string md5_t::as_string() const
{
	std::ostringstream buffer;
	buffer.fill('0');
	buffer << std::hex;
	for (auto & elem : m_raw)
		buffer << std::setw(2) << unsigned(elem);
	return buffer.str();
}



//**************************************************************************
//  CRC-32 HELPERS
//**************************************************************************

//-------------------------------------------------
//  from_string - convert from a string
//-------------------------------------------------

bool crc32_t::from_string(const char *string, int length)
{
	// must be at least long enough to hold everything
	m_raw = 0;
	if (length == -1)
		length = strlen(string);
	if (length < 2 * sizeof(m_raw))
		return false;

	// iterate through our raw buffer
	m_raw = 0;
	for (int bytenum = 0; bytenum < sizeof(m_raw) * 2; bytenum++)
	{
		int nibble = char_to_hex(*string++);
		if (nibble == -1)
			return false;
		m_raw = (m_raw << 4) | nibble;
	}
	return true;
}


//-------------------------------------------------
//  as_string - convert to a string
//-------------------------------------------------

std::string crc32_t::as_string() const
{
	return string_format("%08x", m_raw);
}


//-------------------------------------------------
//  append - hash a block of data, appending to
//  the currently-accumulated value
//-------------------------------------------------

void crc32_creator::append(const void *data, uint32_t length)
{
	m_accum.m_raw = crc32(m_accum, reinterpret_cast<const Bytef *>(data), length);
}



//**************************************************************************
//  CRC-16 HELPERS
//**************************************************************************

//-------------------------------------------------
//  from_string - convert from a string
//-------------------------------------------------

bool crc16_t::from_string(const char *string, int length)
{
	// must be at least long enough to hold everything
	m_raw = 0;
	if (length == -1)
		length = strlen(string);
	if (length < 2 * sizeof(m_raw))
		return false;

	// iterate through our raw buffer
	m_raw = 0;
	for (int bytenum = 0; bytenum < sizeof(m_raw) * 2; bytenum++)
	{
		int nibble = char_to_hex(*string++);
		if (nibble == -1)
			return false;
		m_raw = (m_raw << 4) | nibble;
	}
	return true;
}

/**
 * @fn  std::string crc16_t::as_string() const
 *
 * @brief   -------------------------------------------------
 *            as_string - convert to a string
 *          -------------------------------------------------.
 *
 * @return  a std::string.
 */

std::string crc16_t::as_string() const
{
	return string_format("%04x", m_raw);
}

/**
 * @fn  void crc16_creator::append(const void *data, uint32_t length)
 *
 * @brief   -------------------------------------------------
 *            append - hash a block of data, appending to the currently-accumulated value
 *          -------------------------------------------------.
 *
 * @param   data    The data.
 * @param   length  The length.
 */

void crc16_creator::append(const void *data, uint32_t length)
{
	static const uint16_t s_table[256] =
	{
		0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
		0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
		0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
		0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
		0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
		0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
		0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
		0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
		0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
		0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
		0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
		0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
		0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
		0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
		0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
		0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
		0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
		0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
		0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
		0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
		0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
		0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
		0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
		0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
		0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
		0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
		0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
		0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
		0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
		0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
		0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
		0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
	};

	const auto *src = reinterpret_cast<const uint8_t *>(data);

	// fetch the current value into a local and rip through the source data
	uint16_t crc = m_accum.m_raw;
	while (length-- != 0)
		crc = (crc << 8) ^ s_table[(crc >> 8) ^ *src++];
	m_accum.m_raw = crc;
}

} // namespace util
