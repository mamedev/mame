// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ATPL Sidewise ROM/RAM Expansion board
    https://www.retro-kit.co.uk/page.cfm/content/ATPL-Sidewise-Sideways-ROM-board/

    ATPL Sidewise+
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/ATPL_Sidewise+.html

**********************************************************************/


#include "emu.h"
#include "atpl.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_ATPLSW, bbc_atplsw_device, "bbc_atplsw", "ATPL Sidewise ROM/RAM Expansion");
DEFINE_DEVICE_TYPE(BBC_ATPLSWP, bbc_atplswp_device, "bbc_atplswp", "ATPL Sidewise+ ROM Expansion");


//-------------------------------------------------
//  INPUT_PORTS( atplsw )
//-------------------------------------------------

static INPUT_PORTS_START(atplsw)
	PORT_START("wp")
	PORT_CONFNAME(0x01, 0x00, "Write Protect Sideways RAM")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x01, DEF_STR(Yes))
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_atplsw_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(atplsw);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_atplsw_device::device_add_mconfig(machine_config &config)
{
	/* 12 rom sockets, 16K static ram (battery backed) */
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
	BBC_ROMSLOT16(config, m_rom[15], bbc_rom_devices, "nvram").set_fixed_ram(true);
}

void bbc_atplswp_device::device_add_mconfig(machine_config &config)
{
	/* TODO: romslot2-10 are unavailable when board is fitted, replaced by sockets on board */
	//config.device_remove(":romslot2");
	//config.device_remove(":romslot4");
	//config.device_remove(":romslot6");
	//config.device_remove(":romslot8");
	//config.device_remove(":romslot10");

	/* 10 rom sockets */
	BBC_ROMSLOT16(config, m_rom[2], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[3], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[4], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[5], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[6], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[7], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[8], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[9], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[10], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[11], bbc_rom_devices, nullptr);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_atplsw_device - constructor
//-------------------------------------------------

bbc_atplsw_device::bbc_atplsw_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_ATPLSW, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
	, m_rom(*this, "romslot%u", 0U)
	, m_wp(*this, "wp")
{
}

bbc_atplswp_device::bbc_atplswp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_ATPLSWP, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
	, m_rom(*this, "romslot%u", 0U)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_atplsw_device::device_start()
{
	/* move internal roms to board */
	memmove(m_region_swr->base() + 0x2c000, m_region_swr->base(), 0x10000);
	memset(m_region_swr->base(), 0xff, 0x10000);

	/* register for save states */
	save_item(NAME(m_romsel));
}

void bbc_atplswp_device::device_start()
{
	/* register for save states */
	save_item(NAME(m_romsel));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_atplsw_device::paged_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (m_romsel)
	{
	case 0: case 1: case 2: case 3:
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
	case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
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

void bbc_atplsw_device::paged_w(offs_t offset, uint8_t data)
{
	switch (m_romsel)
	{
	case 0: case 1: case 2: case 3:
		/* motherboard rom sockets */
		if (m_mb_rom[m_romsel])
		{
			m_mb_rom[m_romsel]->write(offset, data);
		}
		break;
	case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12: case 13: case 14:
		/* expansion board sockets */
		if (m_rom[m_romsel])
		{
			m_rom[m_romsel]->write(offset, data);
		}
		break;
	}

	/* sideways ram (if not write protected) */
	if (!m_wp->read())
	{
		m_rom[15]->write(offset, data);
	}
}


uint8_t bbc_atplswp_device::paged_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (m_romsel)
	{
	case 0: case 1: case 12: case 13:
		/* motherboard ram sockets */
		if (m_mb_rom[m_romsel & 0x0e] && m_mb_rom[m_romsel & 0x0e]->present())
		{
			data = m_mb_rom[m_romsel & 0x0e]->read(offset | (m_romsel & 0x01) << 14);
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

void bbc_atplswp_device::paged_w(offs_t offset, uint8_t data)
{
	switch (m_romsel)
	{
	case 0: case 1: case 12: case 13:
		/* motherboard rom sockets */
		if (m_mb_rom[m_romsel & 0x0e])
		{
			m_mb_rom[m_romsel & 0x0e]->write(offset | (m_romsel & 0x01) << 14, data);
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
