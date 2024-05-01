// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

    VideoBrain Info Manager cartridge emulation
    6KB EPROM (3*B2716), 1KB RAM (2*2114)

    It has the same PCB layout as comp_language, with wire mods to put
    the ROM/RAM at fixed addresses. There's a big orange label saying
    "TEST UNIT NO. 14", and a handwritten one with "INFO. MGR SPECIAL".

**********************************************************************/

#include "emu.h"
#include "info_manager.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VB_INFO_MANAGER, videobrain_info_manager_cartridge_device, "vb_info_manager", "VideoBrain Info Manager cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  videobrain_info_manager_cartridge_device - constructor
//-------------------------------------------------

videobrain_info_manager_cartridge_device::videobrain_info_manager_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VB_INFO_MANAGER, tag, owner, clock),
	device_videobrain_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  videobrain_bo_r - cartridge data read
//-------------------------------------------------

uint8_t videobrain_info_manager_cartridge_device::videobrain_bo_r(offs_t offset, int cs1, int cs2)
{
	uint8_t data = 0;

	if (!cs1)
	{
		data = m_rom[offset & 0x7ff & m_rom_mask];
	}
	else if (!cs2)
	{
		data = m_ram[offset & m_ram_mask];
	}
	else if (offset >= 0x3000)
	{
		data = m_rom[offset & m_rom_mask];
	}

	return data;
}


//-------------------------------------------------
//  videobrain_bo_w - cartridge data write
//-------------------------------------------------

void videobrain_info_manager_cartridge_device::videobrain_bo_w(offs_t offset, uint8_t data, int cs1, int cs2)
{
	if (!cs2)
	{
		m_ram[offset & m_ram_mask] = data;
	}
}
