// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Advanced Battery-Backed RAM

***************************************************************************/

#include "emu.h"
#include "abr.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_ABR, electron_abr_device, "electron_abr", "Electron Advanced Battery-Backed RAM cartridge")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_abr_device - constructor
//-------------------------------------------------

electron_abr_device::electron_abr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_ABR, tag, owner, clock)
	, device_electron_cart_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_abr_device::device_start()
{
	m_bank_locked[0] = false;
	m_bank_locked[1] = false;
}


//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t electron_abr_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (oe)
	{
		data = m_nvram[(offset & 0x3fff) | (romqa << 14)];
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void electron_abr_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	if (infc)
	{
		switch (offset & 0xff)
		{
		case 0xdc:
			m_bank_locked[0] = false;
			break;
		case 0xdd:
			m_bank_locked[0] = true;
			break;
		case 0xde:
			m_bank_locked[1] = false;
			break;
		case 0xdf:
			m_bank_locked[1] = true;
			break;
		}
	}
	else if (oe)
	{
		if (!m_bank_locked[romqa])
		{
			m_nvram[(offset & 0x3fff) | (romqa << 14)] = data;
		}
	}
}
