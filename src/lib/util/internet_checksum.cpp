// license:BSD-3-Clause
/***************************************************************************

    internet_checksum.h

    RFC 1071 Internet Checksum

***************************************************************************/

#include "internet_checksum.h"

/*
 * RFC 1071 - Checksum used in IPv4, TCP, UDP, etc.
 * Data is expected to be <= 65535 bytes total as 32-bit overflow could happen if larger.
 * (ethernet packets are <= 1518 bytes, for comparison and TCP/UDP have a 16-bit length field).
 * Data is treated as a collection of 16-bit, big endian unsigned integers.  If length is odd, the
 * last byte is 0-padded to create a 16-bit big endian unsigned integer.
 */

namespace util {

void internet_checksum_creator::append(const void *data, uint32_t length) noexcept
{
	const auto *src = reinterpret_cast<const uint8_t *>(data);

	while (length > 1)
	{
		m_accum += *src++ << 8;
		m_accum += *src++;
		length -= 2;
	}

	if (length)
		m_accum += *src << 8;
}

uint16_t internet_checksum_creator::finish() noexcept
{
	while (m_accum > 0xffff)
		m_accum = (m_accum & 0xffff) + (m_accum >> 16);
	return ~static_cast<uint16_t>(m_accum);
}

} // namespace util
