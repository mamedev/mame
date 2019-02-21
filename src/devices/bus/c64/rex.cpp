// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    REX Datentechnik cartridge emulation

**********************************************************************/

#include "emu.h"
#include "rex.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_REX, c64_rex_cartridge_device, "c64_rex", "C64 Rex cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_rex_cartridge_device - constructor
//-------------------------------------------------

c64_rex_cartridge_device::c64_rex_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_REX, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_rex_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_rex_cartridge_device::device_reset()
{
	m_exrom = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_rex_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		data = m_roml[offset & 0x1fff];
	}
	else if (!io2)
	{
		if ((offset & 0xc0) == 0xc0)
		{
			m_exrom = 0;
		}
		else
		{
			m_exrom = 1;
		}
	}

	return data;
}
