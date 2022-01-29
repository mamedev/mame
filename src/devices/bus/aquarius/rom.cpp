// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Aquarius Software Cartridges

***************************************************************************/

#include "emu.h"
#include "rom.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AQUARIUS_ROM, aquarius_rom_device, "aquarius_rom", "Aquarius ROM Cartridge")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  aquarius_rom_device - constructor
//-------------------------------------------------

aquarius_rom_device::aquarius_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AQUARIUS_ROM, tag, owner, clock)
	, device_aquarius_cartridge_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void aquarius_rom_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t aquarius_rom_device::mreq_ce_r(offs_t offset)
{
	return get_rom_base()[offset & (get_rom_size() - 1)];
}
