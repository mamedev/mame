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
	set_bs(task, bs_ksec_read_kstat,    &alto2_cpu_device::bs_early_read_kstat, 0);
	set_bs(task, bs_ksec_read_kdata,    &alto2_cpu_device::bs_early_read_kdata, 0);

	set_f1(task, f1_block,              &alto2_cpu_device::f1_early_ksec_block, 0);

	set_f1(task, f1_task_10,            0, 0);
	set_f1(task, f1_ksec_strobe,        0, &alto2_cpu_device::f1_late_strobe);
	set_f1(task, f1_ksec_load_kstat,    0, &alto2_cpu_device::f1_late_load_kstat);
	set_f1(task, f1_ksec_increcno,      0, &alto2_cpu_device::f1_late_increcno);
	set_f1(task, f1_ksec_clrstat,       0, &alto2_cpu_device::f1_late_clrstat);
	set_f1(task, f1_ksec_load_kcom,     0, &alto2_cpu_device::f1_late_load_kcom);
	set_f1(task, f1_ksec_load_kadr,     0, &alto2_cpu_device::f1_late_load_kadr);
	set_f1(task, f1_ksec_load_kdata,    0, &alto2_cpu_device::f1_late_load_kdata);

	set_f2(task, f2_ksec_init,          0, &alto2_cpu_device::f2_late_init);
	set_f2(task, f2_ksec_rwc,           0, &alto2_cpu_device::f2_late_rwc);
	set_f2(task, f2_ksec_recno,         0, &alto2_cpu_device::f2_late_recno);
	set_f2(task, f2_ksec_xfrdat,        0, &alto2_cpu_device::f2_late_xfrdat);
	set_f2(task, f2_ksec_swrnrdy,       0, &alto2_cpu_device::f2_late_swrnrdy);
	set_f2(task, f2_ksec_nfer,          0, &alto2_cpu_device::f2_late_nfer);
	set_f2(task, f2_ksec_strobon,       0, &alto2_cpu_device::f2_late_strobon);
	set_f2(task, f2_task_17,            0, 0);

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
