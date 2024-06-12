// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h8s2245.h"

DEFINE_DEVICE_TYPE(H8S2241, h8s2241_device, "h8s2241", "Hitachi H8S/2241")
DEFINE_DEVICE_TYPE(H8S2242, h8s2242_device, "h8s2242", "Hitachi H8S/2242")
DEFINE_DEVICE_TYPE(H8S2245, h8s2245_device, "h8s2245", "Hitachi H8S/2245")
DEFINE_DEVICE_TYPE(H8S2246, h8s2246_device, "h8s2246", "Hitachi H8S/2246")


h8s2245_device::h8s2245_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 start) :
	h8s2000_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h8s2245_device::map), this)),
	m_intc(*this, "intc"),
	m_adc(*this, "adc"),
	m_dtc(*this, "dtc"),
	m_port1(*this, "port1"),
	m_port2(*this, "port2"),
	m_port3(*this, "port3"),
	m_port4(*this, "port4"),
	m_port5(*this, "port5"),
	m_porta(*this, "porta"),
	m_portb(*this, "portb"),
	m_portc(*this, "portc"),
	m_portd(*this, "portd"),
	m_porte(*this, "porte"),
	m_portf(*this, "portf"),
	m_portg(*this, "portg"),
	m_timer8_0(*this, "timer8_0"),
	m_timer8_1(*this, "timer8_1"),
	m_timer16(*this, "timer16"),
	m_timer16_0(*this, "timer16:0"),
	m_timer16_1(*this, "timer16:1"),
	m_timer16_2(*this, "timer16:2"),
	m_watchdog(*this, "watchdog"),
	m_ram_start(start),
	m_syscr(0)
{
}

h8s2245_device::h8s2245_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8s2245_device(mconfig, H8S2245, tag, owner, clock, 0xffec00)
{
}

h8s2241_device::h8s2241_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8s2245_device(mconfig, H8S2241, tag, owner, clock, 0xffec00)
{
}

h8s2242_device::h8s2242_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8s2245_device(mconfig, H8S2242, tag, owner, clock, 0xffdc00)
{
}

h8s2246_device::h8s2246_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8s2245_device(mconfig, H8S2246, tag, owner, clock, 0xffdc00)
{
}

void h8s2245_device::map(address_map &map)
{
	map(m_ram_start, 0xfffbff).ram();

	map(0xfffeb0, 0xfffeb0).rw(m_port1, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfffeb1, 0xfffeb1).rw(m_port2, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfffeb2, 0xfffeb2).rw(m_port3, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfffeb4, 0xfffeb4).rw(m_port5, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfffeb9, 0xfffeb9).rw(m_porta, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfffeba, 0xfffeba).rw(m_portb, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfffebb, 0xfffebb).rw(m_portc, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfffebc, 0xfffebc).rw(m_portd, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfffebd, 0xfffebd).rw(m_porte, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfffebe, 0xfffebe).rw(m_portf, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfffebf, 0xfffebf).rw(m_portg, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfffec0, 0xfffec2).rw(m_intc, FUNC(h8s_intc_device::icr_r), FUNC(h8s_intc_device::icr_w));
	map(0xffff2c, 0xffff2c).rw(m_intc, FUNC(h8s_intc_device::iscrh_r), FUNC(h8s_intc_device::iscrh_w));
	map(0xffff2d, 0xffff2d).rw(m_intc, FUNC(h8s_intc_device::iscrl_r), FUNC(h8s_intc_device::iscrl_w));
	map(0xffff2e, 0xffff2e).rw(m_intc, FUNC(h8s_intc_device::ier_r), FUNC(h8s_intc_device::ier_w));
	map(0xffff2f, 0xffff2f).rw(m_intc, FUNC(h8s_intc_device::isr_r), FUNC(h8s_intc_device::isr_w));
	map(0xffff30, 0xffff35).rw(m_dtc, FUNC(h8_dtc_device::dtcer_r), FUNC(h8_dtc_device::dtcer_w));
	map(0xffff37, 0xffff37).rw(m_dtc, FUNC(h8_dtc_device::dtvecr_r), FUNC(h8_dtc_device::dtvecr_w));
	map(0xffff39, 0xffff39).rw(FUNC(h8s2245_device::syscr_r), FUNC(h8s2245_device::syscr_w));
	map(0xffff3c, 0xffff3d).rw(FUNC(h8s2245_device::mstpcr_r), FUNC(h8s2245_device::mstpcr_w));

	map(0xffff50, 0xffff50).r(m_port1, FUNC(h8_port_device::port_r));
	map(0xffff51, 0xffff51).r(m_port2, FUNC(h8_port_device::port_r));
	map(0xffff52, 0xffff52).r(m_port3, FUNC(h8_port_device::port_r));
	map(0xffff53, 0xffff53).r(m_port4, FUNC(h8_port_device::port_r));
	map(0xffff54, 0xffff54).r(m_port5, FUNC(h8_port_device::port_r));
	map(0xffff59, 0xffff59).r(m_porta, FUNC(h8_port_device::port_r));
	map(0xffff5a, 0xffff5a).r(m_portb, FUNC(h8_port_device::port_r));
	map(0xffff5b, 0xffff5b).r(m_portc, FUNC(h8_port_device::port_r));
	map(0xffff5c, 0xffff5c).r(m_portd, FUNC(h8_port_device::port_r));
	map(0xffff5d, 0xffff5d).r(m_porte, FUNC(h8_port_device::port_r));
	map(0xffff5e, 0xffff5e).r(m_portf, FUNC(h8_port_device::port_r));
	map(0xffff5f, 0xffff5f).r(m_portg, FUNC(h8_port_device::port_r));
	map(0xffff60, 0xffff60).rw(m_port1, FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff61, 0xffff61).rw(m_port2, FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff62, 0xffff62).rw(m_port3, FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff64, 0xffff64).rw(m_port5, FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff69, 0xffff69).rw(m_porta, FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff6a, 0xffff6a).rw(m_portb, FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff6b, 0xffff6b).rw(m_portc, FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff6c, 0xffff6c).rw(m_portd, FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff6d, 0xffff6d).rw(m_porte, FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff6e, 0xffff6e).rw(m_portf, FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff6f, 0xffff6f).rw(m_portg, FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff70, 0xffff70).rw(m_porta, FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffff71, 0xffff71).rw(m_portb, FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffff72, 0xffff72).rw(m_portc, FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffff73, 0xffff73).rw(m_portd, FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffff74, 0xffff74).rw(m_porte, FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(0xffff76, 0xffff76).rw(m_port3, FUNC(h8_port_device::odr_r), FUNC(h8_port_device::odr_w));
	map(0xffff77, 0xffff77).rw(m_porta, FUNC(h8_port_device::odr_r), FUNC(h8_port_device::odr_w));
	map(0xffff78, 0xffff78).rw(m_sci[0], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffff79, 0xffff79).rw(m_sci[0], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffff7a, 0xffff7a).rw(m_sci[0], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffff7b, 0xffff7b).rw(m_sci[0], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffff7c, 0xffff7c).rw(m_sci[0], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffff7d, 0xffff7d).r(m_sci[0], FUNC(h8_sci_device::rdr_r));
	map(0xffff7e, 0xffff7e).rw(m_sci[0], FUNC(h8_sci_device::scmr_r), FUNC(h8_sci_device::scmr_w));
	map(0xffff80, 0xffff80).rw(m_sci[1], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffff81, 0xffff81).rw(m_sci[1], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffff82, 0xffff82).rw(m_sci[1], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffff83, 0xffff83).rw(m_sci[1], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffff84, 0xffff84).rw(m_sci[1], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffff85, 0xffff85).r(m_sci[1], FUNC(h8_sci_device::rdr_r));
	map(0xffff86, 0xffff86).rw(m_sci[1], FUNC(h8_sci_device::scmr_r), FUNC(h8_sci_device::scmr_w));
	map(0xffff88, 0xffff88).rw(m_sci[2], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffff89, 0xffff89).rw(m_sci[2], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffff8a, 0xffff8a).rw(m_sci[2], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffff8b, 0xffff8b).rw(m_sci[2], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffff8c, 0xffff8c).rw(m_sci[2], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffff8d, 0xffff8d).r(m_sci[2], FUNC(h8_sci_device::rdr_r));
	map(0xffff8e, 0xffff8e).rw(m_sci[2], FUNC(h8_sci_device::scmr_r), FUNC(h8_sci_device::scmr_w));
	map(0xffff90, 0xffff97).r(m_adc, FUNC(h8_adc_device::addr8_r));
	map(0xffff98, 0xffff98).rw(m_adc, FUNC(h8_adc_device::adcsr_r), FUNC(h8_adc_device::adcsr_w));
	map(0xffff99, 0xffff99).rw(m_adc, FUNC(h8_adc_device::adcr_r), FUNC(h8_adc_device::adcr_w));
	map(0xffffb0, 0xffffb0).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffffb1, 0xffffb1).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcr_r), FUNC(h8_timer8_channel_device::tcr_w));
	map(0xffffb2, 0xffffb2).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffffb3, 0xffffb3).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcsr_r), FUNC(h8_timer8_channel_device::tcsr_w));
	map(0xffffb4, 0xffffb7).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w)).umask16(0xff00);
	map(0xffffb4, 0xffffb7).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcor_r), FUNC(h8_timer8_channel_device::tcor_w)).umask16(0x00ff);
	map(0xffffb8, 0xffffb8).rw(m_timer8_0, FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(0xffffb9, 0xffffb9).rw(m_timer8_1, FUNC(h8_timer8_channel_device::tcnt_r), FUNC(h8_timer8_channel_device::tcnt_w));
	map(0xffffbc, 0xffffbd).rw(m_watchdog, FUNC(h8_watchdog_device::wd_r), FUNC(h8_watchdog_device::wd_w));
	map(0xffffbe, 0xffffbf).rw(m_watchdog, FUNC(h8_watchdog_device::rst_r), FUNC(h8_watchdog_device::rst_w));
	map(0xffffc0, 0xffffc0).rw(m_timer16, FUNC(h8_timer16_device::tstr_r), FUNC(h8_timer16_device::tstr_w));
	map(0xffffc1, 0xffffc1).rw(m_timer16, FUNC(h8_timer16_device::tsyr_r), FUNC(h8_timer16_device::tsyr_w));
	map(0xffffd0, 0xffffd0).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(0xffffd1, 0xffffd1).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tmdr_r), FUNC(h8_timer16_channel_device::tmdr_w));
	map(0xffffd2, 0xffffd3).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(0xffffd4, 0xffffd4).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(0xffffd5, 0xffffd5).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(0xffffd6, 0xffffd7).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(0xffffd8, 0xffffdf).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(0xffffe0, 0xffffe0).rw(m_timer16_1, FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(0xffffe1, 0xffffe1).rw(m_timer16_1, FUNC(h8_timer16_channel_device::tmdr_r), FUNC(h8_timer16_channel_device::tmdr_w));
	map(0xffffe2, 0xffffe2).rw(m_timer16_1, FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(0xffffe4, 0xffffe4).rw(m_timer16_1, FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(0xffffe5, 0xffffe5).rw(m_timer16_1, FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(0xffffe6, 0xffffe7).rw(m_timer16_1, FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(0xffffe8, 0xffffeb).rw(m_timer16_1, FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(0xfffff0, 0xfffff0).rw(m_timer16_2, FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(0xfffff1, 0xfffff1).rw(m_timer16_2, FUNC(h8_timer16_channel_device::tmdr_r), FUNC(h8_timer16_channel_device::tmdr_w));
	map(0xfffff2, 0xfffff2).rw(m_timer16_2, FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(0xfffff4, 0xfffff4).rw(m_timer16_2, FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(0xfffff5, 0xfffff5).rw(m_timer16_2, FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(0xfffff6, 0xfffff7).rw(m_timer16_2, FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(0xfffff8, 0xfffffb).rw(m_timer16_2, FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
}

void h8s2245_device::device_add_mconfig(machine_config &config)
{
	H8S_INTC(config, m_intc, *this);
	H8_ADC_2245(config, m_adc, *this, m_intc, 28);
	H8_DTC(config, m_dtc, *this, m_intc, 24);
	H8_PORT(config, m_port1, *this, h8_device::PORT_1, 0x00, 0x00);
	H8_PORT(config, m_port2, *this, h8_device::PORT_2, 0x00, 0x00);
	H8_PORT(config, m_port3, *this, h8_device::PORT_3, 0xc0, 0xc0);
	H8_PORT(config, m_port4, *this, h8_device::PORT_4, 0xf0, 0xf0);
	H8_PORT(config, m_port5, *this, h8_device::PORT_5, 0xf0, 0xf0);
	H8_PORT(config, m_porta, *this, h8_device::PORT_A, 0xf0, 0xf0);
	H8_PORT(config, m_portb, *this, h8_device::PORT_B, 0x00, 0x00);
	H8_PORT(config, m_portc, *this, h8_device::PORT_C, 0x00, 0x00);
	H8_PORT(config, m_portd, *this, h8_device::PORT_D, 0x00, 0x00);
	H8_PORT(config, m_porte, *this, h8_device::PORT_E, 0x00, 0x00);
	H8_PORT(config, m_portf, *this, h8_device::PORT_F, 0x00, 0x00);
	H8_PORT(config, m_portg, *this, h8_device::PORT_G, 0xe0, 0x00);
	H8H_TIMER8_CHANNEL(config, m_timer8_0, *this, m_intc, 64, 65, 66, m_timer8_1, h8_timer8_channel_device::CHAIN_OVERFLOW, true, false);
	H8H_TIMER8_CHANNEL(config, m_timer8_1, *this, m_intc, 68, 69, 70, m_timer8_0, h8_timer8_channel_device::CHAIN_A, false, false);
	H8_TIMER16(config, m_timer16, *this, 3, 0x00);
	H8S_TIMER16_CHANNEL(config, m_timer16_0, *this, 4, 0x60, m_intc, 32,
			h8_timer16_channel_device::DIV_1,
			h8_timer16_channel_device::DIV_4,
			h8_timer16_channel_device::DIV_16,
			h8_timer16_channel_device::DIV_64,
			h8_timer16_channel_device::INPUT_A,
			h8_timer16_channel_device::INPUT_B,
			h8_timer16_channel_device::INPUT_C,
			h8_timer16_channel_device::INPUT_D);
	H8S_TIMER16_CHANNEL(config, m_timer16_1, *this, 2, 0x4c, m_intc, 40,
			h8_timer16_channel_device::DIV_1,
			h8_timer16_channel_device::DIV_4,
			h8_timer16_channel_device::DIV_16,
			h8_timer16_channel_device::DIV_64,
			h8_timer16_channel_device::INPUT_A,
			h8_timer16_channel_device::INPUT_B,
			h8_timer16_channel_device::DIV_256,
			h8_timer16_channel_device::CHAIN).set_chain(m_timer16_2);
	H8S_TIMER16_CHANNEL(config, m_timer16_2, *this, 2, 0x4c, m_intc, 44,
			h8_timer16_channel_device::DIV_1,
			h8_timer16_channel_device::DIV_4,
			h8_timer16_channel_device::DIV_16,
			h8_timer16_channel_device::DIV_64,
			h8_timer16_channel_device::INPUT_A,
			h8_timer16_channel_device::INPUT_B,
			h8_timer16_channel_device::INPUT_C,
			h8_timer16_channel_device::DIV_1024);
	H8_SCI(config, m_sci[0], 0, *this, m_intc, 80, 81, 82, 83);
	H8_SCI(config, m_sci[1], 1, *this, m_intc, 84, 85, 86, 87);
	H8_SCI(config, m_sci[2], 2, *this, m_intc, 88, 89, 90, 91);
	H8_WATCHDOG(config, m_watchdog, *this, m_intc, 25, h8_watchdog_device::S);
}

void h8s2245_device::execute_set_input(int inputnum, int state)
{
	m_intc->set_input(inputnum, state);
}

bool h8s2245_device::exr_in_stack() const
{
	return false;
}

int h8s2245_device::trapa_setup()
{
	if(m_syscr & 0x10)
		m_CCR |= F_I|F_UI;
	else
		m_CCR |= F_I;
	return 8;
}

void h8s2245_device::irq_setup()
{
	switch(m_syscr & 0x30) {
	case 0x00:
		m_CCR |= F_I;
		break;
	case 0x10:
		m_CCR |= F_I|F_UI;
		break;
	}
}

void h8s2245_device::update_irq_filter()
{
	switch(m_syscr & 0x30) {
	case 0x00:
		if(m_CCR & F_I)
			m_intc->set_filter(2, -1);
		else
			m_intc->set_filter(0, -1);
		break;
	case 0x10:
		if((m_CCR & (F_I|F_UI)) == (F_I|F_UI))
			m_intc->set_filter(2, -1);
		else if(m_CCR & F_I)
			m_intc->set_filter(1, -1);
		else
			m_intc->set_filter(0, -1);
		break;
	}
}

void h8s2245_device::interrupt_taken()
{
	standard_irq_callback(m_intc->interrupt_taken(m_taken_irq_vector), m_NPC);
}

void h8s2245_device::internal_update(u64 current_time)
{
	u64 event_time = 0;

	add_event(event_time, m_adc->internal_update(current_time));
	add_event(event_time, m_sci[0]->internal_update(current_time));
	add_event(event_time, m_sci[1]->internal_update(current_time));
	add_event(event_time, m_sci[2]->internal_update(current_time));
	add_event(event_time, m_timer8_0->internal_update(current_time));
	add_event(event_time, m_timer8_1->internal_update(current_time));
	add_event(event_time, m_timer16_0->internal_update(current_time));
	add_event(event_time, m_timer16_1->internal_update(current_time));
	add_event(event_time, m_timer16_2->internal_update(current_time));
	add_event(event_time, m_watchdog->internal_update(current_time));

	recompute_bcount(event_time);
}

void h8s2245_device::notify_standby(int state)
{
	m_adc->notify_standby(state);
	m_sci[0]->notify_standby(state);
	m_sci[1]->notify_standby(state);
	m_sci[2]->notify_standby(state);
	m_timer8_0->notify_standby(state);
	m_timer8_1->notify_standby(state);
	m_timer16_0->notify_standby(state);
	m_timer16_1->notify_standby(state);
	m_timer16_2->notify_standby(state);
	m_watchdog->notify_standby(state);
}

void h8s2245_device::device_start()
{
	h8s2000_device::device_start();
	m_dtc_device = m_dtc;

	save_item(NAME(m_syscr));
	save_item(NAME(m_mstpcr));
}

void h8s2245_device::device_reset()
{
	h8s2000_device::device_reset();
	m_syscr = 0x01;
	m_mstpcr = 0x3fff;
}

u8 h8s2245_device::syscr_r()
{
	return m_syscr;
}

void h8s2245_device::syscr_w(u8 data)
{
	m_syscr = data;
	update_irq_filter();
	logerror("syscr = %02x\n", data);
}

u16 h8s2245_device::mstpcr_r()
{
	return m_mstpcr;
}

void h8s2245_device::mstpcr_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 omstpcr = m_mstpcr;
	COMBINE_DATA(&m_mstpcr);
	if((omstpcr ^ m_mstpcr) & 0x72e0) {
		std::ostringstream message;
		message << "Online modules:";
		if(m_mstpcr & 0x0020) message << " sci0";
		if(m_mstpcr & 0x0040) message << " sci1";
		if(m_mstpcr & 0x0080) message << " sci2";
		if(m_mstpcr & 0x0200) message << " adc";
		if(m_mstpcr & 0x1000) message << " timer8";
		if(m_mstpcr & 0x2000) message << " timer16";
		if(m_mstpcr & 0x4000) message << " dtc";
		message << "\n";
		logerror(std::move(message).str());
	}
}
