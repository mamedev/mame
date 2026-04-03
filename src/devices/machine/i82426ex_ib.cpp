// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#include "emu.h"
#include "i82426ex_ib.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

DEFINE_DEVICE_TYPE(I82426EX_IB, i82426ex_ib_device, "i82426ex_ib", "Intel 82425EX ISA Bridge")

i82426ex_ib_device::i82426ex_ib_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, I82426EX_IB, tag, owner, clock)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
	, m_keybc(*this, finder_base::DUMMY_TAG)
	, m_dma(*this, "dma%u", 1U)
	, m_intc(*this, "intc%u", 1U)
	, m_pit(*this, "pit")
	, m_isabus(*this, "isabus")
	, m_write_intr(*this)
	, m_write_spkr(*this)
	, m_write_cpurst(*this)
	, m_rtcale(*this)
	, m_rtccs_read(*this, 0xff)
	, m_rtccs_write(*this)
{
}

void i82426ex_ib_device::device_add_mconfig(machine_config &config)
{
	AM9517A(config, m_dma[0], this->clock() / 3);
	m_dma[0]->out_hreq_callback().set(m_dma[1], FUNC(am9517a_device::dreq0_w));
	m_dma[0]->out_eop_callback().set(FUNC(i82426ex_ib_device::dma1_eop_w));
	m_dma[0]->in_memr_callback().set(FUNC(i82426ex_ib_device::dma_read_byte));
	m_dma[0]->out_memw_callback().set(FUNC(i82426ex_ib_device::dma_write_byte));
	m_dma[0]->in_ior_callback<0>().set([this] () { return m_isabus->dack_r(0); });
	m_dma[0]->in_ior_callback<1>().set([this] () { return m_isabus->dack_r(1); });
	m_dma[0]->in_ior_callback<2>().set([this] () { return m_isabus->dack_r(2); });
	m_dma[0]->in_ior_callback<3>().set([this] () { return m_isabus->dack_r(3); });
	m_dma[0]->out_iow_callback<0>().set([this] (u8 data) { m_isabus->dack_w(0, data); });
	m_dma[0]->out_iow_callback<1>().set([this] (u8 data) { m_isabus->dack_w(1, data); });
	m_dma[0]->out_iow_callback<2>().set([this] (u8 data) { m_isabus->dack_w(2, data); });
	m_dma[0]->out_iow_callback<3>().set([this] (u8 data) { m_isabus->dack_w(3, data); });
	m_dma[0]->out_dack_callback<0>().set([this] (int state) { set_dma_channel(0, state); });
	m_dma[0]->out_dack_callback<1>().set([this] (int state) { set_dma_channel(1, state); });
	m_dma[0]->out_dack_callback<2>().set([this] (int state) { set_dma_channel(2, state); });
	m_dma[0]->out_dack_callback<3>().set([this] (int state) { set_dma_channel(3, state); });

	AM9517A(config, m_dma[1], this->clock() / 3);
	m_dma[1]->out_hreq_callback().set(FUNC(i82426ex_ib_device::dma2_hreq_w));
	m_dma[1]->in_memr_callback().set(FUNC(i82426ex_ib_device::dma_read_word));
	m_dma[1]->out_memw_callback().set(FUNC(i82426ex_ib_device::dma_write_word));
	m_dma[1]->in_ior_callback<1>().set([this] () { return m_isabus->dack_r(5); });
	m_dma[1]->in_ior_callback<2>().set([this] () { return m_isabus->dack_r(6); });
	m_dma[1]->in_ior_callback<3>().set([this] () { return m_isabus->dack_r(7); });
	m_dma[1]->out_iow_callback<1>().set([this] (u8 data) { m_isabus->dack_w(5, data); });
	m_dma[1]->out_iow_callback<2>().set([this] (u8 data) { m_isabus->dack_w(6, data); });
	m_dma[1]->out_iow_callback<3>().set([this] (u8 data) { m_isabus->dack_w(7, data); });
	m_dma[1]->out_dack_callback<0>().set([this] (int state) { m_dma[0]->hack_w(state ? 0 : 1); });
	m_dma[1]->out_dack_callback<1>().set([this] (int state) { set_dma_channel(5, state); });
	m_dma[1]->out_dack_callback<2>().set([this] (int state) { set_dma_channel(6, state); });
	m_dma[1]->out_dack_callback<3>().set([this] (int state) { set_dma_channel(7, state); });

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

	// 82C54
	PIT8254(config, m_pit, 0);
	m_pit->set_clk<0>(this->clock() / 12);
	m_pit->out_handler<0>().set(m_intc[0], FUNC(pic8259_device::ir0_w));
	m_pit->set_clk<1>(this->clock() / 12);
	m_pit->out_handler<1>().set([this] (int state) {
		m_refresh_toggle ^= state;
		m_portb = (m_portb & 0xef) | (m_refresh_toggle << 4);
	});
	m_pit->set_clk<2>(this->clock() / 12);
	m_pit->out_handler<2>().set([this] (int state) {
		m_write_spkr((state & BIT(m_portb, 1)));
		m_portb = (m_portb & 0xdf) | (state << 5);
	});

	ISA16(config, m_isabus, 0);
	m_isabus->irq3_callback().set( m_intc[0], FUNC(pic8259_device::ir3_w));
	m_isabus->irq4_callback().set( m_intc[0], FUNC(pic8259_device::ir4_w));
	m_isabus->irq5_callback().set( m_intc[0], FUNC(pic8259_device::ir5_w));
	m_isabus->irq6_callback().set( m_intc[0], FUNC(pic8259_device::ir6_w));
	m_isabus->irq7_callback().set( m_intc[0], FUNC(pic8259_device::ir7_w));
	m_isabus->irq2_callback().set( m_intc[1], FUNC(pic8259_device::ir1_w));
	m_isabus->irq10_callback().set(m_intc[1], FUNC(pic8259_device::ir2_w));
	m_isabus->irq11_callback().set(m_intc[1], FUNC(pic8259_device::ir3_w));
	m_isabus->irq12_callback().set(m_intc[1], FUNC(pic8259_device::ir4_w));
	m_isabus->irq14_callback().set(m_intc[1], FUNC(pic8259_device::ir6_w));
	m_isabus->irq15_callback().set(m_intc[1], FUNC(pic8259_device::ir7_w));
	m_isabus->drq0_callback().set(m_dma[0], FUNC(am9517a_device::dreq0_w));
	m_isabus->drq1_callback().set(m_dma[0], FUNC(am9517a_device::dreq1_w));
	m_isabus->drq2_callback().set(m_dma[0], FUNC(am9517a_device::dreq2_w));
	m_isabus->drq3_callback().set(m_dma[0], FUNC(am9517a_device::dreq3_w));
	m_isabus->drq5_callback().set(m_dma[1], FUNC(am9517a_device::dreq1_w));
	m_isabus->drq6_callback().set(m_dma[1], FUNC(am9517a_device::dreq2_w));
	m_isabus->drq7_callback().set(m_dma[1], FUNC(am9517a_device::dreq3_w));
	m_isabus->iochck_callback().set(FUNC(i82426ex_ib_device::iochck_w));
}

void i82426ex_ib_device::device_start()
{
	save_item(NAME(m_portb));
	save_item(NAME(m_refresh_toggle));
	save_item(NAME(m_iochck));
	save_item(NAME(m_nmi_mask));

	save_item(NAME(m_dma_eop));
	save_item(NAME(m_dma_page));
	save_item(NAME(m_dma_high_byte));
	save_item(NAME(m_dma_channel));
}

void i82426ex_ib_device::device_reset()
{
	m_dma_channel = -1;

	m_nmi_mask = 1;
//  m_dma[0]->set_unscaled_clock(2'500'000);
//  m_dma[1]->set_unscaled_clock(2'500'000);
}

void i82426ex_ib_device::device_reset_after_children()
{
	// timer 2 default state
	m_pit->write_gate2(1);
}

void i82426ex_ib_device::device_config_complete()
{
	auto isabus = m_isabus.finder_target();
	isabus.first.subdevice<isa16_device>(isabus.second)->set_memspace(m_host_cpu, AS_PROGRAM);
	isabus.first.subdevice<isa16_device>(isabus.second)->set_iospace(m_host_cpu, AS_IO);
}

void i82426ex_ib_device::io_map(address_map &map)
{
	map(0x0000, 0x000f).rw(m_dma[0], FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x0021).rw(m_intc[0], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x0043).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0060, 0x0060).rw(m_keybc, FUNC(at_kbc_device_base::data_r), FUNC(at_kbc_device_base::data_w));
	map(0x0061, 0x0061).rw(FUNC(i82426ex_ib_device::portb_r), FUNC(i82426ex_ib_device::portb_w));
	map(0x0064, 0x0064).rw(m_keybc, FUNC(at_kbc_device_base::status_r), FUNC(at_kbc_device_base::command_w));
	map(0x0070, 0x0070).lw8(
		NAME([this] (u8 data) {
			m_nmi_mask = BIT(data, 7);
			m_rtcale(data);
		})
	);
	map(0x0071, 0x0071).lrw8(
		NAME([this] () {
			return m_rtccs_read();
		}),
		NAME([this] (u8 data) {
			m_rtccs_write(data);
		})
	);
	// ???
	map(0x0078, 0x0078).lr8(NAME([] () { return 0; }));

	map(0x0080, 0x008f).lrw8(
		NAME([this] (offs_t offset) { return m_dma_page[offset]; }),
		NAME([this] (offs_t offset, u8 data) { m_dma_page[offset] = data; })
	);
	map(0x00a0, 0x00a1).rw(m_intc[1], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
//  map(0x00b2, 0x00b2) APMC
//  map(0x00b3, 0x00b3) APMS
	map(0x00c0, 0x00df).lrw8(
		NAME([this] (offs_t offset) { return m_dma[1]->read(offset >> 1); }),
		NAME([this] (offs_t offset, u8 data) { m_dma[1]->write(offset >> 1, data); })
	);
//  map(0x00f0, 0x00f0) Coprocessor Error
//  map(0x0480, 0x048f) DMA high page (not all of it)
//  map(0x04d0, 0x04d0) INT-1 Edge/Level Control
//  map(0x04d1, 0x04d1) INT-2 Edge/Level Control
}

u8 i82426ex_ib_device::portb_r()
{
	return m_portb;
}

// x--- ---- SERR# NMI Source Status (r/o)
// -x-- ---- IOCHK# NMI Source Status (r/o)
// --x- ---- Timer Counter 2 Out Status (r/o)
// ---x ---- Refresh Cycle Toggle (r/o)
// ---- x--- IOCHK# NMI Enable
// ---- -x-- PCI SERR# Enable
// ---- --x- Speaker Data Enable
// ---- ---x Timer Counter 2 Enable
void i82426ex_ib_device::portb_w(u8 data)
{
	m_portb = (m_portb & 0xf0) | (data & 0x0f);

	// bit 5 forced to 1 if timer disabled
	if (!BIT(m_portb, 0))
		m_portb |= 1 << 5;

	m_pit->write_gate2(BIT(m_portb, 0));

	// Other way around wrt other chipsets
	if (!BIT(m_portb, 1))
		m_write_spkr(0);

//  m_write_spkr(BIT(m_portb, 1));

	// clear channel check latch?
	if (BIT(m_portb, 3))
	{
		m_portb &= 0xbf;
		m_host_cpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
}

// NOTE: $cf9 actually belongs to IB, cfr. CPURST/PCIRST# definitions at page 34
void i82426ex_ib_device::trc_w(offs_t offset, u8 data)
{
	LOG("CF9h: TRC Turbo/Reset Control %02x\n", data);
	// TODO: bit 1 actually controls if reset type is soft (0) or hard (1)
	// (former just resets the CPU?)
	if (BIT(data, 2))
		m_write_cpurst(1);
	// TODO: bit 0 for deturbo mode, and whatever is <reserved> bit 5 actually used by entrada
}

/*
 * DMA Controller
 */

void i82426ex_ib_device::dma2_hreq_w(int state)
{
	m_host_cpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma[1]->hack_w(state);
}

offs_t i82426ex_ib_device::page_offset()
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


u8 i82426ex_ib_device::dma_read_byte(offs_t offset)
{
	address_space &prog_space = m_host_cpu->space(AS_PROGRAM);
	if (m_dma_channel == -1)
		return 0xff;
	u8 result = prog_space.read_byte(page_offset() + offset);
	return result;
}

void i82426ex_ib_device::dma_write_byte(offs_t offset, u8 data)
{
	address_space &prog_space = m_host_cpu->space(AS_PROGRAM);
	if (m_dma_channel == -1)
		return;

	prog_space.write_byte(page_offset() + offset, data);
}

u8 i82426ex_ib_device::dma_read_word(offs_t offset)
{
	address_space &prog_space = m_host_cpu->space(AS_PROGRAM);

	if (m_dma_channel == -1)
		return 0xff;

	u16 result = prog_space.read_word((page_offset() & 0xfe0000) | (offset << 1));
	m_dma_high_byte = result >> 8;

	return result;
}

void i82426ex_ib_device::dma_write_word(offs_t offset, u8 data)
{
	if (m_dma_channel == -1)
		return;
	address_space &prog_space = m_host_cpu->space(AS_PROGRAM);

	prog_space.write_word((page_offset() & 0xfe0000) | (offset << 1), m_dma_high_byte | data);
}

void i82426ex_ib_device::dma1_eop_w(int state)
{
	m_dma_eop = state == ASSERT_LINE;
	if (m_dma_channel != -1)
		m_isabus->eop_w(m_dma_channel, m_dma_eop ? ASSERT_LINE : CLEAR_LINE);
}

void i82426ex_ib_device::set_dma_channel(int channel, bool state)
{
	m_isabus->dack_line_w(channel, state);

	if (!state)
	{
		m_dma_channel = channel;
		if (m_dma_eop)
			m_isabus->eop_w(channel, ASSERT_LINE);
	}
	else
	{
		if (m_dma_channel == channel)
		{
			m_dma_channel = -1;
			if (m_dma_eop)
				m_isabus->eop_w(channel, CLEAR_LINE);
		}
	}
}

void i82426ex_ib_device::iochck_w(int state)
{
	if (!state && !BIT(m_portb, 3) && !m_nmi_mask)
		m_host_cpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void i82426ex_ib_device::remap_bridge()
{
	m_isabus->remap(AS_PROGRAM, 0, 1 << 24);
	m_isabus->remap(AS_IO, 0, 0xffff);
}
