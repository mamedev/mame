// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    mc10_pak.cpp

    Code for emulating standard MC-10 cartridges with only ROM

***************************************************************************/

#include "emu.h"
#include "mc10_pak.h"

#define CARTSLOT_TAG            "cart"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

ROM_START( mc10_pak )
	ROM_REGION(0x4000, CARTSLOT_TAG, ROMREGION_ERASE00)
	// this region is filled by mc10cart_slot_device::call_load()
ROM_END


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
	, m_eprom(*this, CARTSLOT_TAG)
{
}

mc10_pak_device::mc10_pak_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mc10_pak_device(mconfig, MC10_PAK, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc10_pak_device::device_start()
{
	owning_slot().install_rom(0x5000, 0x7fff, m_eprom->base());
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mc10_pak_device::device_rom_region() const
{
	return ROM_NAME( mc10_pak );
}


/*-------------------------------------------------
    get_cart_memregion
-------------------------------------------------*/

memory_region *mc10_pak_device::get_cart_memregion()
{
	return m_eprom;
}

