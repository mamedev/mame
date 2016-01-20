// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-20 Standard 8K/16K ROM Cartridge emulation

**********************************************************************/

#include "std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VIC20_STD = &device_creator<vic20_standard_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic20_standard_cartridge_device - constructor
//-------------------------------------------------

vic20_standard_cartridge_device::vic20_standard_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VIC20_STD, "VIC-20 Standard Cartridge", tag, owner, clock, "vic20_standard", __FILE__),
		device_vic20_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic20_standard_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  vic20_cd_r - cartridge data read
//-------------------------------------------------

UINT8 vic20_standard_cartridge_device::vic20_cd_r(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	if (!blk1 && (m_blk1 != nullptr))
	{
		data = m_blk1[offset];
	}
	else if (!blk2 && (m_blk2 != nullptr))
	{
		data = m_blk2[offset];
	}
	else if (!blk3 && (m_blk3 != nullptr))
	{
		data = m_blk3[offset];
	}
	else if (!blk5 && (m_blk5 != nullptr))
	{
		data = m_blk5[offset];
	}

	return data;
}
