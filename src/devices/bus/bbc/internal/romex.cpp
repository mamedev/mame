// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    GCC (Cambridge) Romex 13

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/GCC_Romex13.html

**********************************************************************/


#include "emu.h"
#include "romex.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_ROMEX13, bbc_romex13_device, "bbc_romex13", "GCC Romex13 ROM Expansion");


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_romex13_device::device_add_mconfig(machine_config &config)
{
	/* TODO: romslot2 is unavailable when board is fitted, replaced by socket on board */
	//config.device_remove(":romslot2");

	/* 13 rom sockets */
	BBC_ROMSLOT16(config, m_rom[2], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[4], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[5], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[6], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[7], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[8], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[9], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[10], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[11], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[12], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[13], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[14], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[15], bbc_rom_devices, nullptr);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_romex13_device - constructor
//-------------------------------------------------

bbc_romex13_device::bbc_romex13_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_ROMEX13, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
	, m_rom(*this, "romslot%u", 0U)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_romex13_device::device_start()
{
	/* move internal roms to board */
	memmove(m_region_swr->base() + 0x30000, m_region_swr->base(), 0x10000);
	memset(m_region_swr->base(), 0xff, 0x10000);

	/* register for save states */
	save_item(NAME(m_romsel));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_romex13_device::paged_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (m_romsel)
	{
	case 0: case 1: case 3:
		/* motherboard rom sockets */
		if (m_mb_rom[m_romsel] && m_mb_rom[m_romsel]->present())
		{
			data = m_mb_rom[m_romsel]->read(offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (m_romsel << 14)];
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

void bbc_romex13_device::paged_w(offs_t offset, uint8_t data)
{
	switch (m_romsel)
	{
	case 0: case 1: case 3:
		/* motherboard rom sockets */
		if (m_mb_rom[m_romsel])
		{
			m_mb_rom[m_romsel]->write(offset, data);
		}
		break;
	default:
		/* expansion board sockets */
		if (m_rom[m_romsel])
		{
			m_rom[m_romsel]->write(offset, data);
		}
		break;
	}
}
