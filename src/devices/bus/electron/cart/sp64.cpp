// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Slogger Stop Press 64 cartridge emulation

***************************************************************************/

#include "emu.h"
#include "sp64.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_SP64, electron_sp64_device, "electron_sp64", "Slogger Stop Press 64 cartridge")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_sp64_device - constructor
//-------------------------------------------------

electron_sp64_device::electron_sp64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_SP64, tag, owner, clock)
	, device_electron_cart_interface(mconfig, *this)
	, m_page_register(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_sp64_device::device_start()
{
	save_item(NAME(m_page_register));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void electron_sp64_device::device_reset()
{
	m_page_register = 0;
}


//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t electron_sp64_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (infc)
	{
		switch (offset & 0xff)
		{
		case 0xfa:
			data = m_page_register;
			break;
		}
	}
	else if (oe)
	{
		offs_t rom_page_offset = m_page_register << 14;

		switch (romqa)
		{
		case 0:
			data = m_rom[rom_page_offset | (offset & 0x3fff)];
			break;
		case 1:
			data = m_ram[offset & 0x1fff];
			break;
		}
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void electron_sp64_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	if (infc)
	{
		switch (offset & 0xff)
		{
		case 0xfa:
			m_page_register = data;
			break;
		}
	}
	else if (oe)
	{
		if (romqa == 1)
		{
			m_ram[offset & 0x1fff] = data;
		}
	}
}
