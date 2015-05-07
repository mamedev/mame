// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII parity task
 *
 *****************************************************************************/
#include "alto2cpu.h"

//! called by the CPU when the parity task becomes active
void alto2_cpu_device::activate_part()
{
	m_task_wakeup &= ~(1 << m_task);
}

//! parity task slots initialization
void alto2_cpu_device::init_part(int task)
{
	m_active_callback[task] = &alto2_cpu_device::activate_part;
}

void alto2_cpu_device::exit_part()
{
	// nothing to do yet
}

void alto2_cpu_device::reset_part()
{
	// nothing to do yet
}
