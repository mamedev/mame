// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro Sideways ROM emulation

***************************************************************************/

#include "emu.h"
#include "rom.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_ROM, bbc_rom_device, "bbc_rom", "BBC Micro Sideways ROM")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_rom_device - constructor
//-------------------------------------------------

bbc_rom_device::bbc_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_ROM, tag, owner, clock)
	, device_bbc_rom_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_rom_device::device_start()
{
}

//-------------------------------------------------
//  read
//-------------------------------------------------

READ8_MEMBER(bbc_rom_device::read)
{
	uint32_t size = std::min((int32_t)get_rom_size(), 0x4000);

	return get_rom_base()[offset & (size - 1)];
}
