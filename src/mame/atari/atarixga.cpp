// license:BSD-3-Clause
// copyright-holders:Morten Shearman Kirkegaard, Samuel Neves, Peter Wilhelmsen, Andrea Bogazzi
/*************************************************************************

    atarixga.cpp

    Atari XGA encryption FPGAs

 **************************************************************************

    Part numbers:

        136094-0072   Moto Frenzy
        136095-0072   Space Lords
        ?             Road Riot's Revenge
        136094-0004A  Primal Rage (see below; feedback taps for unseen
                      character words and the step count for unseen key
                      bytes are still unknown)
        ?             T-Mek

*************************************************************************/

#include "emu.h"
#include "atarixga.h"

#include <algorithm>
#include <bit>

#define LOG_QUERY (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"



ALLOW_SAVE_TYPE(atari_136094_0072_device::fpga_mode)
ALLOW_SAVE_TYPE(atari_136095_0072_device::fpga_mode)
ALLOW_SAVE_TYPE(atari_136094_0004a_device::fpga_mode)


/*************************************
 *
 *  Decryption
 *
 *************************************/

constexpr uint16_t parity(uint16_t x)
{
	return std::popcount(x) & 1;
}


/*************************************
 *
 *  136094-0072 (Moto Frenzy)
 *
 *************************************/

DEFINE_DEVICE_TYPE(ATARI_136094_0072, atari_136094_0072_device, "136094_0072", "Atari 136094-0072 XGA")

atari_136094_0072_device::atari_136094_0072_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: atari_xga_device(mconfig, ATARI_136094_0072, tag, owner, clock)
{
}

void atari_136094_0072_device::device_start()
{
	m_ram = std::make_unique<uint16_t []>(RAM_WORDS);

	save_pointer(NAME(m_ram), RAM_WORDS);
	save_item(NAME(m_mode));
	save_item(NAME(m_address));
	save_item(NAME(m_ciphertext));
}

void atari_136094_0072_device::device_reset()
{
	std::fill_n(m_ram.get(), RAM_WORDS, 0);
	m_mode = FPGA_RESET;
	m_address = 0;
	m_ciphertext = 0;
}


uint16_t atari_136094_0072_device::lfsr1(uint16_t x)
{
	uint16_t const bit = parity(x & 0x8016);
	return (x << 1) | bit;
}

uint16_t atari_136094_0072_device::lfsr2(uint16_t x)
{
	uint16_t const bit = parity(x & 0x002d);
	return (x >> 1) | (bit << 15);
}

uint16_t atari_136094_0072_device::powers2(uint8_t k, uint16_t x)
{
	static const uint16_t L[16] =
	{
		0x5e85, 0xbd0b, 0x2493, 0x17a3,
		0x2f47, 0x0005, 0x000b, 0x0017,
		0x002f, 0x005e, 0x00bd, 0x017a,
		0x02f4, 0x05e8, 0x0bd0, 0x17a1
	};

	uint16_t t = (x == 16) ? (L[4] ^ L[5]) : L[x];

	for (size_t i = 0; i < k; ++i)
		t = lfsr1(t);

	return t;
}

uint16_t atari_136094_0072_device::decipher(uint8_t k, uint16_t c)
{
	/* key 0x10 is special, it has 15 "identical twins". */
	static const uint8_t kmap[128] =
	{
		0x6b, 0x11, 0x1b, 0x19, 0x4b, 0x50, 0x17, 0x09,
		0x5d, 0x69, 0x43, 0x33, 0x0f, 0x0c, 0x28, 0x3f,
		0x00, 0x20, 0x15, 0x3c, 0x57, 0x38, 0x00, 0x07,
		0x49, 0x25, 0x61, 0x2f, 0x2b, 0x4e, 0x64, 0x00,
		0x45, 0x41, 0x6d, 0x52, 0x31, 0x66, 0x22, 0x59,
		0x00, 0x70, 0x6f, 0x5b, 0x46, 0x6e, 0x67, 0x5a,
		0x26, 0x30, 0x2c, 0x65, 0x21, 0x3d, 0x58, 0x00,
		0x5e, 0x44, 0x0d, 0x40, 0x6c, 0x1c, 0x51, 0x0a,
		0x35, 0x2a, 0x13, 0x4d, 0x63, 0x00, 0x00, 0x3a,
		0x00, 0x48, 0x54, 0x24, 0x60, 0x1e, 0x2e, 0x01,
		0x56, 0x03, 0x37, 0x00, 0x04, 0x00, 0x05, 0x06,
		0x00, 0x55, 0x1f, 0x02, 0x36, 0x14, 0x00, 0x3b,
		0x5f, 0x0e, 0x1d, 0x0b, 0x27, 0x2d, 0x3e, 0x00,
		0x00, 0x5c, 0x47, 0x68, 0x42, 0x53, 0x32, 0x23,
		0x4a, 0x62, 0x4f, 0x00, 0x00, 0x16, 0x39, 0x08,
		0x6a, 0x34, 0x10, 0x29, 0x12, 0x1a, 0x4c, 0x18
	};

	/* Only 128 keys internally, if high bit set, then find the 7-bit "twin" by xor 0xA8. */
	if (k & 0x80)
		k ^= 0xa8;

	k = kmap[k];

	if (!(c & (c - 1)))
		return powers2(k, std::countr_zero(c));

	uint16_t p = 0;

	for (uint16_t bit = 0; bit < 5; ++bit)
	{
		if (BIT(c, bit))
			p ^= powers2(k, bit);
	}

	for (uint16_t bit = 5; bit < 16; ++bit)
	{
		if (BIT(c, bit))
			p ^= powers2(k, bit + 1);
	}

	uint16_t x = 0x8010;
	for (uint16_t i = 0; i < k + 3; ++i)
	{
		if (x == c)
			return (p == 1) ? 0 : lfsr2(p);
		x = lfsr2(x);
	}

	return p;
}


/*************************************
 *
 *  Write/Read access
 *
 *************************************/

void atari_136094_0072_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (m_mode)
	{
		case FPGA_RESET:
			return;

		case FPGA_SETKEY:
			/* Write table to FPGA SRAM. */
			if (ACCESSING_BITS_16_31)
				m_ram[offset << 1] = uint16_t(data >> 16);
			if (ACCESSING_BITS_0_15)
				m_ram[(offset << 1) + 1] = uint16_t(data & 0xffff);
			break;

		case FPGA_DECIPHER:
			/* Send Ciphertext to FPGA for decryption. */
			if (ACCESSING_BITS_16_31)
			{
				m_address = offset << 2;
				m_ciphertext = uint16_t(data >> 16);
			}
			if (ACCESSING_BITS_0_15)
			{
				m_address = (offset << 2) + 2;
				m_ciphertext = uint16_t(data & 0xffff);
			}
			break;
	}
}

uint32_t atari_136094_0072_device::read(offs_t offset, uint32_t mem_mask)
{
	fpga_mode new_mode = m_mode;
	switch (offset << 2)
	{
		case 0x0fc0:
			new_mode = FPGA_RESET;
			break;
		case 0x0010:
			new_mode = FPGA_SETKEY;
			break;
		case 0x0020:
			new_mode = FPGA_DECIPHER;
			break;
	}
	if (!machine().side_effects_disabled())
		m_mode = new_mode;

	if (new_mode == FPGA_RESET)
		return 0;

	uint32_t plaintext = 0;
	if (new_mode == FPGA_DECIPHER)
	{
		uint16_t address = (offset << 2) - 0x400;

		if (ACCESSING_BITS_0_15)
			address += 2;

		/* Reply with decrypted plaintext */
		if (address == m_address)
		{
			/* Algorithm to select key byte based on offset. */
			uint16_t const key_offset = bitswap<10>(address, 10, 9, 5, 7, 6, 1, 3, 8, 2, 4) ^ 0x099;
			uint16_t const key_byte = m_ram[key_offset];

			/* And now for the full magic. */
			plaintext = decipher(key_byte, m_ciphertext);

			if (ACCESSING_BITS_16_31)
				plaintext <<= 16;
		}
	}

	return plaintext;
}


/*************************************
 *
 *  136095-0072 (Space Lords)
 *
 *************************************/

DEFINE_DEVICE_TYPE(ATARI_136095_0072, atari_136095_0072_device, "136095_0072", "Atari 136095-0072 XGA")

atari_136095_0072_device::atari_136095_0072_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: atari_xga_device(mconfig, ATARI_136095_0072, tag, owner, clock)
	, m_mode(FPGA_SETKEY)
	, m_poly_lsb(0)
	, m_reply(0)
{
}

void atari_136095_0072_device::device_start()
{
	m_ram = std::make_unique<uint16_t []>(RAM_WORDS);
	save_pointer(NAME(m_ram), RAM_WORDS);

	save_item(NAME(m_update.addr));
	save_item(NAME(m_update.data));
	save_item(NAME(m_mode));
	save_item(NAME(m_poly_lsb));
	save_item(NAME(m_reply));
}

void atari_136095_0072_device::device_reset()
{
	std::fill_n(m_ram.get(), RAM_WORDS, 0);
}


uint16_t atari_136095_0072_device::lfsr1(uint16_t x)
{
	uint16_t const bit = parity(x & (0xc100 | m_poly_lsb));
	return (x << 1) | bit;
}

uint16_t atari_136095_0072_device::lfsr2(uint16_t x)
{
	uint16_t const bit = parity(x & (0x8201 | (m_poly_lsb << 1)));
	return (x >> 1) | (bit << 15);
}

uint16_t atari_136095_0072_device::powers2(uint8_t k, uint16_t x)
{
	uint16_t t = 1 << (x % 16);

	size_t const n = k + (((x == 15) || (x == 16)) ? 13 : 14);

	for (size_t i = 0; i < n; ++i)
		t = lfsr1(t);

	return t;
};

uint16_t atari_136095_0072_device::decipher(uint8_t k, uint16_t c)
{
	/* key 0x00 is special, it has 15 "identical twins". */
	static const uint8_t kmap[128] =
	{
		0x00, 0x3c, 0x0d, 0x0b, 0x5e, 0x09, 0x2a, 0x31,
		0x00, 0x56, 0x11, 0x4d, 0x14, 0x34, 0x3a, 0x44,
		0x24, 0x41, 0x51, 0x28, 0x1e, 0x2f, 0x68, 0x00,
		0x5c, 0x49, 0x18, 0x04, 0x37, 0x00, 0x07, 0x6b,
		0x58, 0x46, 0x0f, 0x60, 0x4b, 0x6d, 0x53, 0x20,
		0x00, 0x70, 0x62, 0x6f, 0x59, 0x61, 0x6e, 0x54,
		0x4a, 0x19, 0x38, 0x6c, 0x42, 0x52, 0x1f, 0x01,
		0x57, 0x12, 0x15, 0x45, 0x3d, 0x0e, 0x5f, 0x32,
		0x4f, 0x36, 0x00, 0x2c, 0x06, 0x00, 0x26, 0x6a,
		0x64, 0x5b, 0x48, 0x22, 0x17, 0x3f, 0x1b, 0x03,
		0x66, 0x1d, 0x2e, 0x00, 0x67, 0x00, 0x00, 0x00,
		0x65, 0x23, 0x40, 0x1c, 0x50, 0x2d, 0x00, 0x27,
		0x13, 0x16, 0x3e, 0x33, 0x1a, 0x39, 0x43, 0x02,
		0x00, 0x63, 0x5a, 0x55, 0x47, 0x10, 0x4c, 0x21,
		0x5d, 0x05, 0x00, 0x08, 0x25, 0x29, 0x30, 0x69,
		0x00, 0x4e, 0x35, 0x3b, 0x00, 0x0c, 0x0a, 0x2b,
	};

	/* Only 128 keys internally, if high bit set, then find the 7-bit "twin" by xor 0xA8. */
	if (k & 0x80)
		k ^= 0xa8;

	k = kmap[k];

	if (!(c & (c - 1)))
		return powers2(k, std::countr_zero(c));

	uint16_t p = 0;

	for (uint16_t i = 0; i < 15; ++i)
	{
		if (BIT(c, i))
			p ^= powers2(k, i);
	}

	if (c & 0x8000)
		p ^= powers2(k, 16);

	uint16_t x = 0xc000;
	for (uint16_t i = 0; i < k + 13; ++i)
	{
		if (x == c)
			return (p == 1) ? 0 : lfsr2(p);

		x = lfsr2(x);
	}

	return p;
}

void atari_136095_0072_device::polylsb_write(offs_t offset, uint32_t data)
{
	m_update.addr = offset;
	m_update.data[offset] = data;
}

uint32_t atari_136095_0072_device::polylsb_read(offs_t offset, uint32_t mem_mask)
{
	if ((m_update.addr == offset) && !machine().side_effects_disabled())
	{
		if (ACCESSING_BITS_16_31)
			m_poly_lsb = (m_update.data[offset] >> 16) & 0xff;
		else
			m_poly_lsb = m_update.data[offset] & 0xff;
	}
	return m_update.data[offset];
}

void atari_136095_0072_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint16_t value = 0;

	uint16_t address = offset << 2;
	if (ACCESSING_BITS_16_31)
	{
		value = uint16_t(data >> 16);
	}
	if (ACCESSING_BITS_0_15)
	{
		address += 2;
		value = uint16_t(data & 0xffff);
	}

	switch (m_mode)
	{
		case FPGA_SETKEY:
			/* Write table to FPGA SRAM. */
			if (ACCESSING_BITS_16_31)
				m_ram[offset << 1] = value;
			if (ACCESSING_BITS_0_15)
				m_ram[(offset << 1) + 1] = value;
			break;

		case FPGA_DECIPHER:
			/* Send Ciphertext to FPGA for decryption. */
			uint16_t key_offset, key_byte;

			/* Algorithm to select key byte based on offset. */
			key_offset = bitswap<12>(address, 12, 11, 10, 9, 6, 3, 5, 4, 1, 7, 2, 8) ^ 0x0b3;
			key_byte = m_ram[key_offset];
			m_reply = decipher(key_byte, value);
			break;

		case FPGA_PROCESS:
		case FPGA_RESULT:
		default:
			break;
	}
}

uint32_t atari_136095_0072_device::read(offs_t offset, uint32_t mem_mask)
{
	uint16_t address = offset << 2;
	if (ACCESSING_BITS_0_15)
		address += 2;

	fpga_mode new_mode = m_mode;
	uint32_t reply = 0;
	switch (address)
	{
		case 0x0020:
			new_mode = FPGA_SETKEY;
			break;
		case 0x0042:
			new_mode = FPGA_DECIPHER;
			break;
		case 0x0c00:
			new_mode = FPGA_PROCESS;
			reply = -1;
			break;
		case 0x0fc0:
			new_mode = FPGA_RESULT;
			reply = m_reply << 16;
			break;
		default:
			break;
	}
	if (!machine().side_effects_disabled())
		m_mode = new_mode;

	return reply;
}


/*************************************
 *
 *  136094-0004A (Primal Rage)
 *
 *  Recovered from the game's ciphertext tables and the plaintext the
 *  PlayStation port stores in their place (4177 known pairs, all
 *  reproduced except three PSX entries that differ by one unit).
 *
 *  The chip loads the 16-bit ciphertext into a Fibonacci LFSR and clocks
 *  it n times, n = kmap[key byte] (16..125). The feedback mask depends on
 *  the per-character word the game writes before a query; the key byte
 *  is selected from the 2K uploaded key bytes by a bit permutation of the
 *  query index. If the LFSR passes through state 0x0001 while clocking,
 *  the result is one clock short (zero if that happens on the last clock),
 *  as on the 136095-0072.
 *
 *************************************/

DEFINE_DEVICE_TYPE(ATARI_136094_0004A, atari_136094_0004a_device, "136094_0004a", "Atari 136094-0004A XGA")

// byte offsets inside the 0xD80000 color RAM window the chip is overlaid on
static constexpr offs_t PR_SETKEY   = 0x44010; // read: start key upload
static constexpr offs_t PR_DECIPHER = 0x44022; // read: start a query
static constexpr offs_t PR_STATUS   = 0x44700; // read: bit 15 = result ready
static constexpr offs_t PR_DONE0    = 0x4c7c0; // read: end of key upload (checksum path)
static constexpr offs_t PR_RESULT   = 0x4c7c2; // read: query result
static constexpr offs_t PR_DONE4    = 0x4c7c4; // read: end of key upload (boot path)
static constexpr offs_t PR_DATA     = 0x47800; // write: key byte or ciphertext, one word per index
static constexpr offs_t PR_CHAR0    = 0x48700; // write: character word (player 1 path)
static constexpr offs_t PR_CHAR1    = 0x64000; // write: character word (player 2 path)
static constexpr offs_t PR_CHAR2    = 0x6c000; // write: character word (player 2 path)

atari_136094_0004a_device::atari_136094_0004a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atari_xga_device(mconfig, ATARI_136094_0004A, tag, owner, clock)
{
}

void atari_136094_0004a_device::device_start()
{
	m_ram = std::make_unique<uint16_t []>(RAM_WORDS);

	save_pointer(NAME(m_ram), RAM_WORDS);
	save_item(NAME(m_mode));
	save_item(NAME(m_taps));
	save_item(NAME(m_reply));
}

void atari_136094_0004a_device::device_reset()
{
	std::fill_n(m_ram.get(), RAM_WORDS, 0);
	m_mode = FPGA_IDLE;
	m_taps = 0;
	m_reply = 0;
}

uint16_t atari_136094_0004a_device::key_offset(offs_t index)
{
	return bitswap<11>(index, 10, 9, 8, 6, 7, 1, 0, 4, 2, 5, 3) ^ 0x096;
}

uint16_t atari_136094_0004a_device::lfsr(uint16_t x) const
{
	return (x << 1) | parity(x & m_taps);
}

void atari_136094_0004a_device::set_character(uint16_t data)
{
	// the game writes one of these before each query (ROM table at 0xEB0C0,
	// indexed by character); how the chip derives the taps from the word is
	// not known, so only the five observed values are handled
	static const struct { uint16_t word, taps; } char_taps[5] =
	{
		{ 0x2694, 0xbcc8 }, // Sauron, Diablo
		{ 0x6ee0, 0xaed5 }, // Blizzard, Talon
		{ 0x34f7, 0x9d79 }, // Chaos
		{ 0x32b9, 0xfd10 }, // Vertigo
		{ 0x4d5a, 0x82a3 }  // Armadon
	};

	for (auto const &ct : char_taps)
	{
		if (ct.word == data)
		{
			LOGMASKED(LOG_QUERY, "%s: character word %04X -> taps %04X\n", machine().describe_context(), data, ct.taps);
			m_taps = ct.taps;
			return;
		}
	}

	// the player 1 path follows the character word with 0x8016, the
	// 136094-0072's feedback mask; its effect is unknown
	if (data != 0x8016)
		logerror("%s: unknown character word %04X\n", machine().describe_context(), data);
}

uint16_t atari_136094_0004a_device::decipher(offs_t index, uint16_t c) const
{
	// number of LFSR clocks per key byte; 0 = never used by the game, unknown
	static const uint8_t kmap[256] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x59, 0x17, 0x7b, 0x00, 0x00, 0x4f, 0x27, 0x00, 0x00, 0x3d, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x67, 0x00, 0x6f, 0x00, 0x65, 0x57, 0x00, 0x45, 0x4b, 0x00, 0x00,
		0x1b, 0x1d, 0x19, 0x00, 0x00, 0x61, 0x3f, 0x00, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x2d, 0x00, 0x00,
		0x1f, 0x00, 0x5b, 0x7d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x00, 0x53, 0x15, 0x6d, 0x79, 0x00,
		0x5d, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x6b, 0x2b, 0x13, 0x75, 0x33, 0x00, 0x37,
		0x00, 0x00, 0x69, 0x71, 0x25, 0x55, 0x4d, 0x00, 0x73, 0x00, 0x31, 0x00, 0x00, 0x00, 0x3b, 0x00,
		0x00, 0x00, 0x00, 0x41, 0x00, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x77,
		0x00, 0x00, 0x00, 0x63, 0x29, 0x00, 0x11, 0x2f, 0x00, 0x43, 0x00, 0x49, 0x00, 0x00, 0x35, 0x39,
		0x00, 0x00, 0x00, 0x64, 0x00, 0x22, 0x00, 0x42, 0x00, 0x6a, 0x20, 0x00, 0x00, 0x00, 0x1c, 0x00,
		0x66, 0x00, 0x54, 0x4a, 0x00, 0x6c, 0x00, 0x00, 0x58, 0x32, 0x00, 0x00, 0x50, 0x2c, 0x60, 0x00,
		0x70, 0x00, 0x00, 0x00, 0x7c, 0x48, 0x62, 0x52, 0x00, 0x26, 0x00, 0x12, 0x00, 0x00, 0x40, 0x00,
		0x00, 0x00, 0x6e, 0x00, 0x00, 0x38, 0x2e, 0x00, 0x46, 0x00, 0x7a, 0x36, 0x00, 0x76, 0x00, 0x00,
		0x00, 0x72, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x5c, 0x00, 0x00, 0x5e, 0x1a, 0x00,
		0x00, 0x00, 0x24, 0x44, 0x28, 0x14, 0x00, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x68, 0x56, 0x00, 0x30, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x4e, 0x00, 0x2a, 0x18, 0x00, 0x00, 0x00,
		0x4c, 0x00, 0x00, 0x3a, 0x00, 0x34, 0x10, 0x78, 0x00, 0x3c, 0x16, 0x00, 0x3e, 0x00, 0x00, 0x00,
	};

	uint8_t const k = m_ram[key_offset(index)];
	uint8_t const n = kmap[k];
	if (n == 0)
		logerror("%s: unknown key byte %02X at index %03X\n", machine().describe_context(), k, index);

	// untested: mirrors the 136095-0072, which treats a zero input as the
	// 17th power word rather than as an LFSR stuck at zero
	uint16_t x = !c ? 1 : c;
	int const clocks = !c ? (n - 1) : n;

	bool early = false;
	for (int i = 0; i < clocks; i++)
	{
		x = lfsr(x);
		if (x == 1)
			early = true;
	}

	if (early)
	{
		if (x == 1)
			return 0;
		x = c;
		for (int i = 0; i < clocks - 1; i++)
			x = lfsr(x);
	}

	LOGMASKED(LOG_QUERY, "%s: query index=%03X key=%02X n=%d taps=%04X cipher=%04X -> %04X%s\n",
			machine().describe_context(), index, k, n, m_taps, c, x, early ? " (early)" : "");
	return x;
}


/*************************************
 *
 *  Write/Read access
 *
 *************************************/

void atari_136094_0004a_device::write16(offs_t offset, uint16_t data)
{
	if (offset >= PR_DATA && offset < PR_DATA + RAM_WORDS * 2)
	{
		offs_t const index = (offset - PR_DATA) >> 1;
		if (m_mode == FPGA_SETKEY)
		{
			m_ram[index] = data & 0xff;
			return;
		}
		if (m_mode == FPGA_DECIPHER)
		{
			m_reply = decipher(index, data);
			return;
		}
	}

	if (offset == PR_CHAR0 || offset == PR_CHAR1 || offset == PR_CHAR2)
		set_character(data);
}

bool atari_136094_0004a_device::read16(offs_t offset, uint16_t &data)
{
	bool const side_effects = !machine().side_effects_disabled();
	switch (offset)
	{
		case PR_SETKEY:
			if (side_effects)
			{
				LOGMASKED(LOG_QUERY, "%s: key upload starts\n", machine().describe_context());
				m_mode = FPGA_SETKEY;
			}
			return false;

		case PR_DECIPHER:
			if (side_effects)
				m_mode = FPGA_DECIPHER;
			return false;

		case PR_DONE0:
		case PR_DONE4:
			if (side_effects && m_mode == FPGA_SETKEY)
				m_mode = FPGA_IDLE;
			return false;

		case PR_STATUS:
			data = 0x8000;
			return true;

		case PR_RESULT:
			if (m_mode != FPGA_DECIPHER)
				return false;
			data = m_reply;
			if (side_effects)
				m_mode = FPGA_IDLE;
			return true;

		default:
			return false;
	}
}

void atari_136094_0004a_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_16_31)
		write16(offset << 2, data >> 16);
	if (ACCESSING_BITS_0_15)
		write16((offset << 2) + 2, data & 0xffff);
}

uint32_t atari_136094_0004a_device::read(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	uint16_t data;

	if (ACCESSING_BITS_16_31 && read16(offset << 2, data))
		result |= uint32_t(data) << 16;
	if (ACCESSING_BITS_0_15 && read16((offset << 2) + 2, data))
		result |= data;

	return result;
}
