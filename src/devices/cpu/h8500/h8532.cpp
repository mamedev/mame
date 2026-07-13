// license:BSD-3-Clause
// copyright-holders:R. Belmont, AJR
/***************************************************************************

	Hitachi H8/532

***************************************************************************/

#include "emu.h"
#include "h8532.h"

DEFINE_DEVICE_TYPE(HD6435328, hd6435328_device, "hd6435328", "Hitachi HD6435328 (H8/532)")
DEFINE_DEVICE_TYPE(HD6475328, hd6475328_device, "hd6475328", "Hitachi HD6475328 (H8/532)")

h8532_device::h8532_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: h8500_device(mconfig, type, tag, owner, clock, 20, 8, 10, 4, address_map_constructor(FUNC(h8532_device::internal_map), this))
	  , m_intc(*this, "intc")
	  , m_adc(*this, "adc")
	  , m_frt(*this, "frt")
	  , m_frt1(*this, "frt:0")
	  , m_frt2(*this, "frt:1")
	  , m_frt3(*this, "frt:2")
	  , m_tmr(*this, "tmr")
	  , m_port1(*this, "port1")
	  , m_port2(*this, "port2")
	  , m_port3(*this, "port3")
	  , m_port4(*this, "port4")
	  , m_port5(*this, "port5")
	  , m_port6(*this, "port6")
	  , m_port7(*this, "port7")
	  , m_port8(*this, "port8")
	  , m_port9(*this, "port9")
	  , m_watchdog(*this, "watchdog")
{
}

hd6435328_device::hd6435328_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8532_device(mconfig, HD6435328, tag, owner, clock)
{
}

hd6475328_device::hd6475328_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8532_device(mconfig, HD6475328, tag, owner, clock)
{
}

void h8532_device::device_add_mconfig(machine_config &config)
{
	H8532_INTC(config, m_intc, *this);
	H8_ADC_3337(config, m_adc, *this, m_intc, 56);
	H8_TIMER16(config, m_frt, *this, 3, 0xff);
	H8500_FRT(config, m_frt1, *this, m_intc, 36);
	H8500_FRT(config, m_frt2, *this, m_intc, 40);
	H8500_FRT(config, m_frt3, *this, m_intc, 44);
	H8_TIMER8_CHANNEL(config, m_tmr, *this, m_intc, 48, 49, 50, 8, 8, 64, 64, 1024, 1024);
	H8_PORT(config, m_port1, *this, h8500_device::PORT_1, 0xff, 0x00);
	H8_PORT(config, m_port2, *this, h8500_device::PORT_2, 0xff, 0x00);
	H8_PORT(config, m_port3, *this, h8500_device::PORT_3, 0xff, 0x00);
	H8_PORT(config, m_port4, *this, h8500_device::PORT_4, 0xff, 0x00);
	H8_PORT(config, m_port5, *this, h8500_device::PORT_5, 0xff, 0x00);
	H8_PORT(config, m_port6, *this, h8500_device::PORT_6, 0xff, 0x00);
	H8_PORT(config, m_port7, *this, h8500_device::PORT_7, 0xff, 0x00);
	H8_PORT(config, m_port8, *this, h8500_device::PORT_8, 0xff, 0x00);
	H8_PORT(config, m_port9, *this, h8500_device::PORT_9, 0xff, 0x00);
	H8_SCI(config, m_sci[0], 0, *this, m_intc, 52, 53, 54, 55);
	// on this part the interval interrupt shares IRQ0's vector and priority
	H8_WATCHDOG(config, m_watchdog, *this, m_intc, 32, h8_watchdog_device::H);
}

void h8532_device::internal_map(address_map &map)
{
	if (mode_control() == 2 || mode_control() == 4 || mode_control() == 7)
		map(0x0000, 0x7fff).rom().region(DEVICE_SELF, 0);
	map(0xfb80, 0xff7f).ram(); // TODO: may be disabled by writing 0 to RAME bit in RAMCR

	map(0xff80, 0xff80).rw(m_port1, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xff81, 0xff81).rw(m_port2, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xff82, 0xff82).rw(m_port1, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xff83, 0xff83).rw(m_port2, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	// Port 3 has no DDR on the H8/532
	map(0xff85, 0xff85).rw(m_port4, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xff86, 0xff86).rw(m_port3, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xff87, 0xff87).rw(m_port4, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xff88, 0xff88).rw(m_port5, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xff89, 0xff89).rw(m_port6, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xff8a, 0xff8a).rw(m_port5, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xff8b, 0xff8b).rw(m_port6, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xff8c, 0xff8c).rw(m_port7, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xff8e, 0xff8e).rw(m_port7, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xff8f, 0xff8f).r(m_port8, FUNC(h8_port_device::port_r));	// port 8 is read-only

	// 16-bit free-running timers (8-bit bus, byte accessors with TEMP)
	map(0xff90, 0xff90).rw(m_frt1, FUNC(h8500_frt_device::tcr_r), FUNC(h8500_frt_device::tcr_w));
	map(0xff91, 0xff91).rw(m_frt1, FUNC(h8500_frt_device::tsr_r), FUNC(h8500_frt_device::tsr_w));
	map(0xff92, 0xff93).rw(m_frt1, FUNC(h8500_frt_device::frc8_r), FUNC(h8500_frt_device::frc8_w));
	map(0xff94, 0xff95).rw(m_frt1, FUNC(h8500_frt_device::ocra8_r), FUNC(h8500_frt_device::ocra8_w));
	map(0xff96, 0xff97).rw(m_frt1, FUNC(h8500_frt_device::ocrb8_r), FUNC(h8500_frt_device::ocrb8_w));
	map(0xff98, 0xff99).r(m_frt1, FUNC(h8500_frt_device::icr8_r));
	map(0xffa0, 0xffa0).rw(m_frt2, FUNC(h8500_frt_device::tcr_r), FUNC(h8500_frt_device::tcr_w));
	map(0xffa1, 0xffa1).rw(m_frt2, FUNC(h8500_frt_device::tsr_r), FUNC(h8500_frt_device::tsr_w));
	map(0xffa2, 0xffa3).rw(m_frt2, FUNC(h8500_frt_device::frc8_r), FUNC(h8500_frt_device::frc8_w));
	map(0xffa4, 0xffa5).rw(m_frt2, FUNC(h8500_frt_device::ocra8_r), FUNC(h8500_frt_device::ocra8_w));
	map(0xffa6, 0xffa7).rw(m_frt2, FUNC(h8500_frt_device::ocrb8_r), FUNC(h8500_frt_device::ocrb8_w));
	map(0xffa8, 0xffa9).r(m_frt2, FUNC(h8500_frt_device::icr8_r));
	map(0xffb0, 0xffb0).rw(m_frt3, FUNC(h8500_frt_device::tcr_r), FUNC(h8500_frt_device::tcr_w));
	map(0xffb1, 0xffb1).rw(m_frt3, FUNC(h8500_frt_device::tsr_r), FUNC(h8500_frt_device::tsr_w));
	map(0xffb2, 0xffb3).rw(m_frt3, FUNC(h8500_frt_device::frc8_r), FUNC(h8500_frt_device::frc8_w));
	map(0xffb4, 0xffb5).rw(m_frt3, FUNC(h8500_frt_device::ocra8_r), FUNC(h8500_frt_device::ocra8_w));
	map(0xffb6, 0xffb7).rw(m_frt3, FUNC(h8500_frt_device::ocrb8_r), FUNC(h8500_frt_device::ocrb8_w));
	map(0xffb8, 0xffb9).r(m_frt3, FUNC(h8500_frt_device::icr8_r));

	// 8-bit timer
	map(0xffd0, 0xffd0).rw(m_tmr, FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffd1, 0xffd1).rw(m_tmr, FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffd2, 0xffd3).rw(m_tmr, FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w));
	map(0xffd4, 0xffd4).rw(m_tmr, FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));

	// SCI channel 1 (m_sci[0])
	map(0xffd8, 0xffd8).rw(m_sci[0], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffd9, 0xffd9).rw(m_sci[0], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffda, 0xffda).rw(m_sci[0], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffdb, 0xffdb).rw(m_sci[0], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffdc, 0xffdc).rw(m_sci[0], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffdd, 0xffdd).r(m_sci[0], FUNC(h8_sci_device::rdr_r));

	// A/D converter
	map(0xffe0, 0xffe7).r(m_adc, FUNC(h8_adc_device::addr8_r));
	map(0xffe8, 0xffe8).rw(m_adc, FUNC(h8_adc_device::adcsr_r), FUNC(h8_adc_device::adcsr_w));
	map(0xffe9, 0xffe9).rw(m_adc, FUNC(h8_adc_device::adcr_r), FUNC(h8_adc_device::adcr_w));

	// Interrupt controller (no IPRE/F or DTEE/F on this part)
	map(0xfff0, 0xfff0).rw(m_intc, FUNC(h8500_intc_device::ipra_r), FUNC(h8500_intc_device::ipra_w));
	map(0xfff1, 0xfff1).rw(m_intc, FUNC(h8500_intc_device::iprb_r), FUNC(h8500_intc_device::iprb_w));
	map(0xfff2, 0xfff2).rw(m_intc, FUNC(h8500_intc_device::iprc_r), FUNC(h8500_intc_device::iprc_w));
	map(0xfff3, 0xfff3).rw(m_intc, FUNC(h8500_intc_device::iprd_r), FUNC(h8500_intc_device::iprd_w));
	map(0xfff4, 0xfff4).rw(m_intc, FUNC(h8500_intc_device::dtea_r), FUNC(h8500_intc_device::dtea_w));
	map(0xfff5, 0xfff5).rw(m_intc, FUNC(h8500_intc_device::dteb_r), FUNC(h8500_intc_device::dteb_w));
	map(0xfff6, 0xfff6).rw(m_intc, FUNC(h8500_intc_device::dtec_r), FUNC(h8500_intc_device::dtec_w));
	map(0xfff7, 0xfff7).rw(m_intc, FUNC(h8500_intc_device::dted_r), FUNC(h8500_intc_device::dted_w));

	// P1CR carries the IRQ enables, NMI edge select and bus release enable
	map(0xfffc, 0xfffc).rw(m_intc, FUNC(h8532_intc_device::p1cr_r), FUNC(h8532_intc_device::p1cr_w));

	// Port 9
	map(0xfffe, 0xfffe).rw(m_port9, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xffff, 0xffff).rw(m_port9, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));

#if 0
	map(0xffc0, 0xffc0).rw(FUNC(h8532_device::pwm1_tcr_r), FUNC(h8532_device::pwm1_tcr_w));
	map(0xffc1, 0xffc1).rw(FUNC(h8532_device::pwm1_dtr_r), FUNC(h8532_device::pwm1_dtr_w));
	map(0xffc2, 0xffc2).rw(FUNC(h8532_device::pwm1_tcnt_r), FUNC(h8532_device::pwm1_tcnt_w));
	map(0xffc4, 0xffc4).rw(FUNC(h8532_device::pwm2_tcr_r), FUNC(h8532_device::pwm2_tcr_w));
	map(0xffc5, 0xffc5).rw(FUNC(h8532_device::pwm2_dtr_r), FUNC(h8532_device::pwm2_dtr_w));
	map(0xffc6, 0xffc6).rw(FUNC(h8532_device::pwm2_tcnt_r), FUNC(h8532_device::pwm2_tcnt_w));
	map(0xffc8, 0xffc8).rw(FUNC(h8532_device::pwm3_tcr_r), FUNC(h8532_device::pwm3_tcr_w));
	map(0xffc9, 0xffc9).rw(FUNC(h8532_device::pwm3_dtr_r), FUNC(h8532_device::pwm3_dtr_w));
	map(0xffca, 0xffca).rw(FUNC(h8532_device::pwm3_tcnt_r), FUNC(h8532_device::pwm3_tcnt_w));
	map(0xffec, 0xffec).rw(FUNC(h8532_device::wdt_tcsr_r));
	map(0xffed, 0xffed).r(FUNC(h8532_device::wdt_tcnt_r), FUNC(h8532_device::wdt_tcsr_w));
	map(0xfff8, 0xfff8).rw(FUNC(h8532_device::wcr_r), FUNC(h8532_device::wcr_w));
	map(0xfff9, 0xfff9).rw(FUNC(h8532_device::ramcr_r), FUNC(h8532_device::ramcr_w));
	map(0xfffa, 0xfffa).r(FUNC(h8532_device::mdcr_r));
	map(0xfffb, 0xfffb).rw(FUNC(h8532_device::sbycr_r), FUNC(h8532_device::sbycr_w));
#endif
}

void h8532_device::internal_update(u64 current_time)
{
	u64 event_time = 0;

	add_event(event_time, m_adc->internal_update(current_time));
	add_event(event_time, m_frt1->internal_update(current_time));
	add_event(event_time, m_frt2->internal_update(current_time));
	add_event(event_time, m_frt3->internal_update(current_time));
	add_event(event_time, m_tmr->internal_update(current_time));
	add_event(event_time, m_sci[0]->internal_update(current_time));
	add_event(event_time, m_watchdog->internal_update(current_time));

	recompute_bcount(event_time);
}

void h8532_device::notify_standby(int state)
{
	m_adc->notify_standby(state);
	m_frt1->notify_standby(state);
	m_frt2->notify_standby(state);
	m_frt3->notify_standby(state);
	m_tmr->notify_standby(state);
	m_sci[0]->notify_standby(state);
	m_watchdog->notify_standby(state);
}

void h8532_device::execute_set_input(int inputnum, int state)
{
	m_intc->set_input(inputnum, state);
}

void h8532_device::interrupt_taken()
{
	standard_irq_callback(m_intc->interrupt_taken(m_taken_irq_vector), m_npc);
}

void h8532_device::update_irq_filter()
{
	m_intc->set_filter((m_sr & (SR_I2 | SR_I1 | SR_I0)) >> 8);
}

void h8532_device::irq_setup()
{
	m_sr &= ~(SR_T | SR_I2 | SR_I1 | SR_I0);
	m_sr |= std::min(m_taken_irq_level, 7) << 8;
}
