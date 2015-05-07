// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-10 Standard 8K/16K ROM Cartridge emulation

**********************************************************************/

#include "std.h"



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
	: device_t(mconfig, VIC10_STD, "VIC-10 Standard Cartridge", tag, owner, clock, "vic10_standard", __FILE__),
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
	if (!lorom && m_lorom.bytes())
	{
		data = m_lorom[offset & m_lorom.mask()];
	}
	else if (!exram && m_exram.bytes())
	{
		data = m_exram[offset & m_exram.mask()];
	}
	else if (!uprom && m_uprom.bytes())
	{
		data = m_uprom[offset & m_uprom.mask()];
	}

	return data;
}


//-------------------------------------------------
//  vic10_cd_w - cartridge data write
//-------------------------------------------------

void vic10_standard_cartridge_device::vic10_cd_w(address_space &space, offs_t offset, UINT8 data, int lorom, int uprom, int exram)
{
	if (!exram && m_exram.bytes())
	{
		m_exram[offset & m_exram.mask()] = data;
	}
}
