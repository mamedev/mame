// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h83006.h"

DEFINE_DEVICE_TYPE(H83006, h83006_device, "h83006", "Hitachi H8/3006")
DEFINE_DEVICE_TYPE(H83007, h83007_device, "h83007", "Hitachi H8/3007")


h83006_device::h83006_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t start) :
	h8h_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h83006_device::map), this)),
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
	sci2(*this, "sci2"),
	watchdog(*this, "watchdog"),
	syscr(0),
	ram_start(start)
{
}

h83006_device::h83006_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h83006_device(mconfig, H83006, tag, owner, clock, 0xfff720)
{
}


h83007_device::h83007_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h83006_device(mconfig, H83007, tag, owner, clock, 0xffef20)
{
}

void h83006_device::map(address_map &map)
{
	map(0xfee003, 0xfee003).w("port4", FUNC(h8_port_device::ddr_w));
	map(0xfee005, 0xfee005).w("port6", FUNC(h8_port_device::ddr_w));
	map(0xfee007, 0xfee007).w("port8", FUNC(h8_port_device::ddr_w));
	map(0xfee008, 0xfee008).w("port9", FUNC(h8_port_device::ddr_w));
	map(0xfee009, 0xfee009).w("porta", FUNC(h8_port_device::ddr_w));
	map(0xfee00a, 0xfee00a).w("portb", FUNC(h8_port_device::ddr_w));

	map(0xfee012, 0xfee012).rw(FUNC(h83006_device::syscr_r), FUNC(h83006_device::syscr_w));
	map(0xfee014, 0xfee014).rw("intc", FUNC(h8h_intc_device::iscr_r), FUNC(h8h_intc_device::iscr_w));
	map(0xfee015, 0xfee015).rw("intc", FUNC(h8h_intc_device::ier_r), FUNC(h8h_intc_device::ier_w));
	map(0xfee016, 0xfee016).rw("intc", FUNC(h8h_intc_device::isr_r), FUNC(h8h_intc_device::isr_w));
	map(0xfee018, 0xfee019).rw("intc", FUNC(h8h_intc_device::icr_r), FUNC(h8h_intc_device::icr_w));

	map(0xfee03e, 0xfee03e).rw("port4", FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));

	map(ram_start, 0xffff1f).ram();

	map(0xffff60, 0xffff60).rw("timer16", FUNC(h8_timer16_device::tstr_r), FUNC(h8_timer16_device::tstr_w));
	map(0xffff61, 0xffff61).rw("timer16", FUNC(h8_timer16_device::tsyr_r), FUNC(h8_timer16_device::tsyr_w));
	map(0xffff62, 0xffff62).rw("timer16", FUNC(h8_timer16_device::tmdr_r), FUNC(h8_timer16_device::tmdr_w));
	map(0xffff63, 0xffff63).w("timer16", FUNC(h8_timer16_device::tolr_w));
	map(0xffff64, 0xffff65).rw("timer16", FUNC(h8_timer16_device::tisr_r), FUNC(h8_timer16_device::tisr_w));
	map(0xffff66, 0xffff66).rw("timer16", FUNC(h8_timer16_device::tisrc_r), FUNC(h8_timer16_device::tisrc_w));
	map(0xffff68, 0xffff68).rw("timer16:0", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(0xffff69, 0xffff69).rw("timer16:0", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(0xffff6a, 0xffff6b).rw("timer16:0", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(0xffff6c, 0xffff6f).rw("timer16:0", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(0xffff70, 0xffff70).rw("timer16:1", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(0xffff71, 0xffff71).rw("timer16:1", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(0xffff72, 0xffff73).rw("timer16:1", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(0xffff74, 0xffff77).rw("timer16:1", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(0xffff78, 0xffff78).rw("timer16:2", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(0xffff79, 0xffff79).rw("timer16:2", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(0xffff7a, 0xffff7b).rw("timer16:2", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(0xffff7c, 0xffff7f).rw("timer16:2", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(0xffff80, 0xffff80).rw("timer8_0", FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffff81, 0xffff81).rw("timer8_1", FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffff82, 0xffff82).rw("timer8_0", FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffff83, 0xffff83).rw("timer8_1", FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffff84, 0xffff87).rw("timer8_0", FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w)).umask16(0xff00);
	map(0xffff84, 0xffff87).rw("timer8_1", FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w)).umask16(0x00ff);
	map(0xffff88, 0xffff88).rw("timer8_0", FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(0xffff89, 0xffff89).rw("timer8_1", FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(0xffff8c, 0xffff8d).rw("watchdog", FUNC(h8_watchdog_device::wd_r), FUNC(h8_watchdog_device::wd_w));
	map(0xffff8e, 0xffff8f).rw("watchdog", FUNC(h8_watchdog_device::rst_r), FUNC(h8_watchdog_device::rst_w));
	map(0xffff90, 0xffff90).rw("timer8_2", FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffff91, 0xffff91).rw("timer8_3", FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffff92, 0xffff92).rw("timer8_2", FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffff93, 0xffff93).rw("timer8_3", FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffff94, 0xffff97).rw("timer8_2", FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w)).umask16(0xff00);
	map(0xffff94, 0xffff97).rw("timer8_3", FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w)).umask16(0x00ff);
	map(0xffff98, 0xffff98).rw("timer8_2", FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(0xffff99, 0xffff99).rw("timer8_3", FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));

	map(0xffffb0, 0xffffb0).rw("sci0", FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffffb1, 0xffffb1).rw("sci0", FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffffb2, 0xffffb2).rw("sci0", FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffffb3, 0xffffb3).rw("sci0", FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffffb4, 0xffffb4).rw("sci0", FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffffb5, 0xffffb5).r("sci0", FUNC(h8_sci_device::rdr_r));
	map(0xffffb6, 0xffffb6).rw("sci0", FUNC(h8_sci_device::scmr_r), FUNC(h8_sci_device::scmr_w));
	map(0xffffb8, 0xffffb8).rw("sci1", FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffffb9, 0xffffb9).rw("sci1", FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffffba, 0xffffba).rw("sci1", FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffffbb, 0xffffbb).rw("sci1", FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffffbc, 0xffffbc).rw("sci1", FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffffbd, 0xffffbd).r("sci1", FUNC(h8_sci_device::rdr_r));
	map(0xffffbe, 0xffffbe).rw("sci1", FUNC(h8_sci_device::scmr_r), FUNC(h8_sci_device::scmr_w));
	map(0xffffc0, 0xffffc0).rw("sci2", FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffffc1, 0xffffc1).rw("sci2", FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffffc2, 0xffffc2).rw("sci2", FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffffc3, 0xffffc3).rw("sci2", FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffffc4, 0xffffc4).rw("sci2", FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffffc5, 0xffffc5).r("sci2", FUNC(h8_sci_device::rdr_r));
	map(0xffffc6, 0xffffc6).rw("sci2", FUNC(h8_sci_device::scmr_r), FUNC(h8_sci_device::scmr_w));
	map(0xffffd3, 0xffffd3).rw("port4", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffffd5, 0xffffd5).rw("port6", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffffd6, 0xffffd6).rw("port7", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffffd7, 0xffffd7).rw("port8", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffffd8, 0xffffd8).rw("port9", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffffd9, 0xffffd9).rw("porta", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffffda, 0xffffda).rw("portb", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));

	map(0xffffe0, 0xffffe7).r("adc", FUNC(h8_adc_device::addr8_r));
	map(0xffffe8, 0xffffe8).rw("adc", FUNC(h8_adc_device::adcsr_r), FUNC(h8_adc_device::adcsr_w));
	map(0xffffe9, 0xffffe9).rw("adc", FUNC(h8_adc_device::adcr_r), FUNC(h8_adc_device::adcr_w));
}

void h83006_device::device_add_mconfig(machine_config &config)
{
	H8H_INTC(config, "intc");
	H8_ADC_3006(config, "adc", "intc", 23);
	H8_PORT(config, "port4", h8_device::PORT_4, 0x00, 0x00);
	H8_PORT(config, "port6", h8_device::PORT_6, 0x80, 0x80);
	H8_PORT(config, "port7", h8_device::PORT_7, 0x00, 0x00);
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
	H8_SCI(config, "sci2", "intc", 60, 61, 62, 63);
	H8_WATCHDOG(config, "watchdog", "intc", 20, h8_watchdog_device::H);
}

void h83006_device::execute_set_input(int inputnum, int state)
{
	intc->set_input(inputnum, state);
}

int h83006_device::trapa_setup()
{
	if(syscr & 0x08)
		CCR |= F_I;
	else
		CCR |= F_I|F_UI;
	return 8;
}

void h83006_device::irq_setup()
{
	if(syscr & 0x08)
		CCR |= F_I;
	else
		CCR |= F_I|F_UI;
}

void h83006_device::update_irq_filter()
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

void h83006_device::interrupt_taken()
{
	standard_irq_callback(intc->interrupt_taken(taken_irq_vector));
}

void h83006_device::internal_update(uint64_t current_time)
{
	uint64_t event_time = 0;

	add_event(event_time, adc->internal_update(current_time));
	add_event(event_time, sci0->internal_update(current_time));
	add_event(event_time, sci1->internal_update(current_time));
	add_event(event_time, sci2->internal_update(current_time));
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

void h83006_device::device_start()
{
	h8h_device::device_start();
}

void h83006_device::device_reset()
{
	h8h_device::device_reset();
	syscr = 0x09;
}


READ8_MEMBER(h83006_device::syscr_r)
{
	return syscr;
}

WRITE8_MEMBER(h83006_device::syscr_w)
{
	syscr = data;
	update_irq_filter();
	logerror("syscr = %02x\n", data);
}
