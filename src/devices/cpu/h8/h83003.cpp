// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h83003.h"

DEFINE_DEVICE_TYPE(H83003, h83003_device, "h83003", "Hitachi H8/3003")

h83003_device::h83003_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8h_device(mconfig, H83003, tag, owner, clock, address_map_constructor(FUNC(h83003_device::map), this)),
	intc(*this, "intc"),
	adc(*this, "adc"),
	dma(*this, "dma"),
	dma0(*this, "dma:0"),
	dma1(*this, "dma:1"),
	dma2(*this, "dma:2"),
	port4(*this, "port4"),
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
	tend0_cb(*this),
	tend1_cb(*this)
{
	syscr = 0;
}

void h83003_device::map(address_map &map)
{
	const offs_t base = mode_a20 ? 0xf0000 : 0xff0000;

	map(base | 0xfd10, base | 0xff0f).ram();

	// DMA: only full address mode supported
	map(base | 0xff20, base | 0xff21).rw("dma:0", FUNC(h8_dma_channel_device::marah_r), FUNC(h8_dma_channel_device::marah_w));
	map(base | 0xff22, base | 0xff23).rw("dma:0", FUNC(h8_dma_channel_device::maral_r), FUNC(h8_dma_channel_device::maral_w));
	map(base | 0xff24, base | 0xff25).rw("dma:0", FUNC(h8_dma_channel_device::etcra_r), FUNC(h8_dma_channel_device::etcra_w));
	map(base | 0xff27, base | 0xff27).rw("dma:0", FUNC(h8_dma_channel_device::dtcra_r), FUNC(h8_dma_channel_device::dtcra_w));
	map(base | 0xff28, base | 0xff29).rw("dma:0", FUNC(h8_dma_channel_device::marbh_r), FUNC(h8_dma_channel_device::marbh_w));
	map(base | 0xff2a, base | 0xff2b).rw("dma:0", FUNC(h8_dma_channel_device::marbl_r), FUNC(h8_dma_channel_device::marbl_w));
	map(base | 0xff2c, base | 0xff2d).rw("dma:0", FUNC(h8_dma_channel_device::etcrb_r), FUNC(h8_dma_channel_device::etcrb_w));
	map(base | 0xff2f, base | 0xff2f).rw("dma:0", FUNC(h8_dma_channel_device::dtcrb_r), FUNC(h8_dma_channel_device::dtcrb_w));
	map(base | 0xff30, base | 0xff31).rw("dma:1", FUNC(h8_dma_channel_device::marah_r), FUNC(h8_dma_channel_device::marah_w));
	map(base | 0xff32, base | 0xff33).rw("dma:1", FUNC(h8_dma_channel_device::maral_r), FUNC(h8_dma_channel_device::maral_w));
	map(base | 0xff34, base | 0xff35).rw("dma:1", FUNC(h8_dma_channel_device::etcra_r), FUNC(h8_dma_channel_device::etcra_w));
	map(base | 0xff37, base | 0xff37).rw("dma:1", FUNC(h8_dma_channel_device::dtcra_r), FUNC(h8_dma_channel_device::dtcra_w));
	map(base | 0xff38, base | 0xff39).rw("dma:1", FUNC(h8_dma_channel_device::marbh_r), FUNC(h8_dma_channel_device::marbh_w));
	map(base | 0xff3a, base | 0xff3b).rw("dma:1", FUNC(h8_dma_channel_device::marbl_r), FUNC(h8_dma_channel_device::marbl_w));
	map(base | 0xff3c, base | 0xff3d).rw("dma:1", FUNC(h8_dma_channel_device::etcrb_r), FUNC(h8_dma_channel_device::etcrb_w));
	map(base | 0xff3f, base | 0xff3f).rw("dma:1", FUNC(h8_dma_channel_device::dtcrb_r), FUNC(h8_dma_channel_device::dtcrb_w));
	map(base | 0xff40, base | 0xff41).rw("dma:2", FUNC(h8_dma_channel_device::marah_r), FUNC(h8_dma_channel_device::marah_w));
	map(base | 0xff42, base | 0xff43).rw("dma:2", FUNC(h8_dma_channel_device::maral_r), FUNC(h8_dma_channel_device::maral_w));
	map(base | 0xff44, base | 0xff45).rw("dma:2", FUNC(h8_dma_channel_device::etcra_r), FUNC(h8_dma_channel_device::etcra_w));
	map(base | 0xff47, base | 0xff47).rw("dma:2", FUNC(h8_dma_channel_device::dtcra_r), FUNC(h8_dma_channel_device::dtcra_w));
	map(base | 0xff48, base | 0xff49).rw("dma:2", FUNC(h8_dma_channel_device::marbh_r), FUNC(h8_dma_channel_device::marbh_w));
	map(base | 0xff4a, base | 0xff4b).rw("dma:2", FUNC(h8_dma_channel_device::marbl_r), FUNC(h8_dma_channel_device::marbl_w));
	map(base | 0xff4c, base | 0xff4d).rw("dma:2", FUNC(h8_dma_channel_device::etcrb_r), FUNC(h8_dma_channel_device::etcrb_w));
	map(base | 0xff4f, base | 0xff4f).rw("dma:2", FUNC(h8_dma_channel_device::dtcrb_r), FUNC(h8_dma_channel_device::dtcrb_w));
	map(base | 0xff50, base | 0xff51).rw("dma:3", FUNC(h8_dma_channel_device::marah_r), FUNC(h8_dma_channel_device::marah_w));
	map(base | 0xff52, base | 0xff53).rw("dma:3", FUNC(h8_dma_channel_device::maral_r), FUNC(h8_dma_channel_device::maral_w));
	map(base | 0xff54, base | 0xff55).rw("dma:3", FUNC(h8_dma_channel_device::etcra_r), FUNC(h8_dma_channel_device::etcra_w));
	map(base | 0xff57, base | 0xff57).rw("dma:3", FUNC(h8_dma_channel_device::dtcra_r), FUNC(h8_dma_channel_device::dtcra_w));

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
	map(base | 0xffad, base | 0xffad).rw(FUNC(h83003_device::rtmcsr_r), FUNC(h83003_device::rtmcsr_w));

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
	map(base | 0xffc5, base | 0xffc5).w("port4", FUNC(h8_port_device::ddr_w));
	map(base | 0xffc7, base | 0xffc7).rw("port4", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffc9, base | 0xffc9).w("port6", FUNC(h8_port_device::ddr_w));
	map(base | 0xffcb, base | 0xffcb).rw("port6", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffcd, base | 0xffcd).w("port8", FUNC(h8_port_device::ddr_w));
	map(base | 0xffce, base | 0xffce).r("port7", FUNC(h8_port_device::port_r));
	map(base | 0xffcf, base | 0xffcf).rw("port8", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffd0, base | 0xffd0).w("port9", FUNC(h8_port_device::ddr_w));
	map(base | 0xffd1, base | 0xffd1).w("porta", FUNC(h8_port_device::ddr_w));
	map(base | 0xffd2, base | 0xffd2).rw("port9", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffd3, base | 0xffd3).rw("porta", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffd4, base | 0xffd4).w("portb", FUNC(h8_port_device::ddr_w));
	map(base | 0xffd5, base | 0xffd5).w("portc", FUNC(h8_port_device::ddr_w));
	map(base | 0xffd6, base | 0xffd6).rw("portb", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffd7, base | 0xffd7).rw("portc", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(base | 0xffda, base | 0xffda).rw("port4", FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));

	map(base | 0xffe0, base | 0xffe7).r("adc", FUNC(h8_adc_device::addr8_r));
	map(base | 0xffe8, base | 0xffe8).rw("adc", FUNC(h8_adc_device::adcsr_r), FUNC(h8_adc_device::adcsr_w));
	map(base | 0xffe9, base | 0xffe9).rw("adc", FUNC(h8_adc_device::adcr_r), FUNC(h8_adc_device::adcr_w));

	map(base | 0xfff2, base | 0xfff2).rw(FUNC(h83003_device::syscr_r), FUNC(h83003_device::syscr_w));
	map(base | 0xfff4, base | 0xfff4).rw("intc", FUNC(h8h_intc_device::iscr_r), FUNC(h8h_intc_device::iscr_w));
	map(base | 0xfff5, base | 0xfff5).rw("intc", FUNC(h8h_intc_device::ier_r), FUNC(h8h_intc_device::ier_w));
	map(base | 0xfff6, base | 0xfff6).rw("intc", FUNC(h8h_intc_device::isr_r), FUNC(h8h_intc_device::isr_w));
	map(base | 0xfff8, base | 0xfff9).rw("intc", FUNC(h8h_intc_device::icr_r), FUNC(h8h_intc_device::icr_w));
}

void h83003_device::device_add_mconfig(machine_config &config)
{
	H8H_INTC(config, "intc");
	H8_ADC_3337(config, "adc", "intc", 60);
	H8_DMA(config, "dma");
	// (H8/2002.pdf) Table 8-11 DMAC Activation Sources
	H8_DMA_CHANNEL(config, "dma:0", "intc", 44, h8_dma_channel_device::NONE, 24, h8_dma_channel_device::DREQ_EDGE, h8_dma_channel_device::DREQ_LEVEL, 28, 32, 36, 54, 53);
	H8_DMA_CHANNEL(config, "dma:1", "intc", 46, h8_dma_channel_device::NONE, 24, h8_dma_channel_device::DREQ_EDGE, h8_dma_channel_device::DREQ_LEVEL, 28, 32, 36, 54, 53);
	H8_DMA_CHANNEL(config, "dma:2", "intc", 48, h8_dma_channel_device::NONE, 24, h8_dma_channel_device::DREQ_EDGE, h8_dma_channel_device::DREQ_LEVEL, 28, 32, 36, 54, 53);
	H8_DMA_CHANNEL(config, "dma:3", "intc", 50, h8_dma_channel_device::NONE, 24, h8_dma_channel_device::DREQ_EDGE, h8_dma_channel_device::DREQ_LEVEL, 28, 32, 36, 54, 53);
	H8_PORT(config, "port4", h8_device::PORT_4, 0x00, 0x00);
	H8_PORT(config, "port6", h8_device::PORT_6, 0x80, 0x80);
	H8_PORT(config, "port7", h8_device::PORT_7, 0x00, 0x00);
	H8_PORT(config, "port8", h8_device::PORT_8, 0xf0, 0xe0);
	H8_PORT(config, "port9", h8_device::PORT_9, 0x00, 0xc0);
	H8_PORT(config, "porta", h8_device::PORT_A, 0x00, 0x00);
	H8_PORT(config, "portb", h8_device::PORT_B, 0x00, 0x00);
	H8_PORT(config, "portc", h8_device::PORT_C, 0x00, 0x00);
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

void h83003_device::execute_set_input(int inputnum, int state)
{
	if(inputnum == H8_INPUT_LINE_TEND0 && !tend0_cb.isnull())
		tend0_cb(state);
	else if(inputnum == H8_INPUT_LINE_TEND1 && !tend1_cb.isnull())
		tend1_cb(state);
	else if(inputnum >= H8_INPUT_LINE_DREQ0 && inputnum <= H8_INPUT_LINE_DREQ3)
		dma->set_input(inputnum, state);
	else
		intc->set_input(inputnum, state);
}

int h83003_device::trapa_setup()
{
	if(syscr & 0x08)
		CCR |= F_I;
	else
		CCR |= F_I|F_UI;
	return 8;
}

void h83003_device::irq_setup()
{
	if(syscr & 0x08)
		CCR |= F_I;
	else
		CCR |= F_I|F_UI;
}

void h83003_device::update_irq_filter()
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

void h83003_device::interrupt_taken()
{
	standard_irq_callback(intc->interrupt_taken(taken_irq_vector));
}

void h83003_device::internal_update(uint64_t current_time)
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

void h83003_device::device_start()
{
	h8h_device::device_start();
	dma_device = dma;

	tend0_cb.resolve();
	tend1_cb.resolve();
}

void h83003_device::device_reset()
{
	h8h_device::device_reset();
	syscr = 0x09;
}

READ8_MEMBER(h83003_device::syscr_r)
{
	return syscr;
}

WRITE8_MEMBER(h83003_device::syscr_w)
{
	syscr = data;
	update_irq_filter();
	logerror("syscr = %02x\n", data);
}

READ8_MEMBER(h83003_device::rtmcsr_r)
{
	// set bit 7 -- Compare Match Flag (CMF): This status flag indicates that the RTCNT and RTCOR values have matched.
	return rtmcsr | 0x80;
}

WRITE8_MEMBER(h83003_device::rtmcsr_w)
{
	rtmcsr = data;
	logerror("rtmcsr = %02x\n", data);
}
