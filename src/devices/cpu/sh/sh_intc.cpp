// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    sh_intc.cpp

    SH interrupt controllers family

***************************************************************************/

#include "emu.h"
#include "sh_intc.h"

#include "sh7042.h"

DEFINE_DEVICE_TYPE(SH_INTC, sh_intc_device, "sh_intc", "SH interrupt controller")

const u8 sh_intc_device::pribit[0x100] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 8, 8, 9, 9, 9, 9,
	10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13,
	14, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 17,
	18, 18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21,
	22, 22, 22, 22, 23, 23, 23, 23, 24, 24, 24, 24, 25, 25, 25, 25,
	26, 26, 26, 26, 27, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29,
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
};

sh_intc_device::sh_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SH_INTC, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG)
{
}

void sh_intc_device::device_start()
{
	save_item(NAME(m_ipr));
	save_item(NAME(m_icr));
	save_item(NAME(m_isr));
	save_item(NAME(m_pending));
	save_item(NAME(m_lines));

	std::fill(m_ipr.begin(), m_ipr.end(), 0);
	m_isr = 0;
	m_icr = 0;
	m_lines = 0;
}

void sh_intc_device::device_reset()
{
	std::fill(m_pending.begin(), m_pending.end(), 0);
}

void sh_intc_device::interrupt_taken(int irqline, int vector)
{
	// Don't clear an external interrupt which is level and still active
	if(vector < 64 || vector >= 72 || BIT(m_icr, 7-(vector & 7)) || !BIT(m_lines, vector & 7))
		m_pending[vector >> 5] &= ~(1 << (vector & 31));

	update_irq();
}

void sh_intc_device::update_irq()
{
	int best_level = -1;
	int best_vector = 0;

	for(u32 bv = 64/32; bv != 160/32; bv ++) {
		if(!m_pending[bv])
			continue;
		for(u32 iv = 0; iv != 32; iv++) {
			if(!BIT(m_pending[bv], iv))
				continue;
			u32 vector = bv*32 + iv;
			u32 slot = pribit[vector];
			u32 shift = 12-4*(slot & 3);
			int level = (m_ipr[slot >> 2] >> shift) & 15;
			if(level > best_level) {
				best_level = level;
				best_vector = vector;
			}
		}
	}
	m_cpu->set_internal_interrupt(best_level, best_vector);
}

void sh_intc_device::internal_interrupt(int vector)
{
	m_pending[vector >> 5] |= 1 << (vector & 31);
	update_irq();
}

void sh_intc_device::set_input(int inputnum, int state)
{
	if(BIT(m_lines, inputnum) == state)
		return;
	if(BIT(m_icr, 7-inputnum)) {
		// Level interrupt
		if(state)
			m_pending[64 >> 5] |= 1 << inputnum;
		else
			m_pending[64 >> 5] &= ~(1 << inputnum);
	} else {
		// Edge interrupt
		if(state)
			m_pending[64 >> 5] |= 1 << inputnum;
	}
	update_irq();
}

u16 sh_intc_device::icr_r()
{
	return m_icr;
}

void sh_intc_device::icr_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_icr);
	logerror("icr_w %04x @ %04x\n", data, mem_mask);
}

u16 sh_intc_device::isr_r()
{
	return m_isr;
}

void sh_intc_device::isr_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_isr);
	logerror("isr_w %04x @ %04x\n", data, mem_mask);
}

u16 sh_intc_device::ipr_r(offs_t offset)
{
	return m_ipr[offset];
}

void sh_intc_device::ipr_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_ipr[offset]);
	logerror("ipr_w %x, %04x @ %04x\n", offset, data, mem_mask);
}

