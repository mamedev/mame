// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Acorn Electron standard cartridge emulation

***************************************************************************/

#include "emu.h"
#include "std.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_STDCART, electron_stdcart_device, "electron_stdcart", "Electron standard cartridge")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_stdcart_device - constructor
//-------------------------------------------------

electron_stdcart_device::electron_stdcart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_STDCART, tag, owner, clock)
	, device_electron_cart_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_stdcart_device::device_start()
{
}

//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t electron_stdcart_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (oe)
	{
		data = m_rom[(offset & 0x3fff) | (romqa << 14)];
	}

	return data;
}
