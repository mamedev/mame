// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

VLSI VL82C420 Scamp IV System Controller chipset

TODO:
- No documentation available, have one for VL82C481. The basics seems to match usage in ptpc110;
- In turn: is the UMC UM82C481 directly deriving from that?

**************************************************************************************************/

#include "emu.h"
#include "vl82c420.h"

DEFINE_DEVICE_TYPE(VL82C420, vl82c420_device,   "vl82c420",   "VLSI VL82C420 \"Scamp IV\" System Controller")

vl82c420_device::vl82c420_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VL82C420, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config("config_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(vl82c420_device::config_map), this))
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

void vl82c420_device::device_add_mconfig(machine_config &config)
{
	AM9517A(config, m_dma[0], 0);
	m_dma[0]->out_hreq_callback().set(m_dma[1], FUNC(am9517a_device::dreq0_w));
	m_dma[0]->out_eop_callback().set(FUNC(vl82c420_device::dma1_eop_w));
	m_dma[0]->in_memr_callback().set(FUNC(vl82c420_device::dma_read_byte));
	m_dma[0]->out_memw_callback().set(FUNC(vl82c420_device::dma_write_byte));
	m_dma[0]->in_ior_callback<0>().set(FUNC(vl82c420_device::dma1_ior0_r));
	m_dma[0]->in_ior_callback<1>().set(FUNC(vl82c420_device::dma1_ior1_r));
	m_dma[0]->in_ior_callback<2>().set(FUNC(vl82c420_device::dma1_ior2_r));
	m_dma[0]->in_ior_callback<3>().set(FUNC(vl82c420_device::dma1_ior3_r));
	m_dma[0]->out_iow_callback<0>().set(FUNC(vl82c420_device::dma1_iow0_w));
	m_dma[0]->out_iow_callback<1>().set(FUNC(vl82c420_device::dma1_iow1_w));
	m_dma[0]->out_iow_callback<2>().set(FUNC(vl82c420_device::dma1_iow2_w));
	m_dma[0]->out_iow_callback<3>().set(FUNC(vl82c420_device::dma1_iow3_w));
	m_dma[0]->out_dack_callback<0>().set(FUNC(vl82c420_device::dma1_dack0_w));
	m_dma[0]->out_dack_callback<1>().set(FUNC(vl82c420_device::dma1_dack1_w));
	m_dma[0]->out_dack_callback<2>().set(FUNC(vl82c420_device::dma1_dack2_w));
	m_dma[0]->out_dack_callback<3>().set(FUNC(vl82c420_device::dma1_dack3_w));

	AM9517A(config, m_dma[1], 0);
	m_dma[1]->out_hreq_callback().set(FUNC(vl82c420_device::dma2_hreq_w));
	m_dma[1]->in_memr_callback().set(FUNC(vl82c420_device::dma_read_word));
	m_dma[1]->out_memw_callback().set(FUNC(vl82c420_device::dma_write_word));
	m_dma[1]->in_ior_callback<1>().set(FUNC(vl82c420_device::dma2_ior1_r));
	m_dma[1]->in_ior_callback<2>().set(FUNC(vl82c420_device::dma2_ior2_r));
	m_dma[1]->in_ior_callback<3>().set(FUNC(vl82c420_device::dma2_ior3_r));
	m_dma[1]->out_iow_callback<1>().set(FUNC(vl82c420_device::dma2_iow1_w));
	m_dma[1]->out_iow_callback<2>().set(FUNC(vl82c420_device::dma2_iow2_w));
	m_dma[1]->out_iow_callback<3>().set(FUNC(vl82c420_device::dma2_iow3_w));
	m_dma[1]->out_dack_callback<0>().set(FUNC(vl82c420_device::dma2_dack0_w));
	m_dma[1]->out_dack_callback<1>().set(FUNC(vl82c420_device::dma2_dack1_w));
	m_dma[1]->out_dack_callback<2>().set(FUNC(vl82c420_device::dma2_dack2_w));
	m_dma[1]->out_dack_callback<3>().set(FUNC(vl82c420_device::dma2_dack3_w));

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

	// TODO: most likely wrong type
	DS12885(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(m_intc[1], FUNC(pic8259_device::ir0_w));
	m_rtc->set_century_index(0x32);
}

device_memory_interface::space_config_vector vl82c420_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void vl82c420_device::device_start()
{
	if (!m_ram_dev->started())
		throw device_missing_dependencies();

	m_space_mem = &m_cpu->memory().space(AS_PROGRAM);
	m_space_io = &m_cpu->memory().space(AS_IO);

	m_ram = m_ram_dev->pointer();
	u32 ram_size = m_ram_dev->size();

	// install base memory
	m_space_mem->install_ram(0x0000'0000, 0x0009'ffff, m_ram);
	if (ram_size > 0x10'0000)
		m_space_mem->install_ram(0x0010'0000, 0x0010'0000 + ram_size - 0x10'0000 - 1, m_ram + 0x0010'0000);

	m_space_io->install_device(0x0000, 0x03ff, *this, &vl82c420_device::io_map);

	save_item(NAME(m_portb));
	save_item(NAME(m_refresh_toggle));
	save_item(NAME(m_iochck));
	save_item(NAME(m_nmi_mask));

	save_item(NAME(m_dma_eop));
	save_item(NAME(m_dma_page));
	save_item(NAME(m_dma_high_byte));
	save_item(NAME(m_dma_channel));

	save_item(NAME(m_config_address));
	save_item(NAME(m_config_unlock));
	save_item(NAME(m_ramtmg));
	save_item(NAME(m_ramcfg));
	save_item(NAME(m_ramset));
	save_item(NAME(m_ntbref));
	save_item(NAME(m_clkctl));
	save_item(NAME(m_miscset));
	save_item(NAME(m_dmactl));
	save_item(NAME(m_busctl));
	save_item(NAME(m_fbcr));
	save_item(NAME(m_romset));
	save_item(NAME(m_segment_access));
	save_item(NAME(m_segment_cache));
	save_item(NAME(m_pmra));
	save_item(NAME(m_pmre));

	save_item(NAME(m_cpureset));
	save_item(NAME(m_kbrst));
	save_item(NAME(m_ext_gatea20));
	save_item(NAME(m_fast_gatea20));
}

void vl82c420_device::device_reset()
{
	m_cpureset = 0;
	m_ext_gatea20 = 0;
	m_fast_gatea20 = 0;
	m_dma_channel = -1;

	m_kbrst = 1;

	m_config_unlock = false;

	m_ramtmg = 0xff;
	m_ramcfg[0] = 0x8a;
	m_ramcfg[1] = 0x88;
	// Notes refers to pin states
	// bit 3 (HITM#, r/o), 2 (CCSB#) & 1 (CCSA#)
	m_ramset = 0;
	// bit 4 depends on TURBO pin (r/o)
	m_ntbref = 0;
	// bit 2 (BUSCLK) depends on BUSOSC pin connection (if provided)
	m_clkctl = 0x1b;
	// bit 6 (TAG8)
	m_miscset = 0;
	m_dmactl = 0x38;
	// bit 5 (IRQSH/LD#)
	m_busctl = 0;
	// not provided, assume zero
	m_fbcr = 0;
	// bit 7 (ICA3BA2) and bit 6 (RAMW#)
	m_romset = 0x00;
	std::fill_n(m_segment_access, std::size(m_segment_access), 0);
	std::fill_n(m_segment_cache, std::size(m_segment_cache), 0);
	std::fill_n(m_pmra, std::size(m_pmra), 0);
	std::fill_n(m_pmre, std::size(m_pmre), 0);
	// bit 6 (!MA0X)
	m_xctl = 0;

	m_space_mem->install_rom(0xc0000, 0xfffff, &m_bios[0x00000 / 4]);

	// TODO: temp, DMA clock dynamically provided
	m_dma[0]->set_unscaled_clock(2'500'000);
	m_dma[1]->set_unscaled_clock(2'500'000);
}

void vl82c420_device::device_reset_after_children()
{
	// timer 2 default state
	m_pit->write_gate2(1);
}

void vl82c420_device::io_map(address_map &map)
{
	map(0x0000, 0x000f).rw(m_dma[0], FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x0021).rw(m_intc[0], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x0043).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0060, 0x0060).rw(m_keybc, FUNC(at_kbc_device_base::data_r), FUNC(at_kbc_device_base::data_w));
	map(0x0061, 0x0061).rw(FUNC(vl82c420_device::portb_r), FUNC(vl82c420_device::portb_w));
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
	// system control
	map(0x0092, 0x0092).lrw8(
		NAME([this] (offs_t offset) {
			u8 result = 0;
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
	map(0x00ec, 0x00ec).w(FUNC(vl82c420_device::config_address_w));
	map(0x00ed, 0x00ed).rw(FUNC(vl82c420_device::config_data_r), FUNC(vl82c420_device::config_data_w));
//  map(0x00ee, 0x00ee) dummy read enable Fast A20, dummy write disables it
//  map(0x00ef, 0x00ef) dummy read reset CPU
//  map(0x00f0, 0x00f0) Coprocessor Busy
//  map(0x00f1, 0x00f1) Coprocessor Reset
//  map(0x00f4, 0x00f4) Slow CPU
//  map(0x00f5, 0x00f5) Fast CPU
	// dummy writes to $f9 / $fb disables/enables config access
	map(0x00f9, 0x00f9).lw8(NAME([this] (u8 data) { (void)data; m_config_unlock = false; }));
	map(0x00fb, 0x00fb).lw8(NAME([this] (u8 data) { (void)data; m_config_unlock = true; }));
}

u8 vl82c420_device::portb_r()
{
	return m_portb;
}

void vl82c420_device::portb_w(u8 data)
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

void vl82c420_device::config_address_w(offs_t offset, u8 data)
{
	m_config_address = data;
}

u8 vl82c420_device::config_data_r(offs_t offset)
{
	if (!m_config_unlock)
	{
		if (!machine().side_effects_disabled())
			logerror("Attempt to read config [%02x] while locked\n", offset);
		return 0xff;
	}
	return space(0).read_byte(m_config_address);
}

void vl82c420_device::config_data_w(offs_t offset, u8 data)
{
	if (!m_config_unlock)
	{
		if (!machine().side_effects_disabled())
			logerror("Attempt to write config [%02x] %02x while locked\n", offset, data);
		return;
	}

	space(0).write_byte(m_config_address, data);
}

void vl82c420_device::config_map(address_map &map)
{
//  map(0x00, 0x00) Version, 0x90 for VL82C481
	map(0x01, 0x01).lrw8(
		NAME([this] (offs_t offset) { return m_ramtmg; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Config 01h: DRAM Programmable Timing %02x\n", data);
			m_ramtmg = data;
		})
	);
	map(0x02, 0x03).lrw8(
		NAME([this] (offs_t offset) { return m_ramcfg[offset]; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Config %02xh: DRAM Configuration %d %02x\n", offset + 2, offset, data);
			m_ramcfg[offset] = data;
		})
	);
	map(0x04, 0x04).lrw8(
		NAME([this] (offs_t offset) { return m_ramset; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Config 04h: DRAM Control %02x\n", data);
			m_ramset = data;
		})
	);
	map(0x05, 0x05).lrw8(
		NAME([this] (offs_t offset) { return m_ntbref; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Config 05h: Non-Turbo and Refresh Control %02x\n", data);
			m_ntbref = data;
		})
	);
	map(0x06, 0x06).lrw8(
		NAME([this] (offs_t offset) { return m_clkctl; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Config 06h: Clock Control %02x\n", data);
			m_clkctl = data;
		})
	);
	map(0x07, 0x07).lrw8(
		NAME([this] (offs_t offset) { return m_miscset; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Config 07h: Miscellaneous Control %02x\n", data);
			m_miscset = data;
		})
	);
	map(0x08, 0x08).lrw8(
		NAME([this] (offs_t offset) { return m_dmactl; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Config 08h: DMA Control %02x\n", data);
			m_dmactl = data;
		})
	);
	map(0x09, 0x09).lrw8(
		NAME([this] (offs_t offset) { return m_busctl; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Config 08h: Bus Control %02x\n", data);
			m_busctl = data;
		})
	);
//  map(0x0a, 0x0a) <unknown>
	map(0x0b, 0x0b).lrw8(
		NAME([this] (offs_t offset) { return m_fbcr; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Config 0Bh: Fast Bus Clock Region %06x\n", data << 16);
			m_fbcr = data;
		})
	);
	map(0x0c, 0x0c).lrw8(
		NAME([this] (offs_t offset) { return m_romset; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Config 0Ch: ROM Control %02x\n", data);
			m_romset = data;
			update_segment_settings();
		})
	);
	map(0x0d, 0x12).lrw8(
		NAME([this] (offs_t offset) { return m_segment_access[offset]; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Config %02Xh: Segment Access Control $%05x %02x\n", offset + 0xd, (offset * 0x10000) + 0xa0000, data);
			m_segment_access[offset] = data;
			update_segment_settings();
		})
	);
	map(0x13, 0x18).lrw8(
		NAME([this] (offs_t offset) { return m_segment_cache[offset]; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Config %02Xh: Segment Cache $%05x %02x\n", offset + 0x13, (offset * 0x10000) + 0xa0000, data);
			m_segment_cache[offset] = data;
		})
	);
	map(0x19, 0x19).lrw8(
		NAME([this] (offs_t offset) { return m_cachctl; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Config 19h: Cache Controller Setup %02x\n", data);
			m_cachctl = data;
		})
	);

//  map(0x1c, 0x1c) <unknown>
//  map(0x1c, 0x1d) <unknown>

	map(0x20, 0x20).select(2).lrw8(
		NAME([this] (offs_t offset) { return m_pmra[offset]; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Config %02Xh: Programmed Memory Region Address %d %02x\n", offset + 0x20, offset + 1, data);
			m_pmra[offset] = data;
		})
	);
	map(0x21, 0x21).select(2).lrw8(
		NAME([this] (offs_t offset) { return m_pmre[offset]; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Config %02Xh: Programmed Memory Region Enable %d %02x\n", offset + 0x20, offset + 1, data);
			m_pmre[offset] = data;
		})
	);
	map(0x24, 0x24).lrw8(
		NAME([this] (offs_t offset) { return m_xctl; }),
		NAME([this] (offs_t offset, u8 data) {
			logerror("Config 24h: Extension Control %02x\n", data);
			m_xctl = data;
		})
	);

//  map(0x37, 0x37) <unknown>
}

void vl82c420_device::update_segment_settings()
{
	m_space_mem->unmap_readwrite(0xa0000, 0xfffff);
	m_isabus->remap(AS_PROGRAM, 0xa0000, 0xfffff);

	m_space_mem->install_rom(0xe0000, 0xfffff, &m_bios[0x20000 / 4]);
	// TODO: attempts to read from here, the existing bank fails several string comparisons
	// (and checksum)
//	m_space_mem->install_rom(0xd0000, 0xdffff, &m_bios[0x00000 / 4]);

	// VGA BIOS is copied from $e0000 to $c0000
	// TODO: shadow RAM is disabled, what's really holding this data?
	m_space_mem->install_ram(0xc0000, 0xcffff, m_ram + 0xc0000);

	for (int reg = 0; reg < 6; reg++)
	{
		const u32 seg_base = 0xa0000 + reg * 0x10000;
		for (int i = 0; i < 4; i++)
		{
			const u32 start_offs = seg_base + i * 0x4000;
			const u32 end_offs = start_offs + 0x3fff;

			switch((m_segment_access[reg] >> (i * 2)) & 3)
			{
				// r/w slot bus
				case 0:
					break;
				// read slot, write system board
				case 1:
					m_space_mem->install_writeonly(start_offs, end_offs, m_ram + start_offs);
					break;
				// read system board, write slot bus
				case 2:
					m_space_mem->install_rom(start_offs, end_offs, m_ram + start_offs);
					break;
				// r/w system board
				case 3:
					m_space_mem->install_ram(start_offs, end_offs, m_ram + start_offs);
					break;
			}
		}
	}
}

/******************
 *
 * DMA Controller
 *
 *****************/

offs_t vl82c420_device::page_offset()
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

u8 vl82c420_device::dma_read_byte(offs_t offset)
{
	if (m_dma_channel == -1)
		return 0xff;

	return m_space_mem->read_byte(page_offset() + offset);
}

void vl82c420_device::dma_write_byte(offs_t offset, u8 data)
{
	if (m_dma_channel == -1)
		return;

	m_space_mem->write_byte(page_offset() + offset, data);
}

u8 vl82c420_device::dma_read_word(offs_t offset)
{
	if (m_dma_channel == -1)
		return 0xff;

	u16 result = m_space_mem->read_word((page_offset() & 0xfe0000) | (offset << 1));
	m_dma_high_byte = result >> 8;

	return result;
}

void vl82c420_device::dma_write_word(offs_t offset, u8 data)
{
	if (m_dma_channel == -1)
		return;

	m_space_mem->write_word((page_offset() & 0xfe0000) | (offset << 1), (m_dma_high_byte << 8) | data);
}

void vl82c420_device::dma2_dack0_w(int state)
{
	m_dma[0]->hack_w(state ? 0 : 1); // inverted?
}

void vl82c420_device::dma1_eop_w(int state)
{
	m_dma_eop = state;
	if (m_dma_channel != -1)
		m_write_tc(m_dma_channel, state, 0xff);
}

void vl82c420_device::set_dma_channel(int channel, bool state)
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
