// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1111 16K RAM Expansion Cartridge emulation

**********************************************************************/

#include "vic1111.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VIC1111 = &device_creator<vic1111_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic1111_device - constructor
//-------------------------------------------------

vic1111_device::vic1111_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VIC1111, "VIC1111", tag, owner, clock, "vic1111", __FILE__),
		device_vic20_expansion_card_interface(mconfig, *this),
		m_ram(*this, "ram")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1111_device::device_start()
{
	// allocate memory
	m_ram.allocate(0x4000);
}


//-------------------------------------------------
//  vic20_cd_r - cartridge data read
//-------------------------------------------------

UINT8 vic1111_device::vic20_cd_r(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	if (!blk1)
	{
		data = m_ram[offset];
	}
	else if (!blk2)
	{
		data = m_ram[0x2000 + offset];
	}

	return data;
}


//-------------------------------------------------
//  vic20_cd_w - cartridge data write
//-------------------------------------------------

void vic1111_device::vic20_cd_w(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	if (!blk1)
	{
		m_ram[offset] = data;
	}
	else if (!blk2)
	{
		m_ram[0x2000 + offset] = data;
	}
}
