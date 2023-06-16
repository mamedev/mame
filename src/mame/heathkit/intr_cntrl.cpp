// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Interrupt controller for H89

****************************************************************************/

#include "emu.h"

#include "intr_cntrl.h"

DEFINE_DEVICE_TYPE(HEATH_INTR_CNTRL, heath_intr_cntrl, "heath_intr_cntrl", "Heath H/Z-89 Interrupt Controller");
DEFINE_DEVICE_TYPE(HEATH_Z37_INTR_CNTRL, z37_intr_cntrl, "heath_z37_intr_cntrl", "Heath H/Z-89 with Z-37 Interrupt Controller");

heath_intr_cntrl::heath_intr_cntrl(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: heath_intr_cntrl(mconfig, HEATH_INTR_CNTRL, tag, owner, clock)
{
}

heath_intr_cntrl::heath_intr_cntrl(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, 0)
	, m_irq_line(*this)
{
}

void heath_intr_cntrl::device_start()
{
	save_item(NAME(m_intr_lines));

	m_intr_lines = 0;
}

void heath_intr_cntrl::update_intr_line()
{

	m_irq_line(m_intr_lines == 0 ? CLEAR_LINE : ASSERT_LINE);
}

void heath_intr_cntrl::raise_irq(uint8_t level)
{
	// only 0 to 7 is valid
	level &= 0x7;
	m_intr_lines |= 1 << level;

	update_intr_line();
}

void heath_intr_cntrl::lower_irq(uint8_t level)
{
	// only 0 to 7 is valid
	level &= 0x7;
	m_intr_lines &= ~(1 << level);

	update_intr_line();
}

uint8_t heath_intr_cntrl::get_instruction()
{

	// determine top priority instruction
	if (!m_intr_lines)
	{
		// should not occur.
		// NO-OP ?
		logerror("get instruct: bad m_intr_lines\n");

		return 0x00;
	}

	uint8_t level = 0;
	uint8_t mask = 0x01;

	while (mask)
	{
		if (m_intr_lines & mask)
		{
			break;
		}
		level++;
		mask <<= 1;
	}

	if (level > 7)
	{
		logerror("bad level: %d\n", level);
	}

	// return RST based on level
	return 0xc7 | ((level & 0x7) << 3);
}

IRQ_CALLBACK_MEMBER(heath_intr_cntrl::irq_callback)
{
	return get_instruction();
}

z37_intr_cntrl::z37_intr_cntrl(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: heath_intr_cntrl(mconfig, HEATH_Z37_INTR_CNTRL, tag, owner, clock)
{
	m_interrupts_blocked = false;
	m_drq_raised = false;
	m_fd_irq_raised = false;
}

void z37_intr_cntrl::update_intr_line()
{

	m_irq_line(
		m_fd_irq_raised ||
		m_drq_raised ||
		(!m_interrupts_blocked && (m_intr_lines != 0)) ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t z37_intr_cntrl::get_instruction()
{

	if (m_drq_raised)
	{
		// EI
		return 0xfb;
	}

	if (m_fd_irq_raised)
	{
		// RST 20H (Interrupt 4)
		return 0xe7;
	}


	if (!m_interrupts_blocked)
	{
		return heath_intr_cntrl::get_instruction();
	}

	// shouldn't get here - NO-OP?
	logerror("Warning: z37 intr get_instruction: fd: %d dr: %d ib: %d\n", m_fd_irq_raised, m_drq_raised, m_interrupts_blocked);
	return 0x00;
}

void z37_intr_cntrl::set_drq(uint8_t data)
{
	m_drq_raised = (data != CLEAR_LINE);

	update_intr_line();
}


void z37_intr_cntrl::set_intrq(uint8_t data)
{
	m_fd_irq_raised = (data != CLEAR_LINE);

	update_intr_line();
}

void z37_intr_cntrl::device_start()
{
	heath_intr_cntrl::device_start();

	save_item(NAME(m_interrupts_blocked));
	save_item(NAME(m_drq_raised));
	save_item(NAME(m_fd_irq_raised));

	m_interrupts_blocked = false;
	m_drq_raised = false;
	m_fd_irq_raised = false;
}

void z37_intr_cntrl::block_interrupts(uint8_t data)
{
	m_interrupts_blocked = (data != CLEAR_LINE);

	update_intr_line();
}
