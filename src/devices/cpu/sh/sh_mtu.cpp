// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    sh_mtu.h

    SH DMA controller

***************************************************************************/

#include "emu.h"
#include "sh_mtu.h"
#include "sh7042.h"

#define V 1

DEFINE_DEVICE_TYPE(SH_MTU, sh_mtu_device, "sh_mtu", "SH Multifuntion timer pulse unit")
DEFINE_DEVICE_TYPE(SH_MTU_CHANNEL, sh_mtu_channel_device, "sh_mtu_channel", "SH Multifuntion timer pulse unit channel")

sh_mtu_device::sh_mtu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SH_MTU, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_timer_channel(*this, "%u", 0)
{
}

void sh_mtu_device::device_start()
{
	save_item(NAME(m_tstr));
	save_item(NAME(m_tsyr));
	save_item(NAME(m_toer));
	save_item(NAME(m_tocr));
	save_item(NAME(m_tgcr));
	save_item(NAME(m_tcdr));
	save_item(NAME(m_tddr));
	save_item(NAME(m_tcnts));
	save_item(NAME(m_tcbr));
}

void sh_mtu_device::device_reset()
{
	m_tstr = 0;
	m_tsyr = 0;
	m_toer = 0xc0;
	m_tocr = 0;
	m_tgcr = 0x80;
	m_tcdr = 0xffff;
	m_tddr = 0xffff;
	m_tcnts = 0;
	m_tcbr = 0xffff;
}

u8 sh_mtu_device::tstr_r()
{
	logerror("tstr_r\n");
	return m_tstr;
}

void sh_mtu_device::tstr_w(u8 data)
{
	m_tstr = data;
	logerror("tstr_w %02x\n", m_tstr);
	// To generalize
	m_timer_channel[0]->set_enable(BIT(m_tstr, 0));
	m_timer_channel[1]->set_enable(BIT(m_tstr, 1));
	m_timer_channel[2]->set_enable(BIT(m_tstr, 2));
	m_timer_channel[3]->set_enable(BIT(m_tstr, 6));
	m_timer_channel[4]->set_enable(BIT(m_tstr, 7));
}

u8 sh_mtu_device::tsyr_r()
{
	logerror("tsyr_r\n");
	return m_tsyr;
}

void sh_mtu_device::tsyr_w(u8 data)
{
	m_tsyr = data;
	logerror("tsyr_w %02x\n", m_tsyr);
}

u8 sh_mtu_device::toer_r()
{
	logerror("toer_r\n");
	return m_toer;
}

void sh_mtu_device::toer_w(u8 data)
{
	m_toer = data;
	logerror("toer_w %02x\n", m_toer);
}

u8 sh_mtu_device::tocr_r()
{
	logerror("tocr_r\n");
	return m_tocr;
}

void sh_mtu_device::tocr_w(u8 data)
{
	m_tocr = data;
	logerror("tocr_w %02x\n", m_tocr);
}

u8 sh_mtu_device::tgcr_r()
{
	logerror("tgcr_r\n");
	return m_tgcr;
}

void sh_mtu_device::tgcr_w(u8 data)
{
	m_tgcr = data;
	logerror("tgcr_w %02x\n", m_tgcr);
}

u16 sh_mtu_device::tcdr_r()
{
	logerror("tcdr_r\n");
	return m_tcdr;
}

void sh_mtu_device::tcdr_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tcdr);
	logerror("tcdr_w %04x\n", m_tcdr);
}

u16 sh_mtu_device::tddr_r()
{
	logerror("tddr_r\n");
	return m_tddr;
}

void sh_mtu_device::tddr_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tddr);
	logerror("tddr_w %04x\n", m_tddr);
}

u16 sh_mtu_device::tcnts_r()
{
	logerror("tcnts_r\n");
	return m_tcnts;
}

void sh_mtu_device::tcnts_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tcnts);
	logerror("tcnts_w %04x\n", m_tcnts);
}

u16 sh_mtu_device::tcbr_r()
{
	logerror("tcbr_r\n");
	return m_tcbr;
}

void sh_mtu_device::tcbr_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tcbr);
	logerror("tcbr_w %04x\n", m_tcbr);
}

sh_mtu_channel_device::sh_mtu_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SH_MTU_CHANNEL, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_intc(*this, finder_base::DUMMY_TAG),
	m_chained_timer(*this, finder_base::DUMMY_TAG)
{
}

void sh_mtu_channel_device::device_start()
{
	m_channel_active = false;
	device_reset();

	save_item(NAME(m_tgr_count));
	save_item(NAME(m_tbr_count));
	save_item(NAME(m_tgr_clearing));
	save_item(NAME(m_tcr));
	save_item(NAME(m_tmdr));
	save_item(NAME(m_tior));
	save_item(NAME(m_tier));
	save_item(NAME(m_tsr));
	save_item(NAME(m_clock_type));
	save_item(NAME(m_clock_divider));
	save_item(NAME(m_tcnt));
	save_item(NAME(m_tgr));
	save_item(NAME(m_last_clock_update));
	save_item(NAME(m_event_time));
	save_item(NAME(m_phase));
	save_item(NAME(m_counter_cycle));
	save_item(NAME(m_counter_incrementing));
	save_item(NAME(m_channel_active));

	m_tsr = 0xc0;
}

void sh_mtu_channel_device::device_reset()
{
	// Don't touch channel_active here, top level device handles it.

	m_tbr_count = 0;
	m_tgr_clearing = TGR_CLEAR_NONE;
	m_tcr = 0;
	m_tmdr = 0xc0;
	m_tior = 0;
	m_tier = 0x40 & m_tier_mask;
	m_clock_type = DIV_1;
	m_clock_divider = 0;
	m_tcnt = 0;
	std::fill(m_tgr.begin(), m_tgr.end(), 0xffff);
	m_last_clock_update = 0;
	m_event_time = 0;
	m_phase = 0;
	m_counter_cycle = 1;
	m_counter_incrementing = true;
}

u8 sh_mtu_channel_device::tcr_r()
{
	return m_tcr;
}

void sh_mtu_channel_device::tcr_w(u8 data)
{
	update_counter();
	m_tcr = data;
	logerror("tcr_w %02x\n", m_tcr);
	switch(m_tcr & 0x60) {
	case 0x00:
		m_tgr_clearing = TGR_CLEAR_NONE;
		if(V>=1) logerror("No automatic tcnt clearing\n");
		break;
	case 0x20: case 0x40: {
		m_tgr_clearing = m_tcr & 0x20 ? 0 : 1;
		if(m_tgr_count > 2 && (m_tcr & 0x80))
			m_tgr_clearing += 2;
		if(V>=1) logerror("Auto-clear on tgr%c\n", 'a'+m_tgr_clearing);
		break;
	}
	case 0x60:
		m_tgr_clearing = TGR_CLEAR_EXT;
		if(V>=1) logerror("External sync clear\n");
		break;
	}

	int count_type = m_count_types[m_tcr & 7];
	if(count_type >= DIV_1 && m_clock_type <= DIV_4) {
		m_clock_type = DIV_1;
		m_clock_divider = count_type - DIV_1;
		if(V>=1) logerror("clock divider %d (%d)\n", m_clock_divider, 1 << m_clock_divider);
		if(!m_clock_divider)
			m_phase = 0;
		else {
			switch(m_tcr & 0x18) {
			case 0x00:
				m_phase = 0;
				if(V>=1) logerror("Phase 0\n");
				break;
			case 0x08:
				m_phase = 1 << (m_clock_divider-1);
				if(V>=1) logerror("Phase 180\n");
				break;
			case 0x10: case 0x18:
				m_phase = 0;
				m_clock_divider--;
				if(V>=1) logerror("Phase 0+180\n");
				break;
			}
		}

	} else if(count_type == CHAIN) {
		m_clock_type = CHAIN;
		m_clock_divider = 0;
		m_phase = 0;
		if(V>=1) logerror("chained timer\n");

	} else if(count_type >= INPUT_A && count_type <= INPUT_D) {
		m_clock_type = count_type;
		m_clock_divider = 0;
		m_phase = 0;
		if(V>=1) logerror("counting input %c\n", 'a'+count_type-INPUT_A);
	}
	recalc_event();
}

u8 sh_mtu_channel_device::tmdr_r()
{
	return m_tmdr;
}

void sh_mtu_channel_device::tmdr_w(u8 data)
{
	m_tmdr = data;
	logerror("tmdr_w %02x\n", m_tmdr);
}

u8 sh_mtu_channel_device::tior_r()
{
	return m_tior;
}

void sh_mtu_channel_device::tior_w(u8 data)
{
	m_tior = data;
}

u8 sh_mtu_channel_device::tier_r()
{
	return m_tier;
}

void sh_mtu_channel_device::tier_w(u8 data)
{
	m_tier = data;
	if(0)
		logerror("irq %c%c%c%c%c%c\n",
				 m_tier & IRQ_A ? 'a' : '.',
				 m_tier & IRQ_B ? 'b' : '.',
				 m_tier & IRQ_C ? 'c' : '.',
				 m_tier & IRQ_D ? 'd' : '.',
				 m_tier & IRQ_V ? 'v' : '.',
				 m_tier & IRQ_U ? 'u' : '.');
	recalc_event();
}

u8 sh_mtu_channel_device::tsr_r()
{
	return m_tsr;
}

void sh_mtu_channel_device::tsr_w(u8 data)
{
	update_counter();
	m_tsr = (data & 0x80) | (m_tsr & ~data & 0x7f);
	recalc_event();
}

u16 sh_mtu_channel_device::tcnt_r()
{
	if(!machine().side_effects_disabled())
		update_counter();
	// Need to implement phase counting for the rotary controller on the psr540
	if(m_tmdr & 0xf)
		return 0;
	return m_tcnt;
}

void sh_mtu_channel_device::tcnt_w(offs_t, u16 data, u16 mem_mask)
{
	update_counter();
	COMBINE_DATA(&m_tcnt);
	recalc_event();
}

u16 sh_mtu_channel_device::tgr_r(offs_t reg)
{
	return m_tgr[reg];
}

void sh_mtu_channel_device::tgr_w(offs_t reg, u16 data, u16 mem_mask)
{
	update_counter();
	COMBINE_DATA(&m_tgr[reg]);
	recalc_event();
}

u16 sh_mtu_channel_device::tgrc_r(offs_t reg)
{
	return tgr_r(reg + 2);
}

void sh_mtu_channel_device::tgrc_w(offs_t reg, u16 data, u16 mem_mask)
{
	tgr_w(reg + 2, data, mem_mask);
}

void sh_mtu_channel_device::set_enable(bool enable)
{
	update_counter();
	m_channel_active = enable;
	if(enable)
		logerror("enabled\n");
	else
		logerror("disabled\n");
	recalc_event();
}

u64 sh_mtu_channel_device::internal_update(u64 current_time)
{
	while(m_event_time && current_time >= m_event_time) {
		update_counter(m_event_time);
		recalc_event(m_event_time);
	}

	return m_event_time;
}

void sh_mtu_channel_device::recalc_event(u64 cur_time)
{
	if(!m_channel_active) {
		m_event_time = 0;
		return;
	}

	bool update_cpu = cur_time == 0;
	u64 old_event_time = m_event_time;

	if(m_clock_type != DIV_1) {
		m_event_time = 0;
		if(old_event_time && update_cpu)
			m_cpu->internal_update();

		return;
	}

	if(!cur_time)
		cur_time = m_cpu->current_cycles();

	if(m_counter_incrementing) {
		u32 event_delay = 0xffffffff;
		if(m_tgr_clearing >= 0)
			m_counter_cycle = m_tgr[m_tgr_clearing] + 1;
		else
			m_counter_cycle = 0x10000;
		if((m_tier & IRQ_V && m_interrupt[4] != -1) && (m_counter_cycle == 0x10000 || m_tcnt >= m_counter_cycle))
			event_delay = 0x10000 - m_tcnt;

		for(int i = 0; i < m_tgr_count; i++)
			if(BIT(m_tier, i) && m_interrupt[i] != -1) {
				u32 new_delay = 0xffffffff;
				u16 cmp = m_tgr[i] + 1;
				if(cmp > m_tcnt) {
					if(m_tcnt >= m_counter_cycle || cmp <= m_counter_cycle)
						new_delay = cmp - m_tcnt;
				} else if(cmp <= m_counter_cycle) {
					if(m_tcnt < m_counter_cycle)
						new_delay = (m_counter_cycle - m_tcnt) + cmp;
					else
						new_delay = (0x10000 - m_tcnt) + cmp;
				}

				if(event_delay > new_delay)
					event_delay = new_delay;
		}
		if(event_delay != 0xffffffff)
			m_event_time = ((((cur_time + (1ULL << m_clock_divider) - m_phase) >> m_clock_divider) + event_delay - 1) << m_clock_divider) + m_phase;
		else
			m_event_time = 0;
	} else {
		logerror("decrementing counter\n");
		exit(1);
	}

	if(old_event_time != m_event_time && update_cpu)
		m_cpu->internal_update();
}

void sh_mtu_channel_device::update_counter(u64 cur_time)
{
	if(m_clock_type != DIV_1)
		return;

	if(!cur_time)
		cur_time = m_cpu->current_cycles();

	if(!m_channel_active) {
		m_last_clock_update = cur_time;
		return;
	}

	u64 base_time = m_last_clock_update;
	m_last_clock_update = cur_time;
	u64 new_time = cur_time;
	if(m_clock_divider) {
		base_time = (base_time + m_phase) >> m_clock_divider;
		new_time = (new_time + m_phase) >> m_clock_divider;
	}
	if(new_time == base_time)
		return;

	if(m_counter_incrementing) {
		u16 prev = m_tcnt;
		u64 delta = new_time - base_time;
		u64 tt = m_tcnt + delta;

		if(prev >= m_counter_cycle) {
			if(tt >= 0x10000)
				m_tcnt = (tt - 0x10000) % m_counter_cycle;
			else
				m_tcnt = tt;
		} else
			m_tcnt = tt % m_counter_cycle;

		for(int i = 0; i < m_tgr_count; i++) {
			u16 cmp = m_tgr[i] + 1;
			bool match = m_tcnt == cmp || (tt == cmp && tt == m_counter_cycle);
			if(!match) {
				// Need to do additional checks here for software that polls the flags with interrupts disabled, since recalc_event only schedules IRQ events.
				if(prev >= m_counter_cycle)
					match = (cmp > prev && tt >= cmp) || (cmp <= m_counter_cycle && m_tcnt < m_counter_cycle && (delta - (0x10000 - prev)) >= cmp);
				else if(cmp <= m_counter_cycle)
					match = delta >= m_counter_cycle || (prev < cmp && tt >= cmp) || (m_tcnt <= prev && m_tcnt >= cmp);

				if(match && BIT(m_tier, i) && m_interrupt[i] != -1)
					logerror("update_counter unexpected TGR %d IRQ\n", i);
			}

			if(match) {
				m_tsr |= 1 << i;
				if(BIT(m_tier, i) && m_interrupt[i] != -1)
					m_intc->internal_interrupt(m_interrupt[i]);
			}
		}
		if(tt >= 0x10000 && (m_counter_cycle == 0x10000 || prev >= m_counter_cycle)) {
			m_tsr |= IRQ_V;
			if(m_tier & IRQ_V && m_interrupt[4] != -1)
				m_intc->internal_interrupt(m_interrupt[4]);
		}
	} else {
		logerror("decrementing counter\n");
		exit(1);
	}
}
