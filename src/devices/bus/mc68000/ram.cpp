// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    mc-68000-Computer RAM Expansion Card

***************************************************************************/

#include "emu.h"
#include "ram.h"

DEFINE_DEVICE_TYPE(MC68000_RAM, mc68000_ram_device, "mc68000_ram", "mc-68000 2 MB RAM Expansion")

mc68000_ram_device::mc68000_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MC68000_RAM, tag, owner, clock),
	device_mc68000_sysbus_card_interface(mconfig, *this)
{
}

void mc68000_ram_device::device_start()
{
	// allocate ram
	m_ram = std::make_unique<uint16_t[]>(0x200000 / 2);

	// register for save states
	save_pointer(NAME(m_ram), 0x200000 / 2);
}

void mc68000_ram_device::device_reset()
{
}

uint16_t mc68000_ram_device::slot_r(offs_t offset, uint16_t mem_mask)
{
	return m_ram[offset & (0x1fffff >> 1)] & mem_mask;
}

void mc68000_ram_device::slot_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ram[offset & (0x1fffff >> 1)]);
}
