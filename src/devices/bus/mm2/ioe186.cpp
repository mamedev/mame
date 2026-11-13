// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Nokia MikroMikko 2 IOE186 emulation

*********************************************************************/

#include "emu.h"
#include "ioe186.h"

DEFINE_DEVICE_TYPE(NOKIA_IOE186, ioe186_device, "nokia_ioe186", "Nokia MikroMikko 2 IOE186")

static INPUT_PORTS_START(ioe186)
	PORT_START("SID")
	PORT_CONFNAME(0x03, 0x01, "Base Address (SID)")
	PORT_CONFSETTING(   0x00, "FB00H")
	PORT_CONFSETTING(   0x01, "FA80H (BCS5)")
	PORT_CONFSETTING(   0x02, "FA00H")
	PORT_CONFSETTING(   0x03, "F980H")

	PORT_START("SIC")
	PORT_CONFNAME(0x07, 0x00, "Interrupt Priority (SIC)")
	PORT_CONFSETTING(   0x00, "IR2 (Highest)")
	PORT_CONFSETTING(   0x01, "IR3")
	PORT_CONFSETTING(   0x02, "IR4")
	PORT_CONFSETTING(   0x03, "IR5")
	PORT_CONFSETTING(   0x04, "IR6 (Lowest)")

	PORT_START("S1")
	PORT_CONFNAME(0x07, 0x01, "DMA Priority (S1A/S1B)")
	PORT_CONFSETTING(   0x00, "HOLD1 (Highest)")
	PORT_CONFSETTING(   0x01, "HOLD2")
	PORT_CONFSETTING(   0x02, "HOLD3")
	PORT_CONFSETTING(   0x03, "HOLD4")
	PORT_CONFSETTING(   0x04, "HOLD5 (Lowest)")
INPUT_PORTS_END

ioport_constructor ioe186_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ioe186);
}

ioe186_device::ioe186_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NOKIA_IOE186, tag, owner, clock),
	device_mikromikko2_expansion_bus_card_interface(mconfig, *this),
	m_dmac(*this, "am9517a"),
	m_mpsc(*this, "i8274"),
	m_pit(*this, "pit8253"),
	m_irqs(*this, "irqs"),
	m_ctrl(*this, "control"),
	m_out(*this, "output"),
	m_rs232a(*this, "rs232a"),
	m_rs232b(*this, "rs232b"),
	m_sid(*this, "SID"),
	m_sic(*this, "SIC"),
	m_s1(*this, "S1"),
	m_page(0),
	m_hold_level(0),
	m_int_level(0),
	m_llba(false),
	m_llbb(false),
	m_cla0(false),
	m_cla1(false),
	m_clb0(false),
	m_clb1(false)
{
}

void ioe186_device::map(address_map &map)
{
	map(0x00, 0x1f).rw(m_dmac, FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask16(0x00ff);
	map(0x20, 0x27).rw(m_mpsc, FUNC(i8274_device::cd_ba_r), FUNC(i8274_device::cd_ba_w)).umask16(0x00ff);
	map(0x30, 0x37).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0x40, 0x4f).w(m_ctrl, FUNC(ls259_device::write_d0)).umask16(0x00ff);
	map(0x50, 0x5f).w(m_out, FUNC(ls259_device::write_d0)).umask16(0x00ff);
	map(0x60, 0x61).r(FUNC(ioe186_device::input_r)).umask16(0x00ff);
	map(0x70, 0x71).w(FUNC(ioe186_device::page_w)).umask16(0x00ff);
}

void ioe186_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set(FUNC(ioe186_device::int_w));

	AM9517A(config, m_dmac, XTAL(16'000'000)/4);
	m_dmac->out_hreq_callback().set(FUNC(ioe186_device::hold_w));
	m_dmac->out_eop_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_dmac->in_memr_callback().set(FUNC(ioe186_device::dmac_mem_r));
	m_dmac->out_memw_callback().set(FUNC(ioe186_device::dmac_mem_w));
	m_dmac->out_iow_callback<0>().set(FUNC(ioe186_device::mpsc_txa_w));
	m_dmac->in_ior_callback<1>().set(FUNC(ioe186_device::mpsc_rxa_r));
	m_dmac->out_iow_callback<2>().set(FUNC(ioe186_device::mpsc_txb_w));
	m_dmac->in_ior_callback<3>().set(FUNC(ioe186_device::mpsc_rxb_r));
	m_dmac->out_dack_callback<0>().set(FUNC(ioe186_device::dack0_w));
	m_dmac->out_dack_callback<1>().set(FUNC(ioe186_device::dack1_w));
	m_dmac->out_dack_callback<2>().set(FUNC(ioe186_device::dack2_w));
	m_dmac->out_dack_callback<3>().set(FUNC(ioe186_device::dack3_w));

	I8274(config, m_mpsc, XTAL(16'000'000)/4);
	m_mpsc->out_int_callback().set(m_irqs, FUNC(input_merger_device::in_w<0>));
	m_mpsc->out_txdrqa_callback().set(m_dmac, FUNC(am9517a_device::dreq0_w));
	m_mpsc->out_rxdrqa_callback().set(m_dmac, FUNC(am9517a_device::dreq1_w));
	m_mpsc->out_txdrqb_callback().set(m_dmac, FUNC(am9517a_device::dreq2_w));
	m_mpsc->out_rxdrqb_callback().set(m_dmac, FUNC(am9517a_device::dreq3_w));
	m_mpsc->out_txda_callback().set(FUNC(ioe186_device::mpsc_txda_w));
	m_mpsc->out_rtsa_callback().set(FUNC(ioe186_device::mpsc_rtsa_w));
	m_mpsc->out_txdb_callback().set(FUNC(ioe186_device::mpsc_txdb_w));
	m_mpsc->out_rtsb_callback().set(FUNC(ioe186_device::mpsc_rtsb_w));

	PIT8253(config, m_pit);
	m_pit->set_clk<0>(XTAL(16'000'000)/8);
	m_pit->set_clk<1>(XTAL(16'000'000)/8);
	m_pit->set_clk<2>(XTAL(16'000'000)/8);
	m_pit->out_handler<0>().set(FUNC(ioe186_device::tmr0_w));
	m_pit->out_handler<1>().set(FUNC(ioe186_device::tmr1_w));
	m_pit->out_handler<2>().set(FUNC(ioe186_device::tmr2_w));

	RS232_PORT(config, m_rs232a, default_rs232_devices, nullptr);
	m_rs232a->rxd_handler().set(m_mpsc, FUNC(i8274_device::rxa_w));
	m_rs232a->dcd_handler().set(m_mpsc, FUNC(i8274_device::dcda_w));
	m_rs232a->cts_handler().set(m_mpsc, FUNC(i8274_device::ctsa_w));

	RS232_PORT(config, m_rs232b, default_rs232_devices, nullptr);
	m_rs232b->rxd_handler().set(m_mpsc, FUNC(i8274_device::rxb_w));
	m_rs232b->cts_handler().set(m_mpsc, FUNC(i8274_device::ctsb_w));

	LS259(config, m_ctrl);
	m_ctrl->q_out_cb<0>().set(FUNC(ioe186_device::cla0_w));
	m_ctrl->q_out_cb<1>().set(FUNC(ioe186_device::cla1_w));
	m_ctrl->q_out_cb<2>().set(FUNC(ioe186_device::clb0_w));
	m_ctrl->q_out_cb<3>().set(FUNC(ioe186_device::clb1_w));
	m_ctrl->q_out_cb<4>().set(FUNC(ioe186_device::llba_w));
	m_ctrl->q_out_cb<5>().set(FUNC(ioe186_device::llbb_w));
	//m_ctrl->q_out_cb<6>().set(); NRZI data coding/decoding, channel A (0=NRZ, 1=NRZI)
	//m_ctrl->q_out_cb<7>().set(); MN mininet select

	LS259(config, m_out);
	//m_out->q_out_cb<0>().set(); SRSA signal rate select, channel A
	m_out->q_out_cb<1>().set(m_rs232a, FUNC(rs232_port_device::write_dtr)).invert();
	//m_out->q_out_cb<2>().set(); TSTA test indicator, channel A
	//m_out->q_out_cb<3>().set(); SRSB signal rate select, channel B
	m_out->q_out_cb<4>().set(m_rs232b, FUNC(rs232_port_device::write_dtr)).invert();
	//m_out->q_out_cb<5>().set(); TSTB test indicator, channel B
	//m_out->q_out_cb<6>().set(); X27B X.27 select, channel B
	//m_out->q_out_cb<7>().set(); X27A/V28A X.27 select, channel A
}

void ioe186_device::device_start()
{
	m_mpsc->synca_w(1);

	save_item(NAME(m_page));
	save_item(NAME(m_hold_level));
	save_item(NAME(m_int_level));
	save_item(NAME(m_llba));
	save_item(NAME(m_llbb));
	save_item(NAME(m_cla0));
	save_item(NAME(m_cla1));
	save_item(NAME(m_clb0));
	save_item(NAME(m_clb1));
}

void ioe186_device::device_reset()
{
	static const offs_t bases[4] = { 0xfb00, 0xfa80, 0xfa00, 0xf980 };
	offs_t const base = bases[m_sid->read() & 3];

	m_bus->iospace().install_device(base, base + 0x7f, *this, &ioe186_device::map);

	m_hold_level = m_s1->read();
	m_int_level = m_sic->read();
}

void ioe186_device::hold_w(int state)
{
	switch (m_hold_level)
	{
	case 0: m_bus->hold1_w(state); break;
	case 1: m_bus->hold2_w(state); break;
	case 2: m_bus->hold3_w(state); break;
	case 3: m_bus->hold4_w(state); break;
	case 4: m_bus->hold5_w(state); break;
	}
}

void ioe186_device::int_w(int state)
{
	switch (m_int_level)
	{
	case 0: m_bus->ir2_w(state); break;
	case 1: m_bus->ir3_w(state); break;
	case 2: m_bus->ir4_w(state); break;
	case 3: m_bus->ir5_w(state); break;
	case 4: m_bus->ir6_w(state); break;
	}
}

void ioe186_device::bhlda_w(int state, int bcas)
{
	if (bcas == m_hold_level + 1)
		m_dmac->hack_w(state);
}

void ioe186_device::tmr0_w(int state)
{
	if (!m_cla1 && !m_cla0)
	{
		m_mpsc->txca_w(state);
		m_mpsc->rxca_w(state);
	}
	else if (!m_cla1 && m_cla0)
	{
		m_mpsc->txca_w(state);
	}
}

void ioe186_device::tmr1_w(int state)
{
	if (!m_cla1 && m_cla0)
		m_mpsc->rxca_w(state);

	if (!m_clb1 && m_clb0)
		m_mpsc->rxcb_w(state);
}

void ioe186_device::tmr2_w(int state)
{
	if (!m_clb1 && !m_clb0)
	{
		m_mpsc->txcb_w(state);
		m_mpsc->rxcb_w(state);
	}
	else if (!m_clb1 && m_clb0)
	{
		m_mpsc->txcb_w(state);
	}
}
