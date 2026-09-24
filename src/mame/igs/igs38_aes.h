// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// AES block operations for the IGS38 SPAcc cipher engine (FIPS 197).
// TODO: use 3rdparty AES decoders (3rdparty\aes256cbc only supports one of the needed modes)
#ifndef MAME_IGS_IGS38_AES_H
#define MAME_IGS_IGS38_AES_H

#pragma once

#include <algorithm>
#include <array>
#include <cstdint>

namespace igs38_aes
{

constexpr uint8_t multiply(uint8_t a, uint8_t b)
{
	uint8_t result = 0;
	for (; b; b >>= 1)
	{
		if (b & 1)
			result ^= a;
		a = (a << 1) ^ ((a & 0x80) ? 0x1b : 0);
	}
	return result;
}

constexpr auto make_sbox()
{
	std::array<uint8_t, 256> table{};
	for (unsigned i = 0; i < 256; ++i)
	{
		uint8_t inverse = 1, power = i;
		for (unsigned exponent = 254; exponent; exponent >>= 1)
		{
			if (exponent & 1)
				inverse = multiply(inverse, power);
			power = multiply(power, power);
		}
		uint8_t value = inverse ^ 0x63;
		for (unsigned shift = 1; shift <= 4; ++shift)
			value ^= uint8_t((inverse << shift) | (inverse >> (8 - shift)));
		table[i] = value;
	}
	return table;
}

inline constexpr auto SBOX = make_sbox();
inline constexpr auto INVERSE_SBOX = []
{
	std::array<uint8_t, 256> result{};
	for (unsigned i = 0; i < 256; ++i)
		result[SBOX[i]] = i;
	return result;
}();

class cipher
{
public:
	// The caller validates a 16-, 24- or 32-byte key.
	cipher(uint8_t const *key, unsigned bytes)
			: m_rounds(bytes / 4 + 6)
	{
		std::copy_n(key, bytes, m_key.begin());
		uint8_t rcon = 1;
		for (unsigned pos = bytes; pos < 16 * (m_rounds + 1); pos += 4)
		{
			uint8_t word[4];
			std::copy_n(&m_key[pos - 4], 4, word);
			if (!(pos % bytes))
			{
				uint8_t const first = word[0];
				for (unsigned i = 0; i < 3; ++i)
					word[i] = SBOX[word[i + 1]];
				word[3] = SBOX[first];
				word[0] ^= rcon;
				rcon = multiply(rcon, 2);
			}
			else if (bytes == 32 && pos % bytes == 16)
				for (auto &b : word)
					b = SBOX[b];
			for (unsigned i = 0; i < 4; ++i)
				m_key[pos + i] = m_key[pos + i - bytes] ^ word[i];
		}
	}

	void encrypt(uint8_t *state) const
	{
		add_key(state, 0);
		for (unsigned round = 1; round <= m_rounds; ++round)
		{
			for (unsigned i = 0; i < 16; ++i)
				state[i] = SBOX[state[i]];
			shift_rows(state, false);
			if (round != m_rounds)
				mix_columns(state, false);
			add_key(state, round);
		}
	}

	void decrypt(uint8_t *state) const
	{
		add_key(state, m_rounds);
		for (unsigned round = m_rounds; round; --round)
		{
			shift_rows(state, true);
			for (unsigned i = 0; i < 16; ++i)
				state[i] = INVERSE_SBOX[state[i]];
			add_key(state, round - 1);
			if (round != 1)
				mix_columns(state, true);
		}
	}

private:
	void add_key(uint8_t *state, unsigned round) const
	{
		for (unsigned i = 0; i < 16; ++i)
			state[i] ^= m_key[16 * round + i];
	}

	static void shift_rows(uint8_t *state, bool inverse)
	{
		uint8_t copy[16];
		std::copy_n(state, 16, copy);
		for (unsigned row = 0; row < 4; ++row)
			for (unsigned col = 0; col < 4; ++col)
				state[4 * col + row] = copy[4 * ((col + (inverse ? 4 - row : row)) % 4) + row];
	}

	static void mix_columns(uint8_t *state, bool inverse)
	{
		for (unsigned col = 0; col < 4; ++col)
		{
			uint8_t *s = state + 4 * col;
			if (inverse)
			{
				uint8_t const u = multiply(s[0] ^ s[2], 4), v = multiply(s[1] ^ s[3], 4);
				s[0] ^= u;
				s[2] ^= u;
				s[1] ^= v;
				s[3] ^= v;
			}
			uint8_t const sum = s[0] ^ s[1] ^ s[2] ^ s[3], first = s[0];
			for (unsigned i = 0; i < 3; ++i)
				s[i] ^= sum ^ multiply(s[i] ^ s[i + 1], 2);
			s[3] ^= sum ^ multiply(s[3] ^ first, 2);
		}
	}

	unsigned m_rounds;
	std::array<uint8_t, 240> m_key{};
};
} // namespace igs38_aes

#endif // MAME_IGS_IGS38_AES_H
