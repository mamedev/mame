// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*
  Emulation for the internal firmware mapper in the National FS-4600.
*/

#include "emu.h"
#include "fs4600.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_FS4600, msx_slot_fs4600_device, "msx_slot_fs4600", "MSX Internal FS4600 Firmware")


msx_slot_fs4600_device::msx_slot_fs4600_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_SLOT_FS4600, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_nvram(*this, "nvram")
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_rombank(*this, "rombank%u", 0U)
	, m_view{ {*this, "view0"}, {*this, "view1"}, {*this, "view2"} }
	, m_region_offset(0)
	, m_sram_address(0)
{
}

void msx_slot_fs4600_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);
}

void msx_slot_fs4600_device::device_start()
{
	// Sanity checks
	if (m_rom_region->bytes() < m_region_offset + 0x100000)
	{
		fatalerror("Memory region '%s' is too small for the FS4600 firmware\n", m_rom_region.finder_tag());
	}

	m_sram = std::make_unique<u8[]>(SRAM_SIZE);
	m_nvram->set_base(&m_sram[0], SRAM_SIZE);

	save_item(NAME(m_sram_address));
	save_pointer(NAME(m_sram), SRAM_SIZE);

	for (int i = 0; i < 3; i++)
		m_rombank[i]->configure_entries(0, 0x40, m_rom_region->base() + m_region_offset, 0x4000);

	page(0)->install_view(0x0000, 0x3fff, m_view[0]);
	m_view[0][0].install_read_bank(0x0000, 0x3fff, m_rombank[0]);
	m_view[0][1].install_read_bank(0x0000, 0x3fff, m_rombank[0]);
	m_view[0][1].install_write_handler(0x3ffa, 0x3ffd, emu::rw_delegate(*this, FUNC(msx_slot_fs4600_device::sram_w)));
	m_view[0][1].install_read_handler(0x3ffd, 0x3ffd, emu::rw_delegate(*this, FUNC(msx_slot_fs4600_device::sram_r)));

	page(1)->install_view(0x4000, 0x7fff, m_view[1]);
	m_view[1][0].install_read_bank(0x4000, 0x7fff, m_rombank[1]);
	m_view[1][0].install_write_handler(0x6000, 0x6000, emu::rw_delegate(*this, FUNC(msx_slot_fs4600_device::bank_w<1>)));
	m_view[1][0].install_write_handler(0x6400, 0x6400, emu::rw_delegate(*this, FUNC(msx_slot_fs4600_device::bank_w<0>)));
	m_view[1][0].install_write_handler(0x7000, 0x7000, emu::rw_delegate(*this, FUNC(msx_slot_fs4600_device::bank_w<2>)));
	m_view[1][1].install_read_bank(0x4000, 0x7fff, m_rombank[1]);
	m_view[1][1].install_write_handler(0x7ffa, 0x7ffd, emu::rw_delegate(*this, FUNC(msx_slot_fs4600_device::sram_w)));
	m_view[1][1].install_read_handler(0x7ffd, 0x7ffd, emu::rw_delegate(*this, FUNC(msx_slot_fs4600_device::sram_r)));
	m_view[1][2].install_read_bank(0x4000, 0x7fff, m_rombank[1]);
	m_view[1][2].install_write_handler(0x6000, 0x6000, emu::rw_delegate(*this, FUNC(msx_slot_fs4600_device::bank_w<1>)));
	m_view[1][2].install_write_handler(0x6400, 0x6400, emu::rw_delegate(*this, FUNC(msx_slot_fs4600_device::bank_w<0>)));
	m_view[1][2].install_write_handler(0x7000, 0x7000, emu::rw_delegate(*this, FUNC(msx_slot_fs4600_device::bank_w<2>)));
	m_view[1][2].install_read_handler(0x7ff0, 0x7ff5, emu::rw_delegate(*this, FUNC(msx_slot_fs4600_device::bank_r)));
	m_view[1][3].install_read_bank(0x4000, 0x7fff, m_rombank[1]);
	m_view[1][3].install_read_handler(0x7ff0, 0x7ff5, emu::rw_delegate(*this, FUNC(msx_slot_fs4600_device::bank_r)));
	m_view[1][3].install_write_handler(0x7ffa, 0x7ffd, emu::rw_delegate(*this, FUNC(msx_slot_fs4600_device::sram_w)));
	m_view[1][3].install_read_handler(0x7ffd, 0x7ffd, emu::rw_delegate(*this, FUNC(msx_slot_fs4600_device::sram_r)));

	page(1)->install_write_handler(0x7ff9, 0x7ff9, emu::rw_delegate(*this, FUNC(msx_slot_fs4600_device::control_w)));

	page(2)->install_view(0x8000, 0xbfff, m_view[2]);
	m_view[2][0].install_read_bank(0x8000, 0xbfff, m_rombank[2]);
	m_view[2][1].install_read_bank(0x8000, 0xbfff, m_rombank[2]);
	m_view[2][1].install_write_handler(0xbffa, 0xbffd, emu::rw_delegate(*this, FUNC(msx_slot_fs4600_device::sram_w)));
	m_view[2][1].install_read_handler(0xbffd, 0xbffd, emu::rw_delegate(*this, FUNC(msx_slot_fs4600_device::sram_r)));
}

void msx_slot_fs4600_device::device_reset()
{
	for (int i = 0; i < 3; i++)
	{
		m_view[i].select(0);
		m_rombank[i]->set_entry(0);
	}
}

u8 msx_slot_fs4600_device::sram_r()
{
	u8 data = m_sram[m_sram_address & (SRAM_SIZE - 1)];
	if (!machine().side_effects_disabled())
		m_sram_address++;
	return data;
}

void msx_slot_fs4600_device::sram_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0:
		m_sram_address = (m_sram_address & 0x00ffff) | (data << 16);
		break;

	case 1:
		m_sram_address = (m_sram_address & 0xff00ff) | (data << 8);
		break;

	case 2:
		m_sram_address = (m_sram_address & 0xffff00) | data;
		break;

	case 3:
		m_sram[m_sram_address++ & (SRAM_SIZE - 1)] = data;
		break;
	}
}

u8 msx_slot_fs4600_device::bank_r(offs_t offset)
{
	return m_rombank[offset >> 1]->entry();
}

template <int Bank>
void msx_slot_fs4600_device::bank_w(u8 data)
{
	m_rombank[Bank]->set_entry(data & 0x3f);
}

void msx_slot_fs4600_device::control_w(u8 data)
{
	m_view[0].select((data >> 1) & 0x01);
	m_view[1].select((data >> 1) & 0x03);
	m_view[2].select((data >> 1) & 0x01);
}
