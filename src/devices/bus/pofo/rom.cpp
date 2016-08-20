// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio ROM card emulation

**********************************************************************/

#include "rom.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PORTFOLIO_ROM_CARD = &device_creator<portfolio_rom_card_t>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  portfolio_rom_card_t - constructor
//-------------------------------------------------

portfolio_rom_card_t::portfolio_rom_card_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PORTFOLIO_ROM_CARD, "Atari Portfolio ROM card", tag, owner, clock, "portfolio_rom_card", __FILE__),
	device_portfolio_memory_card_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void portfolio_rom_card_t::device_start()
{
}


//-------------------------------------------------
//  nrdi_r - read
//-------------------------------------------------

UINT8 portfolio_rom_card_t::nrdi_r(address_space &space, offs_t offset)
{
	return m_rom[offset];
}
