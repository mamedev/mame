// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    REX Datentechnik cartridge emulation

**********************************************************************/

#include "rex.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_REX = &device_creator<c64_rex_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_rex_cartridge_device - constructor
//-------------------------------------------------

c64_rex_cartridge_device::c64_rex_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_REX, "C64 Rex cartridge", tag, owner, clock, "c64_rex", __FILE__),
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

UINT8 c64_rex_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
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
