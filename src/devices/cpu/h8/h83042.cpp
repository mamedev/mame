// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h83042.h"

DEFINE_DEVICE_TYPE(H83040, h83040_device, "h83040", "Hitachi H8/3040")
DEFINE_DEVICE_TYPE(H83041, h83041_device, "h83041", "Hitachi H8/3041")
DEFINE_DEVICE_TYPE(H83042, h83042_device, "h83042", "Hitachi H8/3042")

h83042_device::h83042_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	h8h_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h83042_device::map), this)),
	m_intc(*this, "intc"),
	m_adc(*this, "adc"),
	m_dma(*this, "dma"),
	m_dma0(*this, "dma:0"),
	m_dma1(*this, "dma:1"),
	m_port1(*this, "port1"),
	m_port2(*this, "port2"),
	m_port3(*this, "port3"),
	m_port4(*this, "port4"),
	m_port5(*this, "port5"),
	m_port6(*this, "port6"),
	m_port7(*this, "port7"),
	m_port8(*this, "port8"),
	m_port9(*this, "port9"),
	m_porta(*this, "porta"),
	m_portb(*this, "portb"),
	m_timer16(*this, "timer16"),
	m_timer16_0(*this, "timer16:0"),
	m_timer16_1(*this, "timer16:1"),
	m_timer16_2(*this, "timer16:2"),
	m_timer16_3(*this, "timer16:3"),
	m_timer16_4(*this, "timer16:4"),
	m_watchdog(*this, "watchdog"),
	m_syscr(0)
{
	m_mode_a20 = true;
}

h83040_device::h83040_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h83042_device(mconfig, H83040, tag, owner, clock)
{
}

h83041_device::h83041_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h83042_device(mconfig, H83041, tag, owner, clock)
{
}

h83042_device::h83042_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h83042_device(mconfig, H83042, tag, owner, clock)
{
}


void h83042_device::map(address_map &map)
{
	const offs_t base = 0xf0000;

	map(base | 0xf710, base | 0xff0f).ram();

	map(base | 0xff20, base | 0xff21).rw(m_dma0, FUNC(h8h_dma_channel_device::marah_r), FUNC(h8h_dma_channel_device::marah_w));
	map(base | 0xff22, base | 0xff23).rw(m_dma0, FUNC(h8h_dma_channel_device::maral_r), FUNC(h8h_dma_channel_device::maral_w));
	map(base | 0xff24, base | 0xff25).rw(m_dma0, FUNC(h8h_dma_channel_device::etcra_r), FUNC(h8h_dma_channel_device::etcra_w));
	map(base | 0xff26, base | 0xff26).rw(m_dma0, FUNC(h8h_dma_channel_device::ioara8_r), FUNC(h8h_dma_channel_device::ioara8_w));
	map(base | 0xff27, base | 0xff27).rw(m_dma0, FUNC(h8h_dma_channel_device::dtcra_r), FUNC(h8h_dma_channel_device::dtcra_w));
	map(base | 0xff28, base | 0xff29).rw(m_dma0, FUNC(h8h_dma_channel_device::marbh_r), FUNC(h8h_dma_channel_device::marbh_w));
	map(base | 0xff2a, base | 0xff2b).rw(m_dma0, FUNC(h8h_dma_channel_device::marbl_r), FUNC(h8h_dma_channel_device::marbl_w));
	map(base | 0xff2c, base | 0xff2d).rw(m_dma0, FUNC(h8h_dma_channel_device::etcrb_r), FUNC(h8h_dma_channel_device::etcrb_w));
	map(base | 0xff2e, base | 0xff2e).rw(m_dma0, FUNC(h8h_dma_channel_device::ioarb8_r), FUNC(h8h_dma_channel_device::ioarb8_w));
	map(base | 0xff2f, base | 0xff2f).rw(m_dma0, FUNC(h8h_dma_channel_device::dtcrb_r), FUNC(h8h_dma_channel_device::dtcrb_w));
	map(base | 0xff30, base | 0xff31).rw(m_dma1, FUNC(h8h_dma_channel_device::marah_r), FUNC(h8h_dma_channel_device::marah_w));
	map(base | 0xff32, base | 0xff33).rw(m_dma1, FUNC(h8h_dma_channel_device::maral_r), FUNC(h8h_dma_channel_device::maral_w));
	map(base | 0xff34, base | 0xff35).rw(m_dma1, FUNC(h8h_dma_channel_device::etcra_r), FUNC(h8h_dma_channel_device::etcra_w));
	map(base | 0xff36, base | 0xff36).rw(m_dma1, FUNC(h8h_dma_channel_device::ioara8_r), FUNC(h8h_dma_channel_device::ioara8_w));
	map(base | 0xff37, base | 0xff37).rw(m_dma1, FUNC(h8h_dma_channel_device::dtcra_r), FUNC(h8h_dma_channel_device::dtcra_w));
	map(base | 0xff38, base | 0xff39).rw(m_dma1, FUNC(h8h_dma_channel_device::marbh_r), FUNC(h8h_dma_channel_device::marbh_w));
	map(base | 0xff3a, base | 0xff3b).rw(m_dma1, FUNC(h8h_dma_channel_device::marbl_r), FUNC(h8h_dma_channel_device::marbl_w));
	map(base | 0xff3c, base | 0xff3d).rw(m_dma1, FUNC(h8h_dma_channel_device::etcrb_r), FUNC(h8h_dma_channel_device::etcrb_w));
	map(base | 0xff3e, base | 0xff3e).rw(m_dma1, FUNC(h8h_dma_channel_device::ioarb8_r), FUNC(h8h_dma_channel_device::ioarb8_w));
	map(base | 0xff3f, base | 0xff3f).rw(m_dma1, FUNC(h8h_dma_channel_device::dtcrb_r), FUNC(h8h_dma_channel_device::dtcrb_w));
	map(base | 0xff60, base | 0xff60).rw(m_timer16, FUNC(h8_timer16_device::tstr_r), FUNC(h8_timer16_device::tstr_w));
	map(base | 0xff61, base | 0xff61).rw(m_timer16, FUNC(h8_timer16_device::tsyr_r), FUNC(h8_timer16_device::tsyr_w));
	map(base | 0xff62, base | 0xff62).rw(m_timer16, FUNC(h8_timer16_device::tmdr_r), FUNC(h8_timer16_device::tmdr_w));
	map(base | 0xff63, base | 0xff63).rw(m_timer16, FUNC(h8_timer16_device::tfcr_r), FUNC(h8_timer16_device::tfcr_w));
	map(base | 0xff64, base | 0xff64).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(base | 0xff65, base | 0xff65).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(base | 0xff66, base | 0xff66).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(base | 0xff67, base | 0xff67).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(base | 0xff68, base | 0xff69).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(base | 0xff6a, base | 0xff6d).rw(m_timer16_0, FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(base | 0xff6e, base | 0xff6e).rw(m_timer16_1, FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(base | 0xff6f, base | 0xff6f).rw(m_timer16_1, FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(base | 0xff70, base | 0xff70).rw(m_timer16_1, FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(base | 0xff71, base | 0xff71).rw(m_timer16_1, FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(base | 0xff72, base | 0xff73).rw(m_timer16_1, FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(base | 0xff74, base | 0xff77).rw(m_timer16_1, FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(base | 0xff78, base | 0xff78).rw(m_timer16_2, FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(base | 0xff79, base | 0xff79).rw(m_timer16_2, FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(base | 0xff7a, base | 0xff7a).rw(m_timer16_2, FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(base | 0xff7b, base | 0xff7b).rw(m_timer16_2, FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(base | 0xff7c, base | 0xff7d).rw(m_timer16_2, FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(base | 0xff7e, base | 0xff81).rw(m_timer16_2, FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(base | 0xff82, base | 0xff82).rw(m_timer16_3, FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(base | 0xff83, base | 0xff83).rw(m_timer16_3, FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(base | 0xff84, base | 0xff84).rw(m_timer16_3, FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(base | 0xff85, base | 0xff85).rw(m_timer16_3, FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(base | 0xff86, base | 0xff87).rw(m_timer16_3, FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(base | 0xff88, base | 0xff8b).rw(m_timer16_3, FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(base | 0xff8c, base | 0xff8f).rw(m_timer16_3, FUNC(h8_timer16_channel_device::tbr_r), FUNC(h8_timer16_channel_device::tbr_w));
	map(base | 0xff90, base | 0xff90).rw(m_timer16, FUNC(h8_timer16_device::toer_r), FUNC(h8_timer16_device::toer_w));
	map(base | 0xff91, base | 0xff91).rw(m_timer16, FUNC(h8_timer16_device::tocr_r), FUNC(h8_timer16_device::tocr_w));
	map(base | 0xff92, base | 0xff92).rw(m_timer16_4, FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(base | 0xff93, base | 0xff93).rw(m_timer16_4, FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(base | 0xff94, base | 0xff94).rw(m_timer16_4, FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(base | 0xff95, base | 0xff95).rw(m_timer16_4, FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(base | 0xff96, base | 0xff97).rw(m_timer16_4, FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(base | 0xff98, base | 0xff9b).rw(m_timer16_4, FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(base | 0xff9c, base | 0xff9f).rw(m_timer16_4, FUNC(h8_timer16_channel_device::tbr_r), FUNC(h8_timer16_channel_device::tbr_w));

	map(base | 0xffa8, base | 0xffa9).rw(m_watchdog, FUNC(h8_watchdog_device::wd_r), FUNC(h8_watchdog_device::wd_w));
	map(base | 0xffaa, base | 0xffab).rw(m_watchdog, FUNC(h8_watchdog_device::rst_r), FUNC(h8_watchdog_device::rst_w));

	map(base | 0xffb0, base | 0xffb0).rw(m_sci[0], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(base | 0xffb1, base | 0xffb1).rw(m_sci[0], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(base | 0xffb2, base | 0xffb2).rw(m_sci[0], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(base | 0xffb3, base | 0xffb3).rw(m_sci[0], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(base | 0xffb4, base | 0xffb4).rw(m_sci[0], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(base | 0xffb5, base | 0xffb5).r(m_sci[0], FUNC(h8_sci_device::rdr_r));
	map(base | 0xffb8, base | 0xffb8).rw(m_sci[1], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(base | 0xffb9, base | 0xffb9).rw(m_sci[1], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(base | 0xffba, base | 0xffba).rw(m_sci[1], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(base | 0xffbb, base | 0xffbb).rw(m_sci[1], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(base | 0xffbc, base | 0xffbc).rw(m_sci[1], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(base | 0xffbd, base | 0xffbd).r(m_sci[1], FUNC(h8_sci_device::rdr_r));
	map(base | 0xffc0, base | 0xffc0).rw(m_port1, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(base | 0xffc1, base | 0xffc1).rw(m_port2, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(base | 0xffc2, base | 0xffc2).rw(m_port1, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffc3, base | 0xffc3).rw(m_port2, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffc4, base | 0xffc4).rw(m_port3, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(base | 0xffc5, base | 0xffc5).rw(m_port4, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(base | 0xffc6, base | 0xffc6).rw(m_port3, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffc7, base | 0xffc7).rw(m_port4, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffc8, base | 0xffc8).rw(m_port5, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(base | 0xffc9, base | 0xffc9).rw(m_port6, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(base | 0xffca, base | 0xffca).rw(m_port5, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffcb, base | 0xffcb).rw(m_port6, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffcd, base | 0xffcd).rw(m_port8, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(base | 0xffce, base | 0xffce).rw(m_port7, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffcf, base | 0xffcf).rw(m_port8, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffd0, base | 0xffd0).rw(m_port9, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(base | 0xffd1, base | 0xffd1).rw(m_porta, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(base | 0xffd2, base | 0xffd2).rw(m_port9, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffd3, base | 0xffd3).rw(m_porta, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffd4, base | 0xffd4).rw(m_portb, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(base | 0xffd6, base | 0xffd6).rw(m_portb, FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffd8, base | 0xffd8).rw(m_port2, FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(base | 0xffda, base | 0xffda).rw(m_port4, FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));
	map(base | 0xffdb, base | 0xffdb).rw(m_port5, FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));

	map(base | 0xffe0, base | 0xffe7).r(m_adc, FUNC(h8_adc_device::addr8_r));
	map(base | 0xffe8, base | 0xffe8).rw(m_adc, FUNC(h8_adc_device::adcsr_r), FUNC(h8_adc_device::adcsr_w));
	map(base | 0xffe9, base | 0xffe9).rw(m_adc, FUNC(h8_adc_device::adcr_r), FUNC(h8_adc_device::adcr_w));

	map(base | 0xfff2, base | 0xfff2).rw(FUNC(h83042_device::syscr_r), FUNC(h83042_device::syscr_w));

	map(base | 0xfff4, base | 0xfff4).rw(m_intc, FUNC(h8h_intc_device::iscr_r), FUNC(h8h_intc_device::iscr_w));
	map(base | 0xfff5, base | 0xfff5).rw(m_intc, FUNC(h8h_intc_device::ier_r), FUNC(h8h_intc_device::ier_w));
	map(base | 0xfff6, base | 0xfff6).rw(m_intc, FUNC(h8h_intc_device::isr_r), FUNC(h8h_intc_device::isr_w));
	map(base | 0xfff8, base | 0xfff9).rw(m_intc, FUNC(h8h_intc_device::icr_r), FUNC(h8h_intc_device::icr_w));
}

void h83042_device::device_add_mconfig(machine_config &config)
{
	H8H_INTC(config, m_intc, *this);
	H8_ADC_3337(config, m_adc, *this, m_intc, 60);
	H8H_DMA(config, m_dma, *this);
	H8H_DMA_CHANNEL(config, m_dma0, *this, m_dma, m_intc, false, false);
	H8H_DMA_CHANNEL(config, m_dma1, *this, m_dma, m_intc, false, false);
	H8_PORT(config, m_port1, *this, h8_device::PORT_1, 0x00, 0x00);
	H8_PORT(config, m_port2, *this, h8_device::PORT_2, 0x00, 0x00);
	H8_PORT(config, m_port3, *this, h8_device::PORT_3, 0x00, 0x00);
	H8_PORT(config, m_port4, *this, h8_device::PORT_4, 0x00, 0x00);
	H8_PORT(config, m_port5, *this, h8_device::PORT_5, 0xf0, 0xf0);
	H8_PORT(config, m_port6, *this, h8_device::PORT_6, 0x80, 0x80);
	H8_PORT(config, m_port7, *this, h8_device::PORT_7, 0x00, 0x00);
	H8_PORT(config, m_port8, *this, h8_device::PORT_8, 0xe0, 0xe0);
	H8_PORT(config, m_port9, *this, h8_device::PORT_9, 0xc0, 0xc0);
	H8_PORT(config, m_porta, *this, h8_device::PORT_A, 0x00, 0x00);
	H8_PORT(config, m_portb, *this, h8_device::PORT_B, 0x00, 0x00);
	H8_TIMER16(config, m_timer16, *this, 5, 0xe0);
	H8H_TIMER16_CHANNEL(config, m_timer16_0, *this, 2, 2, m_intc, 24);
	H8H_TIMER16_CHANNEL(config, m_timer16_1, *this, 2, 2, m_intc, 28);
	H8H_TIMER16_CHANNEL(config, m_timer16_2, *this, 2, 2, m_intc, 32);
	H8H_TIMER16_CHANNEL(config, m_timer16_3, *this, 2, 2, m_intc, 36);
	H8H_TIMER16_CHANNEL(config, m_timer16_4, *this, 2, 2, m_intc, 40);
	H8_SCI(config, m_sci[0], 0, *this, m_intc, 52, 53, 54, 55);
	H8_SCI(config, m_sci[1], 1, *this, m_intc, 56, 57, 58, 59);
	H8_WATCHDOG(config, m_watchdog, *this, m_intc, 20, h8_watchdog_device::H);
}

void h83042_device::execute_set_input(int inputnum, int state)
{
	m_intc->set_input(inputnum, state);
}

int h83042_device::trapa_setup()
{
	if(m_syscr & 0x08)
		m_CCR |= F_I;
	else
		m_CCR |= F_I|F_UI;
	return 8;
}

void h83042_device::irq_setup()
{
	if(m_syscr & 0x08)
		m_CCR |= F_I;
	else
		m_CCR |= F_I|F_UI;
}

void h83042_device::update_irq_filter()
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

void h83042_device::interrupt_taken()
{
	standard_irq_callback(m_intc->interrupt_taken(m_taken_irq_vector), m_NPC);
}

void h83042_device::internal_update(u64 current_time)
{
	u64 event_time = 0;

	add_event(event_time, m_adc->internal_update(current_time));
	add_event(event_time, m_sci[0]->internal_update(current_time));
	add_event(event_time, m_sci[1]->internal_update(current_time));
	add_event(event_time, m_timer16_0->internal_update(current_time));
	add_event(event_time, m_timer16_1->internal_update(current_time));
	add_event(event_time, m_timer16_2->internal_update(current_time));
	add_event(event_time, m_timer16_3->internal_update(current_time));
	add_event(event_time, m_timer16_4->internal_update(current_time));
	add_event(event_time, m_watchdog->internal_update(current_time));

	recompute_bcount(event_time);
}

void h83042_device::notify_standby(int state)
{
	m_adc->notify_standby(state);
	m_sci[0]->notify_standby(state);
	m_sci[1]->notify_standby(state);
	m_timer16_0->notify_standby(state);
	m_timer16_1->notify_standby(state);
	m_timer16_2->notify_standby(state);
	m_timer16_3->notify_standby(state);
	m_timer16_4->notify_standby(state);
	m_watchdog->notify_standby(state);
}

void h83042_device::device_start()
{
	h8h_device::device_start();
	save_item(NAME(m_syscr));
}

void h83042_device::device_reset()
{
	h8h_device::device_reset();
	m_syscr = 0x0b;
}

u8 h83042_device::syscr_r()
{
	return m_syscr;
}

void h83042_device::syscr_w(u8 data)
{
	m_syscr = data;
	update_irq_filter();
	logerror("syscr = %02x\n", data);
}
