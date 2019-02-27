// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Advanced Quarter Meg Ram

***************************************************************************/

#include "emu.h"
#include "aqr.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_AQR, electron_aqr_device, "electron_aqr", "Electron Advanced Quarter Meg Ram cartridge")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_aqr_device - constructor
//-------------------------------------------------

electron_aqr_device::electron_aqr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_AQR, tag, owner, clock)
	, device_electron_cart_interface(mconfig, *this)
	, m_page_register(0)
	, m_lock_register(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_aqr_device::device_start()
{
	save_item(NAME(m_page_register));
	save_item(NAME(m_lock_register));
}


//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t electron_aqr_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (oe)
	{
		data = m_ram[(offset & 0x3fff) | (m_page_register << 14)];
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void electron_aqr_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	if (infc)
	{
		switch (offset & 0xff)
		{
		case 0xfc:
			m_page_register = data;
			break;
		case 0xfd:
			m_lock_register = false;
			break;
		case 0xfe:
			m_lock_register = true;
			break;
		}
	}
	else if (oe)
	{
		m_ram[(offset & 0x3fff) | (m_page_register << 14)] = data;
	}
}
