// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "pccard.h"

READ16_MEMBER( pccard_interface::read_memory )
{
	//printf("unhandled memory read %08x %04x\n", offset, mem_mask);
	return 0xffff;
}

WRITE16_MEMBER( pccard_interface::write_memory )
{
	//printf("unhandled memory write %08x %04x %04x\n", offset, data, mem_mask);
}

READ16_MEMBER( pccard_interface::read_reg )
{
	//printf("unhandled register read %08x %04x\n", offset, mem_mask);
	return 0xffff;
}

WRITE16_MEMBER( pccard_interface::write_reg )
{
	//printf("unhandled register write %08x %04x %04x\n", offset, data, mem_mask);
}

DEFINE_DEVICE_TYPE(PCCARD_SLOT, pccard_slot_device, "pccard", "PC Card Slot")

pccard_slot_device::pccard_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PCCARD_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_pccard(nullptr)
{
}

void pccard_slot_device::device_start()
{
	m_pccard = dynamic_cast<pccard_interface *>(get_card_device());
}

READ_LINE_MEMBER(pccard_slot_device::read_line_inserted)
{
	return m_pccard != nullptr;
}

READ16_MEMBER( pccard_slot_device::read_memory )
{
	if( m_pccard != nullptr )
	{
		return m_pccard->read_memory( space, offset, mem_mask );
	}

	return 0xffff;
}

WRITE16_MEMBER( pccard_slot_device::write_memory )
{
	if( m_pccard != nullptr )
	{
		m_pccard->write_memory( space, offset, data, mem_mask );
	}
}

READ16_MEMBER( pccard_slot_device::read_reg )
{
	if( m_pccard != nullptr )
	{
		return m_pccard->read_reg( space, offset, mem_mask );
	}

	return 0xffff;
}

WRITE16_MEMBER( pccard_slot_device::write_reg )
{
	if( m_pccard != nullptr )
	{
		m_pccard->write_reg( space, offset, data, mem_mask );
	}
}
