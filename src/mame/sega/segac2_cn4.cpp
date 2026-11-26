// license:BSD-3-Clause
// copyright-holders:Damian Rogers
/***********************************************************************

  Sega System C2 CN4 expansion connector

  CN4 sits in the counter/timer chip select (0x880100 on the 68000 bus)
  and carries the low byte of the data bus, so a board on it sees byte
  accesses at odd addresses.  Offsets passed to a card are register
  numbers (address bits 1-5).

 ***********************************************************************/

#include "emu.h"
#include "segac2_cn4.h"


DEFINE_DEVICE_TYPE(SEGAC2_CN4_SLOT, segac2_cn4_slot_device, "segac2_cn4", "Sega System C2 CN4 expansion connector")


segac2_cn4_slot_device::segac2_cn4_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SEGAC2_CN4_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_segac2_cn4_interface>(mconfig, *this)
	, m_card(nullptr)
{
}


void segac2_cn4_slot_device::device_start()
{
	m_card = get_card_device();
}


u8 segac2_cn4_slot_device::read(offs_t offset)
{
	return m_card ? m_card->read(offset) : 0xff;
}


void segac2_cn4_slot_device::write(offs_t offset, u8 data)
{
	if (m_card)
		m_card->write(offset, data);
}


device_segac2_cn4_interface::device_segac2_cn4_interface(machine_config const &mconfig, device_t &device)
	: device_interface(device, "segac2_cn4")
{
}
