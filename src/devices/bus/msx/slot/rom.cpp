// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "rom.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_ROM, msx_slot_rom_device, "msx_slot_rom", "MSX Internal ROM")


msx_slot_rom_device::msx_slot_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_rom_device(mconfig, MSX_SLOT_ROM, tag, owner, clock)
{
}

msx_slot_rom_device::msx_slot_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_region_offset(0)
{
}

void msx_slot_rom_device::device_start()
{
	// Sanity checks
	if (m_rom_region->bytes() < m_region_offset + m_size)
	{
		fatalerror("Memory region '%s' is too small for rom slot '%s'\n", m_rom_region.finder_tag(), tag());
	}

	u8 *rom = m_rom_region->base() + m_region_offset;
	for (int i = m_start_address >> 14; i < 4 && i * 0x4000 < m_end_address; i++)
	{
		page(i)->install_rom(i * 0x4000, std::min<u32>((i + 1) * 0x4000, m_end_address) - 1, rom);
		rom += 0x4000;
	}
}
