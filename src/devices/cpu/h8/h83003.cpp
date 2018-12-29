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
	map(0xfffd10, 0xffff0f).ram();

	// DMA: only full address mode supported
	map(0xffff20, 0xffff21).rw("dma:0", FUNC(h8_dma_channel_device::marah_r), FUNC(h8_dma_channel_device::marah_w));
	map(0xffff22, 0xffff23).rw("dma:0", FUNC(h8_dma_channel_device::maral_r), FUNC(h8_dma_channel_device::maral_w));
	map(0xffff24, 0xffff25).rw("dma:0", FUNC(h8_dma_channel_device::etcra_r), FUNC(h8_dma_channel_device::etcra_w));
	map(0xffff27, 0xffff27).rw("dma:0", FUNC(h8_dma_channel_device::dtcra_r), FUNC(h8_dma_channel_device::dtcra_w));
	map(0xffff28, 0xffff29).rw("dma:0", FUNC(h8_dma_channel_device::marbh_r), FUNC(h8_dma_channel_device::marbh_w));
	map(0xffff2a, 0xffff2b).rw("dma:0", FUNC(h8_dma_channel_device::marbl_r), FUNC(h8_dma_channel_device::marbl_w));
	map(0xffff2c, 0xffff2d).rw("dma:0", FUNC(h8_dma_channel_device::etcrb_r), FUNC(h8_dma_channel_device::etcrb_w));
	map(0xffff2f, 0xffff2f).rw("dma:0", FUNC(h8_dma_channel_device::dtcrb_r), FUNC(h8_dma_channel_device::dtcrb_w));
	map(0xffff30, 0xffff31).rw("dma:1", FUNC(h8_dma_channel_device::marah_r), FUNC(h8_dma_channel_device::marah_w));
	map(0xffff32, 0xffff33).rw("dma:1", FUNC(h8_dma_channel_device::maral_r), FUNC(h8_dma_channel_device::maral_w));
	map(0xffff34, 0xffff35).rw("dma:1", FUNC(h8_dma_channel_device::etcra_r), FUNC(h8_dma_channel_device::etcra_w));
	map(0xffff37, 0xffff37).rw("dma:1", FUNC(h8_dma_channel_device::dtcra_r), FUNC(h8_dma_channel_device::dtcra_w));
	map(0xffff38, 0xffff39).rw("dma:1", FUNC(h8_dma_channel_device::marbh_r), FUNC(h8_dma_channel_device::marbh_w));
	map(0xffff3a, 0xffff3b).rw("dma:1", FUNC(h8_dma_channel_device::marbl_r), FUNC(h8_dma_channel_device::marbl_w));
	map(0xffff3c, 0xffff3d).rw("dma:1", FUNC(h8_dma_channel_device::etcrb_r), FUNC(h8_dma_channel_device::etcrb_w));
	map(0xffff3f, 0xffff3f).rw("dma:1", FUNC(h8_dma_channel_device::dtcrb_r), FUNC(h8_dma_channel_device::dtcrb_w));
	map(0xffff40, 0xffff41).rw("dma:2", FUNC(h8_dma_channel_device::marah_r), FUNC(h8_dma_channel_device::marah_w));
	map(0xffff42, 0xffff43).rw("dma:2", FUNC(h8_dma_channel_device::maral_r), FUNC(h8_dma_channel_device::maral_w));
	map(0xffff44, 0xffff45).rw("dma:2", FUNC(h8_dma_channel_device::etcra_r), FUNC(h8_dma_channel_device::etcra_w));
	map(0xffff47, 0xffff47).rw("dma:2", FUNC(h8_dma_channel_device::dtcra_r), FUNC(h8_dma_channel_device::dtcra_w));
	map(0xffff48, 0xffff49).rw("dma:2", FUNC(h8_dma_channel_device::marbh_r), FUNC(h8_dma_channel_device::marbh_w));
	map(0xffff4a, 0xffff4b).rw("dma:2", FUNC(h8_dma_channel_device::marbl_r), FUNC(h8_dma_channel_device::marbl_w));
	map(0xffff4c, 0xffff4d).rw("dma:2", FUNC(h8_dma_channel_device::etcrb_r), FUNC(h8_dma_channel_device::etcrb_w));
	map(0xffff4f, 0xffff4f).rw("dma:2", FUNC(h8_dma_channel_device::dtcrb_r), FUNC(h8_dma_channel_device::dtcrb_w));
	map(0xffff50, 0xffff51).rw("dma:3", FUNC(h8_dma_channel_device::marah_r), FUNC(h8_dma_channel_device::marah_w));
	map(0xffff52, 0xffff53).rw("dma:3", FUNC(h8_dma_channel_device::maral_r), FUNC(h8_dma_channel_device::maral_w));
	map(0xffff54, 0xffff55).rw("dma:3", FUNC(h8_dma_channel_device::etcra_r), FUNC(h8_dma_channel_device::etcra_w));
	map(0xffff57, 0xffff57).rw("dma:3", FUNC(h8_dma_channel_device::dtcra_r), FUNC(h8_dma_channel_device::dtcra_w));

	map(0xffff60, 0xffff60).rw("timer16", FUNC(h8_timer16_device::tstr_r), FUNC(h8_timer16_device::tstr_w));
	map(0xffff61, 0xffff61).rw("timer16", FUNC(h8_timer16_device::tsyr_r), FUNC(h8_timer16_device::tsyr_w));
	map(0xffff62, 0xffff62).rw("timer16", FUNC(h8_timer16_device::tmdr_r), FUNC(h8_timer16_device::tmdr_w));
	map(0xffff63, 0xffff63).rw("timer16", FUNC(h8_timer16_device::tfcr_r), FUNC(h8_timer16_device::tfcr_w));
	map(0xffff64, 0xffff64).rw("timer16:0", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(0xffff65, 0xffff65).rw("timer16:0", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(0xffff66, 0xffff66).rw("timer16:0", FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(0xffff67, 0xffff67).rw("timer16:0", FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(0xffff68, 0xffff69).rw("timer16:0", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(0xffff6a, 0xffff6d).rw("timer16:0", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(0xffff6e, 0xffff6e).rw("timer16:1", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(0xffff6f, 0xffff6f).rw("timer16:1", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(0xffff70, 0xffff70).rw("timer16:1", FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(0xffff71, 0xffff71).rw("timer16:1", FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(0xffff72, 0xffff73).rw("timer16:1", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(0xffff74, 0xffff77).rw("timer16:1", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(0xffff78, 0xffff78).rw("timer16:2", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(0xffff79, 0xffff79).rw("timer16:2", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(0xffff7a, 0xffff7a).rw("timer16:2", FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(0xffff7b, 0xffff7b).rw("timer16:2", FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(0xffff7c, 0xffff7d).rw("timer16:2", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(0xffff7e, 0xffff81).rw("timer16:2", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(0xffff82, 0xffff82).rw("timer16:3", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(0xffff83, 0xffff83).rw("timer16:3", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(0xffff84, 0xffff84).rw("timer16:3", FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(0xffff85, 0xffff85).rw("timer16:3", FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(0xffff86, 0xffff87).rw("timer16:3", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(0xffff88, 0xffff8b).rw("timer16:3", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(0xffff8c, 0xffff8f).rw("timer16:3", FUNC(h8_timer16_channel_device::tbr_r), FUNC(h8_timer16_channel_device::tbr_w));
	map(0xffff90, 0xffff90).rw("timer16", FUNC(h8_timer16_device::toer_r), FUNC(h8_timer16_device::toer_w));
	map(0xffff91, 0xffff91).rw("timer16", FUNC(h8_timer16_device::tocr_r), FUNC(h8_timer16_device::tocr_w));
	map(0xffff92, 0xffff92).rw("timer16:4", FUNC(h8_timer16_channel_device::tcr_r), FUNC(h8_timer16_channel_device::tcr_w));
	map(0xffff93, 0xffff93).rw("timer16:4", FUNC(h8_timer16_channel_device::tior_r), FUNC(h8_timer16_channel_device::tior_w));
	map(0xffff94, 0xffff94).rw("timer16:4", FUNC(h8_timer16_channel_device::tier_r), FUNC(h8_timer16_channel_device::tier_w));
	map(0xffff95, 0xffff95).rw("timer16:4", FUNC(h8_timer16_channel_device::tsr_r), FUNC(h8_timer16_channel_device::tsr_w));
	map(0xffff96, 0xffff97).rw("timer16:4", FUNC(h8_timer16_channel_device::tcnt_r), FUNC(h8_timer16_channel_device::tcnt_w));
	map(0xffff98, 0xffff9b).rw("timer16:4", FUNC(h8_timer16_channel_device::tgr_r), FUNC(h8_timer16_channel_device::tgr_w));
	map(0xffff9c, 0xffff9f).rw("timer16:4", FUNC(h8_timer16_channel_device::tbr_r), FUNC(h8_timer16_channel_device::tbr_w));

	map(0xffffa8, 0xffffa9).rw("watchdog", FUNC(h8_watchdog_device::wd_r), FUNC(h8_watchdog_device::wd_w));
	map(0xffffaa, 0xffffab).rw("watchdog", FUNC(h8_watchdog_device::rst_r), FUNC(h8_watchdog_device::rst_w));
	map(0xffffad, 0xffffad).rw(FUNC(h83003_device::rtmcsr_r), FUNC(h83003_device::rtmcsr_w));

	map(0xffffb0, 0xffffb0).rw("sci0", FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffffb1, 0xffffb1).rw("sci0", FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffffb2, 0xffffb2).rw("sci0", FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffffb3, 0xffffb3).rw("sci0", FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffffb4, 0xffffb4).rw("sci0", FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffffb5, 0xffffb5).r("sci0", FUNC(h8_sci_device::rdr_r));
	map(0xffffb8, 0xffffb8).rw("sci1", FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffffb9, 0xffffb9).rw("sci1", FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffffba, 0xffffba).rw("sci1", FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffffbb, 0xffffbb).rw("sci1", FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffffbc, 0xffffbc).rw("sci1", FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffffbd, 0xffffbd).r("sci1", FUNC(h8_sci_device::rdr_r));
	map(0xffffc5, 0xffffc5).w("port4", FUNC(h8_port_device::ddr_w));
	map(0xffffc7, 0xffffc7).rw("port4", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffffc9, 0xffffc9).w("port6", FUNC(h8_port_device::ddr_w));
	map(0xffffcb, 0xffffcb).rw("port6", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffffcd, 0xffffcd).w("port8", FUNC(h8_port_device::ddr_w));
	map(0xffffce, 0xffffce).r("port7", FUNC(h8_port_device::port_r));
	map(0xffffcf, 0xffffcf).rw("port8", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffffd0, 0xffffd0).w("port9", FUNC(h8_port_device::ddr_w));
	map(0xffffd1, 0xffffd1).w("porta", FUNC(h8_port_device::ddr_w));
	map(0xffffd2, 0xffffd2).rw("port9", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffffd3, 0xffffd3).rw("porta", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffffd4, 0xffffd4).w("portb", FUNC(h8_port_device::ddr_w));
	map(0xffffd6, 0xffffd6).rw("portb", FUNC(h8_port_device::port_r), FUNC(h8_port_device::dr_w));
	map(0xffffda, 0xffffda).rw("port4", FUNC(h8_port_device::pcr_r), FUNC(h8_port_device::pcr_w));

	map(0xffffe0, 0xffffe7).r("adc", FUNC(h8_adc_device::addr8_r));
	map(0xffffe8, 0xffffe8).rw("adc", FUNC(h8_adc_device::adcsr_r), FUNC(h8_adc_device::adcsr_w));
	map(0xffffe9, 0xffffe9).rw("adc", FUNC(h8_adc_device::adcr_r), FUNC(h8_adc_device::adcr_w));

	map(0xfffff2, 0xfffff2).rw(FUNC(h83003_device::syscr_r), FUNC(h83003_device::syscr_w));
	map(0xfffff4, 0xfffff4).rw("intc", FUNC(h8h_intc_device::iscr_r), FUNC(h8h_intc_device::iscr_w));
	map(0xfffff5, 0xfffff5).rw("intc", FUNC(h8h_intc_device::ier_r), FUNC(h8h_intc_device::ier_w));
	map(0xfffff6, 0xfffff6).rw("intc", FUNC(h8h_intc_device::isr_r), FUNC(h8h_intc_device::isr_w));
	map(0xfffff8, 0xfffff9).rw("intc", FUNC(h8h_intc_device::icr_r), FUNC(h8h_intc_device::icr_w));
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
