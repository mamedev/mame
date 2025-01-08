// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

  Emulation of the firmware mapper as found in Panasonic FS-A1WX andFS-A1WSX machines.

TODO:
- SRAM has only 2 4KB pages
- SRAM is not accessible at 4000-7fff?
*/

#include "emu.h"
#include "panasonic08.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_PANASONIC08, msx_slot_panasonic08_device, "msx_slot_panasonic08", "MSX Internal Panasonic08")


msx_slot_panasonic08_device::msx_slot_panasonic08_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_SLOT_PANASONIC08, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_nvram(*this, "nvram")
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_bank(*this, "bank%u", 0U)
	, m_view{ {*this, "view0"}, {*this, "view1"}, {*this, "view2"}, {*this, "view3"}, {*this, "view4"}, {*this, "view5"} }
	, m_region_offset(0)
	, m_control(0)
{
}

void msx_slot_panasonic08_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);
}

void msx_slot_panasonic08_device::device_start()
{
	// Sanity checks
	if (m_rom_region->bytes() < m_region_offset + 0x200000)
	{
		fatalerror("Memory region '%s' is too small for the panasonic08 firmware\n", m_rom_region.finder_tag());
	}

	m_sram = std::make_unique<u8[]>(SRAM_SIZE);

	m_nvram->set_base(&m_sram[0], SRAM_SIZE);

	save_item(NAME(m_selected_bank));
	save_item(NAME(m_control));
	save_pointer(NAME(m_sram), SRAM_SIZE);

	for (int i = 0; i < 6; i++)
	{
		m_bank[i]->configure_entries(0, 0x100, m_rom_region->base() + m_region_offset, 0x2000);
		m_bank[i]->configure_entry(0x80, &m_sram[0]);
		m_bank[i]->configure_entry(0x81, &m_sram[0x2000]);
		m_bank[i]->configure_entry(0x82, &m_sram[0]);
		m_bank[i]->configure_entry(0x83, &m_sram[0x2000]);
	}

	page(0)->install_view(0x0000, 0x1fff, m_view[0]);
	m_view[0][0].install_read_bank(0x0000, 0x1fff, m_bank[0]);
	m_view[0][1].install_readwrite_bank(0x0000, 0x1fff, m_bank[0]);

	page(0)->install_view(0x2000, 0x3fff, m_view[1]);
	m_view[1][0].install_read_bank(0x2000, 0x3fff, m_bank[1]);
	m_view[1][1].install_readwrite_bank(0x2000, 0x3fff, m_bank[1]);

	// no sram writing in 4000-7fff?
	page(1)->install_read_bank(0x4000, 0x5fff, m_bank[2]);

	page(1)->install_view(0x6000, 0x7fff, m_view[3]);
	m_view[3][0].install_read_bank(0x6000, 0x7fff, m_bank[3]);
	m_view[3][1].install_read_bank(0x6000, 0x7fff, m_bank[3]);
	m_view[3][1].install_read_handler(0x7ff0, 0x7ff7, emu::rw_delegate(*this, FUNC(msx_slot_panasonic08_device::bank_r)));
	page(1)->install_write_handler(0x6000, 0x6000, emu::rw_delegate(*this, FUNC(msx_slot_panasonic08_device::bank_w<0>)));
	page(1)->install_write_handler(0x6400, 0x6400, emu::rw_delegate(*this, FUNC(msx_slot_panasonic08_device::bank_w<1>)));
	page(1)->install_write_handler(0x6800, 0x6800, emu::rw_delegate(*this, FUNC(msx_slot_panasonic08_device::bank_w<2>)));
	page(1)->install_write_handler(0x6c00, 0x6c00, emu::rw_delegate(*this, FUNC(msx_slot_panasonic08_device::bank_w<3>)));
	page(1)->install_write_handler(0x7000, 0x7000, emu::rw_delegate(*this, FUNC(msx_slot_panasonic08_device::bank_w<4>)));
	page(1)->install_write_handler(0x7800, 0x7800, emu::rw_delegate(*this, FUNC(msx_slot_panasonic08_device::bank_w<5>)));
	page(1)->install_write_handler(0x7ff9, 0x7ff9, emu::rw_delegate(*this, FUNC(msx_slot_panasonic08_device::control_w)));

	page(2)->install_view(0x8000, 0x9fff, m_view[4]);
	m_view[4][0].install_read_bank(0x8000, 0x9fff, m_bank[4]);
	m_view[4][1].install_readwrite_bank(0x8000, 0x9fff, m_bank[4]);

	page(2)->install_view(0xa000, 0xbfff, m_view[5]);
	m_view[5][0].install_read_bank(0xa000, 0xbfff, m_bank[5]);
	m_view[5][1].install_readwrite_bank(0xa000, 0xbfff, m_bank[5]);
}

void msx_slot_panasonic08_device::device_reset()
{
	m_control = 0;
	for (int i = 0 ; i < 6; i++)
	{
		m_selected_bank[i] = 0;
		m_bank[i]->set_entry(0);
		if (i != 2)
			m_view[i].select(0);
	}
}

template <int Bank>
void msx_slot_panasonic08_device::set_view()
{
	bool ram_active = (m_selected_bank[Bank] >= 0x80 && m_selected_bank[Bank] < 0x84);
	if (Bank == 3)
		m_view[3].select(BIT(m_control, 2) ? 1 : 0);
	else if (Bank != 2)
		m_view[Bank].select(ram_active ? 1 : 0);
}

template <int Bank>
void msx_slot_panasonic08_device::bank_w(u8 data)
{
	m_selected_bank[Bank] = data;
	m_bank[Bank]->set_entry(data);
	set_view<Bank>();
}

u8 msx_slot_panasonic08_device::bank_r(offs_t offset)
{
	return (offset < 6) ? m_selected_bank[offset] : 0;
}

void msx_slot_panasonic08_device::control_w(u8 data)
{
	// writing $04 enables read back of banking registers at 7ff0-7ff5
	m_control = data;
	set_view<3>();
}
