// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ABC 80 16 KB RAM expansion card emulation

*********************************************************************/

#include "emu.h"
#include "ram.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ABC80_16KB_RAM_CARD, abc80_16kb_ram_card_device, "abc80_16kb", "ABC 80 16KB RAM card")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc80_16kb_ram_card_device - constructor
//-------------------------------------------------

abc80_16kb_ram_card_device::abc80_16kb_ram_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ABC80_16KB_RAM_CARD, tag, owner, clock),
	device_abcbus_card_interface(mconfig, *this),
	m_ram(*this, "ram", 0x4000, ENDIANNESS_LITTLE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc80_16kb_ram_card_device::device_start()
{
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_xmemfl -
//-------------------------------------------------

uint8_t abc80_16kb_ram_card_device::abcbus_xmemfl(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset >= 0x8000 && offset < 0xc000)
	{
		data = m_ram[offset & 0x3fff];
	}

	return data;
}


//-------------------------------------------------
//  abcbus_xmemw -
//-------------------------------------------------

void abc80_16kb_ram_card_device::abcbus_xmemw(offs_t offset, uint8_t data)
{
	if (offset >= 0x8000 && offset < 0xc000)
	{
		m_ram[offset & 0x3fff] = data;
	}
}
