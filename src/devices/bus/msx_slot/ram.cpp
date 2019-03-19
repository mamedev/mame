// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "ram.h"

DEFINE_DEVICE_TYPE(MSX_SLOT_RAM, msx_slot_ram_device, "msx_slot_ram", "MSX Internal RAM")

msx_slot_ram_device::msx_slot_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_SLOT_RAM, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
{
}

void msx_slot_ram_device::device_start()
{
	m_ram.resize(m_size);
	save_item(NAME(m_ram));
}

uint8_t msx_slot_ram_device::read(offs_t offset)
{
	if ( offset >= m_start_address && offset < m_end_address )
	{
		return m_ram[ offset - m_start_address ];
	}
	return 0xFF;
}

void msx_slot_ram_device::write(offs_t offset, uint8_t data)
{
	if ( offset >= m_start_address && offset < m_end_address )
	{
		m_ram[offset - m_start_address] = data;
	}
}
