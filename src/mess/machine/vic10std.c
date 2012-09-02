/**********************************************************************

    Commodore VIC-10 Standard 8K/16K ROM Cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "vic10std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VIC10_STD = &device_creator<vic10_standard_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic10_standard_cartridge_device - constructor
//-------------------------------------------------

vic10_standard_cartridge_device::vic10_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, VIC10_STD, "VIC-10 Standard Cartridge", tag, owner, clock),
	  device_vic10_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic10_standard_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  vic10_cd_r - cartridge data read
//-------------------------------------------------

UINT8 vic10_standard_cartridge_device::vic10_cd_r(address_space &space, offs_t offset, UINT8 data, int lorom, int uprom, int exram)
{
	if (!lorom && (m_lorom != NULL))
	{
		data = m_lorom[offset & 0x1fff];
	}
	else if (!exram && (m_exram != NULL))
	{
		data = m_exram[offset & 0x7ff];
	}
	else if (!uprom && (m_uprom != NULL))
	{
		data = m_uprom[offset & 0x1fff];
	}

	return data;
}


//-------------------------------------------------
//  vic10_cd_w - cartridge data write
//-------------------------------------------------

void vic10_standard_cartridge_device::vic10_cd_w(address_space &space, offs_t offset, UINT8 data, int lorom, int uprom, int exram)
{
	if (!exram && (m_exram != NULL))
	{
		m_exram[offset & 0x7ff] = data;
	}
}
