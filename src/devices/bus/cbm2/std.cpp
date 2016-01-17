// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore CBM-II Standard cartridge emulation

**********************************************************************/

#include "std.h"



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

cbm2_standard_cartridge_device::cbm2_standard_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CBM2_STD, "CBM-II standard cartridge", tag, owner, clock, "cbm2_standard", __FILE__),
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
	if (!csbank1 && m_bank1.bytes())
	{
		data = m_bank1[offset & m_bank1.mask()];
	}
	else if (!csbank2 && m_bank2.bytes())
	{
		data = m_bank2[offset & m_bank2.mask()];
	}
	else if (!csbank3 && m_bank3.bytes())
	{
		data = m_bank3[offset & m_bank3.mask()];
	}

	return data;
}
