// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

  Emulation of the games mapper as found in AX-230 machines.

*/

#include "emu.h"
#include "ax230.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_AX230, msx_slot_ax230_device, "msx_slot_ax230", "MSX Internal AX230")


msx_slot_ax230_device::msx_slot_ax230_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_SLOT_AX230, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_rombank(*this, "rombank%u", 0U)
	, m_region_offset(0)
{
}

void msx_slot_ax230_device::device_start()
{
	// Sanity checks
	if (m_rom_region->bytes() < m_region_offset + 0x100000)
	{
		fatalerror("Memory region '%s' is too small for the AX230 firmware\n", m_rom_region.finder_tag());
	}

	for (int i = 0; i < 4; i++)
	{
		m_rombank[i]->configure_entries(0, BANKS, m_rom_region->base() + m_region_offset, 0x2000);
	}
	page(1)->install_read_bank(0x4000, 0x5fff, m_rombank[0]);
	page(1)->install_read_bank(0x6000, 0x7fff, m_rombank[1]);
	page(1)->install_write_handler(0x6000, 0x7fff, write8sm_delegate(*this, FUNC(msx_slot_ax230_device::mapper_write)));
	page(2)->install_read_bank(0x8000, 0x9fff, m_rombank[2]);
	page(2)->install_read_bank(0xa000, 0xbfff, m_rombank[3]);
}

void msx_slot_ax230_device::device_reset()
{
	for (int i = 0; i < 4; i++)
		m_rombank[i]->set_entry(0);
}

void msx_slot_ax230_device::mapper_write(offs_t offset, u8 data)
{
	m_rombank[(offset / 0x800) & 0x03]->set_entry(data & BANK_MASK);
}
