// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

  Emulation of the firmware mapper in the Sanyo PHC-77.

*/

#include "emu.h"
#include "msx_write.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_MSX_WRITE, msx_slot_msx_write_device, "msx_slot_msx_write", "MSX Internal MSX-Write")


msx_slot_msx_write_device::msx_slot_msx_write_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_SLOT_MSX_WRITE, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_nvram(*this, "nvram")
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_region_offset(0)
	, m_rom(nullptr)
	, m_bank_base_4000(nullptr)
	, m_bank_base_8000(nullptr)
{
}


void msx_slot_msx_write_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);
}


void msx_slot_msx_write_device::device_start()
{
	// Sanity checks
	if (m_rom_region->bytes() != 0x80000)
	{
		fatalerror("Memory region '%s' is not the correct size for the MSX-Write firmware\n", m_rom_region.finder_tag());
	}

	m_rom = m_rom_region->base() + m_region_offset;
	m_nvram->set_base(m_sram, 0x2000);

	save_item(NAME(m_selected_bank));

	map_bank();
}


void msx_slot_msx_write_device::device_reset()
{
	m_selected_bank[0] = 0x00;
	m_selected_bank[1] = 0x01;

	map_bank();
}


void msx_slot_msx_write_device::device_post_load()
{
	map_bank();
}


void msx_slot_msx_write_device::map_bank()
{
	m_bank_base_4000 = m_rom + m_selected_bank[0] * 0x4000;
	m_bank_base_8000 = m_rom + m_selected_bank[1] * 0x4000;
}


uint8_t msx_slot_msx_write_device::read(offs_t offset)
{
	if (offset < 0x8000)
//		// TODO: How much of the sram is visible?
//		if (offset >= 0x6000 && offset < 0x6400)
//			return m_sram[offset & 0x3ff];
		return m_bank_base_4000[offset & 0x3fff];
	if (offset < 0xc000)
		return m_bank_base_8000[offset & 0x3fff];
	return 0xff;
}


void msx_slot_msx_write_device::write(offs_t offset, uint8_t data)
{
//	// TODO: How much of the sram is visible?
//	if (offset >= 0x6000 && offset <= 0x6400)
//	{
//		m_sram[offset & 0x3ff] = data;
//		return;
//	}
	logerror("write %04x : %02x\n", offset, data);
	if (offset == 0x6fff)
	{
		m_selected_bank[0] = data & 0x1f;
		map_bank();
	}
	if (offset == 0x7fff)
	{
		m_selected_bank[1] = data & 0x1f;
		map_bank();
	}
}
