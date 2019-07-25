// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h83008.h"

DEFINE_DEVICE_TYPE(H83008, h83008_device, "h83008", "Hitachi H8/3008")

h83008_device::h83008_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8h_device(mconfig, H83008, tag, owner, clock, address_map_constructor(FUNC(h83008_device::map), this)),
	intc(*this, "intc"),
	adc(*this, "adc"),
	port4(*this, "port4"),
	port6(*this, "port6"),
	port7(*this, "port7"),
	port8(*this, "port8"),
	port9(*this, "port9"),
	porta(*this, "porta"),
	portb(*this, "portb"),
	timer8_0(*this, "timer8_0"),
	timer8_1(*this, "timer8_1"),
	timer8_2(*this, "timer8_2"),
	timer8_3(*this, "timer8_3"),
	timer16(*this, "timer16"),
	timer16_0(*this, "timer16:0"),
	timer16_1(*this, "timer16:1"),
	timer16_2(*this, "timer16:2"),
	sci0(*this, "sci0"),
	sci1(*this, "sci1"),
	watchdog(*this, "watchdog"),
	syscr(0)
{
}

void h83008_device::map(address_map &map)
{
	const offs_t base = mode_a20 ? 0 : 0xf00000;

	map(base | 0xee003, base | 0xee003).w("port4", FUNC(h8_port_device::ddr_w));
	map(base | 0xee005, base | 0xee005).w("port6", FUNC(h8_port_device::ddr_w));
	map(base | 0xee007, base | 0xee007).w("port8", FUNC(h8_port_device::ddr_w));
	map(base | 0xee008, base | 0xee008).w("port9", FUNC(h8_port_device::ddr_w));
	map(base | 0xee009, base | 0xee009).w("porta", FUNC(h8_port_device::ddr_w));
	map(base | 0xee00a, base | 0xee00a).w("portb", FUNC(h8_port_device::ddr_w));

	map(base | 0xee012, base | 0xee012).rw(FUNC(h83008_device::syscr_r), FUNC(h83008_device::syscr_w));
	map(base | 0xee014, base | 0xee014).rw("intc", FUNC(h8h_intc_device::iscr_r), FUNC(h8h_intc_device::iscr_w));
	map(base | 0xee015, base | 0xee015).rw("intc", FUNC(h8h_intc_device::ier_r), FUNC(h8h_intc_device::ier_w));
	map(base | 0xee016, base | 0xee016).rw("intc", FUNC(h8h_intc_device::isr_r), FUNC(h8h_intc_device::isr_w));
	map(base | 0xee018, base | 0xee019).rw("intc", FUNC(h8h_intc_device::icr_r), FUNC(h8h_intc_device::icr_w));

	map(base | 0xee03e, base | 0xee03e).rw("port4", FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));

	map(base | 0xfef20, base | 0xfff1f).ram();

	map(base | 0xfff60, base | 0xfff60).rw("timer16", FUNC(h8_timer16_device::tstr_r), FUNC(h8_timer16_device::tstr_w));
	map(base | 0xfff61, base | 0xfff61).rw("timer16", FUNC(h8_timer16_device::tsyr_r), FUNC(h8_timer16_device::tsyr_w));
	map(base | 0xfff62, base | 0xfff62).rw("timer16", FUNC(h8_timer16_device::tmdr_r), FUNC(h8_timer16_device::tmdr_w));
	map(base | 0xfff63, base | 0xfff63).w("timer16", FUNC(h8_timer16_device::tolr_w));
	map(base | 0xfff64, base | 0xfff65).rw("timer16", FUNC(h8_timer16_device::tisr_r), FUNC(h8_timer16_device::tisr_w));
	map(base | 0xfff66, base | 0xfff66).rw("timer16", FUNC(h8_timer16_device::tisrc_r), FUNC(h8_timer16_device::tisrc_w));
	map(base | 0xfff68, base | 0xfff68).rw("timer16:0", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(base | 0xfff69, base | 0xfff69).rw("timer16:0", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(base | 0xfff6a, base | 0xfff6b).rw("timer16:0", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(base | 0xfff6c, base | 0xfff6f).rw("timer16:0", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(base | 0xfff70, base | 0xfff70).rw("timer16:1", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(base | 0xfff71, base | 0xfff71).rw("timer16:1", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(base | 0xfff72, base | 0xfff73).rw("timer16:1", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(base | 0xfff74, base | 0xfff77).rw("timer16:1", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(base | 0xfff78, base | 0xfff78).rw("timer16:2", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(base | 0xfff79, base | 0xfff79).rw("timer16:2", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(base | 0xfff7a, base | 0xfff7b).rw("timer16:2", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(base | 0xfff7c, base | 0xfff7f).rw("timer16:2", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(base | 0xfff80, base | 0xfff80).rw("timer8_0", FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(base | 0xfff81, base | 0xfff81).rw("timer8_1", FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(base | 0xfff82, base | 0xfff82).rw("timer8_0", FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(base | 0xfff83, base | 0xfff83).rw("timer8_1", FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(base | 0xfff84, base | 0xfff87).rw("timer8_0", FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w)).umask16(0xff00);
	map(base | 0xfff84, base | 0xfff87).rw("timer8_1", FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w)).umask16(0x00ff);
	map(base | 0xfff88, base | 0xfff88).rw("timer8_0", FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(base | 0xfff89, base | 0xfff89).rw("timer8_1", FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(base | 0xfff8c, base | 0xfff8d).rw("watchdog", FUNC(h8_watchdog_device::wd_r), FUNC(h8_watchdog_device::wd_w));
	map(base | 0xfff8e, base | 0xfff8f).rw("watchdog", FUNC(h8_watchdog_device::rst_r), FUNC(h8_watchdog_device::rst_w));
	map(base | 0xfff90, base | 0xfff90).rw("timer8_2", FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(base | 0xfff91, base | 0xfff91).rw("timer8_3", FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(base | 0xfff92, base | 0xfff92).rw("timer8_2", FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(base | 0xfff93, base | 0xfff93).rw("timer8_3", FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(base | 0xfff94, base | 0xfff97).rw("timer8_2", FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w)).umask16(0xff00);
	map(base | 0xfff94, base | 0xfff97).rw("timer8_3", FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w)).umask16(0x00ff);
	map(base | 0xfff98, base | 0xfff98).rw("timer8_2", FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(base | 0xfff99, base | 0xfff99).rw("timer8_3", FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));

	map(base | 0xfffb0, base | 0xfffb0).rw("sci0", FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(base | 0xfffb1, base | 0xfffb1).rw("sci0", FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(base | 0xfffb2, base | 0xfffb2).rw("sci0", FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(base | 0xfffb3, base | 0xfffb3).rw("sci0", FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(base | 0xfffb4, base | 0xfffb4).rw("sci0", FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(base | 0xfffb5, base | 0xfffb5).r("sci0", FUNC(h8_sci_device::rdr_r));
	map(base | 0xfffb6, base | 0xfffb6).rw("sci0", FUNC(h8_sci_device::scmr_r), FUNC(h8_sci_device::scmr_w));
	map(base | 0xfffb8, base | 0xfffb8).rw("sci1", FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(base | 0xfffb9, base | 0xfffb9).rw("sci1", FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(base | 0xfffba, base | 0xfffba).rw("sci1", FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(base | 0xfffbb, base | 0xfffbb).rw("sci1", FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(base | 0xfffbc, base | 0xfffbc).rw("sci1", FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(base | 0xfffbd, base | 0xfffbd).r("sci1", FUNC(h8_sci_device::rdr_r));
	map(base | 0xfffbe, base | 0xfffbe).rw("sci1", FUNC(h8_sci_device::scmr_r), FUNC(h8_sci_device::scmr_w));
	map(base | 0xfffd3, base | 0xfffd3).rw("port4", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xfffd5, base | 0xfffd5).rw("port6", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xfffd6, base | 0xfffd6).rw("port7", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xfffd7, base | 0xfffd7).rw("port8", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xfffd8, base | 0xfffd8).rw("port9", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xfffd9, base | 0xfffd9).rw("porta", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xfffda, base | 0xfffda).rw("portb", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));

	map(base | 0xfffe0, base | 0xfffe7).r("adc", FUNC(h8_adc_device::addr8_r));
	map(base | 0xfffe8, base | 0xfffe8).rw("adc", FUNC(h8_adc_device::adcsr_r), FUNC(h8_adc_device::adcsr_w));
	map(base | 0xfffe9, base | 0xfffe9).rw("adc", FUNC(h8_adc_device::adcr_r), FUNC(h8_adc_device::adcr_w));
}

void h83008_device::device_add_mconfig(machine_config &config)
{
	H8H_INTC(config, "intc");
	H8_ADC_3006(config, "adc", "intc", 23);
	H8_PORT(config, "port4", h8_device::PORT_4, 0x00, 0x00);
	H8_PORT(config, "port6", h8_device::PORT_6, 0x80, 0x80);
	H8_PORT(config, "port7", h8_device::PORT_7, 0xff, 0x00);
	H8_PORT(config, "port8", h8_device::PORT_8, 0xf0, 0xe0);
	H8_PORT(config, "port9", h8_device::PORT_9, 0xc0, 0xc0);
	H8_PORT(config, "porta", h8_device::PORT_A, 0x80, 0x00);
	H8_PORT(config, "portb", h8_device::PORT_B, 0x00, 0x00);
	H8H_TIMER8_CHANNEL(config, "timer8_0", "intc", 36, 38, 39, "timer8_1", h8_timer8_channel_device::CHAIN_OVERFLOW, true,  false);
	H8H_TIMER8_CHANNEL(config, "timer8_1", "intc", 37, 38, 39, "timer8_0", h8_timer8_channel_device::CHAIN_A,        false, false);
	H8H_TIMER8_CHANNEL(config, "timer8_2", "intc", 40, 42, 43, "timer8_3", h8_timer8_channel_device::CHAIN_OVERFLOW, false, true);
	H8H_TIMER8_CHANNEL(config, "timer8_3", "intc", 41, 42, 43, "timer8_2", h8_timer8_channel_device::CHAIN_A,        false, true);
	H8_TIMER16(config, "timer16", 3, 0xf8);
	H8H_TIMER16_CHANNEL(config, "timer16:0", 2, 2, "intc", 24);
	H8H_TIMER16_CHANNEL(config, "timer16:1", 2, 2, "intc", 28);
	H8H_TIMER16_CHANNEL(config, "timer16:2", 2, 2, "intc", 32);
	H8_SCI(config, "sci0", "intc", 52, 53, 54, 55);
	H8_SCI(config, "sci1", "intc", 56, 57, 58, 59);
	H8_WATCHDOG(config, "watchdog", "intc", 20, h8_watchdog_device::H);
}

void h83008_device::execute_set_input(int inputnum, int state)
{
	intc->set_input(inputnum, state);
}

int h83008_device::trapa_setup()
{
	if(syscr & 0x08)
		CCR |= F_I;
	else
		CCR |= F_I|F_UI;
	return 8;
}

void h83008_device::irq_setup()
{
	if(syscr & 0x08)
		CCR |= F_I;
	else
		CCR |= F_I|F_UI;
}

void h83008_device::update_irq_filter()
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

void h83008_device::interrupt_taken()
{
	standard_irq_callback(intc->interrupt_taken(taken_irq_vector));
}

void h83008_device::internal_update(uint64_t current_time)
{
	uint64_t event_time = 0;

	add_event(event_time, adc->internal_update(current_time));
	add_event(event_time, sci0->internal_update(current_time));
	add_event(event_time, sci1->internal_update(current_time));
	add_event(event_time, timer8_0->internal_update(current_time));
	add_event(event_time, timer8_1->internal_update(current_time));
	add_event(event_time, timer8_2->internal_update(current_time));
	add_event(event_time, timer8_3->internal_update(current_time));
	add_event(event_time, timer16_0->internal_update(current_time));
	add_event(event_time, timer16_1->internal_update(current_time));
	add_event(event_time, timer16_2->internal_update(current_time));
	add_event(event_time, watchdog->internal_update(current_time));

	recompute_bcount(event_time);
}

void h83008_device::device_start()
{
	h8h_device::device_start();
}

void h83008_device::device_reset()
{
	h8h_device::device_reset();
	syscr = 0x09;
}


READ8_MEMBER(h83008_device::syscr_r)
{
	return syscr;
}

WRITE8_MEMBER(h83008_device::syscr_w)
{
	syscr = data;
	update_irq_filter();
	logerror("syscr = %02x\n", data);
}
