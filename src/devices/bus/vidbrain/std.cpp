// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VideoBrain Standard 2K/4K cartridge emulation

**********************************************************************/

#include "emu.h"
#include "std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VB_STD, videobrain_standard_cartridge_device, "vb_std", "VideoBrain standard cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  videobrain_standard_cartridge_device - constructor
//-------------------------------------------------

videobrain_standard_cartridge_device::videobrain_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VB_STD, tag, owner, clock),
	device_videobrain_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void videobrain_standard_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  videobrain_bo_r - cartridge data read
//-------------------------------------------------

uint8_t videobrain_standard_cartridge_device::videobrain_bo_r(offs_t offset, int cs1, int cs2)
{
	uint8_t data = 0;

	if (!cs1)
	{
		data = m_rom[offset & m_rom_mask];
	}
	else if (!cs2)
	{
		data = m_rom[offset & m_rom_mask];
	}
	return data;
}
