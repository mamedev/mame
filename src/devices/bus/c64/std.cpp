// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64 Standard 8K/16K cartridge emulation

**********************************************************************/

#include "std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_STD = &device_creator<c64_standard_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_standard_cartridge_device - constructor
//-------------------------------------------------

c64_standard_cartridge_device::c64_standard_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_STD, "C64 standard cartridge", tag, owner, clock, "c64_standard", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_standard_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_standard_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml && m_roml.bytes())
	{
		data = m_roml[offset & m_roml.mask()];
	}
	else if (!romh)
	{
		if (m_romh.bytes())
		{
			data = m_romh[offset & m_romh.mask()];
		}
		else if (m_roml.mask() == 0x3fff)
		{
			data = m_roml[offset & m_roml.mask()];
		}
	}

	return data;
}
