// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VideoBrain Standard 2K/4K cartridge emulation

**********************************************************************/

#include "std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VB_STD = &device_creator<videobrain_standard_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  videobrain_standard_cartridge_device - constructor
//-------------------------------------------------

videobrain_standard_cartridge_device::videobrain_standard_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VB_STD, "VideoBrain standard cartridge", tag, owner, clock, "vb_std", __FILE__),
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

UINT8 videobrain_standard_cartridge_device::videobrain_bo_r(address_space &space, offs_t offset, int cs1, int cs2)
{
	UINT8 data = 0;

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
