// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-20 Standard 8K/16K ROM Cartridge emulation

**********************************************************************/

#include "emu.h"
#include "std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC20_STD, vic20_standard_cartridge_device, "vic20_standard", "VIC-20 Standard Cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic20_standard_cartridge_device - constructor
//-------------------------------------------------

vic20_standard_cartridge_device::vic20_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VIC20_STD, tag, owner, clock), device_vic20_expansion_card_interface(mconfig, *this)
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

uint8_t vic20_standard_cartridge_device::vic20_cd_r(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
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
