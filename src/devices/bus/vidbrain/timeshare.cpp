// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VideoBrain Timeshare cartridge emulation

**********************************************************************/

#include "emu.h"
#include "timeshare.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VB_TIMESHARE, videobrain_timeshare_cartridge_device, "vb_timeshare", "VideoBrain Timeshare cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  videobrain_timeshare_cartridge_device - constructor
//-------------------------------------------------

videobrain_timeshare_cartridge_device::videobrain_timeshare_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VB_TIMESHARE, tag, owner, clock),
	device_videobrain_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void videobrain_timeshare_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  videobrain_bo_r - cartridge data read
//-------------------------------------------------

uint8_t videobrain_timeshare_cartridge_device::videobrain_bo_r(offs_t offset, int cs1, int cs2)
{
	uint8_t data = 0;

	if (!cs1)
	{
		data = m_rom[offset & m_rom_mask];
	}
	else if (!cs2)
	{
		data = m_ram[offset & m_ram_mask];
	}
	return data;
}


//-------------------------------------------------
//  videobrain_bo_w - cartridge data write
//-------------------------------------------------

void videobrain_timeshare_cartridge_device::videobrain_bo_w(offs_t offset, uint8_t data, int cs1, int cs2)
{
	if (!cs2)
	{
		m_ram[offset & m_ram_mask] = data;
	}
}
