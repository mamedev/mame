// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Mega Games Cartridge

    The Mega Games Cartridge (MGC) is a Cartridge for the Acorn Electron,
    with an Acorn/P.R.E.S. Plus 1 or a Slogger RomBox Plus fitted. It
    contains a 32 Megabit (4M x 8-Bit) FlashRAM, plus control circuitry, to
    give 256 x 16K pages of Memory. These 16K blocks can be viewed in
    various formats. The MGC has been designed to hold and access, via the
    built-in Menu, up-to 254 Games (the Index and Menu ROM take up the other
    two 16K blocks).

***************************************************************************/

#include "emu.h"
#include "mgc.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_MGC, electron_mgc_device, "electron_mgc", "Electron Mega Games Cartridge")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_mgc_device - constructor
//-------------------------------------------------

electron_mgc_device::electron_mgc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_MGC, tag, owner, clock)
	, device_electron_cart_interface(mconfig, *this)
	, m_page_latch(0)
	, m_control_latch(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_mgc_device::device_start()
{
	save_item(NAME(m_page_latch));
	save_item(NAME(m_control_latch));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void electron_mgc_device::device_reset()
{
	m_page_latch = 0;
	m_control_latch = 0;
}

//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t electron_mgc_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (oe)
	{
		int m_page_mode = BIT(m_control_latch, 2) ? BIT(m_control_latch, 1) : !romqa;
		data = m_nvram[(offset & 0x3fff) | (m_page_latch << 14) | (m_page_mode << 21)];
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void electron_mgc_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	if (infc)
	{
		switch (offset)
		{
		case 0x00:
			m_page_latch = data & 0x7f;
			break;

		case 0x08:
			m_control_latch = data & 0x07;
			break;
		}
	}
	else if (oe)
	{
		int m_page_mode = BIT(m_control_latch, 2) ? BIT(m_control_latch, 1) : !romqa;
		m_nvram[(offset & 0x3fff) | (m_page_latch << 14) | (m_page_mode << 21)] = data;
	}
}
