// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ColecoVision standard cartridge emulation

**********************************************************************/

#include "std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type COLECOVISION_STANDARD = &device_creator<colecovision_standard_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  colecovision_standard_cartridge_device - constructor
//-------------------------------------------------

colecovision_standard_cartridge_device::colecovision_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, COLECOVISION_STANDARD, "ColecoVision standard cartridge", tag, owner, clock, "colecovision_standard", __FILE__),
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

UINT8 colecovision_standard_cartridge_device::bd_r(address_space &space, offs_t offset, UINT8 data, int _8000, int _a000, int _c000, int _e000)
{
	if (!_8000 || !_a000 || !_c000 || !_e000)
	{
		if (offset < m_rom_size)
			data = m_rom[offset];
		else
			data = 0xff;
	}

	return data;
}
