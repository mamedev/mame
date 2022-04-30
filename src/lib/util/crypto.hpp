// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_UTIL_CRYPTO_HPP
#define MAME_UTIL_CRYPTO_HPP

#pragma once

#include "hashing.h"

#include <cstdint>
#include <limits>
#include <string>


inline std::string sha1_encode(const std::string& input)
{
	util::sha1_creator digester;
	const char *ptr = input.c_str();
	std::string::size_type remain = input.length();
	while (remain > std::numeric_limits<uint32_t>::max())
	{
		digester.append(ptr, std::numeric_limits<uint32_t>::max());
		ptr += std::numeric_limits<uint32_t>::max();
		remain -= std::numeric_limits<uint32_t>::max();
	}
	digester.append(ptr, uint32_t(remain));

	return std::string(reinterpret_cast<const char *>(digester.finish().m_raw), 20);
}

#endif // MAME_UTIL_CRYPTO_HPP
