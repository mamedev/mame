// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1210 3K RAM Expansion Cartridge emulation
    Commodore VIC-1211A Super Expander with 3K RAM Cartridge emulation

**********************************************************************/

#include "emu.h"
#include "vic1210.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC1210, vic1210_device, "vic1210", "VIC-1210 3K RAM Expansion")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic1210_device - constructor
//-------------------------------------------------

vic1210_device::vic1210_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VIC1210, tag, owner, clock)
	, device_vic20_expansion_card_interface(mconfig, *this)
	, m_ram(*this, "ram")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1210_device::device_start()
{
	// allocate memory
	m_ram.allocate(0xc00);
}


//-------------------------------------------------
//  vic20_cd_r - cartridge data read
//-------------------------------------------------

uint8_t vic1210_device::vic20_cd_r(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	if (!ram1 || !ram2 || !ram3)
	{
		data = m_ram[offset & 0xbff];
	}
	else if (!blk5 && m_blk5)
	{
		data = m_blk5[offset & 0xfff];
	}

	return data;
}


//-------------------------------------------------
//  vic20_cd_w - cartridge data write
//-------------------------------------------------

void vic1210_device::vic20_cd_w(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	if (!ram1 || !ram2 || !ram3)
	{
		m_ram[offset & 0xbff] = data;
	}
}
