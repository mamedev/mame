/*****************************************************************************
 *
 *   Portable Xerox AltoII memory refresh task
 *
 *   Copyright: Juergen Buchmueller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 *
 *****************************************************************************/
#include "alto2.h"

//! f1_mrt_block early: block the display word task
void alto2_cpu_device::f1_mrt_block_0()
{
	/* clear the wakeup for the memory refresh task */
	m_task_wakeup &= ~(1 << m_task);
	LOG((0,2,"	BLOCK %s\n", task_name(m_task)));
}

//! called by the CPU when MRT becomes active
void alto2_cpu_device::mrt_activate()
{
	/* TODO: what do we do here? */
	m_task_wakeup &= ~(1 << m_task);
}

 //! memory refresh task slots initialization
void alto2_cpu_device::init_mrt(int task)
{
	set_f1(task, f1_block,		&alto2_cpu_device::f1_mrt_block_0, 0);
	/* auto block */
	m_active_callback[task] = &alto2_cpu_device::mrt_activate;
}
