// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-20 Standard 8K/16K ROM Cartridge emulation

**********************************************************************/

#include "emu.h"
#include "std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC20_STD, vic20_standard_cartridge_device, "vic20_standard", "VIC-20 Standard Cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic20_standard_cartridge_device - constructor
//-------------------------------------------------

vic20_standard_cartridge_device::vic20_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VIC20_STD, tag, owner, clock), device_vic20_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic20_standard_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic20_standard_cartridge_device::device_reset()
{
	install_rom(m_slot->blk1(), m_slot->memregion("blk1"));
	install_rom(m_slot->blk2(), m_slot->memregion("blk2"));
	install_rom(m_slot->blk3(), m_slot->memregion("blk3"));
	install_rom(m_slot->blk5(), m_slot->memregion("blk5"));
}


//-------------------------------------------------
//  install_rom -
//-------------------------------------------------

void vic20_standard_cartridge_device::install_rom(vic20_expansion_window &window, memory_region *region)
{
	if (region)
		window.install_rom(0x0000, region->bytes() - 1, 0x1fff & ~(region->bytes() - 1), region->base());
}
