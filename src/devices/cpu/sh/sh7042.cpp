// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// SH7042, sh2 variant

#include "emu.h"
#include "sh7042.h"

DEFINE_DEVICE_TYPE(SH7042,  sh7042_device,  "sh7042",  "Hitachi SH-2 (SH7042)")
DEFINE_DEVICE_TYPE(SH7042A, sh7042a_device, "sh7042a", "Hitachi SH-2 (SH7042A)")
DEFINE_DEVICE_TYPE(SH7043,  sh7043_device,  "sh7043",  "Hitachi SH-2 (SH7043)")
DEFINE_DEVICE_TYPE(SH7043A, sh7043a_device, "sh7043a", "Hitachi SH-2 (SH7043A)")

sh7042_device::sh7042_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sh7042_device(mconfig, SH7042, tag, owner, clock)
{
	m_die_a = false;
}

sh7042a_device::sh7042a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sh7042_device(mconfig, SH7042A, tag, owner, clock)
{
	m_die_a = true;
}

sh7043_device::sh7043_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sh7042_device(mconfig, SH7043, tag, owner, clock)
{
	m_die_a = false;
}

sh7043a_device::sh7043a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sh7042_device(mconfig, SH7043A, tag, owner, clock)
{
	m_die_a = true;
}

sh7042_device::sh7042_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	sh2_device(mconfig, type, tag, owner, clock, CPU_TYPE_SH2, address_map_constructor(FUNC(sh7042_device::map), this), 32, 0xffffffff),
	m_intc(*this, "intc"),
	m_adc0(*this, "adc0"),
	m_adc1(*this, "adc1"),
	m_bsc(*this, "bsc"),
	m_cmt(*this, "cmt"),
	m_dmac(*this, "dmac"),
	m_dmac0(*this, "dmac:0"),
	m_dmac1(*this, "dmac:1"),
	m_dmac2(*this, "dmac:2"),
	m_dmac3(*this, "dmac:3"),
	m_mtu(*this, "mtu"),
	m_mtu0(*this, "mtu:0"),
	m_mtu1(*this, "mtu:1"),
	m_mtu2(*this, "mtu:2"),
	m_mtu3(*this, "mtu:3"),
	m_mtu4(*this, "mtu:4"),
	m_porta(*this, "porta"),
	m_portb(*this, "portb"),
	m_portc(*this, "portc"),
	m_portd(*this, "portd"),
	m_porte(*this, "porte"),
	m_portf(*this, "portf"),
	m_sci(*this, "sci%d", 0),
	m_read_adc(*this, 0),
	m_sci_tx(*this),
	m_sci_clk(*this),
	m_read_port16(*this, 0xffff),
	m_write_port16(*this),
	m_read_port32(*this, 0xffffffff),
	m_write_port32(*this)
{
	m_port16_names = "bcef";
	m_port32_names = "ad";
	for(unsigned int i=0; i != m_read_adc.size(); i++)
		m_read_adc[i].bind().set([this, i]() { return adc_default(i); });
	for(unsigned int i=0; i != m_read_port16.size(); i++) {
		m_read_port16[i].bind().set([this, i]() { return port16_default_r(i); });
		m_write_port16[i].bind().set([this, i](u16 data) { port16_default_w(i, data); });
	}
	for(unsigned int i=0; i != m_read_port32.size(); i++) {
		m_read_port32[i].bind().set([this, i]() { return port32_default_r(i); });
		m_write_port32[i].bind().set([this, i](u32 data) { port32_default_w(i, data); });
	}
}

u16 sh7042_device::port16_default_r(int port)
{
	if(!machine().side_effects_disabled())
		logerror("read of un-hooked port %c\n", m_port16_names[port]);
	return 0xffff;
}

void sh7042_device::port16_default_w(int port, u16 data)
{
	logerror("write of un-hooked port %c %04x\n", m_port16_names[port], data);
}

u32 sh7042_device::port32_default_r(int port)
{
	if(!machine().side_effects_disabled())
		logerror("read of un-hooked port %c\n", m_port32_names[port]);
	return 0xffff;
}

void sh7042_device::port32_default_w(int port, u32 data)
{
	logerror("write of un-hooked port %c %04x\n", m_port32_names[port], data);
}


u16 sh7042_device::adc_default(int adc)
{
	logerror("read of un-hooked adc %d\n", adc);
	return 0;
}

void sh7042_device::device_start()
{
	sh2_device::device_start();

	m_event_timer = timer_alloc(FUNC(sh7042_device::event_timer_tick), this);

	save_item(NAME(m_pcf_ah));
	save_item(NAME(m_pcf_al));
	save_item(NAME(m_pcf_b));
	save_item(NAME(m_pcf_c));
	save_item(NAME(m_pcf_dh));
	save_item(NAME(m_pcf_dl));
	save_item(NAME(m_pcf_e));
	save_item(NAME(m_pcf_if));

	m_pcf_ah = 0;
	m_pcf_al = 0;
	m_pcf_b = 0;
	m_pcf_c = 0;
	m_pcf_dh = 0;
	m_pcf_dl = 0;
	m_pcf_e = 0;
	m_pcf_if = 0;
}

void sh7042_device::execute_set_input(int irqline, int state)
{
	m_intc->set_input(irqline, state);
}

void sh7042_device::device_reset()
{
	sh2_device::device_reset();
}


void sh7042_device::map(address_map &map)
{
	map(0xffff81a0, 0xffff81a0).rw(m_sci[0], FUNC(sh_sci_device::smr_r), FUNC(sh_sci_device::smr_w));
	map(0xffff81a1, 0xffff81a1).rw(m_sci[0], FUNC(sh_sci_device::brr_r), FUNC(sh_sci_device::brr_w));
	map(0xffff81a2, 0xffff81a2).rw(m_sci[0], FUNC(sh_sci_device::scr_r), FUNC(sh_sci_device::scr_w));
	map(0xffff81a3, 0xffff81a3).rw(m_sci[0], FUNC(sh_sci_device::tdr_r), FUNC(sh_sci_device::tdr_w));
	map(0xffff81a4, 0xffff81a4).rw(m_sci[0], FUNC(sh_sci_device::ssr_r), FUNC(sh_sci_device::ssr_w));
	map(0xffff81a5, 0xffff81a5).r(m_sci[0], FUNC(sh_sci_device::rdr_r));
	map(0xffff81b0, 0xffff81b0).rw(m_sci[1], FUNC(sh_sci_device::smr_r), FUNC(sh_sci_device::smr_w));
	map(0xffff81b1, 0xffff81b1).rw(m_sci[1], FUNC(sh_sci_device::brr_r), FUNC(sh_sci_device::brr_w));
	map(0xffff81b2, 0xffff81b2).rw(m_sci[1], FUNC(sh_sci_device::scr_r), FUNC(sh_sci_device::scr_w));
	map(0xffff81b3, 0xffff81b3).rw(m_sci[1], FUNC(sh_sci_device::tdr_r), FUNC(sh_sci_device::tdr_w));
	map(0xffff81b4, 0xffff81b4).rw(m_sci[1], FUNC(sh_sci_device::ssr_r), FUNC(sh_sci_device::ssr_w));
	map(0xffff81b5, 0xffff81b5).r(m_sci[1], FUNC(sh_sci_device::rdr_r));

	map(0xffff8200, 0xffff8200).rw(m_mtu3, FUNC(sh_mtu_channel_device::tcr_r), FUNC(sh_mtu_channel_device::tcr_w));
	map(0xffff8201, 0xffff8201).rw(m_mtu4, FUNC(sh_mtu_channel_device::tcr_r), FUNC(sh_mtu_channel_device::tcr_w));
	map(0xffff8202, 0xffff8202).rw(m_mtu3, FUNC(sh_mtu_channel_device::tmdr_r), FUNC(sh_mtu_channel_device::tmdr_w));
	map(0xffff8203, 0xffff8203).rw(m_mtu4, FUNC(sh_mtu_channel_device::tmdr_r), FUNC(sh_mtu_channel_device::tmdr_w));
	map(0xffff8204, 0xffff8205).rw(m_mtu3, FUNC(sh_mtu_channel_device::tior_r), FUNC(sh_mtu_channel_device::tior_w));
	map(0xffff8206, 0xffff8207).rw(m_mtu4, FUNC(sh_mtu_channel_device::tior_r), FUNC(sh_mtu_channel_device::tior_w));
	map(0xffff8208, 0xffff8208).rw(m_mtu3, FUNC(sh_mtu_channel_device::tier_r), FUNC(sh_mtu_channel_device::tier_w));
	map(0xffff8209, 0xffff8209).rw(m_mtu4, FUNC(sh_mtu_channel_device::tier_r), FUNC(sh_mtu_channel_device::tier_w));
	map(0xffff820a, 0xffff820a).rw(m_mtu, FUNC(sh_mtu_device::toer_r), FUNC(sh_mtu_device::toer_w));
	map(0xffff820b, 0xffff820b).rw(m_mtu, FUNC(sh_mtu_device::tocr_r), FUNC(sh_mtu_device::tocr_w));
	map(0xffff820d, 0xffff820d).rw(m_mtu, FUNC(sh_mtu_device::tgcr_r), FUNC(sh_mtu_device::tgcr_w));
	map(0xffff8210, 0xffff8211).rw(m_mtu3, FUNC(sh_mtu_channel_device::tcnt_r), FUNC(sh_mtu_channel_device::tcnt_w));
	map(0xffff8212, 0xffff8213).rw(m_mtu4, FUNC(sh_mtu_channel_device::tcnt_r), FUNC(sh_mtu_channel_device::tcnt_w));
	map(0xffff8214, 0xffff8215).rw(m_mtu, FUNC(sh_mtu_device::tcdr_r), FUNC(sh_mtu_device::tcdr_w));
	map(0xffff8216, 0xffff8217).rw(m_mtu, FUNC(sh_mtu_device::tddr_r), FUNC(sh_mtu_device::tddr_w));
	map(0xffff8218, 0xffff821b).rw(m_mtu3, FUNC(sh_mtu_channel_device::tgr_r), FUNC(sh_mtu_channel_device::tgr_w));
	map(0xffff821c, 0xffff821f).rw(m_mtu4, FUNC(sh_mtu_channel_device::tgr_r), FUNC(sh_mtu_channel_device::tgr_w));
	map(0xffff8220, 0xffff8221).rw(m_mtu, FUNC(sh_mtu_device::tcnts_r), FUNC(sh_mtu_device::tcnts_w));
	map(0xffff8222, 0xffff8223).rw(m_mtu, FUNC(sh_mtu_device::tcbr_r), FUNC(sh_mtu_device::tcbr_w));
	map(0xffff8224, 0xffff8227).rw(m_mtu3, FUNC(sh_mtu_channel_device::tgrc_r), FUNC(sh_mtu_channel_device::tgrc_w));
	map(0xffff8228, 0xffff822b).rw(m_mtu4, FUNC(sh_mtu_channel_device::tgrc_r), FUNC(sh_mtu_channel_device::tgrc_w));
	map(0xffff822c, 0xffff822c).rw(m_mtu3, FUNC(sh_mtu_channel_device::tsr_r), FUNC(sh_mtu_channel_device::tsr_w));
	map(0xffff822d, 0xffff822d).rw(m_mtu4, FUNC(sh_mtu_channel_device::tsr_r), FUNC(sh_mtu_channel_device::tsr_w));
	map(0xffff8240, 0xffff8240).rw(m_mtu, FUNC(sh_mtu_device::tstr_r), FUNC(sh_mtu_device::tstr_w));
	map(0xffff8241, 0xffff8241).rw(m_mtu, FUNC(sh_mtu_device::tsyr_r), FUNC(sh_mtu_device::tsyr_w));
	map(0xffff8260, 0xffff8260).rw(m_mtu0, FUNC(sh_mtu_channel_device::tcr_r), FUNC(sh_mtu_channel_device::tcr_w));
	map(0xffff8261, 0xffff8261).rw(m_mtu0, FUNC(sh_mtu_channel_device::tmdr_r), FUNC(sh_mtu_channel_device::tmdr_w));
	map(0xffff8262, 0xffff8263).rw(m_mtu0, FUNC(sh_mtu_channel_device::tior_r), FUNC(sh_mtu_channel_device::tior_w));
	map(0xffff8264, 0xffff8264).rw(m_mtu0, FUNC(sh_mtu_channel_device::tier_r), FUNC(sh_mtu_channel_device::tier_w));
	map(0xffff8265, 0xffff8265).rw(m_mtu0, FUNC(sh_mtu_channel_device::tsr_r), FUNC(sh_mtu_channel_device::tsr_w));
	map(0xffff8266, 0xffff8267).rw(m_mtu0, FUNC(sh_mtu_channel_device::tcnt_r), FUNC(sh_mtu_channel_device::tcnt_w));
	map(0xffff8268, 0xffff826f).rw(m_mtu0, FUNC(sh_mtu_channel_device::tgr_r), FUNC(sh_mtu_channel_device::tgr_w));
	map(0xffff8280, 0xffff8280).rw(m_mtu1, FUNC(sh_mtu_channel_device::tcr_r), FUNC(sh_mtu_channel_device::tcr_w));
	map(0xffff8281, 0xffff8281).rw(m_mtu1, FUNC(sh_mtu_channel_device::tmdr_r), FUNC(sh_mtu_channel_device::tmdr_w));
	map(0xffff8282, 0xffff8283).rw(m_mtu1, FUNC(sh_mtu_channel_device::tior_r), FUNC(sh_mtu_channel_device::tior_w));
	map(0xffff8284, 0xffff8284).rw(m_mtu1, FUNC(sh_mtu_channel_device::tier_r), FUNC(sh_mtu_channel_device::tier_w));
	map(0xffff8285, 0xffff8285).rw(m_mtu1, FUNC(sh_mtu_channel_device::tsr_r), FUNC(sh_mtu_channel_device::tsr_w));
	map(0xffff8286, 0xffff8287).rw(m_mtu1, FUNC(sh_mtu_channel_device::tcnt_r), FUNC(sh_mtu_channel_device::tcnt_w));
	map(0xffff8288, 0xffff828b).rw(m_mtu1, FUNC(sh_mtu_channel_device::tgr_r), FUNC(sh_mtu_channel_device::tgr_w));
	map(0xffff82a0, 0xffff82a0).rw(m_mtu2, FUNC(sh_mtu_channel_device::tcr_r), FUNC(sh_mtu_channel_device::tcr_w));
	map(0xffff82a1, 0xffff82a1).rw(m_mtu2, FUNC(sh_mtu_channel_device::tmdr_r), FUNC(sh_mtu_channel_device::tmdr_w));
	map(0xffff82a2, 0xffff82a3).rw(m_mtu2, FUNC(sh_mtu_channel_device::tior_r), FUNC(sh_mtu_channel_device::tior_w));
	map(0xffff82a4, 0xffff82a4).rw(m_mtu2, FUNC(sh_mtu_channel_device::tier_r), FUNC(sh_mtu_channel_device::tier_w));
	map(0xffff82a5, 0xffff82a5).rw(m_mtu2, FUNC(sh_mtu_channel_device::tsr_r), FUNC(sh_mtu_channel_device::tsr_w));
	map(0xffff82a6, 0xffff82a7).rw(m_mtu2, FUNC(sh_mtu_channel_device::tcnt_r), FUNC(sh_mtu_channel_device::tcnt_w));
	map(0xffff82a8, 0xffff82ab).rw(m_mtu2, FUNC(sh_mtu_channel_device::tgr_r), FUNC(sh_mtu_channel_device::tgr_w));

	map(0xffff8348, 0xffff8357).rw(m_intc, FUNC(sh_intc_device::ipr_r), FUNC(sh_intc_device::ipr_w));
	map(0xffff8358, 0xffff8359).rw(m_intc, FUNC(sh_intc_device::icr_r), FUNC(sh_intc_device::icr_w));
	map(0xffff835a, 0xffff835b).rw(m_intc, FUNC(sh_intc_device::isr_r), FUNC(sh_intc_device::isr_w));

	map(0xffff8380, 0xffff8383).rw(m_porta, FUNC(sh_port32_device::dr_r), FUNC(sh_port32_device::dr_w));
	map(0xffff8384, 0xffff8387).rw(m_porta, FUNC(sh_port32_device::io_r), FUNC(sh_port32_device::io_w));
	map(0xffff8388, 0xffff8389).rw(FUNC(sh7042_device::pcf_ah_r), FUNC(sh7042_device::pcf_ah_w));
	map(0xffff838c, 0xffff838f).rw(FUNC(sh7042_device::pcf_al_r), FUNC(sh7042_device::pcf_al_w));
	map(0xffff8390, 0xffff8391).rw(m_portb, FUNC(sh_port16_device::dr_r), FUNC(sh_port16_device::dr_w));
	map(0xffff8392, 0xffff8393).rw(m_portc, FUNC(sh_port16_device::dr_r), FUNC(sh_port16_device::dr_w));
	map(0xffff8394, 0xffff8395).rw(m_portb, FUNC(sh_port16_device::io_r), FUNC(sh_port16_device::io_w));
	map(0xffff8396, 0xffff8397).rw(m_portc, FUNC(sh_port16_device::io_r), FUNC(sh_port16_device::io_w));
	map(0xffff8398, 0xffff839b).rw(FUNC(sh7042_device::pcf_b_r), FUNC(sh7042_device::pcf_b_w));
	map(0xffff839c, 0xffff839d).rw(FUNC(sh7042_device::pcf_c_r), FUNC(sh7042_device::pcf_c_w));
	map(0xffff83a0, 0xffff83a3).rw(m_portd, FUNC(sh_port32_device::dr_r), FUNC(sh_port32_device::dr_w));
	map(0xffff83a4, 0xffff83a7).rw(m_portd, FUNC(sh_port32_device::io_r), FUNC(sh_port32_device::io_w));
	map(0xffff83a8, 0xffff83ab).rw(FUNC(sh7042_device::pcf_dh_r), FUNC(sh7042_device::pcf_dh_w));
	map(0xffff83ac, 0xffff83ad).rw(FUNC(sh7042_device::pcf_dl_r), FUNC(sh7042_device::pcf_dl_w));
	map(0xffff83b0, 0xffff83b1).rw(m_porte, FUNC(sh_port16_device::dr_r), FUNC(sh_port16_device::dr_w));
	map(0xffff83b2, 0xffff83b3).r (m_portf, FUNC(sh_port16_device::dr_r));
	map(0xffff83b4, 0xffff83b5).rw(m_porte, FUNC(sh_port16_device::io_r), FUNC(sh_port16_device::io_w));
	map(0xffff83b8, 0xffff83bb).rw(FUNC(sh7042_device::pcf_e_r), FUNC(sh7042_device::pcf_e_w));
	map(0xffff83c8, 0xffff83c9).rw(FUNC(sh7042_device::pcf_if_r), FUNC(sh7042_device::pcf_if_w));
	map(0xffff83d0, 0xffff83d1).rw(m_cmt, FUNC(sh_cmt_device::cmstr_r), FUNC(sh_cmt_device::cmstr_w));
	map(0xffff83d2, 0xffff83d3).rw(m_cmt, FUNC(sh_cmt_device::cmcsr0_r), FUNC(sh_cmt_device::cmcsr0_w));
	map(0xffff83d4, 0xffff83d5).rw(m_cmt, FUNC(sh_cmt_device::cmcnt0_r), FUNC(sh_cmt_device::cmcnt0_w));
	map(0xffff83d6, 0xffff83d7).rw(m_cmt, FUNC(sh_cmt_device::cmcor0_r), FUNC(sh_cmt_device::cmcor0_w));
	map(0xffff83d8, 0xffff83d9).rw(m_cmt, FUNC(sh_cmt_device::cmcsr1_r), FUNC(sh_cmt_device::cmcsr1_w));
	map(0xffff83da, 0xffff83db).rw(m_cmt, FUNC(sh_cmt_device::cmcnt1_r), FUNC(sh_cmt_device::cmcnt1_w));
	map(0xffff83dc, 0xffff83dd).rw(m_cmt, FUNC(sh_cmt_device::cmcor1_r), FUNC(sh_cmt_device::cmcor1_w));

	if(!m_die_a) {
		map(0xffff83e0, 0xffff83e0).rw(m_adc0, FUNC(sh_adc_device::adcsr_r), FUNC(sh_adc_device::adcsr_w));
		map(0xffff83e1, 0xffff83e1).rw(m_adc0, FUNC(sh_adc_device::adcr_r), FUNC(sh_adc_device::adcr_w));
		map(0xffff83f0, 0xffff83ff).r(m_adc0, FUNC(sh_adc_device::addr_r));
	} else {
		map(0xffff8400, 0xffff8407).r(m_adc0, FUNC(sh_adc_device::addr_r));
		map(0xffff8408, 0xffff840f).r(m_adc1, FUNC(sh_adc_device::addr_r));
		map(0xffff8410, 0xffff8410).rw(m_adc0, FUNC(sh_adc_device::adcsr_r), FUNC(sh_adc_device::adcsr_w));
		map(0xffff8411, 0xffff8411).rw(m_adc1, FUNC(sh_adc_device::adcsr_r), FUNC(sh_adc_device::adcsr_w));
		map(0xffff8412, 0xffff8412).rw(m_adc0, FUNC(sh_adc_device::adcr_r), FUNC(sh_adc_device::adcr_w));
		map(0xffff8413, 0xffff8413).rw(m_adc1, FUNC(sh_adc_device::adcr_r), FUNC(sh_adc_device::adcr_w));
	}

	map(0xffff8620, 0xffff8621).rw(m_bsc, FUNC(sh_bsc_device::bcr1_r), FUNC(sh_bsc_device::bcr1_w));
	map(0xffff8622, 0xffff8623).rw(m_bsc, FUNC(sh_bsc_device::bcr2_r), FUNC(sh_bsc_device::bcr2_w));
	map(0xffff8624, 0xffff8625).rw(m_bsc, FUNC(sh_bsc_device::wcr1_r), FUNC(sh_bsc_device::wcr1_w));
	map(0xffff8626, 0xffff8627).rw(m_bsc, FUNC(sh_bsc_device::wcr2_r), FUNC(sh_bsc_device::wcr2_w));
	map(0xffff862a, 0xffff862b).rw(m_bsc, FUNC(sh_bsc_device::dcr_r), FUNC(sh_bsc_device::dcr_w));
	map(0xffff862c, 0xffff862d).rw(m_bsc, FUNC(sh_bsc_device::rtcsr_r), FUNC(sh_bsc_device::rtcsr_w));
	map(0xffff862e, 0xffff862f).rw(m_bsc, FUNC(sh_bsc_device::rtcnt_r), FUNC(sh_bsc_device::rtcnt_w));
	map(0xffff8630, 0xffff8631).rw(m_bsc, FUNC(sh_bsc_device::rtcor_r), FUNC(sh_bsc_device::rtcor_w));
	map(0xffff86b0, 0xffff86b1).rw(m_dmac, FUNC(sh_dmac_device::dmaor_r), FUNC(sh_dmac_device::dmaor_w));
	map(0xffff86c0, 0xffff86c3).rw(m_dmac0, FUNC(sh_dmac_channel_device::sar_r), FUNC(sh_dmac_channel_device::sar_w));
	map(0xffff86c4, 0xffff86c7).rw(m_dmac0, FUNC(sh_dmac_channel_device::dar_r), FUNC(sh_dmac_channel_device::dar_w));
	map(0xffff86c8, 0xffff86cb).rw(m_dmac0, FUNC(sh_dmac_channel_device::dmatcr_r), FUNC(sh_dmac_channel_device::dmatcr_w));
	map(0xffff86cc, 0xffff86cf).rw(m_dmac0, FUNC(sh_dmac_channel_device::chcr_r), FUNC(sh_dmac_channel_device::chcr_w));
	map(0xffff86d0, 0xffff86d3).rw(m_dmac1, FUNC(sh_dmac_channel_device::sar_r), FUNC(sh_dmac_channel_device::sar_w));
	map(0xffff86d4, 0xffff86d7).rw(m_dmac1, FUNC(sh_dmac_channel_device::dar_r), FUNC(sh_dmac_channel_device::dar_w));
	map(0xffff86d8, 0xffff86db).rw(m_dmac1, FUNC(sh_dmac_channel_device::dmatcr_r), FUNC(sh_dmac_channel_device::dmatcr_w));
	map(0xffff86dc, 0xffff86df).rw(m_dmac1, FUNC(sh_dmac_channel_device::chcr_r), FUNC(sh_dmac_channel_device::chcr_w));
	map(0xffff86e0, 0xffff86e3).rw(m_dmac2, FUNC(sh_dmac_channel_device::sar_r), FUNC(sh_dmac_channel_device::sar_w));
	map(0xffff86e4, 0xffff86e7).rw(m_dmac2, FUNC(sh_dmac_channel_device::dar_r), FUNC(sh_dmac_channel_device::dar_w));
	map(0xffff86e8, 0xffff86eb).rw(m_dmac2, FUNC(sh_dmac_channel_device::dmatcr_r), FUNC(sh_dmac_channel_device::dmatcr_w));
	map(0xffff86ec, 0xffff86ef).rw(m_dmac2, FUNC(sh_dmac_channel_device::chcr_r), FUNC(sh_dmac_channel_device::chcr_w));
	map(0xffff86f0, 0xffff86f3).rw(m_dmac3, FUNC(sh_dmac_channel_device::sar_r), FUNC(sh_dmac_channel_device::sar_w));
	map(0xffff86f4, 0xffff86f7).rw(m_dmac3, FUNC(sh_dmac_channel_device::dar_r), FUNC(sh_dmac_channel_device::dar_w));
	map(0xffff86f8, 0xffff86fb).rw(m_dmac3, FUNC(sh_dmac_channel_device::dmatcr_r), FUNC(sh_dmac_channel_device::dmatcr_w));
	map(0xffff86fc, 0xffff86ff).rw(m_dmac3, FUNC(sh_dmac_channel_device::chcr_r), FUNC(sh_dmac_channel_device::chcr_w));

	map(0xfffff000, 0xffffffff).ram();
}

void sh7042_device::device_add_mconfig(machine_config &config)
{
	SH_INTC(config, m_intc, *this);
	if(m_die_a) {
		SH_ADC_MS(config, m_adc0, *this, m_intc, 0, 136);
		SH_ADC_MS(config, m_adc1, *this, m_intc, 4, 137);
	} else
		SH_ADC_HS(config, m_adc0, *this, m_intc, 136);
	SH_BSC(config, m_bsc);
	SH_CMT(config, m_cmt, *this, m_intc, 144, 148);
	SH_DMAC(config, m_dmac, *this);
	SH_DMAC_CHANNEL(config, m_dmac0, *this, m_intc);
	SH_DMAC_CHANNEL(config, m_dmac1, *this, m_intc);
	SH_DMAC_CHANNEL(config, m_dmac2, *this, m_intc);
	SH_DMAC_CHANNEL(config, m_dmac3, *this, m_intc);
	SH_MTU(config, m_mtu, *this, 5);
	SH_MTU_CHANNEL(config, m_mtu0, *this, 4, 0x60, m_intc, 88,
			sh_mtu_channel_device::DIV_1,
			sh_mtu_channel_device::DIV_4,
			sh_mtu_channel_device::DIV_16,
			sh_mtu_channel_device::DIV_64,
			sh_mtu_channel_device::INPUT_A,
			sh_mtu_channel_device::INPUT_B,
			sh_mtu_channel_device::INPUT_C,
			sh_mtu_channel_device::INPUT_D);
	SH_MTU_CHANNEL(config, m_mtu1, *this, 2, 0x4c, m_intc, 96,
			sh_mtu_channel_device::DIV_1,
			sh_mtu_channel_device::DIV_4,
			sh_mtu_channel_device::DIV_16,
			sh_mtu_channel_device::DIV_64,
			sh_mtu_channel_device::INPUT_A,
			sh_mtu_channel_device::INPUT_B,
			sh_mtu_channel_device::DIV_256,
			sh_mtu_channel_device::CHAIN).set_chain(m_mtu2);
	SH_MTU_CHANNEL(config, m_mtu2, *this, 2, 0x4c, m_intc, 104,
			sh_mtu_channel_device::DIV_1,
			sh_mtu_channel_device::DIV_4,
			sh_mtu_channel_device::DIV_16,
			sh_mtu_channel_device::DIV_64,
			sh_mtu_channel_device::INPUT_A,
			sh_mtu_channel_device::INPUT_B,
			sh_mtu_channel_device::INPUT_C,
			sh_mtu_channel_device::DIV_1024);
	SH_MTU_CHANNEL(config, m_mtu3, *this, 4, 0x60, m_intc, 112,
			sh_mtu_channel_device::DIV_1,
			sh_mtu_channel_device::DIV_4,
			sh_mtu_channel_device::DIV_16,
			sh_mtu_channel_device::DIV_64,
			sh_mtu_channel_device::DIV_256,
			sh_mtu_channel_device::DIV_1024,
			sh_mtu_channel_device::INPUT_A,
			sh_mtu_channel_device::INPUT_B);
	SH_MTU_CHANNEL(config, m_mtu4, *this, 4, 0x60, m_intc, 120,
			sh_mtu_channel_device::DIV_1,
			sh_mtu_channel_device::DIV_4,
			sh_mtu_channel_device::DIV_16,
			sh_mtu_channel_device::DIV_64,
			sh_mtu_channel_device::DIV_256,
			sh_mtu_channel_device::DIV_1024,
			sh_mtu_channel_device::INPUT_A,
			sh_mtu_channel_device::INPUT_B);
	SH_PORT32(config, m_porta, *this, 0, 0x00000000, 0xff000000);
	SH_PORT16(config, m_portb, *this, 0, 0x0000, 0xfc00);
	SH_PORT16(config, m_portc, *this, 1, 0x0000, 0x0000);
	SH_PORT32(config, m_portd, *this, 1, 0x0000, 0x0000);
	SH_PORT16(config, m_porte, *this, 2, 0x0000, 0x0000);
	SH_PORT16(config, m_portf, *this, 3, 0x0000, 0xff00);
	SH_SCI(config, m_sci[0], 0, *this, m_intc, 128, 129, 130, 131);
	SH_SCI(config, m_sci[1], 1, *this, m_intc, 132, 133, 134, 135);

}

void sh7042_device::internal_update()
{
	internal_update(current_cycles());
}

void sh7042_device::add_event(u64 &event_time, u64 new_event)
{
	if(!new_event)
		return;
	if(!event_time || event_time > new_event)
		event_time = new_event;
}

void sh7042_device::recompute_timer(u64 event_time)
{
	if(!event_time) {
		m_event_timer->adjust(attotime::never);
		return;
	}

	m_event_timer->adjust(attotime::from_ticks(2*event_time + 1, 2*clock()) - machine().time());
}

TIMER_CALLBACK_MEMBER(sh7042_device::event_timer_tick)
{
	internal_update();
}

void sh7042_device::internal_update(u64 current_time)
{
	u64 event_time = 0;

	add_event(event_time, m_adc0->internal_update(current_time));
	if(m_adc1)
		add_event(event_time, m_adc1->internal_update(current_time));
	add_event(event_time, m_cmt->internal_update(current_time));
	add_event(event_time, m_mtu0->internal_update(current_time));
	add_event(event_time, m_mtu1->internal_update(current_time));
	add_event(event_time, m_mtu2->internal_update(current_time));
	add_event(event_time, m_mtu3->internal_update(current_time));
	add_event(event_time, m_mtu4->internal_update(current_time));
	add_event(event_time, m_sci[0]->internal_update(current_time));
	add_event(event_time, m_sci[1]->internal_update(current_time));

	recompute_timer(event_time);
}

u16 sh7042_device::pcf_ah_r()
{
	return m_pcf_ah;
}

void sh7042_device::pcf_ah_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pcf_ah);
	logerror("pcf ah = %04x\n", m_pcf_ah);
}

u32 sh7042_device::pcf_al_r()
{
	return m_pcf_al;
}

void sh7042_device::pcf_al_w(offs_t, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pcf_al);
	logerror("pcf al = %08x\n", m_pcf_al);
}

u32 sh7042_device::pcf_b_r()
{
	return m_pcf_b;
}

void sh7042_device::pcf_b_w(offs_t, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pcf_b);
	logerror("pcf b = %08x\n", m_pcf_b);
}

u16 sh7042_device::pcf_c_r()
{
	return m_pcf_c;
}

void sh7042_device::pcf_c_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pcf_c);
	logerror("pcf c = %04x\n", m_pcf_c);
}

u32 sh7042_device::pcf_dh_r()
{
	return m_pcf_dh;
}

void sh7042_device::pcf_dh_w(offs_t, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pcf_dh);
	logerror("pcf dh = %08x\n", m_pcf_dh);
}

u16 sh7042_device::pcf_dl_r()
{
	return m_pcf_dl;
}

void sh7042_device::pcf_dl_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pcf_dl);
	logerror("pcf dl = %04x\n", m_pcf_dl);
}

u32 sh7042_device::pcf_e_r()
{
	return m_pcf_e;
}

void sh7042_device::pcf_e_w(offs_t, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pcf_e);
	logerror("pcf e = %08x\n", m_pcf_e);
}

u16 sh7042_device::pcf_if_r()
{
	return m_pcf_if;
}

void sh7042_device::pcf_if_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pcf_if);
	logerror("pcf if = %04x\n", m_pcf_if);
}

void sh7042_device::set_internal_interrupt(int level, u32 vector)
{
	m_sh2_state->internal_irq_level = level;
	m_internal_irq_vector = vector;
	m_test_irq = 1;
}

void sh7042_device::sh2_exception_internal(const char *message, int irqline, int vector)
{
	sh2_device::sh2_exception_internal(message, irqline, vector);
	m_intc->interrupt_taken(irqline, vector);
}
