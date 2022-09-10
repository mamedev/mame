// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

  Emulation of the firmware mapper as found in Frael Bruc 100 machines.

*/

#include "emu.h"
#include "bruc100.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_BRUC100, msx_slot_bruc100_device, "msx_slot_bruc100", "MSX Internal BRUC100")


msx_slot_bruc100_device::msx_slot_bruc100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_SLOT_BRUC100, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_region_offset(0)
	, m_rom(nullptr)
	, m_selected_bank(0)
	, m_bank_base(nullptr)
{
}


void msx_slot_bruc100_device::device_start()
{
	// Sanity checks
	if (m_rom_region->bytes() < m_region_offset + 0x10000)
	{
		fatalerror("Memory region '%s' is too small for the BRUC100 firmware\n", m_rom_region.finder_tag());
	}

	m_rom = m_rom_region->base() + m_region_offset;

	save_item(NAME(m_selected_bank));

	map_bank();
}


void msx_slot_bruc100_device::device_post_load()
{
	map_bank();
}


void msx_slot_bruc100_device::map_bank()
{
	m_bank_base = m_rom + m_selected_bank * 0x8000;
}


uint8_t msx_slot_bruc100_device::read(offs_t offset)
{
	return m_bank_base[offset];
}


void msx_slot_bruc100_device::select_bank(uint8_t bank)
{
	m_selected_bank = bank;
	map_bank();
}
