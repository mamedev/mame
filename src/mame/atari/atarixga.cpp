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

#define LOG_QUERY (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"



/*************************************
 *
 *  Decryption
 *
 *************************************/

static uint16_t ctz(uint16_t x)
{
	uint16_t n = 0;
	if (x == 0) return 16;
	if (!(x & 0x00FF)) n += 8, x >>= 8;
	if (!(x & 0x000F)) n += 4, x >>= 4;
	if (!(x & 0x0003)) n += 2, x >>= 2;
	if (!(x & 0x0001)) n += 1, x >>= 1;
	return n;
}

static size_t popcount(uint16_t x)
{
	size_t count = 0;
	while (x != 0)
	{
		count += 1;
		x &= x - 1;
	}
	return count;
}

static uint16_t parity(uint16_t x)
{
	return popcount(x) & 1;
}


/*************************************
 *
 *  136094-0072 (Moto Frenzy)
 *
 *************************************/

DEFINE_DEVICE_TYPE(ATARI_136094_0072, atari_136094_0072_device, "136094_0072", "Atari 136094-0072 XGA")

atari_136094_0072_device::atari_136094_0072_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atari_xga_device(mconfig, ATARI_136094_0072, tag, owner, clock)
{
}

void atari_136094_0072_device::device_start()
{
	m_ram = std::make_unique<uint16_t[]>(RAM_WORDS);

	save_pointer(NAME(m_ram), RAM_WORDS);
	save_item(NAME(m_address));
	save_item(NAME(m_ciphertext));
}

void atari_136094_0072_device::device_reset()
{
	memset(m_ram.get(), 0, RAM_WORDS * sizeof(uint16_t));
	m_mode = FPGA_RESET;
	m_address = 0;
	m_ciphertext = 0;
}


uint16_t atari_136094_0072_device::lfsr1(uint16_t x)
{
	const uint16_t bit = parity(x & 0x8016);
	return (x << 1) | bit;
}

uint16_t atari_136094_0072_device::lfsr2(uint16_t x)
{
	uint16_t bit = parity(x & 0x002D);
	return (x >> 1) | (bit << 15);
}

uint16_t atari_136094_0072_device::powers2(uint8_t k, uint16_t x)
{
	static const uint16_t L[16] =
	{
		0x5E85,0xBD0B,0x2493,0x17A3,
		0x2F47,0x0005,0x000B,0x0017,
		0x002F,0x005E,0x00BD,0x017A,
		0x02F4,0x05E8,0x0BD0,0x17A1
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
		0x6B,0x11,0x1B,0x19,0x4B,0x50,0x17,0x09,
		0x5D,0x69,0x43,0x33,0x0F,0x0C,0x28,0x3F,
		0x00,0x20,0x15,0x3C,0x57,0x38,0x00,0x07,
		0x49,0x25,0x61,0x2F,0x2B,0x4E,0x64,0x00,
		0x45,0x41,0x6D,0x52,0x31,0x66,0x22,0x59,
		0x00,0x70,0x6F,0x5B,0x46,0x6E,0x67,0x5A,
		0x26,0x30,0x2C,0x65,0x21,0x3D,0x58,0x00,
		0x5E,0x44,0x0D,0x40,0x6C,0x1C,0x51,0x0A,
		0x35,0x2A,0x13,0x4D,0x63,0x00,0x00,0x3A,
		0x00,0x48,0x54,0x24,0x60,0x1E,0x2E,0x01,
		0x56,0x03,0x37,0x00,0x04,0x00,0x05,0x06,
		0x00,0x55,0x1F,0x02,0x36,0x14,0x00,0x3B,
		0x5F,0x0E,0x1D,0x0B,0x27,0x2D,0x3E,0x00,
		0x00,0x5C,0x47,0x68,0x42,0x53,0x32,0x23,
		0x4A,0x62,0x4F,0x00,0x00,0x16,0x39,0x08,
		0x6A,0x34,0x10,0x29,0x12,0x1A,0x4C,0x18
	};

	uint16_t p = 0;

	/* Only 128 keys internally, if high bit set,
	then find the 7-bit "twin" by xor 0xA8. */
	if (k & 0x80)
		k ^= 0xA8;

	k = kmap[k];

	if ((c & (c - 1)) == 0)
		return powers2(k, ctz(c));

	for (uint16_t bit = 0; bit < 5; ++bit)
	{
		if ((c >> bit) & 1)
		{
			p ^= powers2(k, bit);
		}
	}

	for (uint16_t bit = 5; bit < 16; ++bit)
	{
		if ((c >> bit) & 1)
		{
			p ^= powers2(k, bit + 1);
		}
	}

	uint16_t x = 0x8010;
	for (uint16_t i = 0; i < k + 3; ++i)
	{
		if (x == c)
		{
			return (p == 1) ? 0 : lfsr2(p);
		}
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
				m_ram[offset << 1] = uint16_t (data >> 16);
			if (ACCESSING_BITS_0_15)
				m_ram[(offset << 1) + 1] = uint16_t(data & 0xFFFF);
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
				m_ciphertext = uint16_t(data & 0xFFFF);
			}
			break;
	}
}

uint32_t atari_136094_0072_device::read(offs_t offset, uint32_t mem_mask)
{
	switch (offset << 2)
	{
		case 0x0FC0:
			m_mode = FPGA_RESET;
			break;
		case 0x0010:
			m_mode = FPGA_SETKEY;
			break;
		case 0x0020:
			m_mode = FPGA_DECIPHER;
			break;
	}

	if (m_mode == FPGA_RESET)
		return 0;

	uint32_t plaintext = 0;
	if (m_mode == FPGA_DECIPHER)
	{
		uint16_t address = (offset << 2) - 0x400;

		if (ACCESSING_BITS_0_15)
			address += 2;

		/* Reply with decrypted plaintext */
		if (address == m_address)
		{
			uint16_t key_offset, key_byte;

			/* Algorithm to select key byte based on offset. */
			key_offset = ((((address >>  4) & 1) ^ 1) << 0)
						^ ((((address >>  2) & 1) ^ 0) << 1)
						^ ((((address >>  8) & 1) ^ 0) << 2)
						^ ((((address >>  3) & 1) ^ 1) << 3)
						^ ((((address >>  1) & 1) ^ 1) << 4)
						^ ((((address >>  6) & 1) ^ 0) << 5)
						^ ((((address >>  7) & 1) ^ 0) << 6)
						^ ((((address >>  5) & 1) ^ 1) << 7)
						^ ((((address >>  9) & 1) ^ 0) << 8)
						^ ((((address >> 10) & 1) ^ 0) << 9);
			key_byte = m_ram[key_offset];

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

atari_136095_0072_device::atari_136095_0072_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atari_xga_device(mconfig, ATARI_136095_0072, tag, owner, clock)
{
}

void atari_136095_0072_device::device_start()
{
	m_ram = std::make_unique<uint16_t[]>(RAM_WORDS);
	save_pointer(NAME(m_ram), RAM_WORDS);

	save_item(NAME(m_update.addr));
	save_item(NAME(m_update.data));
	save_item(NAME(m_poly_lsb));
	save_item(NAME(m_reply));
}

void atari_136095_0072_device::device_reset()
{
	memset(m_ram.get(), 0, RAM_WORDS * sizeof(uint16_t));
}


uint16_t atari_136095_0072_device::lfsr1(uint16_t x)
{
	uint16_t bit = parity(x & (0xC100 | m_poly_lsb));
	return (x << 1) | bit;
}

uint16_t atari_136095_0072_device::lfsr2(uint16_t x)
{
	uint16_t bit = parity(x & (0x8201 | (m_poly_lsb << 1)));
	return (x >> 1) | (bit << 15);
}

uint16_t atari_136095_0072_device::powers2(uint8_t k, uint16_t x)
{
	size_t i, n;
	uint16_t t = 1 << (x % 16);

	if ((x == 15) || (x == 16))
		n = k + 13;
	else
		n = k + 14;

	for (i = 0; i < n; ++i)
		t = lfsr1(t);

	return t;
};

uint16_t atari_136095_0072_device::decipher(uint8_t k, uint16_t c)
{
	uint16_t i, p = 0;

	/* key 0x00 is special, it has 15 "identical twins". */
	static const uint8_t kmap[128] =
	{
		0x00,0x3C,0x0D,0x0B,0x5E,0x09,0x2A,0x31,
		0x00,0x56,0x11,0x4D,0x14,0x34,0x3A,0x44,
		0x24,0x41,0x51,0x28,0x1E,0x2F,0x68,0x00,
		0x5C,0x49,0x18,0x04,0x37,0x00,0x07,0x6B,
		0x58,0x46,0x0F,0x60,0x4B,0x6D,0x53,0x20,
		0x00,0x70,0x62,0x6F,0x59,0x61,0x6E,0x54,
		0x4A,0x19,0x38,0x6C,0x42,0x52,0x1F,0x01,
		0x57,0x12,0x15,0x45,0x3D,0x0E,0x5F,0x32,
		0x4F,0x36,0x00,0x2C,0x06,0x00,0x26,0x6A,
		0x64,0x5B,0x48,0x22,0x17,0x3F,0x1B,0x03,
		0x66,0x1D,0x2E,0x00,0x67,0x00,0x00,0x00,
		0x65,0x23,0x40,0x1C,0x50,0x2D,0x00,0x27,
		0x13,0x16,0x3E,0x33,0x1A,0x39,0x43,0x02,
		0x00,0x63,0x5A,0x55,0x47,0x10,0x4C,0x21,
		0x5D,0x05,0x00,0x08,0x25,0x29,0x30,0x69,
		0x00,0x4E,0x35,0x3B,0x00,0x0C,0x0A,0x2B,
	};

	/* Only 128 keys internally, if high bit set,
  then find the 7-bit "twin" by xor 0xA8. */
	if (k & 0x80) k ^= 0xA8;

	k = kmap[k];

	if ((c & (c - 1)) == 0) {
		return powers2(k, ctz(c));
	}

	for (i = 0; i < 15; ++i) {
		if (1 & (c >> i)) {
			p ^= powers2(k, i);
		}
	}

	if (c & 0x8000) {
		p ^= powers2(k, 16);
	}

	uint16_t x = 0xC000;
	for (i = 0; i < k + 13; ++i)
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
	if (m_update.addr == offset)
	{
		if (ACCESSING_BITS_16_31)
		{
			m_poly_lsb = (m_update.data[offset] >> 16) & 0xFF;
		}
		else
		{
			m_poly_lsb = m_update.data[offset] & 0xFF;
		}
	}
	return m_update.data[offset];
}

void atari_136095_0072_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint16_t address, value = 0;

	address = offset << 2;
	if (ACCESSING_BITS_16_31)
	{
		value = uint16_t (data >> 16);
	}
	if (ACCESSING_BITS_0_15)
	{
		address += 2;
		value = uint16_t (data & 0xFFFF);
	}

	switch (m_mode)
	{
		case FPGA_SETKEY:
			/* Write table to FPGA SRAM. */
			if (ACCESSING_BITS_16_31)
			{
				m_ram[offset << 1] = value;
			}
			if (ACCESSING_BITS_0_15)
			{
				m_ram[(offset << 1) + 1] = value;
			}
			break;

			/* Send Ciphertext to FPGA for decryption. */
		case FPGA_DECIPHER:
			uint16_t key_offset, key_byte;

			/* Algorithm to select key byte based on offset. */
			key_offset = ((((address >>  8) & 1) ^ 1) <<  0)
			| ((((address >>  2) & 1) ^ 1) <<  1)
			| ((((address >>  7) & 1) ^ 0) <<  2)
			| ((((address >>  1) & 1) ^ 0) <<  3)
			| ((((address >>  4) & 1) ^ 1) <<  4)
			| ((((address >>  5) & 1) ^ 1) <<  5)
			| ((((address >>  3) & 1) ^ 0) <<  6)
			| ((((address >>  6) & 1) ^ 1) <<  7)
			| ((((address >>  9) & 1) ^ 0) <<  8)
			| ((((address >> 10) & 1) ^ 0) <<  9)
			| ((((address >> 11) & 1) ^ 0) << 10)
			| ((((address >> 12) & 1) ^ 0) << 11);
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
	uint16_t address;
	uint32_t reply = 0;

	address = offset << 2;

	if (ACCESSING_BITS_0_15)
		address += 2;

	switch (address)
	{
		case 0x0020:
			m_mode = FPGA_SETKEY;
			break;
		case 0x0042:
			m_mode = FPGA_DECIPHER;
			break;
		case 0x0C00:
			m_mode = FPGA_PROCESS;
			reply = -1;
			break;
		case 0x0FC0:
			m_mode = FPGA_RESULT;
			reply = m_reply << 16;
			break;
		default:
			break;
	}

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
	m_ram = std::make_unique<uint16_t[]>(RAM_WORDS);

	save_pointer(NAME(m_ram), RAM_WORDS);
	save_item(NAME(m_mode));
	save_item(NAME(m_taps));
	save_item(NAME(m_reply));
}

void atari_136094_0004a_device::device_reset()
{
	memset(m_ram.get(), 0, RAM_WORDS * sizeof(uint16_t));
	m_mode = FPGA_IDLE;
	m_taps = 0;
	m_reply = 0;
}

uint16_t atari_136094_0004a_device::key_offset(offs_t index)
{
	return ((BIT(index,  3) ^ 0) <<  0)
		 | ((BIT(index,  5) ^ 1) <<  1)
		 | ((BIT(index,  2) ^ 1) <<  2)
		 | ((BIT(index,  4) ^ 0) <<  3)
		 | ((BIT(index,  0) ^ 1) <<  4)
		 | ((BIT(index,  1) ^ 0) <<  5)
		 | ((BIT(index,  7) ^ 0) <<  6)
		 | ((BIT(index,  6) ^ 1) <<  7)
		 | ((BIT(index,  8) ^ 0) <<  8)
		 | ((BIT(index,  9) ^ 0) <<  9)
		 | ((BIT(index, 10) ^ 0) << 10);
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
		0x00,0x00,0x00,0x00,0x00,0x59,0x17,0x7B,0x00,0x00,0x4F,0x27,0x00,0x00,0x3D,0x00,
		0x00,0x00,0x00,0x00,0x00,0x67,0x00,0x6F,0x00,0x65,0x57,0x00,0x45,0x4B,0x00,0x00,
		0x1B,0x1D,0x19,0x00,0x00,0x61,0x3F,0x00,0x5F,0x00,0x00,0x00,0x00,0x2D,0x00,0x00,
		0x1F,0x00,0x5B,0x7D,0x00,0x00,0x00,0x00,0x00,0x23,0x00,0x53,0x15,0x6D,0x79,0x00,
		0x5D,0x21,0x00,0x00,0x00,0x00,0x00,0x47,0x00,0x6B,0x2B,0x13,0x75,0x33,0x00,0x37,
		0x00,0x00,0x69,0x71,0x25,0x55,0x4D,0x00,0x73,0x00,0x31,0x00,0x00,0x00,0x3B,0x00,
		0x00,0x00,0x00,0x41,0x00,0x51,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x77,
		0x00,0x00,0x00,0x63,0x29,0x00,0x11,0x2F,0x00,0x43,0x00,0x49,0x00,0x00,0x35,0x39,
		0x00,0x00,0x00,0x64,0x00,0x22,0x00,0x42,0x00,0x6A,0x20,0x00,0x00,0x00,0x1C,0x00,
		0x66,0x00,0x54,0x4A,0x00,0x6C,0x00,0x00,0x58,0x32,0x00,0x00,0x50,0x2C,0x60,0x00,
		0x70,0x00,0x00,0x00,0x7C,0x48,0x62,0x52,0x00,0x26,0x00,0x12,0x00,0x00,0x40,0x00,
		0x00,0x00,0x6E,0x00,0x00,0x38,0x2E,0x00,0x46,0x00,0x7A,0x36,0x00,0x76,0x00,0x00,
		0x00,0x72,0x00,0x00,0x00,0x00,0x1E,0x00,0x00,0x00,0x5C,0x00,0x00,0x5E,0x1A,0x00,
		0x00,0x00,0x24,0x44,0x28,0x14,0x00,0x00,0x00,0x74,0x00,0x00,0x00,0x00,0x00,0x00,
		0x68,0x56,0x00,0x30,0x5A,0x00,0x00,0x00,0x00,0x4E,0x00,0x2A,0x18,0x00,0x00,0x00,
		0x4C,0x00,0x00,0x3A,0x00,0x34,0x10,0x78,0x00,0x3C,0x16,0x00,0x3E,0x00,0x00,0x00,
	};

	uint8_t const k = m_ram[key_offset(index)];
	uint8_t const n = kmap[k];
	if (n == 0)
		logerror("%s: unknown key byte %02X at index %03X\n", machine().describe_context(), k, index);

	// untested: mirrors the 136095-0072, which treats a zero input as the
	// 17th power word rather than as an LFSR stuck at zero
	uint16_t x = (c == 0) ? 1 : c;
	int clocks = (c == 0) ? n - 1 : n;

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

bool atari_136094_0004a_device::read16(offs_t offset, uint16_t &data, bool side_effects)
{
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
	bool const side_effects = !machine().side_effects_disabled();
	uint32_t result = 0;
	uint16_t data;

	if (ACCESSING_BITS_16_31 && read16(offset << 2, data, side_effects))
		result |= uint32_t(data) << 16;
	if (ACCESSING_BITS_0_15 && read16((offset << 2) + 2, data, side_effects))
		result |= data;

	return result;
}
