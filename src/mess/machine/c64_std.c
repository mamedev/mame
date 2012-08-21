/**********************************************************************

    Commodore 64 Standard 8K/16K cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c64_std.h"



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

c64_standard_cartridge_device::c64_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_STD, "C64 standard cartridge", tag, owner, clock),
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

UINT8 c64_standard_cartridge_device::c64_cd_r(address_space &space, offs_t offset, int ba, int roml, int romh, int io1, int io2)
{
	UINT8 data = 0;

	if (!roml && m_roml_mask)
	{
		data = m_roml[offset & 0x1fff];
	}
	else if (!romh)
	{
		if (m_romh_mask)
		{
			data = m_romh[offset & 0x1fff];
		}
		else if (m_roml_mask == 0x3fff)
		{
			data = m_roml[offset & 0x3fff];
		}
	}

	return data;
}
