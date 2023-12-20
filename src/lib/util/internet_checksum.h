// license:BSD-3-Clause
/***************************************************************************

    internet_checksum.h

    RFC 1071 Internet Checksum

***************************************************************************/
#ifndef MAME_UTIL_INTERNET_CHECKSUM_H
#define MAME_UTIL_INTERNET_CHECKSUM_H

#pragma once

#include <cstdint>

namespace util {

class internet_checksum_creator
{
public:
	internet_checksum_creator() noexcept = default;

	void reset() noexcept { m_accum = 0; }

	void append(const void *data, uint32_t length) noexcept;
	void append(uint16_t data) noexcept
	{
		m_accum += data;
	}
	void append(uint32_t data) noexcept
	{
		m_accum += data & 0xffff;
		m_accum += data >> 16;
	}

	static uint16_t simple(const void *data, uint32_t length) noexcept
	{
		internet_checksum_creator creator;
		creator.append(data, length);
		return creator.finish();
	}

	uint16_t finish() noexcept;

private:
	uint32_t m_accum = 0;

};

} // namespace util

#endif // MAME_UTIL_INTERNET_CHECKSUM_H
