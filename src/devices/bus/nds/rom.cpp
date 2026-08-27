// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Nintendo DS game card: mask ROM

    Implements the card side of the DS cartridge protocol.

    The card starts in "raw" mode: the BIOS reads the header and the chip ID,
	then sends 3C to switch to KEY1 mode, where every command arrives Blowfish
	encrypted with a keycode derived from the game code in the header.  In
	that mode it reads the secure area block by block and finally sends Ax to
	enter main data mode, where the firmware and games fetch everything else
	with B7 reads.

    Dumped images normally carry the secure area decrypted, so we reconstruct
	and reencrypt it on load to make the handshake work.

    The backup chip is a generic SPI EEPROM/flash: status, write enable,
    read, page write, JEDEC ID and page/sector erase.  Addresses are two
    bytes wide (the 8K-64K EEPROMs most games use) until a game asks for a
    JEDEC ID, which only flash based games do, after which they are three.

***************************************************************************/

#include "emu.h"
#include "rom.h"

#include "multibyte.h"

#define LOG_CMD (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(NDS_ROM_STD, nds_rom_device, "nds_rom", "Nintendo DS Game Card")


nds_rom_device::nds_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nds_cart_interface(mconfig, *this),
	m_mode(MODE_RAW),
	m_fixed_response(true),
	m_fixed_value(0xffffffff),
	m_data_addr(0),
	m_data_mask(0),
	m_remaining(0),
	m_secure_area_ready(false),
	m_bk_cmd(0),
	m_bk_count(0),
	m_bk_addr(0),
	m_bk_addr_bytes(2),
	m_bk_wel(false),
	m_bk_dirty(false)
{
}

nds_rom_device::nds_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nds_rom_device(mconfig, NDS_ROM_STD, tag, owner, clock)
{
}

void nds_rom_device::device_start()
{
	save_item(NAME(m_key1_buf));
	save_item(NAME(m_mode));
	save_item(NAME(m_fixed_response));
	save_item(NAME(m_fixed_value));
	save_item(NAME(m_data_addr));
	save_item(NAME(m_data_mask));
	save_item(NAME(m_remaining));
	save_item(NAME(m_bk_cmd));
	save_item(NAME(m_bk_count));
	save_item(NAME(m_bk_addr));
	save_item(NAME(m_bk_addr_bytes));
	save_item(NAME(m_bk_wel));

	nvram_alloc(BACKUP_SIZE);
	save_item(NAME(m_nvram));
}

void nds_rom_device::device_reset()
{
	m_mode = MODE_RAW;
	m_fixed_response = true;
	m_fixed_value = 0xffffffff;
	m_remaining = 0;
	m_bk_count = 0;
	m_bk_wel = false;

	if (!m_secure_area_ready && m_rom && m_key1_table)
	{
		prepare_secure_area();
		m_secure_area_ready = true;
	}
}

uint32_t nds_rom_device::idcode() const
{
	return m_rom ? get_u32le(&m_rom[0x0c]) : 0;
}

// Macronix ROM, size from the header's device capacity (128K << n)
uint32_t nds_rom_device::chip_id() const
{
	if (!m_rom)
		return 0xffffffff;

	const uint32_t capacity = 0x20000 << (m_rom[0x14] & 0x0f);
	const uint32_t size = (capacity >= 0x100000) ? ((capacity >> 20) - 1) : 0;
	return 0x000000c2 | ((size & 0xff) << 8);
}


/***************************************************************************
    KEY1 (Blowfish) handling
***************************************************************************/

void nds_rom_device::key1_encrypt(uint32_t *data) const
{
	uint32_t y = data[0], x = data[1];
	for (int i = 0; i <= 0x0f; i++)
	{
		const uint32_t z = m_key1_buf[i] ^ x;
		x = m_key1_buf[0x012 + ((z >> 24) & 0xff)];
		x += m_key1_buf[0x112 + ((z >> 16) & 0xff)];
		x ^= m_key1_buf[0x212 + ((z >> 8) & 0xff)];
		x += m_key1_buf[0x312 + (z & 0xff)];
		x ^= y;
		y = z;
	}
	data[0] = x ^ m_key1_buf[0x10];
	data[1] = y ^ m_key1_buf[0x11];
}

void nds_rom_device::key1_decrypt(uint32_t *data) const
{
	uint32_t y = data[0], x = data[1];
	for (int i = 0x11; i >= 0x02; i--)
	{
		const uint32_t z = m_key1_buf[i] ^ x;
		x = m_key1_buf[0x012 + ((z >> 24) & 0xff)];
		x += m_key1_buf[0x112 + ((z >> 16) & 0xff)];
		x ^= m_key1_buf[0x212 + ((z >> 8) & 0xff)];
		x += m_key1_buf[0x312 + (z & 0xff)];
		x ^= y;
		y = z;
	}
	data[0] = x ^ m_key1_buf[0x01];
	data[1] = y ^ m_key1_buf[0x00];
}

void nds_rom_device::key1_apply_keycode(uint32_t *keycode, int modulo)
{
	key1_encrypt(&keycode[1]);
	key1_encrypt(&keycode[0]);

	for (int i = 0; i <= 0x11; i++)
		m_key1_buf[i] ^= swapendian_int32(keycode[i % modulo]);

	uint32_t scratch[2] = { 0, 0 };
	for (int i = 0; i <= 0x410; i += 2)
	{
		key1_encrypt(scratch);
		m_key1_buf[i] = scratch[1];
		m_key1_buf[i + 1] = scratch[0];
	}
}

// level and modulo (in words) follow GBATEK's init_keycode()
void nds_rom_device::key1_init(uint32_t idcode, int level, int modulo)
{
	for (int i = 0; i < 0x412; i++)
	{
		m_key1_buf[i] = get_u32le(&m_key1_table[i * 4]);
	}

	uint32_t keycode[3] = { idcode, idcode >> 1, idcode << 1 };
	if (level >= 1)
	{
		key1_apply_keycode(keycode, modulo);
	}
	if (level >= 2)
	{
		key1_apply_keycode(keycode, modulo);
	}
	keycode[1] <<= 1;
	keycode[2] >>= 1;
	if (level >= 3)
	{
		key1_apply_keycode(keycode, modulo);
	}
}

// encrypt 8 bytes of ROM in place (two little-endian words)
void nds_rom_device::key1_encrypt_block(uint8_t *block) const
{
	uint32_t data[2] = { get_u32le(block), get_u32le(block + 4) };
	key1_encrypt(data);
	put_u32le(block, data[0]);
	put_u32le(block + 4, data[1]);
}

void nds_rom_device::prepare_secure_area()
{
	// only images whose ARM9 binary starts in the secure area have one
	if (get_u32le(&m_rom[0x20]) != 0x4000 || m_rom_size < 0x8000)
	{
		return;
	}

	uint8_t *const secure = &m_rom[0x4000];
	const bool decrypted = !memcmp(secure, "encryObj", 8);
	const bool destroyed = (get_u32le(secure) == 0xe7ffdeff) && (get_u32le(secure + 4) == 0xe7ffdeff);
	if (!decrypted && !destroyed)
	{
		return;
	}

	// The BIOS undoes this in the opposite order: the ID with the level 2 keycode
	// first, then the whole 2K with level 3 (verified against what it leaves in RAM).
	LOG("nds_rom: encrypting the secure area of %c%c%c%c\n", m_rom[0x0c], m_rom[0x0d], m_rom[0x0e], m_rom[0x0f]);
	memcpy(secure, "encryObj", 8);
	key1_init(idcode(), 3, 2);
	for (uint32_t i = 0; i < 0x800; i += 8)
	{
		key1_encrypt_block(secure + i);
	}
	key1_init(idcode(), 2, 2);
	key1_encrypt_block(secure);
}


/***************************************************************************
    Commands
***************************************************************************/

void nds_rom_device::respond_rom(uint32_t addr, uint32_t mask)
{
	m_fixed_response = false;
	m_data_addr = addr;
	m_data_mask = mask;
}

void nds_rom_device::respond_fixed(uint32_t value)
{
	m_fixed_response = true;
	m_fixed_value = value;
}

void nds_rom_device::command_start(const uint8_t *cmd, uint32_t length)
{
	uint8_t c[8];
	memcpy(c, cmd, 8);
	m_remaining = length;
	respond_fixed(0xffffffff);

	if (!m_rom)
	{
		return;
	}

	if (m_mode == MODE_KEY1)
	{
		// the 64-bit command is encrypted as (low word, high word) in big-endian byte order
		uint32_t data[2] = { get_u32be(&c[4]), get_u32be(&c[0]) };
		key1_decrypt(data);
		put_u32be(&c[0], data[1]);
		put_u32be(&c[4], data[0]);
	}

	LOGMASKED(LOG_CMD, "nds_rom: mode %d command %02x%02x%02x%02x%02x%02x%02x%02x, %x bytes\n", m_mode, c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7], length);

	const uint32_t rom_mask = m_rom_size - 1;

	switch (m_mode)
	{
		case MODE_RAW:
			switch (c[0])
			{
				case 0x9f:  // dummy
					break;

				case 0x00:  // header: the first 4K of the chip, repeating
					respond_rom(get_u32be(&c[1]) & 0xfff, 0xfff);
					break;

				case 0x90:  // chip ID
					respond_fixed(chip_id());
					break;

				case 0x3c:  // activate KEY1
					key1_init(idcode(), 2, 2);
					m_mode = MODE_KEY1;
					break;

				default:
					LOG("nds_rom: unknown raw command %02x\n", c[0]);
					break;
			}
			break;

		case MODE_KEY1:
			switch (c[0] >> 4)
			{
				case 0x4:   // activate KEY2: transparent here
					break;

				case 0x1:   // chip ID
					respond_fixed(chip_id());
					break;

				case 0x2:   // secure area block (4K units)
				{
					const uint32_t block = ((c[0] & 0x0f) << 12) | (c[1] << 4) | (c[2] >> 4);
					respond_rom(block * 0x1000, rom_mask);
					break;
				}

				case 0xa:   // enter main data mode
					m_mode = MODE_MAIN;
					break;

				default:
					LOG("nds_rom: unknown KEY1 command %02x\n", c[0]);
					break;
			}
			break;

		case MODE_MAIN:
			switch (c[0])
			{
				case 0xb7:  // normal read command
				{
					uint32_t addr = get_u32be(&c[1]);
					if (addr < 0x8000)
						addr = 0x8000 + (addr & 0x1ff);
					respond_rom(addr, rom_mask);
					break;
				}

				case 0xb8:  // chip ID
					respond_fixed(chip_id());
					break;

				default:
					LOG("nds_rom: unknown main mode command %02x\n", c[0]);
					break;
			}
			break;
	}
}

uint32_t nds_rom_device::data_r()
{
	if (m_remaining == 0)
	{
		return 0xffffffff;
	}
	m_remaining = (m_remaining >= 4) ? (m_remaining - 4) : 0;

	if (m_fixed_response)
		return m_fixed_value;

	const uint32_t data = get_u32le(&m_rom[m_data_addr & m_data_mask]);
	m_data_addr += 4;
	return data;
}


/***************************************************************************
    Backup chip
***************************************************************************/

uint8_t nds_rom_device::spi_transfer(uint8_t data)
{
	const uint32_t count = m_bk_count++;
	if (count == 0)
	{
		m_bk_cmd = data;
		m_bk_addr = 0;
		switch (data)
		{
			case 0x06:  // WREN
				m_bk_wel = true;
				break;
			case 0x04:  // WRDI
				m_bk_wel = false;
				break;
			case 0x9f:  // RDID: only flash is ever asked, and flash has three address bytes
				m_bk_addr_bytes = 3;
				break;
		}
		return 0xff;
	}

	switch (m_bk_cmd)
	{
		case 0x05:  // RDSR: never busy, WEL in bit 1
			return m_bk_wel ? 0x02 : 0x00;

		case 0x9f:  // RDID: ST M45PE10 style
		{
			static constexpr uint8_t id[3] = { 0x20, 0x40, 0x12 };
			return (count <= 3) ? id[count - 1] : 0xff;
		}

		case 0x03:  // READ
		case 0x0b:  // READ, upper half for the 0.5K part / fast read with a dummy byte otherwise
			if (count <= m_bk_addr_bytes)
			{
				m_bk_addr = (m_bk_addr << 8) | data;
				return 0xff;
			}
			if ((m_bk_cmd == 0x0b) && (m_bk_addr_bytes == 3) && (count == m_bk_addr_bytes + 1))
				return 0xff;    // fast read dummy byte
			return m_nvram[m_bk_addr++ & (BACKUP_SIZE - 1)];

		case 0x02:  // PP / WRITE
		case 0x0a:  // WRITE, upper half for the 0.5K part
			if (count <= m_bk_addr_bytes)
			{
				m_bk_addr = (m_bk_addr << 8) | data;
				return 0xff;
			}
			if (m_bk_wel)
			{
				m_nvram[m_bk_addr++ & (BACKUP_SIZE - 1)] = data;
				m_bk_dirty = true;
			}
			return 0xff;

		case 0xdb:  // page erase (256 bytes)
		case 0xd8:  // sector erase (64K)
			if (count <= m_bk_addr_bytes)
			{
				m_bk_addr = (m_bk_addr << 8) | data;
				if ((count == m_bk_addr_bytes) && m_bk_wel)
				{
					const uint32_t size = (m_bk_cmd == 0xdb) ? 0x100 : 0x10000;
					const uint32_t base = m_bk_addr & (BACKUP_SIZE - 1) & ~(size - 1);
					std::fill_n(&m_nvram[base], size, 0xff);
					m_bk_dirty = true;
				}
			}
			return 0xff;

		default:
			return 0xff;
	}
}

void nds_rom_device::spi_deselect()
{
	// a completed write cycle clears write enable
	if ((m_bk_cmd == 0x02) || (m_bk_cmd == 0x0a) || (m_bk_cmd == 0xdb) || (m_bk_cmd == 0xd8) || (m_bk_cmd == 0x01))
	{
		m_bk_wel = false;
	}
	m_bk_count = 0;
}
