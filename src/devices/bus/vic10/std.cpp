// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-10 Standard 8K/16K ROM Cartridge emulation

**********************************************************************/

#include "emu.h"
#include "std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC10_STD, vic10_standard_cartridge_device, "vic10_standard", "VIC-10 Standard Cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic10_standard_cartridge_device - constructor
//-------------------------------------------------

vic10_standard_cartridge_device::vic10_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VIC10_STD, tag, owner, clock), device_vic10_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic10_standard_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic10_standard_cartridge_device::device_reset()
{
	if (memory_region *const lorom = m_slot->memregion("lorom"))
		m_slot->lorom().install_rom(0x0000, 0x1fff, lorom->base());

	if (memory_region *const uprom = m_slot->memregion("uprom"))
		m_slot->uprom().install_rom(0x0000, 0x1fff, uprom->base());
}
