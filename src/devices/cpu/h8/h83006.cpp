// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h83006.h"

DEFINE_DEVICE_TYPE(H83006, h83006_device, "h83006", "Hitachi H8/3006")
DEFINE_DEVICE_TYPE(H83007, h83007_device, "h83007", "Hitachi H8/3007")


h83006_device::h83006_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 start) :
	h8h_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h83006_device::map), this)),
	m_intc(*this, "intc"),
	m_adc(*this, "adc"),
	m_port4(*this, "port4"),
	m_port6(*this, "port6"),
	m_port7(*this, "port7"),
	m_port8(*this, "port8"),
	m_port9(*this, "port9"),
	m_porta(*this, "porta"),
	m_portb(*this, "portb"),
	m_timer8_0(*this, "timer8_0"),
	m_timer8_1(*this, "timer8_1"),
	m_timer8_2(*this, "timer8_2"),
	m_timer8_3(*this, "timer8_3"),
	m_timer16(*this, "timer16"),
	m_timer16_0(*this, "timer16:0"),
	m_timer16_1(*this, "timer16:1"),
	m_timer16_2(*this, "timer16:2"),
	m_watchdog(*this, "watchdog"),
	m_syscr(0),
	m_ram_start(start)
{
}

h83006_device::h83006_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h83006_device(mconfig, H83006, tag, owner, clock, 0xff720)
{
}


h83007_device::h83007_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h83006_device(mconfig, H83007, tag, owner, clock, 0xfef20)
{
}

void h83006_device::map(address_map &map)
{
	const offs_t base = m_mode_a20 ? 0 : 0xf00000;

	map(base | 0xee003, base | 0xee003).rw(m_port4, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(base | 0xee005, base | 0xee005).rw(m_port6, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(base | 0xee007, base | 0xee007).rw(m_port8, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(base | 0xee008, base | 0xee008).rw(m_port9, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(base | 0xee009, base | 0xee009).rw(m_porta, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(base | 0xee00a, base | 0xee00a).rw(m_portb, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));

	map(base | 0xee012, base | 0xee012).rw(FUNC(h83006_device::syscr_r), FUNC(h83006_device::syscr_w));
	map(base | 0xee014, base | 0xee014).rw(m_intc, FUNC(h8h_intc_device::iscr_r), FUNC(h8h_intc_device::iscr_w));
	map(base | 0xee015, base | 0xee015).rw(m_intc, FUNC(h8h_intc_device::ier_r), FUNC(h8h_intc_device::ier_w));
	map(base | 0xee016, base | 0xee016).rw(m_intc, FUNC(h8h_intc_device::isr_r), FUNC(h8h_intc_device::isr_w));
	map(base | 0xee018, base | 0xee019).rw(m_intc, FUNC(h8h_intc_device::icr_r), FUNC(h8h_intc_device::icr_w));

	map(base | 0xee03e, base | 0xee03e).rw(m_port4, FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));

	map(base | m_ram_start, base | 0xfff1f).ram();

	map(base | 0xfff60, base | 0xfff60).rw(m_timer16, FUNC(h8_timer16_device::tstr_r), FUNC(h8_timer16_device::tstr_w));
	map(base | 0xfff61, base | 0xfff61).rw(m_timer16, FUNC(h8_timer16_device::tsyr_r), FUNC(h8_timer16_device::tsyr_w));
	map(base | 0xfff62, base | 0xfff62).rw(m_timer16, FUNC(h8_timer16_device::tmdr_r), FUNC(h8_timer16_device::tmdr_w));
	map(base | 0xfff63, base | 0xfff63).w(m_timer16, FUNC(h8_timer16_device::tolr_w));
	map(base | 0xfff64, base | 0xfff65).rw(m_timer16, FUNC(h8_timer16_device::tisr_r), FUNC(h8_timer16_device::tisr_w));
	map(base | 0xfff66, base | 0xfff66).rw(m_timer16, FUNC(h8_timer16_device::tisrc_r), FUNC(h8_timer16_device::tisrc_w));
	map(base | 0xfff68, base | 0xfff68).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(base | 0xfff69, base | 0xfff69).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(base | 0xfff6a, base | 0xfff6b).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(base | 0xfff6c, base | 0xfff6f).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(base | 0xfff70, base | 0xfff70).rw(m_timer16_1, FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(base | 0xfff71, base | 0xfff71).rw(m_timer16_1, FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(base | 0xfff72, base | 0xfff73).rw(m_timer16_1, FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(base | 0xfff74, base | 0xfff77).rw(m_timer16_1, FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(base | 0xfff78, base | 0xfff78).rw(m_timer16_2, FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(base | 0xfff79, base | 0xfff79).rw(m_timer16_2, FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(base | 0xfff7a, base | 0xfff7b).rw(m_timer16_2, FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(base | 0xfff7c, base | 0xfff7f).rw(m_timer16_2, FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(base | 0xfff80, base | 0xfff80).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(base | 0xfff81, base | 0xfff81).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(base | 0xfff82, base | 0xfff82).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(base | 0xfff83, base | 0xfff83).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(base | 0xfff84, base | 0xfff87).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w)).umask16(0xff00);
	map(base | 0xfff84, base | 0xfff87).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w)).umask16(0x00ff);
	map(base | 0xfff88, base | 0xfff88).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(base | 0xfff89, base | 0xfff89).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(base | 0xfff8c, base | 0xfff8d).rw(m_watchdog, FUNC(h8_watchdog_device::wd_r), FUNC(h8_watchdog_device::wd_w));
	map(base | 0xfff8e, base | 0xfff8f).rw(m_watchdog, FUNC(h8_watchdog_device::rst_r), FUNC(h8_watchdog_device::rst_w));
	map(base | 0xfff90, base | 0xfff90).rw(m_timer8_2, FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(base | 0xfff91, base | 0xfff91).rw(m_timer8_3, FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(base | 0xfff92, base | 0xfff92).rw(m_timer8_2, FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(base | 0xfff93, base | 0xfff93).rw(m_timer8_3, FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(base | 0xfff94, base | 0xfff97).rw(m_timer8_2, FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w)).umask16(0xff00);
	map(base | 0xfff94, base | 0xfff97).rw(m_timer8_3, FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w)).umask16(0x00ff);
	map(base | 0xfff98, base | 0xfff98).rw(m_timer8_2, FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(base | 0xfff99, base | 0xfff99).rw(m_timer8_3, FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));

	map(base | 0xfffb0, base | 0xfffb0).rw(m_sci[0], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(base | 0xfffb1, base | 0xfffb1).rw(m_sci[0], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(base | 0xfffb2, base | 0xfffb2).rw(m_sci[0], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(base | 0xfffb3, base | 0xfffb3).rw(m_sci[0], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(base | 0xfffb4, base | 0xfffb4).rw(m_sci[0], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(base | 0xfffb5, base | 0xfffb5).r(m_sci[0], FUNC(h8_sci_device::rdr_r));
	map(base | 0xfffb6, base | 0xfffb6).rw(m_sci[0], FUNC(h8_sci_device::scmr_r), FUNC(h8_sci_device::scmr_w));
	map(base | 0xfffb8, base | 0xfffb8).rw(m_sci[1], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(base | 0xfffb9, base | 0xfffb9).rw(m_sci[1], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(base | 0xfffba, base | 0xfffba).rw(m_sci[1], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(base | 0xfffbb, base | 0xfffbb).rw(m_sci[1], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(base | 0xfffbc, base | 0xfffbc).rw(m_sci[1], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(base | 0xfffbd, base | 0xfffbd).r(m_sci[1], FUNC(h8_sci_device::rdr_r));
	map(base | 0xfffbe, base | 0xfffbe).rw(m_sci[1], FUNC(h8_sci_device::scmr_r), FUNC(h8_sci_device::scmr_w));
	map(base | 0xfffc0, base | 0xfffc0).rw(m_sci[2], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(base | 0xfffc1, base | 0xfffc1).rw(m_sci[2], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(base | 0xfffc2, base | 0xfffc2).rw(m_sci[2], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(base | 0xfffc3, base | 0xfffc3).rw(m_sci[2], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(base | 0xfffc4, base | 0xfffc4).rw(m_sci[2], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(base | 0xfffc5, base | 0xfffc5).r(m_sci[2], FUNC(h8_sci_device::rdr_r));
	map(base | 0xfffc6, base | 0xfffc6).rw(m_sci[2], FUNC(h8_sci_device::scmr_r), FUNC(h8_sci_device::scmr_w));
	map(base | 0xfffd3, base | 0xfffd3).rw(m_port4, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xfffd5, base | 0xfffd5).rw(m_port6, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xfffd6, base | 0xfffd6).rw(m_port7, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xfffd7, base | 0xfffd7).rw(m_port8, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xfffd8, base | 0xfffd8).rw(m_port9, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xfffd9, base | 0xfffd9).rw(m_porta, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xfffda, base | 0xfffda).rw(m_portb, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));

	map(base | 0xfffe0, base | 0xfffe7).r(m_adc, FUNC(h8_adc_device::addr8_r));
	map(base | 0xfffe8, base | 0xfffe8).rw(m_adc, FUNC(h8_adc_device::adcsr_r), FUNC(h8_adc_device::adcsr_w));
	map(base | 0xfffe9, base | 0xfffe9).rw(m_adc, FUNC(h8_adc_device::adcr_r), FUNC(h8_adc_device::adcr_w));
}

void h83006_device::device_add_mconfig(machine_config &config)
{
	H8H_INTC(config, m_intc, *this);
	H8_ADC_3006(config, m_adc, *this, m_intc, 23);
	H8_PORT(config, m_port4, *this, h8_device::PORT_4, 0x00, 0x00);
	H8_PORT(config, m_port6, *this, h8_device::PORT_6, 0x80, 0x80);
	H8_PORT(config, m_port7, *this, h8_device::PORT_7, 0x00, 0x00);
	H8_PORT(config, m_port8, *this, h8_device::PORT_8, 0xf0, 0xe0);
	H8_PORT(config, m_port9, *this, h8_device::PORT_9, 0xc0, 0xc0);
	H8_PORT(config, m_porta, *this, h8_device::PORT_A, 0x80, 0x00);
	H8_PORT(config, m_portb, *this, h8_device::PORT_B, 0x00, 0x00);
	H8H_TIMER8_CHANNEL(config, m_timer8_0, *this, m_intc, 36, 38, 39, m_timer8_1, h8_timer8_channel_device::CHAIN_OVERFLOW, true,  false);
	H8H_TIMER8_CHANNEL(config, m_timer8_1, *this, m_intc, 37, 38, 39, m_timer8_0, h8_timer8_channel_device::CHAIN_A,        false, false);
	H8H_TIMER8_CHANNEL(config, m_timer8_2, *this, m_intc, 40, 42, 43, m_timer8_3, h8_timer8_channel_device::CHAIN_OVERFLOW, false, true);
	H8H_TIMER8_CHANNEL(config, m_timer8_3, *this, m_intc, 41, 42, 43, m_timer8_2, h8_timer8_channel_device::CHAIN_A,        false, true);
	H8_TIMER16(config, m_timer16, *this, 3, 0xf8);
	H8H_TIMER16_CHANNEL(config, m_timer16_0, *this, 2, 2, m_intc, 24);
	H8H_TIMER16_CHANNEL(config, m_timer16_1, *this, 2, 2, m_intc, 28);
	H8H_TIMER16_CHANNEL(config, m_timer16_2, *this, 2, 2, m_intc, 32);
	H8_SCI(config, m_sci[0], 0, *this, m_intc, 52, 53, 54, 55);
	H8_SCI(config, m_sci[1], 1, *this, m_intc, 56, 57, 58, 59);
	H8_SCI(config, m_sci[2], 2, *this, m_intc, 60, 61, 62, 63);
	H8_WATCHDOG(config, m_watchdog, *this, m_intc, 20, h8_watchdog_device::H);
}

void h83006_device::execute_set_input(int inputnum, int state)
{
	m_intc->set_input(inputnum, state);
}

int h83006_device::trapa_setup()
{
	if(m_syscr & 0x08)
		m_CCR |= F_I;
	else
		m_CCR |= F_I|F_UI;
	return 8;
}

void h83006_device::irq_setup()
{
	if(m_syscr & 0x08)
		m_CCR |= F_I;
	else
		m_CCR |= F_I|F_UI;
}

void h83006_device::update_irq_filter()
{
	switch(m_syscr & 0x08) {
	case 0x00:
		if((m_CCR & (F_I|F_UI)) == (F_I|F_UI))
			m_intc->set_filter(2, -1);
		else if(m_CCR & F_I)
			m_intc->set_filter(1, -1);
		else
			m_intc->set_filter(0, -1);
		break;
	case 0x08:
		if(m_CCR & F_I)
			m_intc->set_filter(2, -1);
		else
			m_intc->set_filter(0, -1);
		break;
	}
}

void h83006_device::interrupt_taken()
{
	standard_irq_callback(m_intc->interrupt_taken(m_taken_irq_vector), m_NPC);
}

void h83006_device::internal_update(u64 current_time)
{
	u64 event_time = 0;

	add_event(event_time, m_adc->internal_update(current_time));
	add_event(event_time, m_sci[0]->internal_update(current_time));
	add_event(event_time, m_sci[1]->internal_update(current_time));
	add_event(event_time, m_sci[2]->internal_update(current_time));
	add_event(event_time, m_timer8_0->internal_update(current_time));
	add_event(event_time, m_timer8_1->internal_update(current_time));
	add_event(event_time, m_timer8_2->internal_update(current_time));
	add_event(event_time, m_timer8_3->internal_update(current_time));
	add_event(event_time, m_timer16_0->internal_update(current_time));
	add_event(event_time, m_timer16_1->internal_update(current_time));
	add_event(event_time, m_timer16_2->internal_update(current_time));
	add_event(event_time, m_watchdog->internal_update(current_time));

	recompute_bcount(event_time);
}

void h83006_device::notify_standby(int state)
{
	m_adc->notify_standby(state);
	m_sci[0]->notify_standby(state);
	m_sci[1]->notify_standby(state);
	m_sci[2]->notify_standby(state);
	m_timer8_0->notify_standby(state);
	m_timer8_1->notify_standby(state);
	m_timer8_2->notify_standby(state);
	m_timer8_3->notify_standby(state);
	m_timer16_0->notify_standby(state);
	m_timer16_1->notify_standby(state);
	m_timer16_2->notify_standby(state);
	m_watchdog->notify_standby(state);
}

void h83006_device::device_start()
{
	h8h_device::device_start();
	save_item(NAME(m_syscr));
}

void h83006_device::device_reset()
{
	h8h_device::device_reset();
	m_syscr = 0x09;
}

u8 h83006_device::syscr_r()
{
	return m_syscr;
}

void h83006_device::syscr_w(u8 data)
{
	m_syscr = data;
	update_irq_filter();
	logerror("syscr = %02x\n", data);
}
