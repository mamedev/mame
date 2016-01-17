// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VideoBrain Money Minder cartridge emulation

**********************************************************************/

#include "money_minder.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VB_MONEY_MINDER = &device_creator<videobrain_money_minder_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  videobrain_money_minder_cartridge_device - constructor
//-------------------------------------------------

videobrain_money_minder_cartridge_device::videobrain_money_minder_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VB_MONEY_MINDER, "VideoBrain Money Minder cartridge", tag, owner, clock, "vb_money_minder", __FILE__),
	device_videobrain_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void videobrain_money_minder_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  videobrain_cd_r - cartridge data read
//-------------------------------------------------

UINT8 videobrain_money_minder_cartridge_device::videobrain_bo_r(address_space &space, offs_t offset, int cs1, int cs2)
{
	UINT8 data = 0;

	if (!cs1 || !cs2)
	{
		data = m_rom[offset & m_rom_mask];
	}
	else if (offset >= 0x3800)
	{
		data = m_ram[offset & m_ram_mask];
	}

	return data;
}


//-------------------------------------------------
//  videobrain_bo_w - cartridge data write
//-------------------------------------------------

void videobrain_money_minder_cartridge_device::videobrain_bo_w(address_space &space, offs_t offset, UINT8 data, int cs1, int cs2)
{
	if (offset >= 0x3800)
	{
		m_ram[offset & m_ram_mask] = data;
	}
}
