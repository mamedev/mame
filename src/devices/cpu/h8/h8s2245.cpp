// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h8s2245.h"

DEFINE_DEVICE_TYPE(H8S2241, h8s2241_device, "h8s2241", "Hitachi H8S/2241")
DEFINE_DEVICE_TYPE(H8S2242, h8s2242_device, "h8s2242", "Hitachi H8S/2242")
DEFINE_DEVICE_TYPE(H8S2245, h8s2245_device, "h8s2245", "Hitachi H8S/2245")
DEFINE_DEVICE_TYPE(H8S2246, h8s2246_device, "h8s2246", "Hitachi H8S/2246")


h8s2245_device::h8s2245_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t start) :
	h8s2000_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h8s2245_device::map), this)),
	intc(*this, "intc"),
	adc(*this, "adc"),
	dtc(*this, "dtc"),
	port1(*this, "port1"),
	port2(*this, "port2"),
	port3(*this, "port3"),
	port4(*this, "port4"),
	port5(*this, "port5"),
	porta(*this, "porta"),
	portb(*this, "portb"),
	portc(*this, "portc"),
	portd(*this, "portd"),
	porte(*this, "porte"),
	portf(*this, "portf"),
	portg(*this, "portg"),
	timer8_0(*this, "timer8_0"),
	timer8_1(*this, "timer8_1"),
	timer16(*this, "timer16"),
	timer16_0(*this, "timer16:0"),
	timer16_1(*this, "timer16:1"),
	timer16_2(*this, "timer16:2"),
	sci0(*this, "sci0"),
	sci1(*this, "sci1"),
	sci2(*this, "sci2"),
	watchdog(*this, "watchdog"),
	ram_start(start),
	syscr(0)
{
}

h8s2245_device::h8s2245_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8s2245_device(mconfig, H8S2245, tag, owner, clock, 0xffec00)
{
}

h8s2241_device::h8s2241_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8s2245_device(mconfig, H8S2241, tag, owner, clock, 0xffec00)
{
}

h8s2242_device::h8s2242_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8s2245_device(mconfig, H8S2242, tag, owner, clock, 0xffdc00)
{
}

h8s2246_device::h8s2246_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8s2245_device(mconfig, H8S2246, tag, owner, clock, 0xffdc00)
{
}

void h8s2245_device::map(address_map &map)
{
	map(ram_start, 0xfffbff).ram();

	map(0xfffeb0, 0xfffeb0).w("port1", FUNC(h8_port_device::ddr_w));
	map(0xfffeb1, 0xfffeb1).w("port2", FUNC(h8_port_device::ddr_w));
	map(0xfffeb2, 0xfffeb2).w("port3", FUNC(h8_port_device::ddr_w));
	map(0xfffeb4, 0xfffeb4).w("port5", FUNC(h8_port_device::ddr_w));
	map(0xfffeb9, 0xfffeb9).w("porta", FUNC(h8_port_device::ddr_w));
	map(0xfffeba, 0xfffeba).w("portb", FUNC(h8_port_device::ddr_w));
	map(0xfffebb, 0xfffebb).w("portc", FUNC(h8_port_device::ddr_w));
	map(0xfffebc, 0xfffebc).w("portd", FUNC(h8_port_device::ddr_w));
	map(0xfffebd, 0xfffebd).w("porte", FUNC(h8_port_device::ddr_w));
	map(0xfffebe, 0xfffebe).w("portf", FUNC(h8_port_device::ddr_w));
	map(0xfffebf, 0xfffebf).w("portg", FUNC(h8_port_device::ddr_w));
	map(0xfffec0, 0xfffec1).rw("intc", FUNC(h8s_intc_device::icr_r), FUNC(h8s_intc_device::icr_w));
	map(0xfffec2, 0xfffec2).rw("intc", FUNC(h8s_intc_device::icrc_r), FUNC(h8s_intc_device::icrc_w));
	map(0xffff2c, 0xffff2c).rw("intc", FUNC(h8s_intc_device::iscrh_r), FUNC(h8s_intc_device::iscrh_w));
	map(0xffff2d, 0xffff2d).rw("intc", FUNC(h8s_intc_device::iscrl_r), FUNC(h8s_intc_device::iscrl_w));
	map(0xffff2e, 0xffff2e).rw("intc", FUNC(h8s_intc_device::ier_r), FUNC(h8s_intc_device::ier_w));
	map(0xffff2f, 0xffff2f).rw("intc", FUNC(h8s_intc_device::isr_r), FUNC(h8s_intc_device::isr_w));
	map(0xffff30, 0xffff35).rw("dtc", FUNC(h8_dtc_device::dtcer_r), FUNC(h8_dtc_device::dtcer_w));
	map(0xffff37, 0xffff37).rw("dtc", FUNC(h8_dtc_device::dtvecr_r), FUNC(h8_dtc_device::dtvecr_w));
	map(0xffff39, 0xffff39).rw(FUNC(h8s2245_device::syscr_r), FUNC(h8s2245_device::syscr_w));
	map(0xffff3c, 0xffff3d).rw(FUNC(h8s2245_device::mstpcr_r), FUNC(h8s2245_device::mstpcr_w));

	map(0xffff50, 0xffff50).r("port1", FUNC(h8_port_device::port_r));
	map(0xffff51, 0xffff51).r("port2", FUNC(h8_port_device::port_r));
	map(0xffff52, 0xffff52).r("port3", FUNC(h8_port_device::port_r));
	map(0xffff53, 0xffff53).r("port4", FUNC(h8_port_device::port_r));
	map(0xffff54, 0xffff54).r("port5", FUNC(h8_port_device::port_r));
	map(0xffff59, 0xffff59).r("porta", FUNC(h8_port_device::port_r));
	map(0xffff5a, 0xffff5a).r("portb", FUNC(h8_port_device::port_r));
	map(0xffff5b, 0xffff5b).r("portc", FUNC(h8_port_device::port_r));
	map(0xffff5c, 0xffff5c).r("portd", FUNC(h8_port_device::port_r));
	map(0xffff5d, 0xffff5d).r("porte", FUNC(h8_port_device::port_r));
	map(0xffff5e, 0xffff5e).r("portf", FUNC(h8_port_device::port_r));
	map(0xffff5f, 0xffff5f).r("portg", FUNC(h8_port_device::port_r));
	map(0xffff60, 0xffff60).rw("port1", FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff61, 0xffff61).rw("port2", FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff62, 0xffff62).rw("port3", FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff64, 0xffff64).rw("port5", FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff69, 0xffff69).rw("porta", FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff6a, 0xffff6a).rw("portb", FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff6b, 0xffff6b).rw("portc", FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff6c, 0xffff6c).rw("portd", FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff6d, 0xffff6d).rw("porte", FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff6e, 0xffff6e).rw("portf", FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff6f, 0xffff6f).rw("portg", FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff70, 0xffff70).rw("porta", FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffff71, 0xffff71).rw("portb", FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffff72, 0xffff72).rw("portc", FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffff73, 0xffff73).rw("portd", FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffff74, 0xffff74).rw("porte", FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffff76, 0xffff76).rw("port3", FUNC(h8_port_device::odr_r), FUNC(h8_port_device::odr_w));
	map(0xffff77, 0xffff77).rw("porta", FUNC(h8_port_device::odr_r), FUNC(h8_port_device::odr_w));
	map(0xffff78, 0xffff78).rw("sci0", FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffff79, 0xffff79).rw("sci0", FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffff7a, 0xffff7a).rw("sci0", FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffff7b, 0xffff7b).rw("sci0", FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffff7c, 0xffff7c).rw("sci0", FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffff7d, 0xffff7d).r("sci0", FUNC(h8_sci_device::rdr_r));
	map(0xffff7e, 0xffff7e).rw("sci0", FUNC(h8_sci_device::scmr_r), FUNC(h8_sci_device::scmr_w));
	map(0xffff80, 0xffff80).rw("sci1", FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffff81, 0xffff81).rw("sci1", FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffff82, 0xffff82).rw("sci1", FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffff83, 0xffff83).rw("sci1", FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffff84, 0xffff84).rw("sci1", FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffff85, 0xffff85).r("sci1", FUNC(h8_sci_device::rdr_r));
	map(0xffff86, 0xffff86).rw("sci1", FUNC(h8_sci_device::scmr_r), FUNC(h8_sci_device::scmr_w));
	map(0xffff88, 0xffff88).rw("sci2", FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffff89, 0xffff89).rw("sci2", FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffff8a, 0xffff8a).rw("sci2", FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffff8b, 0xffff8b).rw("sci2", FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffff8c, 0xffff8c).rw("sci2", FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffff8d, 0xffff8d).r("sci2", FUNC(h8_sci_device::rdr_r));
	map(0xffff8e, 0xffff8e).rw("sci2", FUNC(h8_sci_device::scmr_r), FUNC(h8_sci_device::scmr_w));
	map(0xffff90, 0xffff97).r("adc", FUNC(h8_adc_device::addr8_r));
	map(0xffff98, 0xffff98).rw("adc", FUNC(h8_adc_device::adcsr_r), FUNC(h8_adc_device::adcsr_w));
	map(0xffff99, 0xffff99).rw("adc", FUNC(h8_adc_device::adcr_r), FUNC(h8_adc_device::adcr_w));
	map(0xffffb0, 0xffffb0).rw("timer8_0", FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffffb1, 0xffffb1).rw("timer8_1", FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffffb2, 0xffffb2).rw("timer8_0", FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffffb3, 0xffffb3).rw("timer8_1", FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffffb4, 0xffffb7).rw("timer8_0", FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w)).umask16(0xff00);
	map(0xffffb4, 0xffffb7).rw("timer8_1", FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w)).umask16(0x00ff);
	map(0xffffb8, 0xffffb8).rw("timer8_0", FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(0xffffb9, 0xffffb9).rw("timer8_1", FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(0xffffbc, 0xffffbd).rw("watchdog", FUNC(h8_watchdog_device::wd_r), FUNC(h8_watchdog_device::wd_w));
	map(0xffffbe, 0xffffbf).rw("watchdog", FUNC(h8_watchdog_device::rst_r), FUNC(h8_watchdog_device::rst_w));
	map(0xffffc0, 0xffffc0).rw("timer16", FUNC(h8_timer16_device::tstr_r), FUNC(h8_timer16_device::tstr_w));
	map(0xffffc1, 0xffffc1).rw("timer16", FUNC(h8_timer16_device::tsyr_r), FUNC(h8_timer16_device::tsyr_w));
	map(0xffffd0, 0xffffd0).rw("timer16:0", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(0xffffd1, 0xffffd1).rw("timer16:0", FUNC(h8_timer16_channel_device::tmdr_r), FUNC(h8_timer16_channel_device::tmdr_w));
	map(0xffffd2, 0xffffd3).rw("timer16:0", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(0xffffd4, 0xffffd4).rw("timer16:0", FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(0xffffd5, 0xffffd5).rw("timer16:0", FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(0xffffd6, 0xffffd7).rw("timer16:0", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(0xffffd8, 0xffffdf).rw("timer16:0", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(0xffffe0, 0xffffe0).rw("timer16:1", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(0xffffe1, 0xffffe1).rw("timer16:1", FUNC(h8_timer16_channel_device::tmdr_r), FUNC(h8_timer16_channel_device::tmdr_w));
	map(0xffffe2, 0xffffe2).rw("timer16:1", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(0xffffe4, 0xffffe4).rw("timer16:1", FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(0xffffe5, 0xffffe5).rw("timer16:1", FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(0xffffe6, 0xffffe7).rw("timer16:1", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(0xffffe8, 0xffffeb).rw("timer16:1", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(0xfffff0, 0xfffff0).rw("timer16:2", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(0xfffff1, 0xfffff1).rw("timer16:2", FUNC(h8_timer16_channel_device::tmdr_r), FUNC(h8_timer16_channel_device::tmdr_w));
	map(0xfffff2, 0xfffff2).rw("timer16:2", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(0xfffff4, 0xfffff4).rw("timer16:2", FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(0xfffff5, 0xfffff5).rw("timer16:2", FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(0xfffff6, 0xfffff7).rw("timer16:2", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(0xfffff8, 0xfffffb).rw("timer16:2", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
}

void h8s2245_device::device_add_mconfig(machine_config &config)
{
	H8S_INTC(config, "intc");
	H8_ADC_2245(config, "adc", "intc", 28);
	H8_DTC(config, "dtc", "intc", 24);
	H8_PORT(config, "port1", h8_device::PORT_1, 0x00, 0x00);
	H8_PORT(config, "port2", h8_device::PORT_2, 0x00, 0x00);
	H8_PORT(config, "port3", h8_device::PORT_3, 0xc0, 0xc0);
	H8_PORT(config, "port4", h8_device::PORT_4, 0xf0, 0xf0);
	H8_PORT(config, "port5", h8_device::PORT_5, 0xf0, 0xf0);
	H8_PORT(config, "porta", h8_device::PORT_A, 0xf0, 0xf0);
	H8_PORT(config, "portb", h8_device::PORT_B, 0x00, 0x00);
	H8_PORT(config, "portc", h8_device::PORT_C, 0x00, 0x00);
	H8_PORT(config, "portd", h8_device::PORT_D, 0x00, 0x00);
	H8_PORT(config, "porte", h8_device::PORT_E, 0x00, 0x00);
	H8_PORT(config, "portf", h8_device::PORT_F, 0x00, 0x00);
	H8_PORT(config, "portg", h8_device::PORT_G, 0xe0, 0x00);
	H8H_TIMER8_CHANNEL(config, "timer8_0", "intc", 64, 65, 66, "timer8_1", h8_timer8_channel_device::CHAIN_OVERFLOW, true,  false);
	H8H_TIMER8_CHANNEL(config, "timer8_1", "intc", 68, 69, 70, "timer8_0", h8_timer8_channel_device::CHAIN_A,        false, false);
	H8_TIMER16(config, "timer16", 3, 0x00);
	H8S_TIMER16_CHANNEL(config, "timer16:0", 4, 0x60, "intc", 32,
									h8_timer16_channel_device::DIV_1,
									h8_timer16_channel_device::DIV_4,
									h8_timer16_channel_device::DIV_16,
									h8_timer16_channel_device::DIV_64,
									h8_timer16_channel_device::INPUT_A,
									h8_timer16_channel_device::INPUT_B,
									h8_timer16_channel_device::INPUT_C,
									h8_timer16_channel_device::INPUT_D);
	H8S_TIMER16_CHANNEL(config, "timer16:1", 2, 0x4c, "intc", 40,
									h8_timer16_channel_device::DIV_1,
									h8_timer16_channel_device::DIV_4,
									h8_timer16_channel_device::DIV_16,
									h8_timer16_channel_device::DIV_64,
									h8_timer16_channel_device::INPUT_A,
									h8_timer16_channel_device::INPUT_B,
									h8_timer16_channel_device::DIV_256,
									h8_timer16_channel_device::CHAIN).set_chain("timer16:2");
	H8S_TIMER16_CHANNEL(config, "timer16:2", 2, 0x4c, "intc", 44,
									h8_timer16_channel_device::DIV_1,
									h8_timer16_channel_device::DIV_4,
									h8_timer16_channel_device::DIV_16,
									h8_timer16_channel_device::DIV_64,
									h8_timer16_channel_device::INPUT_A,
									h8_timer16_channel_device::INPUT_B,
									h8_timer16_channel_device::INPUT_C,
									h8_timer16_channel_device::DIV_1024);
	H8_SCI(config, "sci0", "intc", 80, 81, 82, 83);
	H8_SCI(config, "sci1", "intc", 84, 85, 86, 87);
	H8_SCI(config, "sci2", "intc", 88, 89, 90, 91);
	H8_WATCHDOG(config, "watchdog", "intc", 25, h8_watchdog_device::S);
}

void h8s2245_device::execute_set_input(int inputnum, int state)
{
	intc->set_input(inputnum, state);
}

bool h8s2245_device::exr_in_stack() const
{
	return false;
}

int h8s2245_device::trapa_setup()
{
	if(syscr & 0x10)
		CCR |= F_I|F_UI;
	else
		CCR |= F_I;
	return 8;
}

void h8s2245_device::irq_setup()
{
	switch(syscr & 0x30) {
	case 0x00:
		CCR |= F_I;
		break;
	case 0x10:
		CCR |= F_I|F_UI;
		break;
	}
}

void h8s2245_device::update_irq_filter()
{
	switch(syscr & 0x30) {
	case 0x00:
		if(CCR & F_I)
			intc->set_filter(2, -1);
		else
			intc->set_filter(0, -1);
		break;
	case 0x10:
		if((CCR & (F_I|F_UI)) == (F_I|F_UI))
			intc->set_filter(2, -1);
		else if(CCR & F_I)
			intc->set_filter(1, -1);
		else
			intc->set_filter(0, -1);
		break;
	}
}

void h8s2245_device::interrupt_taken()
{
	standard_irq_callback(intc->interrupt_taken(taken_irq_vector));
}

void h8s2245_device::internal_update(uint64_t current_time)
{
	uint64_t event_time = 0;

	add_event(event_time, adc->internal_update(current_time));
	add_event(event_time, sci0->internal_update(current_time));
	add_event(event_time, sci1->internal_update(current_time));
	add_event(event_time, sci2->internal_update(current_time));
	add_event(event_time, timer8_0->internal_update(current_time));
	add_event(event_time, timer8_1->internal_update(current_time));
	add_event(event_time, timer16_0->internal_update(current_time));
	add_event(event_time, timer16_1->internal_update(current_time));
	add_event(event_time, timer16_2->internal_update(current_time));
	add_event(event_time, watchdog->internal_update(current_time));

	recompute_bcount(event_time);
}

void h8s2245_device::device_start()
{
	h8s2000_device::device_start();
	dtc_device = dtc;
}

void h8s2245_device::device_reset()
{
	h8s2000_device::device_reset();
	syscr = 0x01;
	mstpcr = 0x3fff;
}

READ8_MEMBER(h8s2245_device::syscr_r)
{
	return syscr;
}

WRITE8_MEMBER(h8s2245_device::syscr_w)
{
	syscr = data;
	update_irq_filter();
	logerror("syscr = %02x\n", data);
}

READ16_MEMBER(h8s2245_device::mstpcr_r)
{
	return mstpcr;
}

WRITE16_MEMBER(h8s2245_device::mstpcr_w)
{
	uint16_t omstpcr = mstpcr;
	COMBINE_DATA(&mstpcr);
	if((omstpcr ^ mstpcr) & 0x72e0) {
		char buf[4096];
		char *p = buf;
		p += sprintf(p, "Online modules:");
		if(mstpcr & 0x0020) p += sprintf(p, " sci0");
		if(mstpcr & 0x0040) p += sprintf(p, " sci1");
		if(mstpcr & 0x0080) p += sprintf(p, " sci2");
		if(mstpcr & 0x0200) p += sprintf(p, " adc");
		if(mstpcr & 0x1000) p += sprintf(p, " timer8");
		if(mstpcr & 0x2000) p += sprintf(p, " timer16");
		if(mstpcr & 0x4000) p += sprintf(p, " dtc");
		p += sprintf(p, "\n");
		logerror(buf);
	}
}
