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
	set_bs(task, bs_kwd_read_kstat,     &alto2_cpu_device::bs_early_read_kstat, 0);
	set_bs(task, bs_kwd_read_kdata,     &alto2_cpu_device::bs_early_read_kdata, 0);

	set_f1(task, f1_block,              &alto2_cpu_device::f1_early_kwd_block, 0);

	set_f1(task, f1_task_10,            0, 0);
	set_f1(task, f1_kwd_strobe,         0, &alto2_cpu_device::f1_late_strobe);
	set_f1(task, f1_kwd_load_kstat,     0, &alto2_cpu_device::f1_late_load_kstat);
	set_f1(task, f1_kwd_increcno,       0, &alto2_cpu_device::f1_late_increcno);
	set_f1(task, f1_kwd_clrstat,        0, &alto2_cpu_device::f1_late_clrstat);
	set_f1(task, f1_kwd_load_kcom,      0, &alto2_cpu_device::f1_late_load_kcom);
	set_f1(task, f1_kwd_load_kadr,      0, &alto2_cpu_device::f1_late_load_kadr);
	set_f1(task, f1_kwd_load_kdata,     0, &alto2_cpu_device::f1_late_load_kdata);

	set_f2(task, f2_kwd_init,           0, &alto2_cpu_device::f2_late_init);
	set_f2(task, f2_kwd_rwc,            0, &alto2_cpu_device::f2_late_rwc);
	set_f2(task, f2_kwd_recno,          0, &alto2_cpu_device::f2_late_recno);
	set_f2(task, f2_kwd_xfrdat,         0, &alto2_cpu_device::f2_late_xfrdat);
	set_f2(task, f2_kwd_swrnrdy,        0, &alto2_cpu_device::f2_late_swrnrdy);
	set_f2(task, f2_kwd_nfer,           0, &alto2_cpu_device::f2_late_nfer);
	set_f2(task, f2_kwd_strobon,        0, &alto2_cpu_device::f2_late_strobon);
	set_f2(task, f2_task_17,            0, 0);
}

void alto2_cpu_device::exit_kwd()
{
	// nothing to do yet
}

void alto2_cpu_device::reset_kwd()
{
	// nothing to do yet
}
