// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "ram.h"

DEFINE_DEVICE_TYPE(MSX_SLOT_RAM, msx_slot_ram_device, "msx_slot_ram", "MSX Internal RAM")

msx_slot_ram_device::msx_slot_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_SLOT_RAM, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
{
}

void msx_slot_ram_device::device_start()
{
	if (m_size < m_end_address - m_start_address)
		fatalerror("Invalid size %04x for address range %004x - %04x\n", m_start_address, m_end_address);

	m_ram = std::make_unique<u8[]>(m_size);
	save_pointer(NAME(m_ram), m_size);

	u8 *ram = &m_ram[0];
	u32 start_address = m_start_address;
	for (int i = m_start_address >> 14; i < 4 && i * 0x4000 < m_end_address; i++)
	{
		page(i)->install_ram(start_address, std::min<u32>((i + 1) * 0x4000, m_end_address) - 1, ram);
		start_address += 0x4000;
		ram += 0x4000;
	}
}
