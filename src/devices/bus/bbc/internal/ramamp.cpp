// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Ramamp Computers Sideways RAM/ROM Board

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/RAMAMP_SidewaysRAMROM.html

**********************************************************************/


#include "emu.h"
#include "ramamp.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_RAMAMP, bbc_ramamp_device, "bbc_ramamp", "Ramamp Sideways RAM/ROM Board");


//-------------------------------------------------
//  INPUT_PORTS( ramamp )
//-------------------------------------------------

static INPUT_PORTS_START(ramamp)
	PORT_START("wp")
	PORT_CONFNAME(0x01, 0x00, "Write Protect Sideways RAM")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x01, DEF_STR(Yes))
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_ramamp_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ramamp);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_ramamp_device::device_add_mconfig(machine_config &config)
{
	/* 6 rom sockets, 32K dynamic ram */
	BBC_ROMSLOT16(config, m_rom[8], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[9], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[10], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[11], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[12], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[13], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[14], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[15], bbc_rom_devices, nullptr);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_ramamp_device - constructor
//-------------------------------------------------

bbc_ramamp_device::bbc_ramamp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_RAMAMP, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
	, m_rom(*this, "romslot%u", 0U)
	, m_wp(*this, "wp")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_ramamp_device::device_start()
{
	/* move internal roms to board */
	memmove(m_region_swr->base() + 0x2c000, m_region_swr->base(), 0x8000);
	memmove(m_region_swr->base() + 0x38000, m_region_swr->base() + 0x8000, 0x8000);
	memset(m_region_swr->base(), 0xff, 0x10000);

	/* register for save states */
	save_item(NAME(m_romsel));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_ramamp_device::paged_r(offs_t offset)
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

void bbc_ramamp_device::paged_w(offs_t offset, uint8_t data)
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
	case 10: case 13:
		/* expansion board sockets */
		if (m_rom[m_romsel] && !m_wp->read())
		{
			m_rom[m_romsel]->write(offset, data);
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
