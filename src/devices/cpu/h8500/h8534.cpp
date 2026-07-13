// license:BSD-3-Clause
// copyright-holders:R. Belmont, AJR
/***************************************************************************

	Hitachi H8/534 & H8/536

***************************************************************************/

#include "emu.h"
#include "h8534.h"

DEFINE_DEVICE_TYPE(HD6435348, hd6435348_device, "hd6435348", "Hitachi HD6435348 (H8/534)")
DEFINE_DEVICE_TYPE(HD6475348, hd6475348_device, "hd6475348", "Hitachi HD6475348 (H8/534)")
DEFINE_DEVICE_TYPE(HD6435368, hd6435368_device, "hd6435368", "Hitachi HD6435368 (H8/536)")
DEFINE_DEVICE_TYPE(HD6475368, hd6475368_device, "hd6475368", "Hitachi HD6475368 (H8/536)")

h8534_device::h8534_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map)
	: h8500_device(mconfig, type, tag, owner, clock, 20, 8, 11, 4, map)
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

void h8534_device::device_add_mconfig(machine_config &config)
{
	H8534_INTC(config, m_intc, *this);
	H8_ADC_3337(config, m_adc, *this, m_intc, 72);
	H8_TIMER16(config, m_frt, *this, 3, 0xff);
	H8500_FRT(config, m_frt1, *this, m_intc, 48);
	H8500_FRT(config, m_frt2, *this, m_intc, 52);
	H8500_FRT(config, m_frt3, *this, m_intc, 56);
	H8_TIMER8_CHANNEL(config, m_tmr, *this, m_intc, 60, 61, 62, 8, 8, 64, 64, 1024, 1024);
	H8_PORT(config, m_port1, *this, h8500_device::PORT_1, 0xff, 0x00);
	H8_PORT(config, m_port2, *this, h8500_device::PORT_2, 0xff, 0x00);
	H8_PORT(config, m_port3, *this, h8500_device::PORT_3, 0xff, 0x00);
	H8_PORT(config, m_port4, *this, h8500_device::PORT_4, 0xff, 0x00);
	H8_PORT(config, m_port5, *this, h8500_device::PORT_5, 0xff, 0x00);
	H8_PORT(config, m_port6, *this, h8500_device::PORT_6, 0xff, 0x00);
	H8_PORT(config, m_port7, *this, h8500_device::PORT_7, 0xff, 0x00);
	H8_PORT(config, m_port8, *this, h8500_device::PORT_8, 0xff, 0x00);
	H8_PORT(config, m_port9, *this, h8500_device::PORT_9, 0xff, 0x00);
	H8_SCI(config, m_sci[0], 0, *this, m_intc, 64, 65, 66, 67);
	H8_SCI(config, m_sci[1], 1, *this, m_intc, 68, 69, 70, 71);
	// the interval interrupt shares the IRQ0 priority level in IPRA but has its own vector
	H8_WATCHDOG(config, m_watchdog, *this, m_intc, 33, h8_watchdog_device::H);
}

void h8534_device::internal_update(u64 current_time)
{
	u64 event_time = 0;

	add_event(event_time, m_adc->internal_update(current_time));
	add_event(event_time, m_frt1->internal_update(current_time));
	add_event(event_time, m_frt2->internal_update(current_time));
	add_event(event_time, m_frt3->internal_update(current_time));
	add_event(event_time, m_tmr->internal_update(current_time));
	add_event(event_time, m_sci[0]->internal_update(current_time));
	add_event(event_time, m_sci[1]->internal_update(current_time));
	add_event(event_time, m_watchdog->internal_update(current_time));

	recompute_bcount(event_time);
}

void h8534_device::notify_standby(int state)
{
	m_adc->notify_standby(state);
	m_frt1->notify_standby(state);
	m_frt2->notify_standby(state);
	m_frt3->notify_standby(state);
	m_tmr->notify_standby(state);
	m_sci[0]->notify_standby(state);
	m_sci[1]->notify_standby(state);
	m_watchdog->notify_standby(state);
}

void h8534_device::execute_set_input(int inputnum, int state)
{
	m_intc->set_input(inputnum, state);
}

void h8534_device::interrupt_taken()
{
	standard_irq_callback(m_intc->interrupt_taken(m_taken_irq_vector), m_npc);
}

void h8534_device::update_irq_filter()
{
	m_intc->set_filter((m_sr & (SR_I2 | SR_I1 | SR_I0)) >> 8);
}

void h8534_device::irq_setup()
{
	// clear the T bit and copy the priority level of the accepted
	// interrupt to I2-I0; NMI (level 8) sets the mask to 7
	m_sr &= ~(SR_T | SR_I2 | SR_I1 | SR_I0);
	m_sr |= std::min(m_taken_irq_level, 7) << 8;
}

h8534_device::h8534_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: h8534_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h8534_device::internal_map), this))
{
}

hd6435348_device::hd6435348_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8534_device(mconfig, HD6435348, tag, owner, clock)
{
}

hd6475348_device::hd6475348_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8534_device(mconfig, HD6475348, tag, owner, clock)
{
}

h8536_device::h8536_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: h8534_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h8536_device::internal_map), this))
{
}

hd6435368_device::hd6435368_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8536_device(mconfig, HD6435368, tag, owner, clock)
{
}

hd6475368_device::hd6475368_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8536_device(mconfig, HD6475368, tag, owner, clock)
{
}

void h8534_device::internal_map(address_map &map)
{
	if (mode_control() == 2 || mode_control() == 4 || mode_control() == 7)
		map(0x0000, 0x7fff).rom().region(DEVICE_SELF, 0);
	map(0xf680, 0xfe7f).ram(); // TODO: may be disabled by writing 0 to RAME bit in RAMCR
	register_field_map(map);
}

void h8536_device::internal_map(address_map &map)
{
	if (mode_control() == 2)
		map(0x0000, 0xee7f).rom().region(DEVICE_SELF, 0);
	else if (mode_control() == 4 || mode_control() == 7)
		map(0x0000, 0xf67f).rom().region(DEVICE_SELF, 0);
	map(0xf680, 0xfe7f).ram(); // TODO: may be disabled by writing 0 to RAME bit in RAMCR
	register_field_map(map);
}

void h8534_device::register_field_map(address_map &map)
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
	map(0xfe8c, 0xfe8c).rw(m_port7, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfe8e, 0xfe8e).rw(m_port7, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xfe8f, 0xfe8f).r(m_port8, FUNC(h8_port_device::port_r));	// port 8 is read-only

	// 16-bit free-running timers (8-bit bus, byte accessors with TEMP)
	map(0xfe90, 0xfe90).rw(m_frt1, FUNC(h8500_frt_device::tcr_r), FUNC(h8500_frt_device::tcr_w));
	map(0xfe91, 0xfe91).rw(m_frt1, FUNC(h8500_frt_device::tsr_r), FUNC(h8500_frt_device::tsr_w));
	map(0xfe92, 0xfe93).rw(m_frt1, FUNC(h8500_frt_device::frc8_r), FUNC(h8500_frt_device::frc8_w));
	map(0xfe94, 0xfe95).rw(m_frt1, FUNC(h8500_frt_device::ocra8_r), FUNC(h8500_frt_device::ocra8_w));
	map(0xfe96, 0xfe97).rw(m_frt1, FUNC(h8500_frt_device::ocrb8_r), FUNC(h8500_frt_device::ocrb8_w));
	map(0xfe98, 0xfe99).r(m_frt1, FUNC(h8500_frt_device::icr8_r));
	map(0xfea0, 0xfea0).rw(m_frt2, FUNC(h8500_frt_device::tcr_r), FUNC(h8500_frt_device::tcr_w));
	map(0xfea1, 0xfea1).rw(m_frt2, FUNC(h8500_frt_device::tsr_r), FUNC(h8500_frt_device::tsr_w));
	map(0xfea2, 0xfea3).rw(m_frt2, FUNC(h8500_frt_device::frc8_r), FUNC(h8500_frt_device::frc8_w));
	map(0xfea4, 0xfea5).rw(m_frt2, FUNC(h8500_frt_device::ocra8_r), FUNC(h8500_frt_device::ocra8_w));
	map(0xfea6, 0xfea7).rw(m_frt2, FUNC(h8500_frt_device::ocrb8_r), FUNC(h8500_frt_device::ocrb8_w));
	map(0xfea8, 0xfea9).r(m_frt2, FUNC(h8500_frt_device::icr8_r));
	map(0xfeb0, 0xfeb0).rw(m_frt3, FUNC(h8500_frt_device::tcr_r), FUNC(h8500_frt_device::tcr_w));
	map(0xfeb1, 0xfeb1).rw(m_frt3, FUNC(h8500_frt_device::tsr_r), FUNC(h8500_frt_device::tsr_w));
	map(0xfeb2, 0xfeb3).rw(m_frt3, FUNC(h8500_frt_device::frc8_r), FUNC(h8500_frt_device::frc8_w));
	map(0xfeb4, 0xfeb5).rw(m_frt3, FUNC(h8500_frt_device::ocra8_r), FUNC(h8500_frt_device::ocra8_w));
	map(0xfeb6, 0xfeb7).rw(m_frt3, FUNC(h8500_frt_device::ocrb8_r), FUNC(h8500_frt_device::ocrb8_w));
	map(0xfeb8, 0xfeb9).r(m_frt3, FUNC(h8500_frt_device::icr8_r));

	// 8-bit timer
	map(0xfed0, 0xfed0).rw(m_tmr, FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xfed1, 0xfed1).rw(m_tmr, FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xfed2, 0xfed3).rw(m_tmr, FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w));
	map(0xfed4, 0xfed4).rw(m_tmr, FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));

	// A/D converter
	map(0xfee0, 0xfee7).r(m_adc, FUNC(h8_adc_device::addr8_r));
	map(0xfee8, 0xfee8).rw(m_adc, FUNC(h8_adc_device::adcsr_r), FUNC(h8_adc_device::adcsr_w));
	map(0xfee9, 0xfee9).rw(m_adc, FUNC(h8_adc_device::adcr_r), FUNC(h8_adc_device::adcr_w));

	// SCI channel 1 (m_sci[0])
	map(0xfed8, 0xfed8).rw(m_sci[0], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xfed9, 0xfed9).rw(m_sci[0], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xfeda, 0xfeda).rw(m_sci[0], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xfedb, 0xfedb).rw(m_sci[0], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xfedc, 0xfedc).rw(m_sci[0], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xfedd, 0xfedd).r(m_sci[0], FUNC(h8_sci_device::rdr_r));

	// SCI channel 2 (m_sci[1])
	map(0xfef0, 0xfef0).rw(m_sci[1], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xfef1, 0xfef1).rw(m_sci[1], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xfef2, 0xfef2).rw(m_sci[1], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xfef3, 0xfef3).rw(m_sci[1], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xfef4, 0xfef4).rw(m_sci[1], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xfef5, 0xfef5).r(m_sci[1], FUNC(h8_sci_device::rdr_r));

	// SYSCR1/SYSCR2 carry the interrupt pin enables and NMI edge select
	map(0xfefc, 0xfefc).rw(m_intc, FUNC(h8534_intc_device::syscr1_r), FUNC(h8534_intc_device::syscr1_w));
	map(0xfefd, 0xfefd).rw(m_intc, FUNC(h8534_intc_device::syscr2_r), FUNC(h8534_intc_device::syscr2_w));

	// Port 9
	map(0xfefe, 0xfefe).rw(m_port9, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfeff, 0xfeff).rw(m_port9, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));

	// Interrupt controller (H8/534 adds IPRE/IPRF and DTEE/DTEF)
	map(0xff00, 0xff00).rw(m_intc, FUNC(h8500_intc_device::ipra_r), FUNC(h8500_intc_device::ipra_w));
	map(0xff01, 0xff01).rw(m_intc, FUNC(h8500_intc_device::iprb_r), FUNC(h8500_intc_device::iprb_w));
	map(0xff02, 0xff02).rw(m_intc, FUNC(h8500_intc_device::iprc_r), FUNC(h8500_intc_device::iprc_w));
	map(0xff03, 0xff03).rw(m_intc, FUNC(h8500_intc_device::iprd_r), FUNC(h8500_intc_device::iprd_w));
	map(0xff04, 0xff04).rw(m_intc, FUNC(h8500_intc_device::ipre_r), FUNC(h8500_intc_device::ipre_w));
	map(0xff05, 0xff05).rw(m_intc, FUNC(h8500_intc_device::iprf_r), FUNC(h8500_intc_device::iprf_w));
	map(0xff08, 0xff08).rw(m_intc, FUNC(h8500_intc_device::dtea_r), FUNC(h8500_intc_device::dtea_w));
	map(0xff09, 0xff09).rw(m_intc, FUNC(h8500_intc_device::dteb_r), FUNC(h8500_intc_device::dteb_w));
	map(0xff0a, 0xff0a).rw(m_intc, FUNC(h8500_intc_device::dtec_r), FUNC(h8500_intc_device::dtec_w));
	map(0xff0b, 0xff0b).rw(m_intc, FUNC(h8500_intc_device::dted_r), FUNC(h8500_intc_device::dted_w));
	map(0xff0c, 0xff0c).rw(m_intc, FUNC(h8500_intc_device::dtee_r), FUNC(h8500_intc_device::dtee_w));
	map(0xff0d, 0xff0d).rw(m_intc, FUNC(h8500_intc_device::dtef_r), FUNC(h8500_intc_device::dtef_w));

#if 0
	map(0xfec0, 0xfec0).rw(FUNC(h8534_device::pwm1_tcr_r), FUNC(h8534_device::pwm1_tcr_w));
	map(0xfec1, 0xfec1).rw(FUNC(h8534_device::pwm1_dtr_r), FUNC(h8534_device::pwm1_dtr_w));
	map(0xfec2, 0xfec2).rw(FUNC(h8534_device::pwm1_tcnt_r), FUNC(h8534_device::pwm1_tcnt_w));
	map(0xfec4, 0xfec4).rw(FUNC(h8534_device::pwm2_tcr_r), FUNC(h8534_device::pwm2_tcr_w));
	map(0xfec5, 0xfec5).rw(FUNC(h8534_device::pwm2_dtr_r), FUNC(h8534_device::pwm2_dtr_w));
	map(0xfec6, 0xfec6).rw(FUNC(h8534_device::pwm2_tcnt_r), FUNC(h8534_device::pwm2_tcnt_w));
	map(0xfec8, 0xfec8).rw(FUNC(h8534_device::pwm3_tcr_r), FUNC(h8534_device::pwm3_tcr_w));
	map(0xfec9, 0xfec9).rw(FUNC(h8534_device::pwm3_dtr_r), FUNC(h8534_device::pwm3_dtr_w));
	map(0xfeca, 0xfeca).rw(FUNC(h8534_device::pwm3_tcnt_r), FUNC(h8534_device::pwm3_tcnt_w));
	map(0xfeec, 0xfeec).rw(FUNC(h8534_device::wdt_tcsr_r));
	map(0xfeed, 0xfeed).r(FUNC(h8534_device::wdt_tcnt_r), FUNC(h8534_device::wdt_tcsr_w));
	map(0xff10, 0xff10).rw(FUNC(h8534_device::wcr_r), FUNC(h8534_device::wcr_w));
	map(0xff11, 0xff11).rw(FUNC(h8534_device::ramcr_r), FUNC(h8534_device::ramcr_w));
	map(0xff12, 0xff12).r(FUNC(h8534_device::mdcr_r));
	map(0xff13, 0xff13).rw(FUNC(h8534_device::sbycr_r), FUNC(h8534_device::sbycr_w));
	map(0xff14, 0xff14).w(FUNC(h8534_device::wdt_rstcsr_w));
	map(0xff15, 0xff15).r(FUNC(h8534_device::wdt_rstcsr_r));
#endif
}
