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
	set_bs(task, bs_kwd_read_kstat,     &alto2_cpu_device::bs_early_read_kstat, nullptr);
	set_bs(task, bs_kwd_read_kdata,     &alto2_cpu_device::bs_early_read_kdata, nullptr);

	set_f1(task, f1_block,              &alto2_cpu_device::f1_early_kwd_block, nullptr);

	set_f1(task, f1_task_10,            nullptr, nullptr);
	set_f1(task, f1_kwd_strobe,         nullptr, &alto2_cpu_device::f1_late_strobe);
	set_f1(task, f1_kwd_load_kstat,     nullptr, &alto2_cpu_device::f1_late_load_kstat);
	set_f1(task, f1_kwd_increcno,       nullptr, &alto2_cpu_device::f1_late_increcno);
	set_f1(task, f1_kwd_clrstat,        nullptr, &alto2_cpu_device::f1_late_clrstat);
	set_f1(task, f1_kwd_load_kcom,      nullptr, &alto2_cpu_device::f1_late_load_kcom);
	set_f1(task, f1_kwd_load_kadr,      nullptr, &alto2_cpu_device::f1_late_load_kadr);
	set_f1(task, f1_kwd_load_kdata,     nullptr, &alto2_cpu_device::f1_late_load_kdata);

	set_f2(task, f2_kwd_init,           nullptr, &alto2_cpu_device::f2_late_init);
	set_f2(task, f2_kwd_rwc,            nullptr, &alto2_cpu_device::f2_late_rwc);
	set_f2(task, f2_kwd_recno,          nullptr, &alto2_cpu_device::f2_late_recno);
	set_f2(task, f2_kwd_xfrdat,         nullptr, &alto2_cpu_device::f2_late_xfrdat);
	set_f2(task, f2_kwd_swrnrdy,        nullptr, &alto2_cpu_device::f2_late_swrnrdy);
	set_f2(task, f2_kwd_nfer,           nullptr, &alto2_cpu_device::f2_late_nfer);
	set_f2(task, f2_kwd_strobon,        nullptr, &alto2_cpu_device::f2_late_strobon);
	set_f2(task, f2_task_17,            nullptr, nullptr);
}

void alto2_cpu_device::exit_kwd()
{
	// nothing to do yet
}

void alto2_cpu_device::reset_kwd()
{
	// nothing to do yet
}
