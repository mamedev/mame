// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "ram.h"

const device_type MSX_SLOT_RAM = &device_creator<msx_slot_ram_device>;

msx_slot_ram_device::msx_slot_ram_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_SLOT_RAM, "MSX Internal RAM", tag, owner, clock, "msx_slot_ram", __FILE__)
	, msx_internal_slot_interface()
{
}

void msx_slot_ram_device::device_start()
{
	m_ram.resize(m_size);
	save_item(NAME(m_ram));
}

READ8_MEMBER(msx_slot_ram_device::read)
{
	if ( offset >= m_start_address && offset < m_end_address )
	{
		return m_ram[ offset - m_start_address ];
	}
	return 0xFF;
}

WRITE8_MEMBER(msx_slot_ram_device::write)
{
	if ( offset >= m_start_address && offset < m_end_address )
	{
		m_ram[offset - m_start_address] = data;
	}
}
