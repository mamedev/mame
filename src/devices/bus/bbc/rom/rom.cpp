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

uint8_t bbc_rom_device::read(offs_t offset)
{
	return get_rom_base()[offset & (get_rom_size() - 1)];
}
