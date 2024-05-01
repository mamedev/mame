// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ColecoVision standard cartridge emulation

**********************************************************************/

#include "emu.h"
#include "std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COLECOVISION_STANDARD, colecovision_standard_cartridge_device, "colecovision_standard", "ColecoVision standard cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  colecovision_standard_cartridge_device - constructor
//-------------------------------------------------

colecovision_standard_cartridge_device::colecovision_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, COLECOVISION_STANDARD, tag, owner, clock),
	device_colecovision_cartridge_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void colecovision_standard_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t colecovision_standard_cartridge_device::read(offs_t offset, int _8000, int _a000, int _c000, int _e000)
{
	uint8_t data = 0xff;

	if (!_8000 || !_a000 || !_c000 || !_e000)
	{
		if (offset < m_rom_size)
			data = m_rom[offset];
	}

	return data;
}
