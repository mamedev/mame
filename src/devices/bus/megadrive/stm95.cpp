// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, MetalliC
/***************************************************************************


 MegaDrive / Genesis Cart + STM95 EEPROM device


***************************************************************************/


#include "emu.h"
#include "stm95.h"


DEFINE_DEVICE_TYPE(MD_EEPROM_STM95, md_eeprom_stm95_device, "md_eeprom_stm95", "MD Cart + EEPROM STM95")


md_eeprom_stm95_device::md_eeprom_stm95_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_md_cart_interface(mconfig, *this)
	, m_stm95(*this, "eeprom")
	, m_legacy_nvram(*this, "eeprom")
	, m_rdcnt(0)
{
}

md_eeprom_stm95_device::md_eeprom_stm95_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: md_eeprom_stm95_device(mconfig, MD_EEPROM_STM95, tag, owner, clock)
{
}


ROM_START( md_eeprom_stm95 )
	ROM_REGION(0x1000, "eeprom", ROMREGION_ERASEFF)
ROM_END

const tiny_rom_entry *md_eeprom_stm95_device::device_rom_region() const
{
	return ROM_NAME(md_eeprom_stm95);
}

void md_eeprom_stm95_device::device_add_mconfig(machine_config &config)
{
	EEPROM_M95320(config, m_stm95);
}

void md_eeprom_stm95_device::load_nvram(device_image_interface &image)
{
	// Import the old battery file as defaults; existing device NVRAM takes precedence.
	image.battery_load(m_legacy_nvram->base(), m_legacy_nvram->bytes(), 0xff);
}

void md_eeprom_stm95_device::device_start()
{
	save_item(NAME(m_rdcnt));
	save_item(NAME(m_bank));
}

void md_eeprom_stm95_device::device_reset()
{
	m_rdcnt = 0;
	m_bank[0] = 0;
	m_bank[1] = 0;
	m_bank[2] = 0;
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

uint16_t md_eeprom_stm95_device::read(offs_t offset)
{
	if (offset == 0x0015e6/2 || offset == 0x0015e8/2)
	{
		// ugly hack until we don't know much about game protection
		// first 3 reads from 15e6 return 0x00000010, then normal 0x00018010 value for crc check
		uint16_t res;
		offset -= 0x0015e6/2;
		logerror("read 0x15e6 %d\n", m_rdcnt);
		if (m_rdcnt < 6)
		{
			m_rdcnt++;
			res = offset ? 0x10 : 0;
		}
		else
			res = offset ? 0x8010 : 0x0001;
		return res;
	}
	if (offset < 0x280000/2)
		return m_rom[offset];
	else    // last 0x180000 are bankswitched
	{
		uint8_t bank = (offset - 0x280000/2) >> 18;
		return m_rom[(offset & 0x7ffff/2) + (m_bank[bank] * 0x80000)/2];
	}
}

uint16_t md_eeprom_stm95_device::read_a13(offs_t offset)
{
	if (offset == 0x0a/2)
	{
		return m_stm95->so_r() & 1;
	}
	return 0xffff;
}

void md_eeprom_stm95_device::write_a13(offs_t offset, uint16_t data)
{
	if (offset == 0x00/2)
	{
		logerror("A13001 write %02x\n", data);
	}
	else if (offset < 0x08/2)
	{
		m_bank[offset - 1] = data & 0x0f;
	}
	else if (offset < 0x0a/2)
	{
		m_stm95->si_w(BIT(data, 0));
		m_stm95->sck_w(BIT(data, 1));
		m_stm95->hold_w(BIT(data, 2));
		m_stm95->cs_w(BIT(data, 3));
	}
}
