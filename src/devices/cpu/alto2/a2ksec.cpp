// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII disk sector task
 *
 *****************************************************************************/
#include "alto2cpu.h"

//! f1_ksec_block early: block the disk sector task
void alto2_cpu_device::f1_early_ksec_block()
{
	LOG((this,LOG_KSEC,2,"   BLOCK %s\n", task_name(m_task)));
	disk_block(m_task);
}

//! disk sector task slot initialization
void alto2_cpu_device::init_ksec(int task)
{
	m_task_wakeup |= 1 << task;
}

void alto2_cpu_device::exit_ksec()
{
	// nothing to do yet
}

void alto2_cpu_device::reset_ksec()
{
	// nothing to do yet
}
