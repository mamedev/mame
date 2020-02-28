// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore CBM-II Standard cartridge emulation

**********************************************************************/

#include "emu.h"
#include "std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CBM2_STD, cbm2_standard_cartridge_device, "cbm2_standard", "CBM-II standard cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm2_standard_cartridge_device - constructor
//-------------------------------------------------

cbm2_standard_cartridge_device::cbm2_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CBM2_STD, tag, owner, clock),
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

uint8_t cbm2_standard_cartridge_device::cbm2_bd_r(offs_t offset, uint8_t data, int csbank1, int csbank2, int csbank3)
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
