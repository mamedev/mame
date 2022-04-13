// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Watford Electronics 12 ROM Board
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/WE_12ROMboard.html

    Watford Electronics 13 ROM Board
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/WE_13ROMBoard.html

**********************************************************************/


#include "emu.h"
#include "werom.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_WE12ROM, bbc_we12rom_device, "bbc_we12rom", "Watford Electronics 12 ROM Board");
DEFINE_DEVICE_TYPE(BBC_WE13ROM, bbc_we13rom_device, "bbc_we13rom", "Watford Electronics 13 ROM Board");


//-------------------------------------------------
//  INPUT_PORTS( werom )
//-------------------------------------------------

static INPUT_PORTS_START(werom)
	PORT_START("wp")
	PORT_CONFNAME(0x01, 0x00, "Write Protect Sideways RAM")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x01, DEF_STR(Yes))
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_we12rom_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(werom);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_we12rom_device::device_add_mconfig(machine_config &config)
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
	BBC_ROMSLOT16(config, m_rom[14], bbc_rom_devices, "nvram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[15], bbc_rom_devices, nullptr);
}

void bbc_we13rom_device::device_add_mconfig(machine_config &config)
{
	/* TODO: romslot2 is unavailable when board is fitted, replaced by socket on board */
	//config.device_remove(":romslot2");

	/* 13 rom sockets, 16K static ram (battery backed) */
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
	BBC_ROMSLOT16(config, m_rom[15], bbc_rom_devices, "nvram").set_fixed_ram(true);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_werom_device - constructor
//-------------------------------------------------

bbc_werom_device::bbc_werom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
	, m_rom(*this, "romslot%u", 0U)
{
}

bbc_we12rom_device::bbc_we12rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_werom_device(mconfig, BBC_WE12ROM, tag, owner, clock)
	, m_wp(*this, "wp")
{
}

bbc_we13rom_device::bbc_we13rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_werom_device(mconfig, BBC_WE13ROM, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_we12rom_device::device_start()
{
	/* move internal roms to board */
	memmove(m_region_swr->base() + 0x2c000, m_region_swr->base(), 0xc000);
	memmove(m_region_swr->base() + 0x3c000, m_region_swr->base() + 0xc000, 0x4000);
	memset(m_region_swr->base(), 0xff, 0x10000);

	/* register for save states */
	save_item(NAME(m_romsel));
}

void bbc_we13rom_device::device_start()
{
	/* move internal roms to board */
	memmove(m_region_swr->base() + 0x2c000, m_region_swr->base(), 0x10000);
	memset(m_region_swr->base(), 0xff, 0x10000);

	/* register for save states */
	save_item(NAME(m_romsel));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_we12rom_device::paged_r(offs_t offset)
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

void bbc_we12rom_device::paged_w(offs_t offset, uint8_t data)
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
	default:
		/* expansion board sockets */
		if (m_rom[m_romsel])
		{
			m_rom[m_romsel]->write(offset, data);
		}
		/* sideways ram (if not write protected) */
		if (!m_wp->read())
		{
			m_rom[14]->write(offset, data);
		}
		break;
	}
}


uint8_t bbc_we13rom_device::paged_r(offs_t offset)
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

void bbc_we13rom_device::paged_w(offs_t offset, uint8_t data)
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
		/* sideways ram */
		m_rom[15]->write(offset, data);
		break;
	}
}
