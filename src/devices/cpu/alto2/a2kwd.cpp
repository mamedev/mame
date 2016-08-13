// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII disk word task
 *
 *****************************************************************************/
#include "alto2cpu.h"

//! f1_kwd_block early: block the disk word task
void alto2_cpu_device::f1_early_kwd_block()
{
	LOG((this,LOG_KWD,2,"    BLOCK %s\n", task_name(m_task)));
	disk_block(m_task);
}

//! disk word task slot initialization
void alto2_cpu_device::init_kwd(int task)
{
}

void alto2_cpu_device::exit_kwd()
{
	// nothing to do yet
}

void alto2_cpu_device::reset_kwd()
{
	// nothing to do yet
}
