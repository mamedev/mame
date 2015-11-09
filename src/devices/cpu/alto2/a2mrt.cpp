// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII memory refresh task
 *
 *****************************************************************************/
#include "alto2cpu.h"

//! f1_mrt_block early: block the display word task
void alto2_cpu_device::f1_early_mrt_block()
{
	/* clear the wakeup for the memory refresh task */
	m_task_wakeup &= ~(1 << m_task);
	LOG((this,LOG_MRT,2,"    BLOCK %s\n", task_name(m_task)));
}

//! called by the CPU when MRT becomes active
void alto2_cpu_device::activate_mrt()
{
	m_task_wakeup &= ~(1 << m_task);
	if (m_ewfct)
	{
		// The Ether task wants a wakeup, too
		m_task_wakeup |= 1 << task_ether;
	}
}

	//! memory refresh task slots initialization
void alto2_cpu_device::init_mrt(int task)
{
	set_f1(task, f1_block,      &alto2_cpu_device::f1_early_mrt_block, 0);
	/* auto block */
	m_active_callback[task] = &alto2_cpu_device::activate_mrt;
}

void alto2_cpu_device::exit_mrt()
{
	// nothing to do yet
}

void alto2_cpu_device::reset_mrt()
{
	// nothing to do yet
}
