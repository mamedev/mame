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
//  ncc2_w - CCM bank select
//-------------------------------------------------

void portfolio_rom_card_device::ncc2_w(int state)
{
	if (state)
	{
		m_tap = m_slot->memspace().install_read_tap(0xc0000, 0xdffff, "ccm_rom",
			[this] (offs_t offset, u8 &data, u8) { data = m_rom ? m_rom[offset & 0x1ffff] : 0xff; }, &m_tap);
	}
	else
	{
		m_tap.remove();
	}
}
