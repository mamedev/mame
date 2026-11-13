// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore MAX BASIC cartridge emulation

**********************************************************************/

#include "emu.h"
#include "basic.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC10_BASIC, vic10_basic_cartridge_device, "vic10_basic", "VIC-10 MAX BASIC Cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic10_basic_cartridge_device - constructor
//-------------------------------------------------

vic10_basic_cartridge_device::vic10_basic_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VIC10_BASIC, tag, owner, clock),
	device_vic10_expansion_card_interface(mconfig, *this),
	m_ram(*this, "ram", 0x800, ENDIANNESS_LITTLE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic10_basic_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic10_basic_cartridge_device::device_reset()
{
	m_slot->lorom().install_rom(0x0000, 0x1fff, m_slot->memregion("lorom")->base());
	m_slot->uprom().install_rom(0x0000, 0x1fff, m_slot->memregion("uprom")->base());
	m_slot->exram().install_ram(0x000, 0x7ff, m_ram.target());
}
