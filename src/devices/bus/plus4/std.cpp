// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore Plus/4 standard cartridge emulation

**********************************************************************/

#include "emu.h"
#include "std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PLUS4_STD, plus4_standard_cartridge_device, "plus4_standard", "Plus/4 standard cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  plus4_standard_cartridge_device - constructor
//-------------------------------------------------

plus4_standard_cartridge_device::plus4_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PLUS4_STD, tag, owner, clock),
	device_plus4_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void plus4_standard_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void plus4_standard_cartridge_device::device_reset()
{
	install_rom(m_slot->c1l(), m_slot->memregion("c1l"));
	install_rom(m_slot->c1h(), m_slot->memregion("c1h"));
	install_rom(m_slot->c2l(), m_slot->memregion("c2l"));
	install_rom(m_slot->c2h(), m_slot->memregion("c2h"));
}


//-------------------------------------------------
//  install_rom -
//-------------------------------------------------

void plus4_standard_cartridge_device::install_rom(plus4_expansion_window &window, memory_region *region)
{
	if (region)
		window.install_rom(0x0000, region->bytes() - 1, 0x3fff & ~(region->bytes() - 1), region->base());
}
