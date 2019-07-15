// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 EE interrupt controller device skeleton
*
*   To Do:
*     Everything
*
*/

#include "emu.h"
#include "ps2intc.h"

DEFINE_DEVICE_TYPE(SONYPS2_INTC, ps2_intc_device, "ps2intc", "PlayStation 2 EE INTC")

ps2_intc_device::ps2_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYPS2_INTC, tag, owner, clock)
	, m_ee(*this, finder_base::DUMMY_TAG)
{
}

ps2_intc_device::~ps2_intc_device()
{
}

void ps2_intc_device::device_start()
{
	save_item(NAME(m_status));
	save_item(NAME(m_mask));
}

void ps2_intc_device::device_reset()
{
	m_status = 0;
	m_mask = 0;
}

READ32_MEMBER(ps2_intc_device::read)
{
	switch (offset)
	{
		case 0: // I_STAT
			//logerror("%s: read: I_STAT %08x & %08x\n", machine().describe_context(), m_status, mem_mask);
			return m_status;
		case 2: // I_MASK
			logerror("%s: read: I_MASK %08x & %08x\n", machine().describe_context(), m_mask, mem_mask);
			return m_mask;
		default:
			logerror("%s: read: Unknown offset %08x & %08x\n", machine().describe_context(), 0x1000f000 + (offset << 2), mem_mask);
			return 0;
	}
}

WRITE32_MEMBER(ps2_intc_device::write)
{
	switch (offset)
	{
		case 0: // I_STAT
			logerror("%s: write: I_STAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_status &= ~data;
			update_interrupts();
			break;
		case 2: // I_MASK
			logerror("%s: write: I_MASK = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_mask ^= data & 0x7fff;
			update_interrupts();
			break;
		default:
			logerror("%s: intc_w: Unknown offset %08x = %08x & %08x\n", machine().describe_context(), 0x1000f000 + (offset << 2), data, mem_mask);
			break;
	}
}

void ps2_intc_device::update_interrupts()
{
	m_ee->set_input_line(MIPS3_IRQ0, (m_status & m_mask) ? ASSERT_LINE : CLEAR_LINE);
}

void ps2_intc_device::raise_interrupt(uint32_t line)
{
	m_status |= (1 << line);
	update_interrupts();
}
