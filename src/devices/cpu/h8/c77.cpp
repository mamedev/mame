// license:BSD-3-Clause
// copyright-holders:smf, Olivier Galibert
/***************************************************************************

    Namco C77

    Similar to H8337, but some onboard peripherals and vectors were moved.

    Only the peripherals used in Cyber Lead I/O boards have been mapped.

***************************************************************************/

#include "emu.h"
#include "c77.h"

DEFINE_DEVICE_TYPE(NAMCO_C77, namco_c77_device, "namco_c77", "Namco C77")

namco_c77_device::namco_c77_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t start) :
	h8_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(namco_c77_device::map), this)),
	m_intc(*this, "intc"),
	m_port1(*this, "port1"),
	m_port2(*this, "port2"),
	m_port3(*this, "port3"),
	m_port4(*this, "port4"),
	m_port5(*this, "port5"),
	m_port6(*this, "port6"),
	m_port7(*this, "port7"),
	m_port8(*this, "port8"),
	m_port9(*this, "port9"),
	m_timer8_0(*this, "timer8_0"),
	//m_timer8_1(*this, "timer8_1"),
	m_syscr(0),
	m_ram_start(start)
{
}

namco_c77_device::namco_c77_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	namco_c77_device(mconfig, NAMCO_C77, tag, owner, clock, 0xf780)
{
}

void namco_c77_device::device_add_mconfig(machine_config &config)
{
	H8_INTC(config, m_intc, *this);
	//H8_ADC_3337(config, m_adc, *this, m_intc, 35);
	H8_PORT(config, m_port1, *this, h8_device::PORT_1, 0x00, 0x00);
	H8_PORT(config, m_port2, *this, h8_device::PORT_2, 0x00, 0x00);
	H8_PORT(config, m_port3, *this, h8_device::PORT_3, 0x00, 0x00);
	H8_PORT(config, m_port4, *this, h8_device::PORT_4, 0x00, 0x00);
	H8_PORT(config, m_port5, *this, h8_device::PORT_5, 0xf8, 0xf8);
	H8_PORT(config, m_port6, *this, h8_device::PORT_6, 0x00, 0x00);
	H8_PORT(config, m_port7, *this, h8_device::PORT_7, 0x00, 0x00);
	H8_PORT(config, m_port8, *this, h8_device::PORT_8, 0x80, 0x80);
	H8_PORT(config, m_port9, *this, h8_device::PORT_9, 0x00, 0x00);
	H8_TIMER8_CHANNEL(config, m_timer8_0, *this, m_intc, 23, 24, 25, 8, 2, 64, 32, 1024, 256);
	//H8_TIMER8_CHANNEL(config, m_timer8_1, *this, m_intc, 26, 27, 28, 8, 2, 64, 128, 1024, 2048);
	H8_SCI(config, m_sci[0], 0, *this, m_intc, 29, 30, 31, 32);
	H8_SCI(config, m_sci[1], 1, *this, m_intc, 33, 34, 35, 36);
}

void namco_c77_device::device_start()
{
	h8_device::device_start();

	save_item(NAME(m_syscr));
}

void namco_c77_device::device_reset()
{
	h8_device::device_reset();

	m_syscr = 0x09;
}

void namco_c77_device::execute_set_input(int inputnum, int state)
{
	m_intc->set_input(inputnum, state);
}

void namco_c77_device::irq_setup()
{
	m_CCR |= F_I;
}

void namco_c77_device::update_irq_filter()
{
	if(m_CCR & F_I)
		m_intc->set_filter(2, -1);
	else
		m_intc->set_filter(0, -1);
}

void namco_c77_device::interrupt_taken()
{
	standard_irq_callback(m_intc->interrupt_taken(m_taken_irq_vector), m_NPC);
}

void namco_c77_device::internal_update(uint64_t current_time)
{
	uint64_t event_time = 0;

	add_event(event_time, m_sci[0]->internal_update(current_time));
	add_event(event_time, m_sci[1]->internal_update(current_time));
	add_event(event_time, m_timer8_0->internal_update(current_time));
	//add_event(event_time, m_timer8_1->internal_update(current_time));

	recompute_bcount(event_time);
}

void namco_c77_device::notify_standby(int state)
{
	//m_adc->notify_standby(state);
	m_sci[0]->notify_standby(state);
	m_sci[1]->notify_standby(state);
	m_timer8_0->notify_standby(state);
	//m_timer8_1->notify_standby(state);
}

void namco_c77_device::map(address_map &map)
{
	map(m_ram_start, 0xff7f).ram();

	//map(0xffac, 0xffac).rw(m_port1, FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	//map(0xffad, 0xffad).rw(m_port2, FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	//map(0xffae, 0xffae).rw(m_port3, FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffb0, 0xffb0).w(m_port1, FUNC(h8_port_device::ddr_w));
	map(0xffb1, 0xffb1).w(m_port2, FUNC(h8_port_device::ddr_w));
	map(0xffb2, 0xffb2).rw(m_port1, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb3, 0xffb3).rw(m_port2, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb4, 0xffb4).w(m_port3, FUNC(h8_port_device::ddr_w));
	map(0xffb5, 0xffb5).w(m_port4, FUNC(h8_port_device::ddr_w));
	map(0xffb6, 0xffb6).rw(m_port3, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb7, 0xffb7).rw(m_port4, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb8, 0xffb8).w(m_port5, FUNC(h8_port_device::ddr_w));
	map(0xffb9, 0xffb9).w(m_port6, FUNC(h8_port_device::ddr_w));
	map(0xffba, 0xffba).rw(m_port5, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffbb, 0xffbb).rw(m_port6, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffbc, 0xffbc).w(m_port7, FUNC(h8_port_device::ddr_w));
	map(0xffbd, 0xffbd).w(m_port8, FUNC(h8_port_device::ddr_w));
	map(0xffbe, 0xffbe).rw(m_port7, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffbf, 0xffbf).rw(m_port8, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffc0, 0xffc0).w(m_port9, FUNC(h8_port_device::ddr_w));
	map(0xffc1, 0xffc1).rw(m_port9, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffc2, 0xffc2).rw(FUNC(namco_c77_device::wscr_r), FUNC(namco_c77_device::wscr_w));
	map(0xffc3, 0xffc3).rw(FUNC(namco_c77_device::stcr_r), FUNC(namco_c77_device::stcr_w));
	map(0xffc4, 0xffc4).rw(FUNC(namco_c77_device::syscr_r), FUNC(namco_c77_device::syscr_w));
	map(0xffc5, 0xffc5).rw(FUNC(namco_c77_device::mdcr_r), FUNC(namco_c77_device::mdcr_w));
	map(0xffc6, 0xffc6).rw(m_intc, FUNC(h8_intc_device::iscr_r), FUNC(h8_intc_device::iscr_w));
	map(0xffc7, 0xffc7).rw(m_intc, FUNC(h8_intc_device::ier_r), FUNC(h8_intc_device::ier_w));
	map(0xffc8, 0xffc8).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffc9, 0xffc9).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffca, 0xffcb).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w));
	map(0xffcc, 0xffcc).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	//map(0xffd0, 0xffd0).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	//map(0xffd1, 0xffd1).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	//map(0xffd2, 0xffd3).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w));
	//map(0xffd4, 0xffd4).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(0xffd8, 0xffd8).rw(m_sci[0], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffd9, 0xffd9).rw(m_sci[0], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffda, 0xffda).rw(m_sci[0], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffdb, 0xffdb).rw(m_sci[0], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffdc, 0xffdc).rw(m_sci[0], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffdd, 0xffdd).r(m_sci[0], FUNC(h8_sci_device::rdr_r));
	map(0xffe0, 0xffe0).rw(m_sci[1], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffe1, 0xffe1).rw(m_sci[1], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffe2, 0xffe2).rw(m_sci[1], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffe3, 0xffe3).rw(m_sci[1], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffe4, 0xffe4).rw(m_sci[1], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffe5, 0xffe5).r(m_sci[1], FUNC(h8_sci_device::rdr_r));

	//map(0xfff1, 0xfff1) // kmimr? Cyber Lead LED board writes 0xbf here
	//map(0xfff2, 0xfff2).rw(m_port6, FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
}

uint8_t namco_c77_device::wscr_r()
{
	return 0x00;
}

void namco_c77_device::wscr_w(uint8_t data)
{
	logerror("wscr = %02x\n", data);
}

uint8_t namco_c77_device::stcr_r()
{
	return 0x00;
}

void namco_c77_device::stcr_w(uint8_t data)
{
	logerror("stcr = %02x\n", data);
	m_timer8_0->set_extra_clock_bit(data & 0x01);
	//m_timer8_1->set_extra_clock_bit(data & 0x02);
}

uint8_t namco_c77_device::syscr_r()
{
	return m_syscr;
}

void namco_c77_device::syscr_w(uint8_t data)
{
	m_syscr = data;
	logerror("syscr = %02x\n", data);
}

uint8_t namco_c77_device::mdcr_r()
{
	return 0x00;
}

void namco_c77_device::mdcr_w(uint8_t data)
{
	logerror("mdcr = %02x\n", data);
}
