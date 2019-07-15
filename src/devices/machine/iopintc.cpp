// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 IOP interrupt controller device skeleton
*
*   To Do:
*     Everything
*
*/

#include "emu.h"
#include "iopintc.h"

DEFINE_DEVICE_TYPE(SONYIOP_INTC, iop_intc_device, "iopintc", "PlayStation 2 IOP INTC")

iop_intc_device::iop_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYIOP_INTC, tag, owner, clock)
	, m_iop(*this, finder_base::DUMMY_TAG)
{
}

iop_intc_device::~iop_intc_device()
{
}

void iop_intc_device::device_start()
{
	save_item(NAME(m_status));
	save_item(NAME(m_mask));
	save_item(NAME(m_enabled));
}

void iop_intc_device::device_reset()
{
	m_status = 0;
	m_mask = 0;
	m_enabled = false;
}

READ32_MEMBER(iop_intc_device::read)
{
	uint32_t ret = 0;
	switch (offset)
	{
		case 0: // I_STAT
			ret = m_status;
			//logerror("%s: read: I_STAT %08x & %08x\n", machine().describe_context(), ret, mem_mask);
			break;
		case 1: // I_MASK
			ret = m_mask;
			//logerror("%s: read: I_MASK %08x & %08x\n", machine().describe_context(), ret, mem_mask);
			break;
		case 2: // I_ENABLE
			ret = m_enabled ? 1 : 0;
			m_enabled = false;
			update_interrupts();
			//logerror("%s: read: I_ENABLE %08x & %08x\n", machine().describe_context(), ret, mem_mask);
			break;
		default:
			logerror("%s: read: Unknown offset %08x & %08x\n", machine().describe_context(), 0x1f801070 + (offset << 2), mem_mask);
			break;
	}
	return ret;
}

WRITE32_MEMBER(iop_intc_device::write)
{
	switch (offset)
	{
		case 0: // I_STAT
			//logerror("%s: write: I_STAT = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_status &= data;
			update_interrupts();
			break;
		case 1: // I_MASK
			//logerror("%s: write: I_MASK = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_mask = data;
			update_interrupts();
			break;
		case 2: // I_ENABLE
			//logerror("%s: write: I_ENABLE = %08x & %08x\n", machine().describe_context(), data, mem_mask);
			m_enabled = BIT(data, 0);
			update_interrupts();
			break;
		default:
			logerror("%s: write: Unknown offset %08x = %08x & %08x\n", machine().describe_context(), 0x1f801070 + (offset << 2), data, mem_mask);
			break;
	}
}

void iop_intc_device::raise_interrupt(uint32_t line)
{
	//logerror("%s: raise_interrupt: %d\n", machine().describe_context(), line);
	m_status |= (1 << line);
	update_interrupts();
}

void iop_intc_device::update_interrupts()
{
	bool active = (m_enabled && (m_status & m_mask));
	//printf("iop_intc: %d && (%08x & %08x) = %d\n", m_enabled, m_status, m_mask, active);
	m_iop->set_input_line(INPUT_LINE_IRQ0, active ? ASSERT_LINE : CLEAR_LINE);
}
