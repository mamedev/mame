// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h8s2655.h"

DEFINE_DEVICE_TYPE(H8S2655, h8s2655_device, "h8s2655", "Hitachi H8S/2655")
DEFINE_DEVICE_TYPE(H8S2653, h8s2653_device, "h8s2653", "Hitachi H8S/2653")

h8s2655_device::h8s2655_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	h8s2600_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h8s2655_device::map), this)),
	intc(*this, "intc"),
	adc(*this, "adc"),
	port1(*this, "port1"),
	port2(*this, "port2"),
	port3(*this, "port3"),
	port4(*this, "port4"),
	port5(*this, "port5"),
	port6(*this, "port6"),
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
	timer16_3(*this, "timer16:3"),
	timer16_4(*this, "timer16:4"),
	timer16_5(*this, "timer16:5"),
	sci0(*this, "sci0"),
	sci1(*this, "sci1"),
	sci2(*this, "sci2"),
	watchdog(*this, "watchdog"),
	syscr(0)
{
	has_trace = true;
}

h8s2655_device::h8s2655_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8s2655_device(mconfig, H8S2655, tag, owner, clock)

{
}

h8s2653_device::h8s2653_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8s2655_device(mconfig, H8S2653, tag, owner, clock)
{
}

void h8s2655_device::map(address_map &map)
{
	map(0xffec00, 0xfffbff).ram();
	map(0xfffe80, 0xfffe80).rw("timer16:3", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(0xfffe81, 0xfffe81).rw("timer16:3", FUNC(h8_timer16_channel_device::tmdr_r), FUNC(h8_timer16_channel_device::tmdr_w));
	map(0xfffe82, 0xfffe83).rw("timer16:3", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(0xfffe84, 0xfffe84).rw("timer16:3", FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(0xfffe85, 0xfffe85).rw("timer16:3", FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(0xfffe86, 0xfffe87).rw("timer16:3", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(0xfffe88, 0xfffe8f).rw("timer16:3", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(0xfffe90, 0xfffe90).rw("timer16:4", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(0xfffe91, 0xfffe91).rw("timer16:4", FUNC(h8_timer16_channel_device::tmdr_r), FUNC(h8_timer16_channel_device::tmdr_w));
	map(0xfffe92, 0xfffe92).rw("timer16:4", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(0xfffe94, 0xfffe94).rw("timer16:4", FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(0xfffe95, 0xfffe95).rw("timer16:4", FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(0xfffe96, 0xfffe97).rw("timer16:4", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(0xfffe98, 0xfffe9b).rw("timer16:4", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(0xfffea0, 0xfffea0).rw("timer16:5", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(0xfffea1, 0xfffea1).rw("timer16:5", FUNC(h8_timer16_channel_device::tmdr_r), FUNC(h8_timer16_channel_device::tmdr_w));
	map(0xfffea2, 0xfffea2).rw("timer16:5", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(0xfffea4, 0xfffea4).rw("timer16:5", FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(0xfffea5, 0xfffea5).rw("timer16:5", FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(0xfffea6, 0xfffea7).rw("timer16:5", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(0xfffea8, 0xfffeab).rw("timer16:5", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(0xfffeb0, 0xfffeb0).w("port1", FUNC(h8_port_device::ddr_w));
	map(0xfffeb1, 0xfffeb1).w("port2", FUNC(h8_port_device::ddr_w));
	map(0xfffeb2, 0xfffeb2).w("port3", FUNC(h8_port_device::ddr_w));
	map(0xfffeb4, 0xfffeb4).w("port5", FUNC(h8_port_device::ddr_w));
	map(0xfffeb5, 0xfffeb5).w("port6", FUNC(h8_port_device::ddr_w));
	map(0xfffeb9, 0xfffeb9).w("porta", FUNC(h8_port_device::ddr_w));
	map(0xfffeba, 0xfffeba).w("portb", FUNC(h8_port_device::ddr_w));
	map(0xfffebb, 0xfffebb).w("portc", FUNC(h8_port_device::ddr_w));
	map(0xfffebc, 0xfffebc).w("portd", FUNC(h8_port_device::ddr_w));
	map(0xfffebd, 0xfffebd).w("porte", FUNC(h8_port_device::ddr_w));
	map(0xfffebe, 0xfffebe).w("portf", FUNC(h8_port_device::ddr_w));
	map(0xfffebf, 0xfffebf).w("portg", FUNC(h8_port_device::ddr_w));
	map(0xfffec0, 0xfffec1).rw("intc", FUNC(h8s_intc_device::icr_r), FUNC(h8s_intc_device::icr_w));
	map(0xfffec2, 0xfffec2).rw("intc", FUNC(h8s_intc_device::icrc_r), FUNC(h8s_intc_device::icrc_w));
	map(0xfffec4, 0xfffecd).rw("intc", FUNC(h8s_intc_device::ipr_r), FUNC(h8s_intc_device::ipr_w));
	map(0xfffece, 0xfffece).rw("intc", FUNC(h8s_intc_device::iprk_r), FUNC(h8s_intc_device::iprk_w));
	map(0xffff2c, 0xffff2c).rw("intc", FUNC(h8s_intc_device::iscrh_r), FUNC(h8s_intc_device::iscrh_w));
	map(0xffff2d, 0xffff2d).rw("intc", FUNC(h8s_intc_device::iscrl_r), FUNC(h8s_intc_device::iscrl_w));
	map(0xffff2e, 0xffff2e).rw("intc", FUNC(h8s_intc_device::ier_r), FUNC(h8s_intc_device::ier_w));
	map(0xffff2f, 0xffff2f).rw("intc", FUNC(h8s_intc_device::isr_r), FUNC(h8s_intc_device::isr_w));
	map(0xffff39, 0xffff39).rw(FUNC(h8s2655_device::syscr_r), FUNC(h8s2655_device::syscr_w));
	map(0xffff50, 0xffff50).r("port1", FUNC(h8_port_device::port_r));
	map(0xffff51, 0xffff51).r("port2", FUNC(h8_port_device::port_r));
	map(0xffff52, 0xffff52).r("port3", FUNC(h8_port_device::port_r));
	map(0xffff53, 0xffff53).r("port4", FUNC(h8_port_device::port_r));
	map(0xffff54, 0xffff54).r("port5", FUNC(h8_port_device::port_r));
	map(0xffff55, 0xffff55).r("port6", FUNC(h8_port_device::port_r));
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
	map(0xffff65, 0xffff65).rw("port6", FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
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
	map(0xffff90, 0xffff9f).r("adc", FUNC(h8_adc_device::addr16_r));
	map(0xffffa0, 0xffffa0).rw("adc", FUNC(h8_adc_device::adcsr_r), FUNC(h8_adc_device::adcsr_w));
	map(0xffffa1, 0xffffa1).rw("adc", FUNC(h8_adc_device::adcr_r), FUNC(h8_adc_device::adcr_w));
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

void h8s2655_device::device_add_mconfig(machine_config &config)
{
	H8S_INTC(config, "intc");
	H8_ADC_2655(config, "adc", "intc", 28);
	H8_PORT(config, "port1", h8_device::PORT_1, 0x00, 0x00);
	H8_PORT(config, "port2", h8_device::PORT_2, 0x00, 0x00);
	H8_PORT(config, "port3", h8_device::PORT_3, 0xc0, 0xc0);
	H8_PORT(config, "port4", h8_device::PORT_4, 0x00, 0x00);
	H8_PORT(config, "port5", h8_device::PORT_5, 0xf0, 0xf0);
	H8_PORT(config, "port6", h8_device::PORT_6, 0x00, 0x00);
	H8_PORT(config, "porta", h8_device::PORT_A, 0x00, 0x00);
	H8_PORT(config, "portb", h8_device::PORT_B, 0x00, 0x00);
	H8_PORT(config, "portc", h8_device::PORT_C, 0x00, 0x00);
	H8_PORT(config, "portd", h8_device::PORT_D, 0x00, 0x00);
	H8_PORT(config, "porte", h8_device::PORT_E, 0x00, 0x00);
	H8_PORT(config, "portf", h8_device::PORT_F, 0x00, 0x00);
	H8_PORT(config, "portg", h8_device::PORT_G, 0xe0, 0xe0);
	H8H_TIMER8_CHANNEL(config, "timer8_0", "intc", 64, 65, 66, "timer8_1", h8_timer8_channel_device::CHAIN_OVERFLOW, true,  false);
	H8H_TIMER8_CHANNEL(config, "timer8_1", "intc", 68, 69, 70, "timer8_0", h8_timer8_channel_device::CHAIN_A,        false, false);
	H8_TIMER16(config, "timer16", 6, 0x00);
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
	H8S_TIMER16_CHANNEL(config, "timer16:3", 4, 0x60, "intc", 48,
									h8_timer16_channel_device::DIV_1,
									h8_timer16_channel_device::DIV_4,
									h8_timer16_channel_device::DIV_16,
									h8_timer16_channel_device::DIV_64,
									h8_timer16_channel_device::INPUT_A,
									h8_timer16_channel_device::DIV_1024,
									h8_timer16_channel_device::DIV_256,
									h8_timer16_channel_device::DIV_4096);
	H8S_TIMER16_CHANNEL(config, "timer16:4", 2, 0x4c, "intc", 56,
									h8_timer16_channel_device::DIV_1,
									h8_timer16_channel_device::DIV_4,
									h8_timer16_channel_device::DIV_16,
									h8_timer16_channel_device::DIV_64,
									h8_timer16_channel_device::INPUT_A,
									h8_timer16_channel_device::INPUT_C,
									h8_timer16_channel_device::DIV_1024,
									h8_timer16_channel_device::CHAIN).set_chain("timer16:5");
	H8S_TIMER16_CHANNEL(config, "timer16:5", 2, 0x4c, "intc", 60,
									h8_timer16_channel_device::DIV_1,
									h8_timer16_channel_device::DIV_4,
									h8_timer16_channel_device::DIV_16,
									h8_timer16_channel_device::DIV_64,
									h8_timer16_channel_device::INPUT_A,
									h8_timer16_channel_device::INPUT_C,
									h8_timer16_channel_device::DIV_256,
									h8_timer16_channel_device::INPUT_D);
	H8_SCI(config, "sci0", "intc", 80, 81, 82, 83);
	H8_SCI(config, "sci1", "intc", 84, 85, 86, 87);
	H8_SCI(config, "sci2", "intc", 88, 89, 90, 91);
	H8_WATCHDOG(config, "watchdog", "intc", 25, h8_watchdog_device::S);
}

void h8s2655_device::execute_set_input(int inputnum, int state)
{
	intc->set_input(inputnum, state);
}

bool h8s2655_device::exr_in_stack() const
{
	return syscr & 0x20;
}

int h8s2655_device::trace_setup()
{
	if(syscr & 0x10)
		CCR |= F_I|F_UI;
	else
		CCR |= F_I;
	EXR &= ~EXR_T;
	return 5;
}

int h8s2655_device::trapa_setup()
{
	if(syscr & 0x10)
		CCR |= F_I|F_UI;
	else
		CCR |= F_I;
	if(syscr & 0x20)
		EXR &= ~EXR_T;
	return 8;
}

void h8s2655_device::irq_setup()
{
	switch(syscr & 0x30) {
	case 0x00:
		CCR |= F_I;
		break;
	case 0x10:
		CCR |= F_I|F_UI;
		break;
	case 0x20:
		EXR = EXR & (EXR_NC);
		if(taken_irq_level == 8)
			EXR |= 7;
		else
			EXR |= taken_irq_level;
		break;
	case 0x30:
		CCR |= F_I|F_UI;
		EXR = EXR & (EXR_NC);
		if(taken_irq_level == 8)
			EXR |= 7;
		else
			EXR |= taken_irq_level;
		break;
	}
}

void h8s2655_device::update_irq_filter()
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
	case 0x20:
		intc->set_filter(0, EXR & 7);
		break;
	case 0x30:
		if((CCR & (F_I|F_UI)) == (F_I|F_UI))
			intc->set_filter(2, EXR & 7);
		else if(CCR & F_I)
			intc->set_filter(1, EXR & 7);
		else
			intc->set_filter(0, EXR & 7);
		break;
	}
}

void h8s2655_device::interrupt_taken()
{
	standard_irq_callback(intc->interrupt_taken(taken_irq_vector));
}

void h8s2655_device::internal_update(uint64_t current_time)
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
	add_event(event_time, timer16_3->internal_update(current_time));
	add_event(event_time, timer16_4->internal_update(current_time));
	add_event(event_time, timer16_5->internal_update(current_time));
	add_event(event_time, watchdog->internal_update(current_time));

	recompute_bcount(event_time);
}

void h8s2655_device::device_start()
{
	h8s2600_device::device_start();
}

void h8s2655_device::device_reset()
{
	h8s2600_device::device_reset();
	syscr = 0x01;
}

READ8_MEMBER(h8s2655_device::syscr_r)
{
	return syscr;
}

WRITE8_MEMBER(h8s2655_device::syscr_w)
{
	syscr = data;
	update_irq_filter();
	logerror("syscr = %02x\n", data);
}
