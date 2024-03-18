// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// SH7042, sh2 variant

#include "emu.h"
#include "sh7042.h"

DEFINE_DEVICE_TYPE(SH7042, sh7042_device, "sh7042", "Hitachi SH-2 (SH7042)")
DEFINE_DEVICE_TYPE(SH7043, sh7043_device, "sh7043", "Hitachi SH-2 (SH7043)")

sh7042_device::sh7042_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sh7042_device(mconfig, SH7042, tag, owner, clock)
{
}

sh7043_device::sh7043_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sh7042_device(mconfig, SH7043, tag, owner, clock)
{
}

sh7042_device::sh7042_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	sh2_device(mconfig, type, tag, owner, clock, CPU_TYPE_SH2, address_map_constructor(FUNC(sh7042_device::map), this), 32, 0xffffffff),
	m_intc(*this, "intc"),
	m_adc(*this, "adc"),
	m_cmt(*this, "cmt"),
	m_porta(*this, "porta"),
	m_portb(*this, "portb"),
	m_portc(*this, "portc"),
	m_portd(*this, "portd"),
	m_porte(*this, "porte"),
	m_portf(*this, "portf"),
	m_read_adc(*this, 0),
	m_sci_tx(*this),
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

void sh7042_device::device_reset()
{
	sh2_device::device_reset();
}


void sh7042_device::map(address_map &map)
{
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

	map(0xffff83e0, 0xffff83e0).rw(m_adc, FUNC(sh_adc_device::adcsr_r), FUNC(sh_adc_device::adcsr_w));
	map(0xffff83e1, 0xffff83e1).rw(m_adc, FUNC(sh_adc_device::adcr_r), FUNC(sh_adc_device::adcr_w));
	map(0xffff83f0, 0xffff83ff).r(m_adc, FUNC(sh_adc_device::addr_r));

	map(0xfffff000, 0xffffffff).ram();
}

void sh7042_device::device_add_mconfig(machine_config &config)
{
	SH_INTC(config, m_intc, *this);
	SH_ADC(config, m_adc, *this, m_intc, 136);
	SH_CMT(config, m_cmt, *this, m_intc, 144, 148);
	SH_PORT32(config, m_porta, *this, 0, 0x00000000, 0xff000000);
	SH_PORT16(config, m_portb, *this, 0, 0x0000, 0xfc00);
	SH_PORT16(config, m_portc, *this, 1, 0x0000, 0x0000);
	SH_PORT32(config, m_portd, *this, 1, 0x0000, 0x0000);
	SH_PORT16(config, m_porte, *this, 2, 0x0000, 0x0000);
	SH_PORT16(config, m_portf, *this, 3, 0x0000, 0xff00);
}

void sh7042_device::do_sci_w(int sci, int state)
{
	logerror("sci %d %d\n", sci, state);
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

	add_event(event_time, m_adc->internal_update(current_time));
	add_event(event_time, m_cmt->internal_update(current_time));

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
