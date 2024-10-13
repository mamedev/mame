// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

  Emulation of the firmware mapper as found in Sony HB-F1XDJ and HB-F1XV machines.

*/

#include "emu.h"
#include "sony08.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_SONY08, msx_slot_sony08_device, "msx_slot_sony08", "MSX Internal SONY08")


msx_slot_sony08_device::msx_slot_sony08_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_SLOT_SONY08, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_nvram(*this, "nvram")
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_rombank(*this, "rombank%u", 0U)
	, m_view{ {*this, "view0"}, {*this, "view1"} }
	, m_region_offset(0)
{
}


void msx_slot_sony08_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);
}

void msx_slot_sony08_device::device_start()
{
	// Sanity checks
	if (m_rom_region->bytes() < m_region_offset + 0x100000)
	{
		fatalerror("Memory region '%s' is too small for the SONY08 firmware\n", m_rom_region.finder_tag());
	}

	m_sram= std::make_unique<u8[]>(SRAM_SIZE);
	m_nvram->set_base(&m_sram[0], SRAM_SIZE);

	save_pointer(NAME(m_sram), SRAM_SIZE);

	for (int i = 0; i < 4; i++)
		m_rombank[i]->configure_entries(0, 0x80, m_rom_region->base() + m_region_offset, 0x2000);
	m_rombank[4]->configure_entries(0, 0x100, m_rom_region->base() + m_region_offset + 0x80000, 0x800);
	m_rombank[5]->configure_entries(0, 0x100, m_rom_region->base() + m_region_offset + 0x80000, 0x800);

	page(0)->install_view(0x0000, 0x3fff, m_view[0]);
	m_view[0][0];
	m_view[0][1].install_ram(0x0000, 0x3fff, &m_sram[0]);

	page(1)->install_read_bank(0x4000, 0x5fff, m_rombank[0]);
	page(1)->install_write_handler(0x4fff, 0x4fff, emu::rw_delegate(*this, FUNC(msx_slot_sony08_device::bank_w<0>)));
	page(1)->install_view(0x6000, 0x7fff, m_view[1]);
	m_view[1][0].install_read_bank(0x6000, 0x7fff, m_rombank[1]);
	m_view[1][1].install_read_bank(0x6000, 0x6fff, m_rombank[1]);
	m_view[1][1].install_read_bank(0x7000, 0x77ff, m_rombank[4]);
	m_view[1][1].install_read_bank(0x7800, 0x7fff, m_rombank[5]);
	page(1)->install_write_handler(0x6fff, 0x6fff, emu::rw_delegate(*this, FUNC(msx_slot_sony08_device::bank_w<1>)));
	page(1)->install_write_handler(0x77ff, 0x77ff, emu::rw_delegate(*this, FUNC(msx_slot_sony08_device::bank_w<4>)));
	page(1)->install_write_handler(0x7fff, 0x7fff, emu::rw_delegate(*this, FUNC(msx_slot_sony08_device::bank_w<5>)));
	page(2)->install_read_bank(0x8000, 0x9fff, m_rombank[2]);
	page(2)->install_write_handler(0x8fff, 0x8fff, emu::rw_delegate(*this, FUNC(msx_slot_sony08_device::bank_w<2>)));
	page(2)->install_read_bank(0xa000, 0xbfff, m_rombank[3]);
	page(2)->install_write_handler(0xafff, 0xafff, emu::rw_delegate(*this, FUNC(msx_slot_sony08_device::bank_w<3>)));
}

void msx_slot_sony08_device::device_reset()
{
	for (int i = 0; i < 2; i++)
		m_view[i].select(0);
	for (int i = 0; i < 6; i++)
		m_rombank[i]->set_entry(0);
}

template <int Bank>
void msx_slot_sony08_device::bank_w(u8 data)
{
	if (Bank >= 4)
		m_rombank[Bank]->set_entry(data);
	else
		m_rombank[Bank]->set_entry(data & 0x7f);

	if (Bank < 2)
		m_view[Bank].select(BIT(data, 7) ? 1 : 0);
}
