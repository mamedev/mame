// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

  Emulation of the games mapper as found in AX-230 machines.

*/

#include "emu.h"
#include "ax230.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_AX230, msx_slot_ax230_device, "msx_slot_ax230", "MSX Internal AX230")


msx_slot_ax230_device::msx_slot_ax230_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_SLOT_AX230, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_region_offset(0)
	, m_rom(nullptr)
	, m_selected_bank{ 0, 0, 0, 0 }
	, m_bank_base{ nullptr, nullptr, nullptr, nullptr }
{
}


void msx_slot_ax230_device::device_start()
{
	// Sanity checks
	if (m_rom_region->bytes() < m_region_offset + 0x100000)
	{
		fatalerror("Memory region '%s' is too small for the AX230 firmware\n", m_rom_region.finder_tag());
	}

	m_rom = m_rom_region->base() + m_region_offset;
	m_bank_mask = 0x7f;

	save_item(NAME(m_selected_bank));
}


void msx_slot_ax230_device::device_reset()
{
	for (int i = 0; i < 4; i++)
		m_selected_bank[i] = 0;

	restore_banks();
}


void msx_slot_ax230_device::device_post_load()
{
	restore_banks();
}


void msx_slot_ax230_device::restore_banks()
{
	for (int i = 0; i < 4; i++)
	{
		m_bank_base[i] = m_rom + (m_selected_bank[i] & m_bank_mask) * 0x2000;
	}
}


uint8_t msx_slot_ax230_device::read(offs_t offset)
{
	if (offset >= 0x4000 && offset < 0xc000)
	{
		return m_bank_base[(offset - 0x4000) >> 13][offset & 0x1fff];
	}
	return 0xff;
}


void msx_slot_ax230_device::write(offs_t offset, uint8_t data)
{
	if (offset >= 0x6000 && offset < 0x8000)
	{
		uint8_t bank = (offset / 0x800) & 0x03;

		m_selected_bank[bank] = data;
		m_bank_base[bank] = m_rom + (m_selected_bank[bank] & m_bank_mask) * 0x2000;
	}
}
