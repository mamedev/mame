// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h83337.cpp

    H8-3337 family emulation

    TODO:
    - 16-bit timer module is different from how it's implemented in h8_timer16.cpp
    - PWM timer module
    - Host Interface module
    - finish WSCR emulation, CKDBL flag would need support in peripherals
    - finish STCR emulation
    - finish SYSCR emulation

***************************************************************************/

#include "emu.h"
#include "h83337.h"

DEFINE_DEVICE_TYPE(H83334, h83334_device, "h83334", "Hitachi H8/3334")
DEFINE_DEVICE_TYPE(H83336, h83336_device, "h83336", "Hitachi H8/3336")
DEFINE_DEVICE_TYPE(H83337, h83337_device, "h83337", "Hitachi H8/3337")


h83337_device::h83337_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 start) :
	h8_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h83337_device::map), this)),
	m_intc(*this, "intc"),
	m_adc(*this, "adc"),
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
	m_timer8_1(*this, "timer8_1"),
	m_timer16(*this, "timer16"),
	m_timer16_0(*this, "timer16:0"),
	m_watchdog(*this, "watchdog"),
	m_ram_start(start)
{
}

h83337_device::h83337_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h83337_device(mconfig, H83337, tag, owner, clock, 0xf780)
{
}

h83334_device::h83334_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h83337_device(mconfig, H83334, tag, owner, clock, 0xfb80)
{
}

h83336_device::h83336_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h83337_device(mconfig, H83336, tag, owner, clock, 0xf780)
{
}

void h83337_device::map(address_map &map)
{
	map(m_ram_start, 0xff7f).ram();

	map(0xff88, 0xff88).rw(m_sci[1], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xff89, 0xff89).rw(m_sci[1], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xff8a, 0xff8a).rw(m_sci[1], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xff8b, 0xff8b).rw(m_sci[1], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xff8c, 0xff8c).rw(m_sci[1], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xff8d, 0xff8d).r(m_sci[1], FUNC(h8_sci_device::rdr_r));

	map(0xff90, 0xff90).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(0xff91, 0xff91).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(0xff92, 0xff93).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
//  map(0xff94, 0xff95).rw(m_timer16_0, FUNC(h8_timer16_channel_device::ocr_r, FUNC(h8_timer16_channel_device:ocr_w));
	map(0xff96, 0xff96).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
//  map(0xff97, 0xff97).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tocr_r, FUNC(h8_timer16_channel_device:tocr_w));
	map(0xff98, 0xff9f).r(m_timer16_0, FUNC(h8_timer16_channel_device::tgr_r));

	map(0xffa8, 0xffa9).rw(m_watchdog, FUNC(h8_watchdog_device::wd_r), FUNC(h8_watchdog_device::wd_w));

	map(0xffac, 0xffac).rw(m_port1, FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffad, 0xffad).rw(m_port2, FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffae, 0xffae).rw(m_port3, FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffb0, 0xffb0).rw(m_port1, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xffb1, 0xffb1).rw(m_port2, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xffb2, 0xffb2).rw(m_port1, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb3, 0xffb3).rw(m_port2, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb4, 0xffb4).rw(m_port3, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xffb5, 0xffb5).rw(m_port4, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xffb6, 0xffb6).rw(m_port3, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb7, 0xffb7).rw(m_port4, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb8, 0xffb8).rw(m_port5, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xffb9, 0xffb9).rw(m_port6, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xffba, 0xffba).rw(m_port5, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffbb, 0xffbb).rw(m_port6, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffbd, 0xffbd).rw(m_port8, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xffbe, 0xffbe).rw(m_port7, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffbf, 0xffbf).rw(m_port8, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffc0, 0xffc0).rw(m_port9, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xffc1, 0xffc1).rw(m_port9, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));

	map(0xffc2, 0xffc2).rw(FUNC(h83337_device::wscr_r), FUNC(h83337_device::wscr_w));
	map(0xffc3, 0xffc3).rw(FUNC(h83337_device::stcr_r), FUNC(h83337_device::stcr_w));
	map(0xffc4, 0xffc4).rw(FUNC(h83337_device::syscr_r), FUNC(h83337_device::syscr_w));
	map(0xffc5, 0xffc5).rw(FUNC(h83337_device::mdcr_r), FUNC(h83337_device::mdcr_w));
	map(0xffc6, 0xffc6).rw(m_intc, FUNC(h8_intc_device::iscr_r), FUNC(h8_intc_device::iscr_w));
	map(0xffc7, 0xffc7).rw(m_intc, FUNC(h8_intc_device::ier_r), FUNC(h8_intc_device::ier_w));

	map(0xffc8, 0xffc8).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffc9, 0xffc9).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffca, 0xffcb).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w));
	map(0xffcc, 0xffcc).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(0xffd0, 0xffd0).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffd1, 0xffd1).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffd2, 0xffd3).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w));
	map(0xffd4, 0xffd4).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));

	map(0xffd8, 0xffd8).rw(m_sci[0], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffd9, 0xffd9).rw(m_sci[0], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffda, 0xffda).rw(m_sci[0], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffdb, 0xffdb).rw(m_sci[0], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffdc, 0xffdc).rw(m_sci[0], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffdd, 0xffdd).r(m_sci[0], FUNC(h8_sci_device::rdr_r));

	map(0xffe0, 0xffe7).r(m_adc, FUNC(h8_adc_device::addr8_r));
	map(0xffe8, 0xffe8).rw(m_adc, FUNC(h8_adc_device::adcsr_r), FUNC(h8_adc_device::adcsr_w));
	map(0xffe9, 0xffe9).rw(m_adc, FUNC(h8_adc_device::adcr_r), FUNC(h8_adc_device::adcr_w));

	map(0xfff2, 0xfff2).rw(m_port6, FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
}

void h83337_device::device_add_mconfig(machine_config &config)
{
	H8_INTC(config, m_intc, *this);
	H8_ADC_3337(config, m_adc, *this, m_intc, 35);
	H8_PORT(config, m_port1, *this, h8_device::PORT_1, 0x00, 0x00);
	H8_PORT(config, m_port2, *this, h8_device::PORT_2, 0x00, 0x00);
	H8_PORT(config, m_port3, *this, h8_device::PORT_3, 0x00, 0x00);
	H8_PORT(config, m_port4, *this, h8_device::PORT_4, 0x00, 0x00);
	H8_PORT(config, m_port5, *this, h8_device::PORT_5, 0xf8, 0xf8);
	H8_PORT(config, m_port6, *this, h8_device::PORT_6, 0x00, 0x00);
	H8_PORT(config, m_port7, *this, h8_device::PORT_7, 0x00, 0x00);
	H8_PORT(config, m_port8, *this, h8_device::PORT_8, 0x80, 0x80);
	H8_PORT(config, m_port9, *this, h8_device::PORT_9, 0x00, 0x00);
	H8_TIMER8_CHANNEL(config, m_timer8_0, *this, m_intc, 19, 20, 21, 8, 2, 64, 32, 1024, 256);
	H8_TIMER8_CHANNEL(config, m_timer8_1, *this, m_intc, 22, 23, 24, 8, 2, 64, 128, 1024, 2048);
	H8_TIMER16(config, m_timer16, *this, 1, 0xff);
	H8_TIMER16_CHANNEL(config, m_timer16_0, *this, 4, 0, m_intc, 32);
	H8_SCI(config, m_sci[0], 0, *this, m_intc, 27, 28, 29, 30);
	H8_SCI(config, m_sci[1], 1, *this, m_intc, 31, 32, 33, 34);
	H8_WATCHDOG(config, m_watchdog, *this, m_intc, 36, h8_watchdog_device::B);
}

void h83337_device::execute_set_input(int inputnum, int state)
{
	m_intc->set_input(inputnum, state);
}

void h83337_device::irq_setup()
{
	m_CCR |= F_I;
}

void h83337_device::update_irq_filter()
{
	if(m_CCR & F_I)
		m_intc->set_filter(2, -1);
	else
		m_intc->set_filter(0, -1);
}

void h83337_device::interrupt_taken()
{
	standard_irq_callback(m_intc->interrupt_taken(m_taken_irq_vector), m_NPC);
}

void h83337_device::internal_update(u64 current_time)
{
	u64 event_time = 0;

	add_event(event_time, m_adc->internal_update(current_time));
	add_event(event_time, m_sci[0]->internal_update(current_time));
	add_event(event_time, m_sci[1]->internal_update(current_time));
	add_event(event_time, m_timer8_0->internal_update(current_time));
	add_event(event_time, m_timer8_1->internal_update(current_time));
	add_event(event_time, m_timer16_0->internal_update(current_time));
	add_event(event_time, m_watchdog->internal_update(current_time));

	recompute_bcount(event_time);
}

void h83337_device::notify_standby(int state)
{
	m_adc->notify_standby(state);
	m_sci[0]->notify_standby(state);
	m_sci[1]->notify_standby(state);
	m_timer8_0->notify_standby(state);
	m_timer8_1->notify_standby(state);
	m_timer16_0->notify_standby(state);
	m_watchdog->notify_standby(state);
}

void h83337_device::device_start()
{
	h8_device::device_start();

	m_wscr = 0;
	m_stcr = 0;
	m_syscr = 0;

	save_item(NAME(m_wscr));
	save_item(NAME(m_stcr));
	save_item(NAME(m_syscr));
}

void h83337_device::device_reset()
{
	h8_device::device_reset();

	m_wscr = 0x08;
	m_stcr = 0x00;
	m_syscr = 0x09;
}

u8 h83337_device::syscr_r()
{
	return m_syscr;
}

void h83337_device::syscr_w(u8 data)
{
	logerror("syscr = %02x\n", data);
	m_syscr = (m_syscr & 0x08) | (data & 0xf7);
}

u8 h83337_device::wscr_r()
{
	return m_wscr;
}

void h83337_device::wscr_w(u8 data)
{
	logerror("wscr = %02x\n", data);
	m_wscr = data;
}

u8 h83337_device::stcr_r()
{
	return m_stcr;
}

void h83337_device::stcr_w(u8 data)
{
	logerror("stcr = %02x\n", data);

	// ICKS0/1
	m_timer8_0->set_extra_clock_bit(BIT(data, 0));
	m_timer8_1->set_extra_clock_bit(BIT(data, 1));

	m_stcr = data;
}

u8 h83337_device::mdcr_r()
{
	return 0x00;
}

void h83337_device::mdcr_w(u8 data)
{
	logerror("mdcr = %02x\n", data);
}
