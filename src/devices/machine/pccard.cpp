// license:BSD-3-Clause
// copyright-holders:smf
#include "pccard.h"

READ16_MEMBER( pccard_interface::read_memory )
{
	return 0xffff;
}

WRITE16_MEMBER( pccard_interface::write_memory )
{
}

READ16_MEMBER( pccard_interface::read_reg )
{
	return 0xffff;
}

WRITE16_MEMBER( pccard_interface::write_reg )
{
}

const device_type PCCARD_SLOT = &device_creator<pccard_slot_device>;

pccard_slot_device::pccard_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PCCARD_SLOT, "PCCARD SLOT", tag, owner, clock, "pccard", __FILE__),
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
