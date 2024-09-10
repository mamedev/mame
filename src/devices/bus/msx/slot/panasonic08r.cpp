// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

  Emulation of the firmware mapper as found in Panasonic FS-A1ST and FS-A1GT Turbo-R machines.


*/

#include "emu.h"
#include "panasonic08r.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_PANASONIC08R, msx_slot_panasonic08r_device, "msx_slot_panasonic08r", "MSX Internal Panasonic08r")


msx_slot_panasonic08r_device::msx_slot_panasonic08r_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_SLOT_PANASONIC08R, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_sram_size(0)
	, m_nvram(*this, "nvram")
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_bank(*this, "bank%u", 0U)
	, m_view{
		{*this, "view0"}, {*this, "view1"}, {*this, "view2"}, {*this, "view3"},
		{*this, "view4"}, {*this, "view5"}, {*this, "view6"}, {*this, "view7"}
	}
	, m_mm(*this, finder_base::DUMMY_TAG)
	, m_region_offset(0)
	, m_control(0)
{
}


void msx_slot_panasonic08r_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);
}


void msx_slot_panasonic08r_device::device_start()
{
	// Sanity checks
	if (m_rom_region->bytes() < m_region_offset + 0x400000)
		fatalerror("Memory region '%s' is too small for the panasonic08r firmware\n", m_rom_region.finder_tag());
	if (m_sram_size != 0x4000 && m_sram_size != 0x8000)
		fatalerror("Invalid SRAM size for the panasonic08r firmware\n");

	m_sram = std::make_unique<u8[]>(m_sram_size);
	m_nvram->set_base(&m_sram[0], m_sram_size);

	save_item(NAME(m_selected_bank));
	save_item(NAME(m_control));
	save_item(NAME(m_bank9));
	save_pointer(NAME(m_sram), m_sram_size);

	for (int i = 0; i < 8; i++)
	{
		m_bank[i]->configure_entries(0, 0x200, m_rom_region->base() + m_region_offset, 0x2000);
		m_bank[i]->configure_entry(0x80, &m_sram[0]);
		m_bank[i]->configure_entry(0x81, &m_sram[0x2000]);
		if (m_sram_size >= 0x8000)
		{
			m_bank[i]->configure_entry(0x82, &m_sram[0x4000]);
			m_bank[i]->configure_entry(0x83, &m_sram[0x6000]);
		}

		// Assuming smaller internal RAM is mirrored.
		int internal_ram_banks = m_mm->get_ram_size() / 0x2000;
		int start_bank = 0x180;
		do {
			int nr_banks = internal_ram_banks;
			if (start_bank + nr_banks > 0x200)
				nr_banks = 0x200 - start_bank;
			m_bank[i]->configure_entries(start_bank, nr_banks, m_mm->get_ram_base(), 0x2000);
			start_bank += internal_ram_banks;
		}
		while (start_bank < 0x200);
	}

	for (int i = 0; i < 8; i++)
	{
		const u16 start = 0x2000 * i;
		const u16 end = start + 0x1fff;

		page(i/2)->install_view(start, end, m_view[i]);
		m_view[i][0].install_read_bank(start, end, m_bank[i]);
		m_view[i][1].install_readwrite_bank(start, end, m_bank[i]);
	}

	m_view[3][2].install_read_bank(0x6000, 0x7fff, m_bank[3]);
	m_view[3][2].install_read_handler(0x7ff0, 0x7ff7, read8sm_delegate(*this, [this] (offs_t offset) { return m_selected_bank[offset] & 0xff; }, "bank_r"));
	m_view[3][3].install_readwrite_bank(0x6000, 0x7fff, m_bank[3]);
	m_view[3][3].install_read_handler(0x7ff0, 0x7ff7, read8sm_delegate(*this, [this] (offs_t offset) { return m_selected_bank[offset] & 0xff; }, "bank_r"));
	page(1)->install_write_handler(0x6000, 0x6000, emu::rw_delegate(*this, FUNC(msx_slot_panasonic08r_device::bank_w<0>)));
	page(1)->install_write_handler(0x6400, 0x6400, emu::rw_delegate(*this, FUNC(msx_slot_panasonic08r_device::bank_w<1>)));
	page(1)->install_write_handler(0x6800, 0x6800, emu::rw_delegate(*this, FUNC(msx_slot_panasonic08r_device::bank_w<2>)));
	page(1)->install_write_handler(0x6c00, 0x6c00, emu::rw_delegate(*this, FUNC(msx_slot_panasonic08r_device::bank_w<3>)));
	page(1)->install_write_handler(0x7000, 0x7000, emu::rw_delegate(*this, FUNC(msx_slot_panasonic08r_device::bank_w<4>)));
	page(1)->install_write_handler(0x7400, 0x7400, emu::rw_delegate(*this, FUNC(msx_slot_panasonic08r_device::bank_w<6>)));
	page(1)->install_write_handler(0x7800, 0x7800, emu::rw_delegate(*this, FUNC(msx_slot_panasonic08r_device::bank_w<5>)));
	page(1)->install_write_handler(0x7c00, 0x7c00, emu::rw_delegate(*this, FUNC(msx_slot_panasonic08r_device::bank_w<7>)));
	page(1)->install_readwrite_handler(0x7ff8, 0x7ff8,
		read8smo_delegate(*this, [this] () { return m_bank9; }, "bank9_r"),
		write8smo_delegate(*this, [this] (u8 data)
		{
			m_bank9 = data;
			select_banks();
		}, "bank9_w"));
	page(1)->install_write_handler(0x7ff9, 0x7ff9, write8smo_delegate(*this, [this] (u8 data)
		{
			m_control = data;
			select_banks();
		}, "control_w"
	));
}


void msx_slot_panasonic08r_device::device_reset()
{
	m_control = 0;
	m_bank9 = 0;
	for (int i = 0 ; i < 8; i++)
	{
		m_selected_bank[i] = 0;
		m_bank[i]->set_entry(0);
		m_view[i].select(0);
	}
}


template <int Bank>
void msx_slot_panasonic08r_device::set_view()
{
	const bool sram_active = (m_selected_bank[Bank] >= 0x80 && m_selected_bank[Bank] < 0x84);
	const bool ram_active = (m_selected_bank[Bank] >= 0x180);
	const int view = ((sram_active || ram_active) ? 1 : 0) | ((Bank == 3 && BIT(m_control, 2)) ? 2 : 0);

	m_view[Bank].select(view);
}


template <int Bank>
void msx_slot_panasonic08r_device::bank_w(u8 data)
{
	u16 bank = data;

	if (BIT(m_control, 4))
		bank |= (BIT(m_bank9, Bank) << 8);
	m_selected_bank[Bank] = bank;
	m_bank[Bank]->set_entry(bank);
	set_view<Bank>();
}


void msx_slot_panasonic08r_device::select_banks()
{
	bank_w<0>(m_selected_bank[0]);
	bank_w<1>(m_selected_bank[1]);
	bank_w<2>(m_selected_bank[2]);
	bank_w<3>(m_selected_bank[3]);
	bank_w<4>(m_selected_bank[4]);
	bank_w<5>(m_selected_bank[5]);
	bank_w<6>(m_selected_bank[6]);
	bank_w<7>(m_selected_bank[7]);
}
