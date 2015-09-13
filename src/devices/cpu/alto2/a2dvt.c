// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII display vertical task
 *
 *****************************************************************************/
#include "alto2cpu.h"

/**
 * @brief f1_dvt_block early: disable the display word task
 */
void alto2_cpu_device::f1_early_dvt_block()
{
//  m_task_wakeup &= ~(1 << m_task);
	LOG((LOG_DVT,2,"    BLOCK %s\n", task_name(m_task)));
}


/**
 * @brief called by the CPU when the display vertical task becomes active
 */
void alto2_cpu_device::activate_dvt()
{
	m_task_wakeup &= ~(1 << m_task);
}

/**
 * @brief initialize display vertical task
 */
void alto2_cpu_device::init_dvt(int task)
{
	set_f1(task, f1_block,          &alto2_cpu_device::f1_early_dvt_block, 0);
	set_f2(task, f2_dvt_evenfield,  0, &alto2_cpu_device::f2_late_evenfield);
	m_active_callback[task] = &alto2_cpu_device::activate_dvt;
}

void alto2_cpu_device::exit_dvt()
{
	// nothing to do yet
}

void alto2_cpu_device::reset_dvt()
{
	// nothing to do yet
}
