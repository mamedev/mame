// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

  Emulation of the firmware mapper as found in Frael Bruc 100 machines.

*/

#include "emu.h"
#include "bruc100.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_BRUC100, msx_slot_bruc100_device, "msx_slot_bruc100", "MSX Internal BRUC100")


msx_slot_bruc100_device::msx_slot_bruc100_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_SLOT_BRUC100, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_rombank(*this, "rombank%u", 0U)
	, m_region_offset(0)
{
}

void msx_slot_bruc100_device::device_start()
{
	// Sanity checks
	if (m_rom_region->bytes() < m_region_offset + 0x10000)
	{
		fatalerror("Memory region '%s' is too small for the BRUC100 firmware\n", m_rom_region.finder_tag());
	}

	m_rombank[0]->configure_entries(0, 2, m_rom_region->base() + m_region_offset, 0x8000);
	m_rombank[1]->configure_entries(0, 2, m_rom_region->base() + m_region_offset + 0x4000, 0x8000);

	page(0)->install_read_bank(0x0000, 0x3fff, m_rombank[0]);
	page(1)->install_read_bank(0x4000, 0x7fff, m_rombank[1]);
}

void msx_slot_bruc100_device::device_reset()
{
	m_rombank[0]->set_entry(0);
	m_rombank[1]->set_entry(0);
}

void msx_slot_bruc100_device::select_bank(u8 bank)
{
	m_rombank[0]->set_entry(bank);
	m_rombank[1]->set_entry(bank);
}
