// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"
#include "i82426ex_ib.h"

DEFINE_DEVICE_TYPE(I82426EX_IB, i82426ex_ib_device, "i82426ex_ib", "Intel 82425EX ISA Bridge")

i82426ex_ib_device::i82426ex_ib_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, I82426EX_IB, tag, owner, clock)
	, m_keybc(*this, finder_base::DUMMY_TAG)
	, m_dma(*this, "dma%u", 1U)
	, m_intc(*this, "intc%u", 1U)
	, m_pit(*this, "pit")
	, m_write_intr(*this)
	, m_write_spkr(*this)
	, m_rtcale(*this)
	, m_rtccs_read(*this, 0xff)
	, m_rtccs_write(*this)
{
}

void i82426ex_ib_device::device_add_mconfig(machine_config &config)
{
	AM9517A(config, m_dma[0], 0);
//	m_dma[0]->out_hreq_callback().set(m_dma[1], FUNC(am9517a_device::dreq0_w));
//	m_dma[0]->out_eop_callback().set(FUNC(i82426ex_ib_device::dma1_eop_w));
//	m_dma[0]->in_memr_callback().set(FUNC(i82426ex_ib_device::dma_read_byte));
//	m_dma[0]->out_memw_callback().set(FUNC(i82426ex_ib_device::dma_write_byte));
//	m_dma[0]->in_ior_callback<0>().set(FUNC(i82426ex_ib_device::dma1_ior0_r));
//	m_dma[0]->in_ior_callback<1>().set(FUNC(i82426ex_ib_device::dma1_ior1_r));
//	m_dma[0]->in_ior_callback<2>().set(FUNC(i82426ex_ib_device::dma1_ior2_r));
//	m_dma[0]->in_ior_callback<3>().set(FUNC(i82426ex_ib_device::dma1_ior3_r));
//	m_dma[0]->out_iow_callback<0>().set(FUNC(i82426ex_ib_device::dma1_iow0_w));
//	m_dma[0]->out_iow_callback<1>().set(FUNC(i82426ex_ib_device::dma1_iow1_w));
//	m_dma[0]->out_iow_callback<2>().set(FUNC(i82426ex_ib_device::dma1_iow2_w));
//	m_dma[0]->out_iow_callback<3>().set(FUNC(i82426ex_ib_device::dma1_iow3_w));
//	m_dma[0]->out_dack_callback<0>().set(FUNC(i82426ex_ib_device::dma1_dack0_w));
//	m_dma[0]->out_dack_callback<1>().set(FUNC(i82426ex_ib_device::dma1_dack1_w));
//	m_dma[0]->out_dack_callback<2>().set(FUNC(i82426ex_ib_device::dma1_dack2_w));
//	m_dma[0]->out_dack_callback<3>().set(FUNC(i82426ex_ib_device::dma1_dack3_w));

	AM9517A(config, m_dma[1], 0);
//	m_dma[1]->out_hreq_callback().set(FUNC(i82426ex_ib_device::dma2_hreq_w));
//	m_dma[1]->in_memr_callback().set(FUNC(i82426ex_ib_device::dma_read_word));
//	m_dma[1]->out_memw_callback().set(FUNC(i82426ex_ib_device::dma_write_word));
//	m_dma[1]->in_ior_callback<1>().set(FUNC(i82426ex_ib_device::dma2_ior1_r));
//	m_dma[1]->in_ior_callback<2>().set(FUNC(i82426ex_ib_device::dma2_ior2_r));
//	m_dma[1]->in_ior_callback<3>().set(FUNC(i82426ex_ib_device::dma2_ior3_r));
//	m_dma[1]->out_iow_callback<1>().set(FUNC(i82426ex_ib_device::dma2_iow1_w));
//	m_dma[1]->out_iow_callback<2>().set(FUNC(i82426ex_ib_device::dma2_iow2_w));
//	m_dma[1]->out_iow_callback<3>().set(FUNC(i82426ex_ib_device::dma2_iow3_w));
//	m_dma[1]->out_dack_callback<0>().set(FUNC(i82426ex_ib_device::dma2_dack0_w));
//	m_dma[1]->out_dack_callback<1>().set(FUNC(i82426ex_ib_device::dma2_dack1_w));
//	m_dma[1]->out_dack_callback<2>().set(FUNC(i82426ex_ib_device::dma2_dack2_w));
//	m_dma[1]->out_dack_callback<3>().set(FUNC(i82426ex_ib_device::dma2_dack3_w));

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
	m_pit->set_clk<0>(DERIVED_CLOCK(1, 12));
	m_pit->out_handler<0>().set(m_intc[0], FUNC(pic8259_device::ir0_w));
	m_pit->set_clk<1>(DERIVED_CLOCK(1, 12));
	m_pit->out_handler<1>().set([this] (int state) {
		m_refresh_toggle ^= state;
		m_portb = (m_portb & 0xef) | (m_refresh_toggle << 4);
	});
	m_pit->set_clk<2>(DERIVED_CLOCK(1, 12));
	m_pit->out_handler<2>().set([this] (int state) {
		m_write_spkr(!(state & BIT(m_portb, 1)));
		m_portb = (m_portb & 0xdf) | (state << 5);
	});
}

void i82426ex_ib_device::device_start()
{
	save_item(NAME(m_portb));
	save_item(NAME(m_refresh_toggle));
	save_item(NAME(m_nmi_mask));
}

void i82426ex_ib_device::device_reset()
{
	m_nmi_mask = 1;
	m_dma[0]->set_unscaled_clock(2'500'000);
	m_dma[1]->set_unscaled_clock(2'500'000);
}

void i82426ex_ib_device::device_reset_after_children()
{
	// timer 2 default state
	m_pit->write_gate2(1);
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

//	map(0x0080, 0x008f) DMA page
	map(0x00a0, 0x00a1).rw(m_intc[1], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
//	map(0x00b2, 0x00b2) APMC
//	map(0x00b3, 0x00b3) APMS
//	map(0x00c0, 0x00df) DMA2
//	map(0x00f0, 0x00f0) Coprocessor Error
//	map(0x0480, 0x048f) DMA high page (not all of it)
//	map(0x04d0, 0x04d0) INT-1 Edge/Level Control
//	map(0x04d1, 0x04d1) INT-2 Edge/Level Control
}

u8 i82426ex_ib_device::portb_r()
{
	return m_portb;
}

void i82426ex_ib_device::portb_w(u8 data)
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
