// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ABC 80 16 KB RAM expansion card emulation

*********************************************************************/

#include "ram.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ABC80_16KB_RAM_CARD = &device_creator<abc80_16kb_ram_card_t>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc80_16kb_ram_card_t - constructor
//-------------------------------------------------

abc80_16kb_ram_card_t::abc80_16kb_ram_card_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ABC80_16KB_RAM_CARD, "ABC 80 16KB RAM card", tag, owner, clock, "abc80_16kb", __FILE__),
	device_abcbus_card_interface(mconfig, *this),
	m_ram(*this, "ram")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc80_16kb_ram_card_t::device_start()
{
	m_ram.allocate(0x4000);
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_xmemfl -
//-------------------------------------------------

UINT8 abc80_16kb_ram_card_t::abcbus_xmemfl(offs_t offset)
{
	UINT8 data = 0xff;

	if (offset >= 0x8000 && offset < 0xc000)
	{
		data = m_ram[offset & 0x3fff];
	}

	return data;
}


//-------------------------------------------------
//  abcbus_xmemw -
//-------------------------------------------------

void abc80_16kb_ram_card_t::abcbus_xmemw(offs_t offset, UINT8 data)
{
	if (offset >= 0x8000 && offset < 0xc000)
	{
		m_ram[offset & 0x3fff] = data;
	}
}
