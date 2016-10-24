// license:BSD-3-Clause
// copyright-holders:smf
#include "pccard.h"

uint16_t pccard_interface::read_memory(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return 0xffff;
}

void pccard_interface::write_memory(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
}

uint16_t pccard_interface::read_reg(address_space &space, offs_t offset, uint16_t mem_mask)
{
	return 0xffff;
}

void pccard_interface::write_reg(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
}

const device_type PCCARD_SLOT = &device_creator<pccard_slot_device>;

pccard_slot_device::pccard_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PCCARD_SLOT, "PCCARD SLOT", tag, owner, clock, "pccard", __FILE__),
	device_slot_interface(mconfig, *this),
	m_pccard(nullptr)
{
}

void pccard_slot_device::device_start()
{
	m_pccard = dynamic_cast<pccard_interface *>(get_card_device());
}

int pccard_slot_device::read_line_inserted()
{
	return m_pccard != nullptr;
}

uint16_t pccard_slot_device::read_memory(address_space &space, offs_t offset, uint16_t mem_mask)
{
	if( m_pccard != nullptr )
	{
		return m_pccard->read_memory( space, offset, mem_mask );
	}

	return 0xffff;
}

void pccard_slot_device::write_memory(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if( m_pccard != nullptr )
	{
		m_pccard->write_memory( space, offset, data, mem_mask );
	}
}

uint16_t pccard_slot_device::read_reg(address_space &space, offs_t offset, uint16_t mem_mask)
{
	if( m_pccard != nullptr )
	{
		return m_pccard->read_reg( space, offset, mem_mask );
	}

	return 0xffff;
}

void pccard_slot_device::write_reg(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if( m_pccard != nullptr )
	{
		m_pccard->write_reg( space, offset, data, mem_mask );
	}
}
