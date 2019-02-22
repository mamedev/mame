// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1010 Expansion Module emulation

**********************************************************************/

#include "emu.h"
#include "vic1010.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC1010, vic1010_device, "vic1010", "VIC-1010 Expansion Module")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vic1010_device::device_add_mconfig(machine_config &config)
{
	vic20_expansion_slot_device::add_passthrough(config, "slot1");
	vic20_expansion_slot_device::add_passthrough(config, "slot2");
	vic20_expansion_slot_device::add_passthrough(config, "slot3");
	vic20_expansion_slot_device::add_passthrough(config, "slot4");
	vic20_expansion_slot_device::add_passthrough(config, "slot5");
	vic20_expansion_slot_device::add_passthrough(config, "slot6");
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic1010_device - constructor
//-------------------------------------------------

vic1010_device::vic1010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VIC1010, tag, owner, clock)
	, device_vic20_expansion_card_interface(mconfig, *this)
	, m_expansion_slot(*this, "slot%u", 1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1010_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic1010_device::device_reset()
{
	for (auto & elem : m_expansion_slot)
	{
		elem->reset();
	}
}


//-------------------------------------------------
//  vic20_cd_r - cartridge data read
//-------------------------------------------------

uint8_t vic1010_device::vic20_cd_r(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	for (auto elem : m_expansion_slot)
	{
		uint8_t slot_data = elem->cd_r(offset, data, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);

		if (data != slot_data)
		{
			data = slot_data;
		}
	}

	return data;
}


//-------------------------------------------------
//  vic20_cd_w - cartridge data write
//-------------------------------------------------

void vic1010_device::vic20_cd_w(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	for (auto & elem : m_expansion_slot)
	{
		elem->cd_w(offset, data, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);
	}
}
