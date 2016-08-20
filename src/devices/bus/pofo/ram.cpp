// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio 128KB RAM card emulation

**********************************************************************/

#include "ram.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PORTFOLIO_RAM_CARD = &device_creator<portfolio_ram_card_t>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  portfolio_ram_card_t - constructor
//-------------------------------------------------

portfolio_ram_card_t::portfolio_ram_card_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PORTFOLIO_RAM_CARD, "Atari Portfolio RAM card", tag, owner, clock, "portfolio_ram_card", __FILE__),
	device_portfolio_memory_card_slot_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void portfolio_ram_card_t::device_start()
{
	m_nvram.allocate(0x20000);
}


//-------------------------------------------------
//  nrdi_r - read
//-------------------------------------------------

UINT8 portfolio_ram_card_t::nrdi_r(address_space &space, offs_t offset)
{
	return m_nvram[offset];
}


//-------------------------------------------------
//  nwri_w - write
//-------------------------------------------------

void portfolio_ram_card_t::nwri_w(address_space &space, offs_t offset, UINT8 data)
{
	m_nvram[offset] = data;
}
