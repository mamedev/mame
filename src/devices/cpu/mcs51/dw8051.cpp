// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Synopsys DW8051, in the four-clock configuration used by the nRF24E1.
    The shared MCS-51 core owns instruction dispatch and interrupt arbitration;
    this variant supplies timing, dual data pointers and timer clock selection.

    TODO: Subinstruction interrupt sampling and configurable core options.

***************************************************************************/

#include "emu.h"
#include "dw8051.h"

#include <utility>

DEFINE_DEVICE_TYPE(DW8051, dw8051_device, "dw8051", "Synopsys DW8051")

constexpr u8 dw8051_device::INSTRUCTION_CYCLES[256] =
{
	1, 3, 4, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	4, 3, 4, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	4, 3, 4, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	4, 3, 4, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	3, 3, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	3, 3, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	3, 3, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	3, 3, 2, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 2, 3, 5, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 3, 2, 3, 5, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 3, 2, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	2, 3, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 3, 2, 1, 1, 4, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 3, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 3, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

dw8051_device::dw8051_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dw8051_device(mconfig, DW8051, tag, owner, clock)
{
}

dw8051_device::dw8051_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: i8052_device(mconfig, type, tag, owner, clock, 0)
{
}

void dw8051_device::device_start()
{
	i8052_device::device_start();
	save_item(NAME(m_other_dptr));
	save_item(NAME(m_dps));
	save_item(NAME(m_ckcon));
	save_item(NAME(m_timer_divider));
	save_item(NAME(m_irq_delay));
}

void dw8051_device::device_reset()
{
	i8052_device::device_reset();
	m_other_dptr = 0;
	m_dps = 0;
	m_ckcon = 1;
	m_timer_divider = 0;
	m_irq_delay = 0;
}

void dw8051_device::sfr_map(address_map &map)
{
	i8052_device::sfr_map(map);
	map(0x82, 0x85).rw(FUNC(dw8051_device::pointer_r), FUNC(dw8051_device::pointer_w));
	map(0x86, 0x86).lr8(NAME([this] () -> u8 { return m_dps; })).w(FUNC(dw8051_device::pointer_select_w));
	map(0x8e, 0x8e).lrw8(NAME([this] () -> u8 { return m_ckcon; }), NAME([this] (u8 data) { m_ckcon = data; }));
	map(0xa8, 0xa8).r(FUNC(dw8051_device::ie_r)).w(FUNC(dw8051_device::interrupt_enable_w));
	map(0xb8, 0xb8).r(FUNC(dw8051_device::ip_r)).w(FUNC(dw8051_device::interrupt_priority_w));
}

u8 dw8051_device::pointer_r(offs_t offset)
{
	u16 const pointer = (BIT(m_dps, 0) == BIT(offset, 1)) ? m_dptr : m_other_dptr;
	return pointer >> (8 * BIT(offset, 0));
}

void dw8051_device::pointer_w(offs_t offset, u8 data)
{
	u16 &pointer = (BIT(m_dps, 0) == BIT(offset, 1)) ? m_dptr : m_other_dptr;
	unsigned const shift = 8 * BIT(offset, 0);
	pointer = (pointer & ~(0xff << shift)) | (u16(data) << shift);
}

void dw8051_device::pointer_select_w(u8 data)
{
	data &= 1;
	if (data != m_dps)
		std::swap(m_dptr, m_other_dptr);
	m_dps = data;
}

void dw8051_device::interrupt_enable_w(u8 data)
{
	ie_w(data);
	m_irq_delay = 1;
}

void dw8051_device::interrupt_priority_w(u8 data)
{
	ip_w(data);
	m_irq_delay = 1;
}

void dw8051_device::prepare_instruction(u8 op)
{
	if (op == 0x32)
		m_irq_delay = 1;
	unsigned cycles = INSTRUCTION_CYCLES[op];
	if ((op & 0xef) == 0xe0 || (op & 0xee) == 0xe2)
		cycles += m_ckcon & 7;
	m_inst_cycles += cycles;
}

u8 dw8051_device::interrupt_mask()
{
	if (m_irq_delay)
	{
		--m_irq_delay;
		return 0;
	}
	return (m_pcon & 2) ? 0 : mcs51_cpu_device::interrupt_mask();
}

void dw8051_device::interrupt_taken()
{
	m_pcon &= ~1;
	burn_cycles(4);
	// The first ISR instruction runs before the next interrupt poll.
	m_irq_delayed = 0;
}

void dw8051_device::update_peripherals()
{
	m_timer_divider = (m_timer_divider + 1) % 3;
	// External counters do not depend on the selected internal timer clock.
	update_timer_t0((BIT(m_ckcon, 3) || !m_timer_divider) ? 1 : 0);
	update_timer_t1((BIT(m_ckcon, 4) || !m_timer_divider) ? 1 : 0);
	update_timer_t2((m_t2con & 0x30) ? 2 : (BIT(m_ckcon, 5) || !m_timer_divider) ? 1 : 0);
	if (!m_timer_divider || (m_scon & 0xe0) == 0x20)
		transmit_receive(0);
}

void dw8051_device::update_timer_t2(int cycles)
{
	if (!BIT(m_t2con, T2CON_TR2))
	{
		m_t2_cnt = 0;
		return;
	}
	u32 const delta = BIT(m_t2con, T2CON_CT2) ? m_t2_cnt : cycles;
	m_t2_cnt = 0;
	u32 count = m_t2 + delta;
	while (count > 0xffff)
	{
		count -= 0x10000;
		if (m_t2con & 0x30)
			transmit_receive(2);
		else
			set_tf2(1);
		if (!BIT(m_t2con, T2CON_CP) || (m_t2con & 0x30))
			count += m_rcap2;
	}
	m_t2 = count;
}

void dw8051_device::handle_irq(int irqline, int state, u32 new_state, u32 tr_state)
{
	switch (irqline)
	{
	case MCS51_T2_LINE:
		if (BIT(tr_state, MCS51_T2_LINE) && BIT(m_t2con, T2CON_TR2))
			m_t2_cnt++;
		break;

	case MCS51_T2EX_LINE:
		if (BIT(tr_state, MCS51_T2EX_LINE) && BIT(m_t2con, T2CON_EXEN2))
		{
			set_exf2(1);
			if (!(m_t2con & 0x30))
			{
				if (BIT(m_t2con, T2CON_CP))
					m_rcap2 = m_t2;
				else
					m_t2 = m_rcap2;
			}
		}
		break;

	default:
		i8052_device::handle_irq(irqline, state, new_state, tr_state);
		break;
	}
}
