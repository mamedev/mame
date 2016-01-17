// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Westermann Learning cartridge emulation

**********************************************************************/

#include "westermann.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_WESTERMANN = &device_creator<c64_westermann_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_westermann_cartridge_device - constructor
//-------------------------------------------------

c64_westermann_cartridge_device::c64_westermann_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_WESTERMANN, "C64 Westermann cartridge", tag, owner, clock, "c64_westermann", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_westermann_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_westermann_cartridge_device::device_reset()
{
	m_game = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_westermann_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		data = m_roml[offset & m_roml.mask()];
	}
	else if (!romh)
	{
		if (m_romh.bytes())
		{
			data = m_romh[offset & m_romh.mask()];
		}
		else
		{
			data = m_roml[offset & m_roml.mask()];
		}
	}
	else if (!io2)
	{
		m_game = 1;
	}

	return data;
}
