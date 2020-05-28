// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Solidisk Mega 256 cartridge emulation

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Solidisk_Mega256.html

***************************************************************************/

#include "emu.h"
#include "mega256.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_MEGA256, bbc_mega256_device, "bbc_mega256", "Solidisk Mega 256 cartridge")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_mega256_device - constructor
//-------------------------------------------------

bbc_mega256_device::bbc_mega256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_MEGA256, tag, owner, clock)
	, device_bbc_cart_interface(mconfig, *this)
	, m_page(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_mega256_device::device_start()
{
	// register for save states
	save_item(NAME(m_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_mega256_device::device_reset()
{
	m_page = 0x00;
}

//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t bbc_mega256_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (oe)
	{
		if (romqa)
		{
			data = m_rom[offset & 0x1fff];
		}
		else
		{
			data = m_ram[(offset & 0x3fff) | (m_page << 14)];
		}
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void bbc_mega256_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	if (oe)
	{
		if (romqa)
		{
			// Not known whether this latch is fully decoded.
			if (offset == 0x3fff) m_page = data & 0x0f;
		}
		else
		{
			m_ram[(offset & 0x3fff) | (m_page << 14)] = data;
		}
	}
}
