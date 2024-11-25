// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

  Emulation of the firmware mapper in the Panasonic FS-A1FM.

*/

#include "emu.h"
#include "fsa1fm.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_FSA1FM, msx_slot_fsa1fm_device, "msx_slot_fs1afm", "MSX Internal FS-A1FM")
DEFINE_DEVICE_TYPE(MSX_SLOT_FSA1FM2, msx_slot_fsa1fm2_device, "msx_slot_fs1afm2", "MSX Internal FS-A1FM part 2")


msx_slot_fsa1fm2_device::msx_slot_fsa1fm2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_SLOT_FSA1FM2, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_bank(*this, "bank%u", 0U)
	, m_view{ {*this, "view0"}, {*this, "view1"}, {*this, "view2"}, {*this, "view3"}, {*this, "view4"}, {*this, "view5"} }
	, m_region_offset(0)
{
}

void msx_slot_fsa1fm2_device::device_start()
{
	// Sanity checks
	if (m_rom_region->bytes() != 0x100000)
	{
		fatalerror("Memory region '%s' is not the correct size for the FS-A1FM firmware\n", m_rom_region.finder_tag());
	}

	m_ram = std::make_unique<u8[]>(RAM_SIZE);
	m_empty_bank = std::make_unique<u8[]>(8 * 1024);
	for (int i = 0; i < 8 * 1024; i++)
		m_empty_bank[i] = 0xff;

	save_item(NAME(m_selected_bank));
	save_item(NAME(m_control));
	save_item(NAME(m_ram_active));
	save_pointer(NAME(m_ram), m_size);

	for (int i = 0; i < 6; i++)
	{
		m_bank[i]->configure_entries(0, 0x80, m_rom_region->base() + m_region_offset, 0x2000);
		m_bank[i]->configure_entries(0x80, 0x80, m_rom_region->base() + m_region_offset, 0x2000);
		for (int j = 0; j < 4; j++)
		{
			m_bank[i]->configure_entry(0x80 + j, &m_empty_bank[0]);  // 0x80-0x83 empty
			m_bank[i]->configure_entry(0x84 + j, &m_ram[0]);  // 0x84-0x87 ram
			m_bank[i]->configure_entry(0x88 + j, &m_empty_bank[0]);  // 0x88-0x8b empty
			m_bank[i]->configure_entry(0x8c + j, &m_ram[0]);  // 0x8c-0x8f ram
		}
	}

	page(0)->install_view(0x0000, 0x1fff, m_view[0]);
	m_view[0][0].install_read_bank(0x0000, 0x1fff, m_bank[0]);
	m_view[0][1].install_readwrite_bank(0x0000, 0x1fff, m_bank[0]);

	page(0)->install_view(0x2000, 0x3fff, m_view[1]);
	m_view[1][0].install_read_bank(0x2000, 0x3fff, m_bank[1]);
	m_view[1][1].install_readwrite_bank(0x2000, 0x3fff, m_bank[1]);

	page(1)->install_view(0x4000, 0x5fff, m_view[2]);
	m_view[2][0].install_read_bank(0x4000, 0x5fff, m_bank[2]);
	m_view[2][1].install_readwrite_bank(0x4000, 0x5fff, m_bank[2]);

	page(1)->install_view(0x6000, 0x7fff, m_view[3]);
	m_view[3][0].install_read_bank(0x6000, 0x7fff, m_bank[3]);
	m_view[3][1].install_readwrite_bank(0x6000, 0x7fff, m_bank[3]);
	m_view[3][2].install_read_bank(0x6000, 0x7fff, m_bank[3]);
	m_view[3][2].install_read_handler(0x7ff0, 0x7ff7, emu::rw_delegate(*this, FUNC(msx_slot_fsa1fm2_device::bank_r)));
	m_view[3][3].install_readwrite_bank(0x6000, 0x7fff, m_bank[3]);
	m_view[3][3].install_read_handler(0x7ff0, 0x7ff7, emu::rw_delegate(*this, FUNC(msx_slot_fsa1fm2_device::bank_r)));
	page(1)->install_write_handler(0x6000, 0x6000, emu::rw_delegate(*this, FUNC(msx_slot_fsa1fm2_device::bank_w<2>)));
	page(1)->install_write_handler(0x6400, 0x6400, emu::rw_delegate(*this, FUNC(msx_slot_fsa1fm2_device::bank_w<0>)));
	page(1)->install_write_handler(0x6800, 0x6800, emu::rw_delegate(*this, FUNC(msx_slot_fsa1fm2_device::bank_w<3>)));
	page(1)->install_write_handler(0x6c00, 0x6c00, emu::rw_delegate(*this, FUNC(msx_slot_fsa1fm2_device::bank_w<1>)));
	page(1)->install_write_handler(0x7000, 0x7000, emu::rw_delegate(*this, FUNC(msx_slot_fsa1fm2_device::bank_w<4>)));
	page(1)->install_write_handler(0x7800, 0x7800, emu::rw_delegate(*this, FUNC(msx_slot_fsa1fm2_device::bank_w<5>)));
	page(1)->install_write_handler(0x7ff9, 0x7ff9, emu::rw_delegate(*this, FUNC(msx_slot_fsa1fm2_device::control_w)));

	page(2)->install_view(0x8000, 0x9fff, m_view[4]);
	m_view[4][0].install_read_bank(0x8000, 0x9fff, m_bank[4]);
	m_view[4][1].install_readwrite_bank(0x8000, 0x9fff, m_bank[4]);

	page(2)->install_view(0xa000, 0xbfff, m_view[5]);
	m_view[5][0].install_read_bank(0xa000, 0xbfff, m_bank[5]);
	m_view[5][1].install_readwrite_bank(0xa000, 0xbfff, m_bank[5]);
}

void msx_slot_fsa1fm2_device::device_reset()
{
	m_control = 0;

	for (int i = 0; i < 6 ; i++)
	{
		m_view[i].select(0);
		m_selected_bank[i] = 0xa8;
		m_bank[i]->set_entry(0x28);
	}
}

template <int Bank>
void msx_slot_fsa1fm2_device::set_view()
{
	bool ram_active = (m_selected_bank[Bank] & 0xf4) == 0x84;
	if (Bank == 3)
		m_view[3].select((ram_active ? 1 : 0) | (BIT(m_control, 2) ? 2 : 0));
	else if (Bank != 2)
		m_view[Bank].select(ram_active ? 1 : 0);
}

template <int Bank>
void msx_slot_fsa1fm2_device::bank_w(u8 data)
{
	m_selected_bank[Bank] = data;
	m_bank[Bank]->set_entry(data);
	set_view<Bank>();
}

u8 msx_slot_fsa1fm2_device::bank_r(offs_t offset)
{
	return (offset < 6) ? m_selected_bank[offset] : 0;
}

void msx_slot_fsa1fm2_device::control_w(u8 data)
{
	// writing $04 enables read back of banking registers at 7ff0-7ff5
	m_control = data;
	set_view<3>();
}




msx_slot_fsa1fm_device::msx_slot_fsa1fm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_SLOT_FSA1FM, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_nvram(*this, "nvram")
	, m_i8251(*this, "i8251")
	, m_i8255(*this, "i8255")
	, m_switch_port(*this, "SWITCH")
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_rombank(*this, "rombank")
	, m_region_offset(0)
{
}

static INPUT_PORTS_START(fsa1fm)
	PORT_START("SWITCH")
	PORT_CONFNAME(0x04, 0x00, "Enable Firmware")
	PORT_CONFSETTING(0x04, DEF_STR(No))
	PORT_CONFSETTING(0x00, DEF_STR(Yes))
INPUT_PORTS_END

ioport_constructor msx_slot_fsa1fm_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(fsa1fm);
}

void msx_slot_fsa1fm_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);

	// Unknown how these are connected
	I8251(config, m_i8251, 0);

	I8255(config, m_i8255);
	m_i8255->out_pa_callback().set(FUNC(msx_slot_fsa1fm_device::i8255_port_a_w));
	m_i8255->out_pb_callback().set(FUNC(msx_slot_fsa1fm_device::i8255_port_b_w));
	m_i8255->in_pc_callback().set(FUNC(msx_slot_fsa1fm_device::i8255_port_c_r));
}

void msx_slot_fsa1fm_device::device_start()
{
	// Sanity checks
	if (m_rom_region->bytes() != 0x100000)
	{
		fatalerror("Memory region '%s' is not the correct size for the FS-A1FM firmware\n", m_rom_region.finder_tag());
	}

	m_sram = std::make_unique<u8[]>(SRAM_SIZE);
	m_nvram->set_base(&m_sram[0], SRAM_SIZE);

	save_pointer(NAME(m_sram), SRAM_SIZE);

	m_rombank->configure_entries(0, 16, m_rom_region->base(), 0x2000);

	page(1)->install_read_bank(0x4000, 0x5fff, m_rombank);
	// SRAM is always visible?
	page(1)->install_ram(0x6000, 0x7fff, &m_sram[0]);
	page(1)->install_write_handler(0x7fc0, 0x7fc0, emu::rw_delegate(*m_i8251, FUNC(i8251_device::data_w)));
	page(1)->install_read_handler(0x7fc0, 0x7fc0, emu::rw_delegate(*m_i8251, FUNC(i8251_device::data_r)));
	page(1)->install_write_handler(0x7fc1, 0x7fc1, emu::rw_delegate(*m_i8251, FUNC(i8251_device::control_w)));
	page(1)->install_write_handler(0x7fc4, 0x7fc7, emu::rw_delegate(*m_i8255, FUNC(i8255_device::write)));
	page(1)->install_read_handler(0x7fc4, 0x7fc7, emu::rw_delegate(*m_i8255, FUNC(i8255_device::read)));
}

void msx_slot_fsa1fm_device::device_reset()
{
	m_rombank->set_entry(0);
}

void msx_slot_fsa1fm_device::i8255_port_a_w(u8 data)
{
	// 0xc0 enables sram?
	logerror("port A write %02x\n", data);
	m_rombank->set_entry(data & 0x0f);
}

void msx_slot_fsa1fm_device::i8255_port_b_w(u8 data)
{
	logerror("port B write %02x\n", data);
}

u8 msx_slot_fsa1fm_device::i8255_port_c_r()
{
	logerror("port C read\n");
	return m_switch_port->read();
}
