// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore CBM-II Standard cartridge emulation

**********************************************************************/

#include "emu.h"
#include "std.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CBM2_STD, cbm2_standard_cartridge_device, "cbm2_standard", "CBM-II standard cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm2_standard_cartridge_device - constructor
//-------------------------------------------------

cbm2_standard_cartridge_device::cbm2_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CBM2_STD, tag, owner, clock),
	device_cbm2_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm2_standard_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cbm2_standard_cartridge_device::device_reset()
{
	if (memory_region *const bank1 = m_slot->memregion("bank1"))
		m_slot->bank1().install_rom(0x0000, 0x1fff, bank1->base());

	if (memory_region *const bank2 = m_slot->memregion("bank2"))
		m_slot->bank2().install_rom(0x0000, 0x1fff, bank2->base());

	if (memory_region *const bank3 = m_slot->memregion("bank3"))
		m_slot->bank3().install_rom(0x0000, 0x1fff, bank3->base());
}
