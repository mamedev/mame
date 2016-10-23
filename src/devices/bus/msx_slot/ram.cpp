// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "ram.h"

const device_type MSX_SLOT_RAM = &device_creator<msx_slot_ram_device>;

msx_slot_ram_device::msx_slot_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_SLOT_RAM, "MSX Internal RAM", tag, owner, clock, "msx_slot_ram", __FILE__)
	, msx_internal_slot_interface()
{
}

void msx_slot_ram_device::device_start()
{
	m_ram.resize(m_size);
	save_item(NAME(m_ram));
}

uint8_t msx_slot_ram_device::read(address_space &space, offs_t offset, uint8_t mem_mask)
{
	if ( offset >= m_start_address && offset < m_end_address )
	{
		return m_ram[ offset - m_start_address ];
	}
	return 0xFF;
}

void msx_slot_ram_device::write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if ( offset >= m_start_address && offset < m_end_address )
	{
		m_ram[offset - m_start_address] = data;
	}
}
