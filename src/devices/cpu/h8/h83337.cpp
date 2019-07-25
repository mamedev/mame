// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h83337.h"

DEFINE_DEVICE_TYPE(H83334, h83334_device, "h83334", "Hitachi H8/3334")
DEFINE_DEVICE_TYPE(H83336, h83336_device, "h83336", "Hitachi H8/3336")
DEFINE_DEVICE_TYPE(H83337, h83337_device, "h83337", "Hitachi H8/3337")


h83337_device::h83337_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t start) :
	h8_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h83337_device::map), this)),
	intc(*this, "intc"),
	adc(*this, "adc"),
	port1(*this, "port1"),
	port2(*this, "port2"),
	port3(*this, "port3"),
	port4(*this, "port4"),
	port5(*this, "port5"),
	port6(*this, "port6"),
	port7(*this, "port7"),
	port8(*this, "port8"),
	port9(*this, "port9"),
	timer8_0(*this, "timer8_0"),
	timer8_1(*this, "timer8_1"),
	timer16(*this, "timer16"),
	timer16_0(*this, "timer16:0"),
	sci0(*this, "sci0"),
	sci1(*this, "sci1"),
	watchdog(*this, "watchdog"),
	syscr(0),
	ram_start(start)
{
}

h83337_device::h83337_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h83337_device(mconfig, H83337, tag, owner, clock, 0xf780)
{
}

h83334_device::h83334_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h83337_device(mconfig, H83334, tag, owner, clock, 0xfb80)
{
}

h83336_device::h83336_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h83337_device(mconfig, H83336, tag, owner, clock, 0xf780)
{
}

void h83337_device::map(address_map &map)
{
	map(ram_start, 0xff7f).ram();

	map(0xff88, 0xff88).rw("sci1", FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xff89, 0xff89).rw("sci1", FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xff8a, 0xff8a).rw("sci1", FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xff8b, 0xff8b).rw("sci1", FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xff8c, 0xff8c).rw("sci1", FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xff8d, 0xff8d).r("sci1", FUNC(h8_sci_device::rdr_r));
	map(0xff90, 0xff90).rw("timer16:0", FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(0xff91, 0xff91).rw("timer16:0", FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(0xff92, 0xff93).rw("timer16:0", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
//  AM_RANGE(0xff94, 0xff95) AM_DEVREADWRITE( "timer16:0", h8_timer16_channel_device, ocr_r,   ocr_w          )
	map(0xff96, 0xff96).rw("timer16:0", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
//  AM_RANGE(0xff96, 0xff97) AM_DEVREADWRITE8("timer16:0", h8_timer16_channel_device, tocr_r,  tocr_w,  0x00ff)
	map(0xff98, 0xff9f).r("timer16:0", FUNC(h8_timer16_channel_device::tgr_r));

	map(0xffa8, 0xffa9).rw("watchdog", FUNC(h8_watchdog_device::wd_r), FUNC(h8_watchdog_device::wd_w));
	map(0xffac, 0xffac).rw("port1", FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffad, 0xffad).rw("port2", FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffae, 0xffae).rw("port3", FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffb0, 0xffb0).w("port1", FUNC(h8_port_device::ddr_w));
	map(0xffb1, 0xffb1).w("port2", FUNC(h8_port_device::ddr_w));
	map(0xffb2, 0xffb2).rw("port1", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb3, 0xffb3).rw("port2", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb4, 0xffb4).w("port3", FUNC(h8_port_device::ddr_w));
	map(0xffb5, 0xffb5).w("port4", FUNC(h8_port_device::ddr_w));
	map(0xffb6, 0xffb6).rw("port3", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb7, 0xffb7).rw("port4", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffb8, 0xffb8).w("port5", FUNC(h8_port_device::ddr_w));
	map(0xffb9, 0xffb9).w("port6", FUNC(h8_port_device::ddr_w));
	map(0xffba, 0xffba).rw("port5", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffbb, 0xffbb).rw("port6", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffbd, 0xffbd).w("port8", FUNC(h8_port_device::ddr_w));
	map(0xffbe, 0xffbe).rw("port7", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffbf, 0xffbf).rw("port8", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffc0, 0xffc0).w("port9", FUNC(h8_port_device::ddr_w));
	map(0xffc1, 0xffc1).rw("port9", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffc2, 0xffc2).rw(FUNC(h83337_device::wscr_r), FUNC(h83337_device::wscr_w));
	map(0xffc3, 0xffc3).rw(FUNC(h83337_device::stcr_r), FUNC(h83337_device::stcr_w));
	map(0xffc4, 0xffc4).rw(FUNC(h83337_device::syscr_r), FUNC(h83337_device::syscr_w));
	map(0xffc5, 0xffc5).rw(FUNC(h83337_device::mdcr_r), FUNC(h83337_device::mdcr_w));
	map(0xffc6, 0xffc6).rw("intc", FUNC(h8_intc_device::iscr_r), FUNC(h8_intc_device::iscr_w));
	map(0xffc7, 0xffc7).rw("intc", FUNC(h8_intc_device::ier_r), FUNC(h8_intc_device::ier_w));
	map(0xffc8, 0xffc8).rw("timer8_0", FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffc9, 0xffc9).rw("timer8_0", FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffca, 0xffcb).rw("timer8_0", FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w));
	map(0xffcc, 0xffcc).rw("timer8_0", FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(0xffd0, 0xffd0).rw("timer8_1", FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffd1, 0xffd1).rw("timer8_1", FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffd2, 0xffd3).rw("timer8_1", FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w));
	map(0xffd4, 0xffd4).rw("timer8_1", FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(0xffd8, 0xffd8).rw("sci0", FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffd9, 0xffd9).rw("sci0", FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffda, 0xffda).rw("sci0", FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffdb, 0xffdb).rw("sci0", FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffdc, 0xffdc).rw("sci0", FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffdd, 0xffdd).r("sci0", FUNC(h8_sci_device::rdr_r));

	map(0xffe0, 0xffe7).r("adc", FUNC(h8_adc_device::addr8_r));
	map(0xffe8, 0xffe8).rw("adc", FUNC(h8_adc_device::adcsr_r), FUNC(h8_adc_device::adcsr_w));
	map(0xffe9, 0xffe9).rw("adc", FUNC(h8_adc_device::adcr_r), FUNC(h8_adc_device::adcr_w));

	map(0xfff2, 0xfff2).rw("port6", FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
}

void h83337_device::device_add_mconfig(machine_config &config)
{
	H8_INTC(config, "intc");
	H8_ADC_3337(config, "adc", "intc", 35);
	H8_PORT(config, "port1", h8_device::PORT_1, 0x00, 0x00);
	H8_PORT(config, "port2", h8_device::PORT_2, 0x00, 0x00);
	H8_PORT(config, "port3", h8_device::PORT_3, 0x00, 0x00);
	H8_PORT(config, "port4", h8_device::PORT_4, 0x00, 0x00);
	H8_PORT(config, "port5", h8_device::PORT_5, 0xf8, 0xf8);
	H8_PORT(config, "port6", h8_device::PORT_6, 0x00, 0x00);
	H8_PORT(config, "port7", h8_device::PORT_7, 0x00, 0x00);
	H8_PORT(config, "port8", h8_device::PORT_8, 0x80, 0x80);
	H8_PORT(config, "port9", h8_device::PORT_9, 0x00, 0x00);
	H8_TIMER8_CHANNEL(config, "timer8_0", "intc", 19, 20, 21, 8, 2, 64, 32, 1024, 256);
	H8_TIMER8_CHANNEL(config, "timer8_1", "intc", 22, 23, 24, 8, 2, 64, 128, 1024, 2048);
	H8_TIMER16(config, "timer16", 1, 0xff);
	H8_TIMER16_CHANNEL(config, "timer16:0", 4, 0, "intc", 32);
	H8_SCI(config, "sci0", "intc", 27, 28, 29, 30);
	H8_SCI(config, "sci1", "intc", 31, 32, 33, 34);
	H8_WATCHDOG(config, "watchdog", "intc", 36, h8_watchdog_device::B);
}

void h83337_device::execute_set_input(int inputnum, int state)
{
	intc->set_input(inputnum, state);
}

void h83337_device::irq_setup()
{
	CCR |= F_I;
}

void h83337_device::update_irq_filter()
{
	if(CCR & F_I)
		intc->set_filter(2, -1);
	else
		intc->set_filter(0, -1);
}

void h83337_device::interrupt_taken()
{
	standard_irq_callback(intc->interrupt_taken(taken_irq_vector));
}

void h83337_device::internal_update(uint64_t current_time)
{
	uint64_t event_time = 0;

	add_event(event_time, adc->internal_update(current_time));
	add_event(event_time, sci0->internal_update(current_time));
	add_event(event_time, sci1->internal_update(current_time));
	add_event(event_time, timer8_0->internal_update(current_time));
	add_event(event_time, timer8_1->internal_update(current_time));
	add_event(event_time, timer16_0->internal_update(current_time));
	add_event(event_time, watchdog->internal_update(current_time));

	recompute_bcount(event_time);
}

void h83337_device::device_start()
{
	h8_device::device_start();
}

void h83337_device::device_reset()
{
	h8_device::device_reset();
	syscr = 0x09;
}

READ8_MEMBER(h83337_device::syscr_r)
{
	return syscr;
}

WRITE8_MEMBER(h83337_device::syscr_w)
{
	syscr = data;
	logerror("syscr = %02x\n", data);
}

READ8_MEMBER(h83337_device::wscr_r)
{
	return 0x00;
}

WRITE8_MEMBER(h83337_device::wscr_w)
{
	logerror("wscr = %02x\n", data);
}

READ8_MEMBER(h83337_device::stcr_r)
{
	return 0x00;
}

WRITE8_MEMBER(h83337_device::stcr_w)
{
	logerror("stcr = %02x\n", data);
	timer8_0->set_extra_clock_bit(data & 0x01);
	timer8_1->set_extra_clock_bit(data & 0x02);
}

READ8_MEMBER(h83337_device::mdcr_r)
{
	return 0x00;
}

WRITE8_MEMBER(h83337_device::mdcr_w)
{
	logerror("mdcr = %02x\n", data);
}
