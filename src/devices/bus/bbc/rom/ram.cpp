// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro Sideways RAM emulation

***************************************************************************/

#include "emu.h"
#include "ram.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_RAM, bbc_ram_device, "bbc_ram", "BBC Micro Sideways RAM")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_ram_device - constructor
//-------------------------------------------------

bbc_ram_device::bbc_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_RAM, tag, owner, clock)
	, device_bbc_rom_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_ram_device::device_start()
{
}

//-------------------------------------------------
//  read
//-------------------------------------------------

uint8_t bbc_ram_device::read(offs_t offset)
{
	return get_ram_base()[offset & (get_ram_size() - 1)];
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void bbc_ram_device::write(offs_t offset, uint8_t data)
{
	get_ram_base()[offset & (get_ram_size() - 1)] = data;
}
