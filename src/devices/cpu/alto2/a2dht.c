// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII display horizontal task
 *
 *****************************************************************************/
#include "alto2cpu.h"

/**
 * @brief f1_dht_block early: disable the display word task
 */
void alto2_cpu_device::f1_early_dht_block()
{
	m_dsp.dht_blocks = true;
	// clear the wakeup for the display horizontal task
	m_task_wakeup &= ~(1 << m_task);
	LOG((LOG_DHT,2,"    BLOCK %s\n", task_name(m_task)));
}

/**
 * @brief f2_dht_setmode late: set the next scanline's mode inverse and half clock and branch
 *
 * BUS[0] selects the pixel clock (0), or half pixel clock (1)
 * BUS[1] selects normal mode (0), or inverse mode (1)
 *
 * The current BUS[0] drives the NEXT[09] line, i.e. branches to 0 or 1
 */
void alto2_cpu_device::f2_late_dht_setmode()
{
	UINT16 r = X_RDBITS(m_bus,16,0,0);
	m_dsp.setmode = m_bus;
	LOG((LOG_DHT,2,"    SETMODE<- BUS (%#o), branch on BUS[0] (%#o | %#o)\n", m_bus, m_next2, r));
	m_next2 |= r;
}

/**
 * @brief called by the CPU when the display horizontal task becomes active
 */
void alto2_cpu_device::activate_dht()
{
	m_task_wakeup &= ~(1 << m_task);
}

/**
 * @brief initialize the display horizontal task
 *
 * @param task task number
 */
void alto2_cpu_device::init_dht(int task)
{
	set_f1(task, f1_block,          &alto2_cpu_device::f1_early_dht_block, 0);
	set_f2(task, f2_dht_evenfield,  0, &alto2_cpu_device::f2_late_evenfield);
	set_f2(task, f2_dht_setmode,    0, &alto2_cpu_device::f2_late_dht_setmode);
	m_active_callback[task] = &alto2_cpu_device::activate_dht;
}

void alto2_cpu_device::exit_dht()
{
	// nothing to do yet
}

void alto2_cpu_device::reset_dht()
{
	m_dsp.dht_blocks = true;
	m_dsp.setmode = 0;
}
