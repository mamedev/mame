/**********************************************************************

    Commodore Plus/4 standard cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "plus4_std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PLUS4_STD = &device_creator<plus4_standard_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  plus4_standard_cartridge_device - constructor
//-------------------------------------------------

plus4_standard_cartridge_device::plus4_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PLUS4_STD, "Plus/4 standard cartridge", tag, owner, clock),
	device_plus4_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void plus4_standard_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  plus4_cd_r - cartridge data read
//-------------------------------------------------

UINT8 plus4_standard_cartridge_device::plus4_cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h)
{
	if (!c1l && m_c1l_mask)
	{
		data = m_c1l[offset & m_c1l_mask];
	}
	else if (!c1h && m_c1h_mask)
	{
		data = m_c1h[offset & m_c1h_mask];
	}
	else if (!c2l && m_c2l_mask)
	{
		data = m_c2l[offset & m_c2l_mask];
	}
	else if (!c2h && m_c2h_mask)
	{
		data = m_c2h[offset & m_c2h_mask];
	}

	return data;
}
