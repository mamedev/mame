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

	std::fill(m_ipr.begin(), m_ipr.end(), 0);
	m_isr = 0;
	m_icr = 0;
}

void sh_intc_device::device_reset()
{
}

int sh_intc_device::interrupt_taken(int vector)
{
	return 0;
}

void sh_intc_device::internal_interrupt(int vector)
{
	u32 slot = pribit[vector];
	u32 shift = 12-4*(slot & 3);
	u32 level = (m_ipr[slot >> 2] >> shift) & 15;
	logerror("Internal interrupt %d / %d (ipr%c %d-%d)\n", vector, level, 'a' + (slot >> 2), shift + 3, shift);	
	m_cpu->set_internal_interrupt(level, vector);
}

void sh_intc_device::set_input(int inputnum, int state)
{
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

