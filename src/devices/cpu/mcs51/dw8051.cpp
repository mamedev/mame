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
}

void dw8051_device::device_reset()
{
	i8052_device::device_reset();
	m_other_dptr = 0;
	m_dps = 0;
	m_ckcon = 1;
	m_timer_divider = 0;
}

void dw8051_device::sfr_map(address_map &map)
{
	i8052_device::sfr_map(map);
	map(0x82, 0x85).rw(FUNC(dw8051_device::pointer_r), FUNC(dw8051_device::pointer_w));
	map(0x86, 0x86).lr8(NAME([this] () -> u8 { return m_dps; })).w(FUNC(dw8051_device::pointer_select_w));
	map(0x8e, 0x8e).lrw8(NAME([this] () -> u8 { return m_ckcon; }), NAME([this] (u8 data) { m_ckcon = data; }));
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

void dw8051_device::prepare_instruction(u8 op)
{
	unsigned cycles = INSTRUCTION_CYCLES[op];
	if ((op & 0xef) == 0xe0 || (op & 0xee) == 0xe2)
		cycles += m_ckcon & 7;
	m_inst_cycles += cycles;
}

u8 dw8051_device::interrupt_mask()
{
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
	if (BIT(m_t2con, T2CON_TR2) && !BIT(m_t2con, T2CON_CT2))
		increment_timer_t2(cycles);
}
