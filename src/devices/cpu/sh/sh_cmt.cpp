// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    sh_cmt.h

    SH Compare/Match timer subsystem


***************************************************************************/

#include "emu.h"
#include "sh7042.h"
#include "sh_intc.h"

DEFINE_DEVICE_TYPE(SH_CMT, sh_cmt_device, "sh_cmt", "SH2/704x CMT")

sh_cmt_device::sh_cmt_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SH_CMT, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_intc(*this, finder_base::DUMMY_TAG),
	m_intc_vector{0, 0},
	m_str(0),
	m_csr{ 0, 0 },
	m_cnt{ 0, 0 },
	m_cor{ 0xffff, 0xffff }
{
}

void sh_cmt_device::device_start()
{
	save_item(NAME(m_next_event));
	save_item(NAME(m_str));
	save_item(NAME(m_csr));
	save_item(NAME(m_cnt));
	save_item(NAME(m_cor));

}

void sh_cmt_device::device_reset()
{
	std::fill(m_next_event.begin(), m_next_event.end(), 0);
	m_str = 0;
	std::fill(m_csr.begin(), m_csr.end(), 0);
	std::fill(m_cnt.begin(), m_cnt.end(), 0);
	std::fill(m_cor.begin(), m_cor.end(), 0xffff);
}

u64 sh_cmt_device::internal_update(u64 current_time)
{
	u64 next = 0;
	for(int i = 0; i != 2; i++) {
		if(m_next_event[i] && current_time >= m_next_event[i]) {
			m_csr[i] |= 0x80;
			if(BIT(m_csr[i], 6))
				m_intc->internal_interrupt(m_intc_vector[i]);
			cnt_update(i, current_time);
		}
		if(!next || (m_next_event[i] && m_next_event[i] < next))
			next = m_next_event[i];
	}
	return next;
}


u16 sh_cmt_device::cmstr_r()
{
	return m_str;
}

u16 sh_cmt_device::cmcsr0_r()
{
	return m_csr[0];
}

u16 sh_cmt_device::cmcnt0_r()
{
	cnt_update(0, m_cpu->current_cycles());
	return m_cnt[0];
}

u16 sh_cmt_device::cmcor0_r()
{
	return m_cor[0];
}

u16 sh_cmt_device::cmcsr1_r()
{
	return m_csr[1];
}

u16 sh_cmt_device::cmcnt1_r()
{
	cnt_update(1, m_cpu->current_cycles());
	return m_cnt[1];
}

u16 sh_cmt_device::cmcor1_r()
{
	return m_cor[1];
}

void sh_cmt_device::cmcsr0_w(offs_t, u16 data, u16 mem_mask)
{
	csr_w(0, data, mem_mask);
}

void sh_cmt_device::cmcsr1_w(offs_t, u16 data, u16 mem_mask)
{
	csr_w(1, data, mem_mask);
}

void sh_cmt_device::cmcnt0_w(offs_t, u16 data, u16 mem_mask)
{
	cnt_w(0, data, mem_mask);
}

void sh_cmt_device::cmcnt1_w(offs_t, u16 data, u16 mem_mask)
{
	cnt_w(1, data, mem_mask);
}

void sh_cmt_device::cmcor0_w(offs_t, u16 data, u16 mem_mask)
{
	cor_w(0, data, mem_mask);
}

void sh_cmt_device::cmcor1_w(offs_t, u16 data, u16 mem_mask)
{
	cor_w(1, data, mem_mask);
}

void sh_cmt_device::cmstr_w(offs_t, u16 data, u16 mem_mask)
{
	cnt_update(0, m_cpu->current_cycles());
	cnt_update(1, m_cpu->current_cycles());
	u16 old = m_str;
	COMBINE_DATA(&m_str);
	for(int i=0; i != 2; i++)
		if(!BIT(old, i) && BIT(m_str, i))
			clock_start(i);
		else if(!BIT(m_str, i))
			m_next_event[i] = 0;
	m_cpu->internal_update();
}

void sh_cmt_device::csr_w(int reg, u16 data, u16 mem_mask)
{
	cnt_update(reg, m_cpu->current_cycles());
	COMBINE_DATA(&m_csr[reg]);
}

void sh_cmt_device::cnt_w(int reg, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_cnt[reg]);
	if((m_str >> reg) & 1) {
		compute_next_event(reg);
		m_cpu->internal_update();
	}
}

void sh_cmt_device::cor_w(int reg, u16 data, u16 mem_mask)
{
	cnt_update(reg, m_cpu->current_cycles());
	COMBINE_DATA(&m_cor[reg]);
	if((m_str >> reg) & 1) {
		compute_next_event(reg);
		m_cpu->internal_update();
	}
}

void sh_cmt_device::clock_start(int clk)
{
	//logerror("start clock %d %dHz\n", clk, (m_cpu->clock() >> (3 + 2*BIT(m_csr[clk], 0, 2))) / (m_cor[clk] + 1));
	compute_next_event(clk);
}

void sh_cmt_device::compute_next_event(int clk)
{
	u64 step1 = 1 << (3 + 2*BIT(m_csr[clk], 0, 2));
	u64 time = m_cpu->current_cycles();
	if(time & (step1 - 1))
		time = (time | (step1 - 1)) + 1;
	s32 counts = m_cor[clk] + 1 - m_cnt[clk];
	if(counts < 0)
		counts += 0x10000;
	time += step1 * counts;
	m_next_event[clk] = time;
}

void sh_cmt_device::cnt_update(int clk, u64 current_time)
{
	if(!((m_str >> clk) & 1))
		return;
	u64 step = (m_cor[clk] + 1) << (3 + 2*BIT(m_csr[clk], 0, 2));
	if(m_next_event[clk]) {
		while(current_time >= m_next_event[clk])
			m_next_event[clk] += step;
		u64 delta = m_next_event[clk] - current_time;
		m_cnt[clk] = m_cor[clk] - ((delta - 1) >> (3 + 2*BIT(m_csr[clk], 0, 2)));
	}
}
