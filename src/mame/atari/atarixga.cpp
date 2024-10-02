// license:BSD-3-Clause
// copyright-holders:Morten Shearman Kirkegaard, Samuel Neves, Peter Wilhelmsen
/*************************************************************************

    atarixga.cpp

    Atari XGA encryption FPGAs

 **************************************************************************

    Part numbers:

        136094-0072   Moto Frenzy
        136095-0072   Space Lords
        ?             Road Riot's Revenge
        136094-0004A  Primal Rage
        ?             T-Mek

*************************************************************************/

#include "emu.h"
#include "atarixga.h"



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
