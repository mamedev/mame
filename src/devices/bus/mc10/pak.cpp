// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    pak.cpp

    Code for emulating standard MC-10 cartridges with only ROM

***************************************************************************/

#include "emu.h"
#include "pak.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MC10_PAK, mc10_pak_device, "mc10pak", "MC-10 Program PAK")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mc10_pak_device - constructor
//-------------------------------------------------
mc10_pak_device::mc10_pak_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_mc10cart_interface(mconfig, *this)
{
}

mc10_pak_device::mc10_pak_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mc10_pak_device(mconfig, MC10_PAK, tag, owner, clock)
{
}

//-------------------------------------------------
//  max_rom_length - device-specific startup
//-------------------------------------------------

int mc10_pak_device::max_rom_length() const
{
	return 1024 * 16;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc10_pak_device::device_start()
{
}

//-------------------------------------------------
//  load - install rom region
//-------------------------------------------------

image_init_result mc10_pak_device::load()
{
	// if the host has supplied a ROM space, install it
	memory_region *const romregion(memregion("^rom"));
	if (romregion)
		owning_slot().memspace().install_rom(0x5000, 0x5000 + romregion->bytes(), romregion->base());
	else
		return image_init_result::FAIL;

	return image_init_result::PASS;
}
