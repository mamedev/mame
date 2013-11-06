/*****************************************************************************
 *
 *   Portable Xerox AltoII display horizontal task
 *
 *   Copyright: Juergen Buchmueller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 *
 *****************************************************************************/
#include "alto2.h"

/**
 * @brief f1_dht_block early: disable the display word task
 */
void alto2_cpu_device::f1_dht_block_0()
{
	m_dsp.dht_blocks = 1;
	/* clear the wakeup for the display horizontal task */
	m_task_wakeup &= ~(1 << m_task);
	LOG((0,2,"	BLOCK %s\n", task_name(m_task)));
}

/**
 * @brief f2_dht_setmode late: set the next scanline's mode inverse and half clock and branch
 *
 * BUS[0] selects the pixel clock (0), or half pixel clock (1)
 * BUS[1] selects normal mode (0), or inverse mode (1)
 *
 * The current BUS[0] drives the NEXT[09] line, i.e. branches to 0 or 1
 */
void alto2_cpu_device::f2_dht_setmode_1()
{
	UINT16 r = A2_GET32(m_bus,16,0,0);
	m_dsp.setmode = m_bus;
	LOG((0,2,"	SETMODE<- BUS (%#o), branch on BUS[0] (%#o | %#o)\n", m_bus, m_next2, r));
	m_next2 |= r;
}

/**
 * @brief called by the CPU when the display horizontal task becomes active
 */
void alto2_cpu_device::activate_dht()
{
	/* TODO: what do we do here? */
	m_task_wakeup &= ~(1 << m_task);
}

/**
 * @brief initialize the display horizontal task
 *
 * @param task task number
 */
void alto2_cpu_device::init_dht(int task)
{
	set_f1(task, f1_block,			&alto2_cpu_device::f1_dht_block_0, 0);
	set_f2(task, f2_dht_evenfield,	0, &alto2_cpu_device::f2_evenfield_1);
	set_f2(task, f2_dht_setmode,	0, &alto2_cpu_device::f2_dht_setmode_1);
	m_active_callback[task] = &alto2_cpu_device::activate_dht;
}

