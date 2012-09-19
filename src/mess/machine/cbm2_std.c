/**********************************************************************

    Commodore CBM-II Standard cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "cbm2_std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CBM2_STD = &device_creator<cbm2_standard_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm2_standard_cartridge_device - constructor
//-------------------------------------------------

cbm2_standard_cartridge_device::cbm2_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CBM2_STD, "CBM-II standard cartridge", tag, owner, clock),
	device_cbm2_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm2_standard_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  cbm2_bd_r - cartridge data read
//-------------------------------------------------

UINT8 cbm2_standard_cartridge_device::cbm2_bd_r(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3)
{
	if (!csbank1 && m_bank1_mask)
	{
		data = m_bank1[offset & m_bank1_mask];
	}
	else if (!csbank2 && m_bank2_mask)
	{
		data = m_bank2[offset & m_bank2_mask];
	}
	else if (!csbank3 && m_bank3_mask)
	{
		data = m_bank3[offset & m_bank3_mask];
	}

	return data;
}
