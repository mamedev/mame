// license:MIT
// copyright-holders:Miodrag Milanovic
#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include "base64.hpp"
#include "sha1.hpp"

inline std::string sha1_encode(const std::string& input)
{
	char message_digest[20];
	sha1::calc(input.c_str(),input.length(),reinterpret_cast<unsigned char*>(message_digest));

	return std::string(message_digest, sizeof(message_digest));

}
#endif  /* CRYPTO_HPP */

