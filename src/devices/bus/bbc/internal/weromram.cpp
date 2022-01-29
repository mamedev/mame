// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Watford Electronics ROM/RAM Board

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/WE_RAMROMBoard.html

**********************************************************************/


#include "emu.h"
#include "weromram.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_WEROMRAM, bbc_weromram_device, "bbc_weromram", "Watford Electronics ROM/RAM Board");


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_weromram_device::device_add_mconfig(machine_config &config)
{
	/* 3 rom sockets, 64K dynamic ram, 16K static ram (battery backed) */
	BBC_ROMSLOT16(config, m_rom[0], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[1], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[2], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[3], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[4], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[5], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[6], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[7], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[12], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[13], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[14], bbc_rom_devices, "nvram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[15], bbc_rom_devices, nullptr);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_weromram_device - constructor
//-------------------------------------------------

bbc_weromram_device::bbc_weromram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_WEROMRAM, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
	, m_rom(*this, "romslot%u", 0U)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_weromram_device::device_start()
{
	/* move internal roms to board */
	memmove(m_region_swr->base() + 0x2c000, m_region_swr->base(), 0xc000);
	memmove(m_region_swr->base() + 0x3c000, m_region_swr->base() + 0xc000, 0x4000);
	memset(m_region_swr->base(), 0xff, 0x10000);

	/* register for save states */
	save_item(NAME(m_romsel));
	save_item(NAME(m_ramsel));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_weromram_device::paged_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (m_romsel)
	{
	case 8: case 9: case 10: case 11:
		/* motherboard rom sockets 0-3 are re-assigned to 8-11 */
		if (m_mb_rom[m_romsel & 0x03] && m_mb_rom[m_romsel & 0x03]->present())
		{
			data = m_mb_rom[m_romsel & 0x03]->read(offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (m_romsel << 14) - 0x20000];
		}
		break;
	default:
		/* expansion board sockets */
		if (m_rom[m_romsel] && m_rom[m_romsel]->present())
		{
			data = m_rom[m_romsel]->read(offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (m_romsel << 14)];
		}
		break;
	}

	return data;
}

void bbc_weromram_device::paged_w(offs_t offset, uint8_t data)
{
	switch (m_romsel)
	{
	case 8: case 9: case 10: case 11:
		/* motherboard rom sockets 0-3 are re-assigned to 8-11 */
		if (m_mb_rom[m_romsel & 0x03])
		{
			m_mb_rom[m_romsel & 0x03]->write(offset, data);
		}
		break;
	}

	/* expansion board sockets */
	if (m_rom[m_ramsel])
	{
		m_rom[m_ramsel]->write(offset, data);
	}
}
