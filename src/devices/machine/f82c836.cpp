// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Chips & Technologies SCATsx single chipset

Based over later CS4031 chipset

TODO:
- EMS;
- Other misc features missing;

**************************************************************************************************/

#include "emu.h"
#include "f82c836.h"

DEFINE_DEVICE_TYPE(F82C836A, f82c836a_device, "f82c836a", "Chips and Technologies F82C836A SCATsx")

f82c836a_device::f82c836a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, F82C836A, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config("config_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(f82c836a_device::config_map), this))
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_keybc(*this, finder_base::DUMMY_TAG)
	, m_bios(*this, finder_base::DUMMY_TAG)
	, m_space_mem(nullptr)
	, m_space_io(nullptr)
	, m_ram(nullptr)
	, m_dma(*this, "dma%u", 1U)
	, m_intc(*this, "intc%u", 1U)
	, m_pit(*this, "pit")
	, m_rtc(*this, "rtc")
	, m_ram_dev(*this, finder_base::DUMMY_TAG)
	, m_isabus(*this, finder_base::DUMMY_TAG)
	, m_read_ior(*this, 0)
	, m_write_iow(*this)
	, m_write_tc(*this)
	, m_write_hold(*this)
	, m_write_nmi(*this)
	, m_write_intr(*this)
	, m_write_cpureset(*this)
	, m_write_a20m(*this)
	, m_write_spkr(*this)
{
}

void f82c836a_device::device_add_mconfig(machine_config &config)
{
	AM9517A(config, m_dma[0], 0);
	m_dma[0]->out_hreq_callback().set(m_dma[1], FUNC(am9517a_device::dreq0_w));
	m_dma[0]->out_eop_callback().set(FUNC(f82c836a_device::dma1_eop_w));
	m_dma[0]->in_memr_callback().set(FUNC(f82c836a_device::dma_read_byte));
	m_dma[0]->out_memw_callback().set(FUNC(f82c836a_device::dma_write_byte));
	m_dma[0]->in_ior_callback<0>().set(FUNC(f82c836a_device::dma1_ior0_r));
	m_dma[0]->in_ior_callback<1>().set(FUNC(f82c836a_device::dma1_ior1_r));
	m_dma[0]->in_ior_callback<2>().set(FUNC(f82c836a_device::dma1_ior2_r));
	m_dma[0]->in_ior_callback<3>().set(FUNC(f82c836a_device::dma1_ior3_r));
	m_dma[0]->out_iow_callback<0>().set(FUNC(f82c836a_device::dma1_iow0_w));
	m_dma[0]->out_iow_callback<1>().set(FUNC(f82c836a_device::dma1_iow1_w));
	m_dma[0]->out_iow_callback<2>().set(FUNC(f82c836a_device::dma1_iow2_w));
	m_dma[0]->out_iow_callback<3>().set(FUNC(f82c836a_device::dma1_iow3_w));
	m_dma[0]->out_dack_callback<0>().set(FUNC(f82c836a_device::dma1_dack0_w));
	m_dma[0]->out_dack_callback<1>().set(FUNC(f82c836a_device::dma1_dack1_w));
	m_dma[0]->out_dack_callback<2>().set(FUNC(f82c836a_device::dma1_dack2_w));
	m_dma[0]->out_dack_callback<3>().set(FUNC(f82c836a_device::dma1_dack3_w));

	AM9517A(config, m_dma[1], 0);
	m_dma[1]->out_hreq_callback().set(FUNC(f82c836a_device::dma2_hreq_w));
	m_dma[1]->in_memr_callback().set(FUNC(f82c836a_device::dma_read_word));
	m_dma[1]->out_memw_callback().set(FUNC(f82c836a_device::dma_write_word));
	m_dma[1]->in_ior_callback<1>().set(FUNC(f82c836a_device::dma2_ior1_r));
	m_dma[1]->in_ior_callback<2>().set(FUNC(f82c836a_device::dma2_ior2_r));
	m_dma[1]->in_ior_callback<3>().set(FUNC(f82c836a_device::dma2_ior3_r));
	m_dma[1]->out_iow_callback<1>().set(FUNC(f82c836a_device::dma2_iow1_w));
	m_dma[1]->out_iow_callback<2>().set(FUNC(f82c836a_device::dma2_iow2_w));
	m_dma[1]->out_iow_callback<3>().set(FUNC(f82c836a_device::dma2_iow3_w));
	m_dma[1]->out_dack_callback<0>().set(FUNC(f82c836a_device::dma2_dack0_w));
	m_dma[1]->out_dack_callback<1>().set(FUNC(f82c836a_device::dma2_dack1_w));
	m_dma[1]->out_dack_callback<2>().set(FUNC(f82c836a_device::dma2_dack2_w));
	m_dma[1]->out_dack_callback<3>().set(FUNC(f82c836a_device::dma2_dack3_w));

	PIC8259(config, m_intc[0], 0);
	m_intc[0]->out_int_callback().set([this] (int state) { m_write_intr(state); });
	m_intc[0]->in_sp_callback().set_constant(1);
	m_intc[0]->read_slave_ack_callback().set([this] (offs_t offset) -> u8 {
		if (offset == 2)
			return m_intc[1]->acknowledge();
		return 0;
	});

	PIC8259(config, m_intc[1], 0);
	m_intc[1]->out_int_callback().set(m_intc[0], FUNC(pic8259_device::ir2_w));
	m_intc[1]->in_sp_callback().set_constant(0);

	PIT8254(config, m_pit, 0);
	m_pit->set_clk<0>(XTAL(14'318'181) / 12.0);
	m_pit->out_handler<0>().set(m_intc[0], FUNC(pic8259_device::ir0_w));
	m_pit->set_clk<1>(XTAL(14'318'181) / 12.0);
	m_pit->out_handler<1>().set([this] (int state) {
		m_refresh_toggle ^= state;
		m_portb = (m_portb & 0xef) | (m_refresh_toggle << 4);
	});
	m_pit->set_clk<2>(XTAL(14'318'181) / 12.0);
	m_pit->out_handler<2>().set([this] (int state) {
		m_write_spkr(!(state & BIT(m_portb, 1)));
		m_portb = (m_portb & 0xdf) | (state << 5);
	});

	// "MC146818 compatible", but plonking that won't save CMOS in mb1320 at very least
	DS12885(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(m_intc[1], FUNC(pic8259_device::ir0_w));
	m_rtc->set_century_index(0x32);
}

device_memory_interface::space_config_vector f82c836a_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void f82c836a_device::device_start()
{
	if (!m_ram_dev->started())
		throw device_missing_dependencies();

	m_space_mem = &m_cpu->memory().space(AS_PROGRAM);
	m_space_io = &m_cpu->memory().space(AS_IO);

	m_ram = m_ram_dev->pointer();
	u32 ram_size = m_ram_dev->size();

	// install base memory
	m_space_mem->install_ram(0x000000, 0x09ffff, m_ram);
	if (ram_size > 0xa0000)
		m_space_mem->install_ram(0x100000, 0x100000 + ram_size - 0xa0000 - 1, m_ram + 0xa0000);

	m_space_io->install_device(0x0000, 0x03ff, *this, &f82c836a_device::io_map);

	save_item(NAME(m_portb));
	save_item(NAME(m_refresh_toggle));
	save_item(NAME(m_iochck));
	save_item(NAME(m_nmi_mask));

	save_item(NAME(m_dma_eop));
	save_item(NAME(m_dma_page));
	save_item(NAME(m_dma_high_byte));
	save_item(NAME(m_dma_channel));

	save_item(NAME(m_config_address));

	save_item(NAME(m_cpureset));
	save_item(NAME(m_kbrst));
	save_item(NAME(m_ext_gatea20));
	save_item(NAME(m_fast_gatea20));
//  save_item(NAME(m_emu_gatea20));
//  save_item(NAME(m_keybc_d1_written));
//  save_item(NAME(m_keybc_data_blocked));

	save_item(NAME(m_rom_enable));
	save_item(NAME(m_ram_write_protect));
	save_item(NAME(m_shadow_reg));
	save_item(NAME(m_dram_config));
	save_item(NAME(m_chan_env));
	save_item(NAME(m_ems_control));
	save_item(NAME(m_ext_boundary));

	// assumed being internal to the chipset
	m_shadow_ram.resize(0x60000);
	save_item(NAME(m_shadow_ram));
}

void f82c836a_device::device_reset()
{
	m_cpureset = 0;
	m_ext_gatea20 = 0;
	m_fast_gatea20 = 0;
	m_dma_channel = -1;

	m_kbrst = 1;

	m_dma_ws_control = 0;
	// BUSCLK CXIN/5, Refresh Width 280ns
	m_chan_env = 4 | 2;
	m_rom_enable = 0xc0;
	m_ram_write_protect = 0;
	m_shadow_reg[0] = m_shadow_reg[1] = m_shadow_reg[2] = 0;
	m_refresh_toggle = 0;
	m_portb = 0;

	m_dram_config = 1;

	// map just a base subset, ISA bus slots may not be ready for update_romram_settings
	// to work properly. Chipset is supposed to remap at POST anyway.
	m_space_mem->install_rom(0xf0000, 0xfffff, &m_bios[0x30000 / 2]);
}

void f82c836a_device::device_reset_after_children()
{
	// timer 2 default state
	m_pit->write_gate2(1);
}

void f82c836a_device::io_map(address_map &map)
{
	map(0x0000, 0x000f).rw(m_dma[0], FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x0021).rw(m_intc[0], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	// config address/data
	map(0x0022, 0x0022).lw8(NAME([this] (offs_t offset, u8 data) { m_config_address = data; }));
	map(0x0023, 0x0023).lrw8(
		NAME([this] (offs_t offset) { return space().read_byte(m_config_address); }),
		NAME([this] (offs_t offset, u8 data) { space().write_byte(m_config_address, data); })
	);
	map(0x0040, 0x0043).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0060, 0x0060).rw(m_keybc, FUNC(at_kbc_device_base::data_r), FUNC(at_kbc_device_base::data_w));
	map(0x0061, 0x0061).rw(FUNC(f82c836a_device::portb_r), FUNC(f82c836a_device::portb_w));
	map(0x0064, 0x0064).rw(m_keybc, FUNC(at_kbc_device_base::status_r), FUNC(at_kbc_device_base::command_w));
	map(0x0070, 0x0070).lw8(NAME([this] (u8 data) {
		m_nmi_mask = !BIT(data, 7);
		data &= 0x7f;

		m_rtc->address_w(data);
	}));
	map(0x0071, 0x0071).rw(m_rtc, FUNC(ds12885_device::data_r), FUNC(ds12885_device::data_w));
	map(0x0080, 0x008f).lrw8(
		NAME([this] (offs_t offset) { return m_dma_page[offset]; }),
		NAME([this] (offs_t offset, u8 data) { m_dma_page[offset] = data; })
	);
	// system control, identical to cs4031
	map(0x0092, 0x0092).lrw8(
		NAME([this] (offs_t offset) {
			u8 result = 0; // reserved bits read as 0?
			result |= m_cpureset << 0;
			result |= m_fast_gatea20 << 1;
			return result;
		}),
		NAME([this] (offs_t offset, u8 data) {
			fast_gatea20(BIT(data, 1));

			if (m_cpureset == 0 && BIT(data, 0))
			{
				// pulse reset line
				m_write_cpureset(1);
				m_write_cpureset(0);
			}

			m_cpureset = BIT(data, 0);
		})
	);
	map(0x00a0, 0x00a1).rw(m_intc[1], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x00c0, 0x00df).lrw8(
		NAME([this] (offs_t offset) { return m_dma[1]->read(offset >> 1); }),
		NAME([this] (offs_t offset, u8 data) { m_dma[1]->write(offset >> 1, data); })
	);
//  map(0x0208, 0x020a) PnP EMS paging
//  map(0x0218, 0x021a) alt of above (depends on a register setting)
}

u8 f82c836a_device::portb_r()
{
	return m_portb;
}

void f82c836a_device::portb_w(u8 data)
{
	m_portb = (m_portb & 0xf0) | (data & 0x0f);

	// bit 5 forced to 1 if timer disabled
	if (!BIT(m_portb, 0))
		m_portb |= 1 << 5;

	m_pit->write_gate2(BIT(m_portb, 0));

	m_write_spkr(!BIT(m_portb, 1));

	// clear channel check latch?
	if (BIT(m_portb, 3))
		m_portb &= 0xbf;
}

/******************
 *
 * Config
 *
 *****************/

void f82c836a_device::config_map(address_map &map)
{
	// DMA Wait-State Control
	map(0x01, 0x01).lrw8(
		NAME([this] (offs_t offset) {
			return m_dma_ws_control;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_dma_ws_control = data;
			update_dma_clock();
		})
	);
	// version (r/o)
	// xxxx ---- family type 0001
	// ---- xxxx revision
	// ---- 0011 82C836A
	// ---- 0100 82C836B
	map(0x40, 0x40).lr8(NAME([] (offs_t offset) { return 0x13; }));
	// Channel Environment
	map(0x41, 0x41).lrw8(
		NAME([this] (offs_t offset) { return m_chan_env; }),
		NAME([this] (offs_t offset, u8 data) { m_chan_env = data; update_dma_clock(); })
	);
//  map(0x44, 0x44) Peripheral Control
	// Misc. Status
	map(0x45, 0x45).lr8(
		NAME([this] (offs_t offset) { return (m_nmi_mask << 7) | (m_ext_gatea20 << 6); })
	);
//  map(0x46, 0x46) Power Management
	// ROM enable
	map(0x48, 0x48).lrw8(
		NAME([this] (offs_t offset) { return m_rom_enable; }),
		NAME([this] (offs_t offset, u8 data) { m_rom_enable = data; update_romram_settings(); })
	);
	// RAM Write Protect
	map(0x49, 0x49).lrw8(
		NAME([this] (offs_t offset) { return m_ram_write_protect; }),
		NAME([this] (offs_t offset, u8 data) { m_ram_write_protect = data; update_romram_settings(); })
	);
	// Shadow RAM #1/#2/#3
	map(0x4a, 0x4c).lrw8(
		NAME([this] (offs_t offset) { return m_shadow_reg[offset]; }),
		NAME([this] (offs_t offset, u8 data) { m_shadow_reg[offset] = data; update_romram_settings(); })
	);
	// DRAM Configuration
	map(0x4d, 0x4d).lrw8(
		NAME([this] (offs_t offset) { return m_dram_config; }),
		NAME([this] (offs_t offset, u8 data) { m_dram_config = data; })
	);
	// Extended Boundary
	map(0x4e, 0x4e).lrw8(
		NAME([this] (offs_t offset) { return m_ext_boundary; }),
		NAME([this] (offs_t offset, u8 data) { m_ext_boundary = data; })
	);
	// EMS control
	map(0x4f, 0x4f).lrw8(
		NAME([this] (offs_t offset) { return m_ems_control; }),
		NAME([this] (offs_t offset, u8 data) { m_ems_control = data; })
	);
//  map(0x60, 0x60) Laptop Features
//  map(0x61, 0x61) Fast Video Control
//  map(0x62, 0x62) Fast Video RAM Enable
//  map(0x63, 0x63) High Performance Refresh
//  map(0x64, 0x64) CAS Timing for DMA/Master
}

void f82c836a_device::update_romram_settings()
{
//  printf("%02x %02x %02x %02x %02x\n", m_rom_enable, m_ram_write_protect, m_shadow_reg[0], m_shadow_reg[1], m_shadow_reg[2]);

	// reconfigure space
	// Despite what documentation claims shadow RAM actually wins over ROM, it will just map on
	// top of it (i.e. IBM VGA BIOS will get shadowed while leaving the ROM bit on)
	m_space_mem->unmap_readwrite(0xa0000, 0xfffff);
	m_isabus->remap(AS_PROGRAM, 0xa0000, 0xfffff);

	// TODO: optimize, add write only paths
	for (int i = 0; i < 8; i++)
	{
		const u32 start_offs = 0xc0000 + i * 0x8000;
		const u32 end_offs = start_offs + 0x7fff;

		if (BIT(m_rom_enable, i))
			m_space_mem->install_rom(start_offs, end_offs, &m_bios[(i * 0x8000) / 2]);
	}

	for (int i = 0; i < 8; i++)
	{
		const u32 start_offs = 0xa0000 + i * 0x4000;
		const u32 end_offs = start_offs + 0x3fff;

		if (BIT(m_shadow_reg[0], i))
			m_space_mem->install_ram(start_offs, end_offs, &m_shadow_ram[i * 0x4000]);
	}

	for (int i = 0; i < 8; i++)
	{
		const u32 start_offs = 0xc0000 + i * 0x4000;
		const u32 end_offs = start_offs + 0x3fff;

		if (BIT(m_shadow_reg[1], i))
			m_space_mem->install_ram(start_offs, end_offs, &m_shadow_ram[0x20000 + i * 0x4000]);
	}

	for (int i = 0; i < 8; i++)
	{
		const u32 start_offs = 0xe0000 + i * 0x4000;
		const u32 end_offs = start_offs + 0x3fff;

		if (BIT(m_shadow_reg[2], i))
			m_space_mem->install_ram(start_offs, end_offs, &m_shadow_ram[0x40000 + i * 0x4000]);
	}

}

void f82c836a_device::update_dma_clock()
{
	// similar concept as the one in cs4031 except smaller set
	const int busclk_sel_settings[] = { 4, 5, 6, 0 };
	const int busclk_sel = busclk_sel_settings[(m_chan_env >> 2) & 3];

	if (busclk_sel == 0)
		return;

	const int dma_clock_sel = BIT(m_dma_ws_control, 0);

	u32 dma_clock = clock() / busclk_sel;

	if (!dma_clock_sel)
		dma_clock /= 2;

	logerror("update_dma_clock: dma clock is now %u (%d %d)\n", dma_clock, busclk_sel, dma_clock_sel);

	m_dma[0]->set_unscaled_clock(dma_clock);
	m_dma[1]->set_unscaled_clock(dma_clock);
}

/******************
 *
 * DMA Controller
 *
 *****************/

offs_t f82c836a_device::page_offset()
{
	switch (m_dma_channel)
	{
		case 0: return (offs_t) m_dma_page[0x07] << 16;
		case 1: return (offs_t) m_dma_page[0x03] << 16;
		case 2: return (offs_t) m_dma_page[0x01] << 16;
		case 3: return (offs_t) m_dma_page[0x02] << 16;
		case 5: return (offs_t) m_dma_page[0x0b] << 16;
		case 6: return (offs_t) m_dma_page[0x09] << 16;
		case 7: return (offs_t) m_dma_page[0x0a] << 16;
	}

	// should never get here
	return 0xff0000;
}

u8 f82c836a_device::dma_read_byte(offs_t offset)
{
	if (m_dma_channel == -1)
		return 0xff;

	return m_space_mem->read_byte(page_offset() + offset);
}

void f82c836a_device::dma_write_byte(offs_t offset, u8 data)
{
	if (m_dma_channel == -1)
		return;

	m_space_mem->write_byte(page_offset() + offset, data);
}

u8 f82c836a_device::dma_read_word(offs_t offset)
{
	if (m_dma_channel == -1)
		return 0xff;

	u16 result = m_space_mem->read_word((page_offset() & 0xfe0000) | (offset << 1));
	m_dma_high_byte = result >> 8;

	return result;
}

void f82c836a_device::dma_write_word(offs_t offset, u8 data)
{
	if (m_dma_channel == -1)
		return;

	m_space_mem->write_word((page_offset() & 0xfe0000) | (offset << 1), (m_dma_high_byte << 8) | data);
}

void f82c836a_device::dma2_dack0_w(int state)
{
	m_dma[0]->hack_w(state ? 0 : 1); // inverted?
}

void f82c836a_device::dma1_eop_w(int state)
{
	m_dma_eop = state;
	if (m_dma_channel != -1)
		m_write_tc(m_dma_channel, state, 0xff);
}

void f82c836a_device::set_dma_channel(int channel, bool state)
{
	//m_write_dack(channel, state);

	if (!state)
	{
		m_dma_channel = channel;
		if (m_dma_eop)
			m_write_tc(channel, 1, 0xff);
	}
	else
	{
		if (m_dma_channel == channel)
		{
			m_dma_channel = -1;
			if (m_dma_eop)
				m_write_tc(channel, 0, 0xff);
		}
	}
}
