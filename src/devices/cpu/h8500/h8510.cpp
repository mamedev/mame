// license:BSD-3-Clause
// copyright-holders:R. Belmont, AJR
/***************************************************************************

	Hitachi H8/510

***************************************************************************/

#include "emu.h"
#include "h8510.h"

DEFINE_DEVICE_TYPE(HD6415108, hd6415108_device, "hd6415108", "Hitachi HD6415108 (H8/510)")

h8510_device::h8510_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: h8500_device(mconfig, type, tag, owner, clock, 24, 16, 0, 4, address_map_constructor(FUNC(h8510_device::internal_map), this))
	  , m_intc(*this, "intc")
	  , m_adc(*this, "adc")
	  , m_frt(*this, "frt")
	  , m_frt1(*this, "frt:0")
	  , m_frt2(*this, "frt:1")
	  , m_tmr(*this, "tmr")
	  , m_port1(*this, "port1")
	  , m_port2(*this, "port2")
	  , m_port3(*this, "port3")
	  , m_port4(*this, "port4")
	  , m_port5(*this, "port5")
	  , m_port6(*this, "port6")
	  , m_port7(*this, "port7")
	  , m_port8(*this, "port8")
	  , m_watchdog(*this, "watchdog")
{
}

hd6415108_device::hd6415108_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8510_device(mconfig, HD6415108, tag, owner, clock)
{
}

void h8510_device::device_add_mconfig(machine_config &config)
{
	H8500_INTC(config, m_intc, *this);
	H8_ADC_3337(config, m_adc, *this, m_intc, 60);
	H8_TIMER16(config, m_frt, *this, 2, 0xff);
	H8500_FRT(config, m_frt1, *this, m_intc, 40);
	H8500_FRT(config, m_frt2, *this, m_intc, 44);
	H8_TIMER8_CHANNEL(config, m_tmr, *this, m_intc, 48, 49, 50, 8, 8, 64, 64, 1024, 1024);
	H8_PORT(config, m_port1, *this, h8500_device::PORT_1, 0xff, 0x00);
	H8_PORT(config, m_port2, *this, h8500_device::PORT_2, 0xff, 0x00);
	H8_PORT(config, m_port3, *this, h8500_device::PORT_3, 0xff, 0x00);
	H8_PORT(config, m_port4, *this, h8500_device::PORT_4, 0xff, 0x00);
	H8_PORT(config, m_port5, *this, h8500_device::PORT_5, 0xff, 0x00);
	H8_PORT(config, m_port6, *this, h8500_device::PORT_6, 0xff, 0x00);
	H8_PORT(config, m_port7, *this, h8500_device::PORT_7, 0xff, 0xf0);
	H8_PORT(config, m_port8, *this, h8500_device::PORT_8, 0xff, 0x00);
	H8_SCI(config, m_sci[0], 0, *this, m_intc, 52, 53, 54, 55);
	H8_SCI(config, m_sci[1], 1, *this, m_intc, 56, 57, 58, 59);
	// the interval interrupt shares the IRQ0 priority level in IPRA but has its own vector
	H8_WATCHDOG(config, m_watchdog, *this, m_intc, 33, h8_watchdog_device::H);
}

void h8510_device::internal_map(address_map &map)
{
	map(0xfe80, 0xfe80).rw(m_port1, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfe81, 0xfe81).rw(m_port2, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfe82, 0xfe82).rw(m_port1, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xfe83, 0xfe83).rw(m_port2, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xfe84, 0xfe84).rw(m_port3, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfe85, 0xfe85).rw(m_port4, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfe86, 0xfe86).rw(m_port3, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xfe87, 0xfe87).rw(m_port4, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xfe88, 0xfe88).rw(m_port5, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfe89, 0xfe89).rw(m_port6, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfe8a, 0xfe8a).rw(m_port5, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xfe8b, 0xfe8b).rw(m_port6, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xfe8d, 0xfe8d).rw(m_port8, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfe8e, 0xfe8e).rw(m_port7, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xfe8f, 0xfe8f).rw(m_port8, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xfe90, 0xfe97).r(m_adc, FUNC(h8_adc_device::addr8_r));
	map(0xfe98, 0xfe98).rw(m_adc, FUNC(h8_adc_device::adcsr_r), FUNC(h8_adc_device::adcsr_w));
	map(0xfe99, 0xfe99).rw(m_adc, FUNC(h8_adc_device::adcr_r), FUNC(h8_adc_device::adcr_w));

	map(0xfea0, 0xfea0).rw(m_frt1, FUNC(h8500_frt_device::tcr_r), FUNC(h8500_frt_device::tcr_w));
	map(0xfea1, 0xfea1).rw(m_frt1, FUNC(h8500_frt_device::tsr_r), FUNC(h8500_frt_device::tsr_w));
	map(0xfea2, 0xfea3).rw(m_frt1, FUNC(h8500_frt_device::tcnt_r), FUNC(h8500_frt_device::tcnt_w));
	map(0xfea4, 0xfea5).rw(m_frt1, FUNC(h8500_frt_device::ocra_r), FUNC(h8500_frt_device::ocra_w));
	map(0xfea6, 0xfea7).rw(m_frt1, FUNC(h8500_frt_device::ocrb_r), FUNC(h8500_frt_device::ocrb_w));
	map(0xfea8, 0xfea9).r(m_frt1, FUNC(h8500_frt_device::icr_r));
	map(0xfeb0, 0xfeb0).rw(m_frt2, FUNC(h8500_frt_device::tcr_r), FUNC(h8500_frt_device::tcr_w));
	map(0xfeb1, 0xfeb1).rw(m_frt2, FUNC(h8500_frt_device::tsr_r), FUNC(h8500_frt_device::tsr_w));
	map(0xfeb2, 0xfeb3).rw(m_frt2, FUNC(h8500_frt_device::tcnt_r), FUNC(h8500_frt_device::tcnt_w));
	map(0xfeb4, 0xfeb5).rw(m_frt2, FUNC(h8500_frt_device::ocra_r), FUNC(h8500_frt_device::ocra_w));
	map(0xfeb6, 0xfeb7).rw(m_frt2, FUNC(h8500_frt_device::ocrb_r), FUNC(h8500_frt_device::ocrb_w));
	map(0xfeb8, 0xfeb9).r(m_frt2, FUNC(h8500_frt_device::icr_r));

	// 8-bit timer
	map(0xfec0, 0xfec0).rw(m_tmr, FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xfec1, 0xfec1).rw(m_tmr, FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xfec2, 0xfec3).rw(m_tmr, FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w));
	map(0xfec4, 0xfec4).rw(m_tmr, FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));

	map(0xfec8, 0xfec8).rw(m_sci[0], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xfec9, 0xfec9).rw(m_sci[0], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xfeca, 0xfeca).rw(m_sci[0], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xfecb, 0xfecb).rw(m_sci[0], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xfecc, 0xfecc).rw(m_sci[0], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xfecd, 0xfecd).r(m_sci[0], FUNC(h8_sci_device::rdr_r));
	map(0xfed0, 0xfed0).rw(m_sci[1], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xfed1, 0xfed1).rw(m_sci[1], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xfed2, 0xfed2).rw(m_sci[1], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xfed3, 0xfed3).rw(m_sci[1], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xfed4, 0xfed4).rw(m_sci[1], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xfed5, 0xfed5).r(m_sci[1], FUNC(h8_sci_device::rdr_r));

	map(0xff00, 0xff00).rw(m_intc, FUNC(h8500_intc_device::ipra_r), FUNC(h8500_intc_device::ipra_w));
	map(0xff01, 0xff01).rw(m_intc, FUNC(h8500_intc_device::iprb_r), FUNC(h8500_intc_device::iprb_w));
	map(0xff02, 0xff02).rw(m_intc, FUNC(h8500_intc_device::iprc_r), FUNC(h8500_intc_device::iprc_w));
	map(0xff03, 0xff03).rw(m_intc, FUNC(h8500_intc_device::iprd_r), FUNC(h8500_intc_device::iprd_w));
	map(0xff08, 0xff08).rw(m_intc, FUNC(h8500_intc_device::dtea_r), FUNC(h8500_intc_device::dtea_w));
	map(0xff09, 0xff09).rw(m_intc, FUNC(h8500_intc_device::dteb_r), FUNC(h8500_intc_device::dteb_w));
	map(0xff0a, 0xff0a).rw(m_intc, FUNC(h8500_intc_device::dtec_r), FUNC(h8500_intc_device::dtec_w));
	map(0xff0b, 0xff0b).rw(m_intc, FUNC(h8500_intc_device::dted_r), FUNC(h8500_intc_device::dted_w));
	map(0xff1c, 0xff1c).rw(m_intc, FUNC(h8500_intc_device::nmicr_r), FUNC(h8500_intc_device::nmicr_w));
	map(0xff1d, 0xff1d).rw(m_intc, FUNC(h8500_intc_device::irqcr_r), FUNC(h8500_intc_device::irqcr_w));

#if 0
	map(0xfed8, 0xfed8).rw(FUNC(h8510_device::rfshcr_r), FUNC(h8510_device::rfshcr_w));

	map(0xff14, 0xff14).rw(FUNC(h8510_device::wcr_r), FUNC(h8510_device::wcr_w));
	map(0xff16, 0xff16).rw(FUNC(h8510_device::arbt_r), FUNC(h8510_device::arbt_w));
	map(0xff17, 0xff17).rw(FUNC(h8510_device::ar3t_r), FUNC(h8510_device::ar3t_w));
	map(0xff19, 0xff19).r(FUNC(h8510_device::mdcr_r));
	map(0xff1a, 0xff1a).rw(FUNC(h8510_device::sbycr_r), FUNC(h8510_device::sbycr_w));
	map(0xff1b, 0xff1b).rw(FUNC(h8510_device::brcr_r), FUNC(h8510_device::brcr_w));
	map(0xff1e, 0xff1e).r(FUNC(h8510_device::wdt_rstcsr_r));
	map(0xff1e, 0xff1f).w(FUNC(h8510_device::wdt_rstcsr_w));
#endif
}

void h8510_device::internal_update(u64 current_time)
{
	u64 event_time = 0;

	add_event(event_time, m_adc->internal_update(current_time));
	add_event(event_time, m_frt1->internal_update(current_time));
	add_event(event_time, m_frt2->internal_update(current_time));
	add_event(event_time, m_tmr->internal_update(current_time));
	add_event(event_time, m_sci[0]->internal_update(current_time));
	add_event(event_time, m_sci[1]->internal_update(current_time));
	add_event(event_time, m_watchdog->internal_update(current_time));

	recompute_bcount(event_time);
}

void h8510_device::notify_standby(int state)
{
	m_adc->notify_standby(state);
	m_frt1->notify_standby(state);
	m_frt2->notify_standby(state);
	m_tmr->notify_standby(state);
	m_sci[0]->notify_standby(state);
	m_sci[1]->notify_standby(state);
	m_watchdog->notify_standby(state);
}

void h8510_device::execute_set_input(int inputnum, int state)
{
	m_intc->set_input(inputnum, state);
}

void h8510_device::interrupt_taken()
{
	standard_irq_callback(m_intc->interrupt_taken(m_taken_irq_vector), m_npc);
}

void h8510_device::update_irq_filter()
{
	m_intc->set_filter((m_sr & (SR_I2 | SR_I1 | SR_I0)) >> 8);
}

void h8510_device::irq_setup()
{
	m_sr &= ~(SR_T | SR_I2 | SR_I1 | SR_I0);
	m_sr |= std::min(m_taken_irq_level, 7) << 8;
}
