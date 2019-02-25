// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio ROM card emulation

**********************************************************************/

#include "emu.h"
#include "rom.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PORTFOLIO_ROM_CARD, portfolio_rom_card_device, "portfolio_rom_card", "Atari Portfolio ROM card")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  portfolio_rom_card_device - constructor
//-------------------------------------------------

portfolio_rom_card_device::portfolio_rom_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PORTFOLIO_ROM_CARD, tag, owner, clock),
	device_portfolio_memory_card_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void portfolio_rom_card_device::device_start()
{
}


//-------------------------------------------------
//  nrdi_r - read
//-------------------------------------------------

uint8_t portfolio_rom_card_device::nrdi_r(offs_t offset)
{
	return m_rom[offset];
}
