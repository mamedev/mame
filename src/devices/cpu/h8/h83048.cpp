// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h83048.h"

DEFINE_DEVICE_TYPE(H83044, h83044_device, "h83044", "Hitachi H8/3044")
DEFINE_DEVICE_TYPE(H83045, h83045_device, "h83045", "Hitachi H8/3045")
DEFINE_DEVICE_TYPE(H83047, h83047_device, "h83047", "Hitachi H8/3047")
DEFINE_DEVICE_TYPE(H83048, h83048_device, "h83048", "Hitachi H8/3048")

h83048_device::h83048_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t start) :
	h8h_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h83048_device::map), this)),
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
	porta(*this, "porta"),
	portb(*this, "portb"),
	timer16(*this, "timer16"),
	timer16_0(*this, "timer16:0"),
	timer16_1(*this, "timer16:1"),
	timer16_2(*this, "timer16:2"),
	timer16_3(*this, "timer16:3"),
	timer16_4(*this, "timer16:4"),
	sci0(*this, "sci0"),
	sci1(*this, "sci1"),
	watchdog(*this, "watchdog"),
	ram_start(start),
	syscr(0)
{
}

h83048_device::h83048_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h83048_device(mconfig, H83048, tag, owner, clock, 0xef10)
{
}

h83044_device::h83044_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h83048_device(mconfig, H83044, tag, owner, clock, 0xf710)
{
}

h83045_device::h83045_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h83048_device(mconfig, H83045, tag, owner, clock, 0xf710)
{
}

h83047_device::h83047_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h83048_device(mconfig, H83047, tag, owner, clock, 0xef10)
{
}

void h83048_device::map(address_map &map)
{
	const offs_t base = mode_a20 ? 0xf0000 : 0xff0000;

	map(base | ram_start, base | 0xff0f).ram();

	map(base | 0xff60, base | 0xff60).rw("timer16", FUNC(h8_timer16_device::tstr_r), FUNC(h8_timer16_device::tstr_w));
	map(base | 0xff61, base | 0xff61).rw("timer16", FUNC(h8_timer16_device::tsyr_r), FUNC(h8_timer16_device::tsyr_w));
	map(base | 0xff62, base | 0xff62).rw("timer16", FUNC(h8_timer16_device::tmdr_r), FUNC(h8_timer16_device::tmdr_w));
	map(base | 0xff63, base | 0xff63).rw("timer16", FUNC(h8_timer16_device::tfcr_r), FUNC(h8_timer16_device::tfcr_w));
	map(base | 0xff64, base | 0xff64).rw("timer16:0", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(base | 0xff65, base | 0xff65).rw("timer16:0", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(base | 0xff66, base | 0xff66).rw("timer16:0", FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(base | 0xff67, base | 0xff67).rw("timer16:0", FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(base | 0xff68, base | 0xff69).rw("timer16:0", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(base | 0xff6a, base | 0xff6d).rw("timer16:0", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(base | 0xff6e, base | 0xff6e).rw("timer16:1", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(base | 0xff6f, base | 0xff6f).rw("timer16:1", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(base | 0xff70, base | 0xff70).rw("timer16:1", FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(base | 0xff71, base | 0xff71).rw("timer16:1", FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(base | 0xff72, base | 0xff73).rw("timer16:1", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(base | 0xff74, base | 0xff77).rw("timer16:1", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(base | 0xff78, base | 0xff78).rw("timer16:2", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(base | 0xff79, base | 0xff79).rw("timer16:2", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(base | 0xff7a, base | 0xff7a).rw("timer16:2", FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(base | 0xff7b, base | 0xff7b).rw("timer16:2", FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(base | 0xff7c, base | 0xff7d).rw("timer16:2", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(base | 0xff7e, base | 0xff81).rw("timer16:2", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(base | 0xff82, base | 0xff82).rw("timer16:3", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(base | 0xff83, base | 0xff83).rw("timer16:3", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(base | 0xff84, base | 0xff84).rw("timer16:3", FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(base | 0xff85, base | 0xff85).rw("timer16:3", FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(base | 0xff86, base | 0xff87).rw("timer16:3", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(base | 0xff88, base | 0xff8b).rw("timer16:3", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(base | 0xff8c, base | 0xff8f).rw("timer16:3", FUNC(h8_timer16_channel_device::tbr_r), FUNC(h8_timer16_channel_device::tbr_w));
	map(base | 0xff90, base | 0xff90).rw("timer16", FUNC(h8_timer16_device::toer_r), FUNC(h8_timer16_device::toer_w));
	map(base | 0xff91, base | 0xff91).rw("timer16", FUNC(h8_timer16_device::tocr_r), FUNC(h8_timer16_device::tocr_w));
	map(base | 0xff92, base | 0xff92).rw("timer16:4", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(base | 0xff93, base | 0xff93).rw("timer16:4", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(base | 0xff94, base | 0xff94).rw("timer16:4", FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(base | 0xff95, base | 0xff95).rw("timer16:4", FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(base | 0xff96, base | 0xff97).rw("timer16:4", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(base | 0xff98, base | 0xff9b).rw("timer16:4", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(base | 0xff9c, base | 0xff9f).rw("timer16:4", FUNC(h8_timer16_channel_device::tbr_r), FUNC(h8_timer16_channel_device::tbr_w));

	map(base | 0xffa8, base | 0xffa9).rw("watchdog", FUNC(h8_watchdog_device::wd_r), FUNC(h8_watchdog_device::wd_w));
	map(base | 0xffaa, base | 0xffab).rw("watchdog", FUNC(h8_watchdog_device::rst_r), FUNC(h8_watchdog_device::rst_w));

	map(base | 0xffb0, base | 0xffb0).rw("sci0", FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(base | 0xffb1, base | 0xffb1).rw("sci0", FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(base | 0xffb2, base | 0xffb2).rw("sci0", FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(base | 0xffb3, base | 0xffb3).rw("sci0", FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(base | 0xffb4, base | 0xffb4).rw("sci0", FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(base | 0xffb5, base | 0xffb5).r("sci0", FUNC(h8_sci_device::rdr_r));
	map(base | 0xffb8, base | 0xffb8).rw("sci1", FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(base | 0xffb9, base | 0xffb9).rw("sci1", FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(base | 0xffba, base | 0xffba).rw("sci1", FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(base | 0xffbb, base | 0xffbb).rw("sci1", FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(base | 0xffbc, base | 0xffbc).rw("sci1", FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(base | 0xffbd, base | 0xffbd).r("sci1", FUNC(h8_sci_device::rdr_r));
	map(base | 0xffc0, base | 0xffc0).w("port1", FUNC(h8_port_device::ddr_w));
	map(base | 0xffc1, base | 0xffc1).w("port2", FUNC(h8_port_device::ddr_w));
	map(base | 0xffc2, base | 0xffc2).rw("port1", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffc3, base | 0xffc3).rw("port2", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffc4, base | 0xffc4).w("port3", FUNC(h8_port_device::ddr_w));
	map(base | 0xffc5, base | 0xffc5).w("port4", FUNC(h8_port_device::ddr_w));
	map(base | 0xffc6, base | 0xffc6).rw("port3", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffc7, base | 0xffc7).rw("port4", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffc8, base | 0xffc8).w("port5", FUNC(h8_port_device::ddr_w));
	map(base | 0xffc9, base | 0xffc9).w("port6", FUNC(h8_port_device::ddr_w));
	map(base | 0xffca, base | 0xffca).rw("port5", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffcb, base | 0xffcb).rw("port6", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffcd, base | 0xffcd).w("port8", FUNC(h8_port_device::ddr_w));
	map(base | 0xffce, base | 0xffce).rw("port7", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffcf, base | 0xffcf).rw("port8", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffd0, base | 0xffd0).w("port9", FUNC(h8_port_device::ddr_w));
	map(base | 0xffd1, base | 0xffd1).w("porta", FUNC(h8_port_device::ddr_w));
	map(base | 0xffd2, base | 0xffd2).rw("port9", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffd3, base | 0xffd3).rw("porta", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffd4, base | 0xffd4).w("portb", FUNC(h8_port_device::ddr_w));
	map(base | 0xffd6, base | 0xffd6).rw("portb", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffd8, base | 0xffd8).rw("port2", FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(base | 0xffda, base | 0xffda).rw("port4", FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(base | 0xffdb, base | 0xffdb).rw("port5", FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));

	map(base | 0xffe0, base | 0xffe7).r("adc", FUNC(h8_adc_device::addr8_r));
	map(base | 0xffe8, base | 0xffe8).rw("adc", FUNC(h8_adc_device::adcsr_r), FUNC(h8_adc_device::adcsr_w));
	map(base | 0xffe9, base | 0xffe9).rw("adc", FUNC(h8_adc_device::adcr_r), FUNC(h8_adc_device::adcr_w));

	map(base | 0xfff2, base | 0xfff2).rw(FUNC(h83048_device::syscr_r), FUNC(h83048_device::syscr_w));

	map(base | 0xfff4, base | 0xfff4).rw("intc", FUNC(h8h_intc_device::iscr_r), FUNC(h8h_intc_device::iscr_w));
	map(base | 0xfff5, base | 0xfff5).rw("intc", FUNC(h8h_intc_device::ier_r), FUNC(h8h_intc_device::ier_w));
	map(base | 0xfff6, base | 0xfff6).rw("intc", FUNC(h8h_intc_device::isr_r), FUNC(h8h_intc_device::isr_w));
	map(base | 0xfff8, base | 0xfff9).rw("intc", FUNC(h8h_intc_device::icr_r), FUNC(h8h_intc_device::icr_w));
}

void h83048_device::device_add_mconfig(machine_config &config)
{
	H8H_INTC(config, "intc");
	H8_ADC_3337(config, "adc", "intc", 60);
	H8_PORT(config, "port1", h8_device::PORT_1, 0x00, 0x00);
	H8_PORT(config, "port2", h8_device::PORT_2, 0x00, 0x00);
	H8_PORT(config, "port3", h8_device::PORT_3, 0x00, 0x00);
	H8_PORT(config, "port4", h8_device::PORT_4, 0x00, 0x00);
	H8_PORT(config, "port5", h8_device::PORT_5, 0xf0, 0xf0);
	H8_PORT(config, "port6", h8_device::PORT_6, 0x80, 0x80);
	H8_PORT(config, "port7", h8_device::PORT_7, 0x00, 0x00);
	H8_PORT(config, "port8", h8_device::PORT_8, 0xe0, 0xe0);
	H8_PORT(config, "port9", h8_device::PORT_9, 0xc0, 0xc0);
	H8_PORT(config, "porta", h8_device::PORT_A, 0x00, 0x00);
	H8_PORT(config, "portb", h8_device::PORT_B, 0x00, 0x00);
	H8_TIMER16(config, "timer16", 5, 0xe0);
	H8H_TIMER16_CHANNEL(config, "timer16:0", 2, 2, "intc", 24);
	H8H_TIMER16_CHANNEL(config, "timer16:1", 2, 2, "intc", 28);
	H8H_TIMER16_CHANNEL(config, "timer16:2", 2, 2, "intc", 32);
	H8H_TIMER16_CHANNEL(config, "timer16:3", 2, 2, "intc", 36);
	H8H_TIMER16_CHANNEL(config, "timer16:4", 2, 2, "intc", 40);
	H8_SCI(config, "sci0", "intc", 52, 53, 54, 55);
	H8_SCI(config, "sci1", "intc", 56, 57, 58, 59);
	H8_WATCHDOG(config, "watchdog", "intc", 20, h8_watchdog_device::H);
}

void h83048_device::execute_set_input(int inputnum, int state)
{
	intc->set_input(inputnum, state);
}

int h83048_device::trapa_setup()
{
	if(syscr & 0x08)
		CCR |= F_I;
	else
		CCR |= F_I|F_UI;
	return 8;
}

void h83048_device::irq_setup()
{
	if(syscr & 0x08)
		CCR |= F_I;
	else
		CCR |= F_I|F_UI;
}

void h83048_device::update_irq_filter()
{
	switch(syscr & 0x08) {
	case 0x00:
		if((CCR & (F_I|F_UI)) == (F_I|F_UI))
			intc->set_filter(2, -1);
		else if(CCR & F_I)
			intc->set_filter(1, -1);
		else
			intc->set_filter(0, -1);
		break;
	case 0x08:
		if(CCR & F_I)
			intc->set_filter(2, -1);
		else
			intc->set_filter(0, -1);
		break;
	}
}

void h83048_device::interrupt_taken()
{
	standard_irq_callback(intc->interrupt_taken(taken_irq_vector));
}

void h83048_device::internal_update(uint64_t current_time)
{
	uint64_t event_time = 0;

	add_event(event_time, adc->internal_update(current_time));
	add_event(event_time, sci0->internal_update(current_time));
	add_event(event_time, sci1->internal_update(current_time));
	add_event(event_time, timer16_0->internal_update(current_time));
	add_event(event_time, timer16_1->internal_update(current_time));
	add_event(event_time, timer16_2->internal_update(current_time));
	add_event(event_time, timer16_3->internal_update(current_time));
	add_event(event_time, timer16_4->internal_update(current_time));
	add_event(event_time, watchdog->internal_update(current_time));

	recompute_bcount(event_time);
}

void h83048_device::device_start()
{
	h8h_device::device_start();
}

void h83048_device::device_reset()
{
	h8h_device::device_reset();
	syscr = 0x0b;
}

READ8_MEMBER(h83048_device::syscr_r)
{
	return syscr;
}

WRITE8_MEMBER(h83048_device::syscr_w)
{
	syscr = data;
	update_irq_filter();
	logerror("syscr = %02x\n", data);
}
