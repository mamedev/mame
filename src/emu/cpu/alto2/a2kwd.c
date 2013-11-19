/*****************************************************************************
 *
 *   Portable Xerox AltoII disk word task
 *
 *   Copyright: Juergen Buchmueller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 *
 *****************************************************************************/
#include "alto2cpu.h"

//! f1_kwd_block early: block the disk word task
void alto2_cpu_device::f1_kwd_block_0()
{
	LOG((LOG_KWD,2,"	BLOCK %s\n", task_name(m_task)));
	disk_block(m_task);
}

//! disk word task slot initialization
void alto2_cpu_device::init_kwd(int task)
{
	set_bs(task, bs_kwd_read_kstat,		&alto2_cpu_device::bs_read_kstat_0, 0);
	set_bs(task, bs_kwd_read_kdata,		&alto2_cpu_device::bs_read_kdata_0, 0);

	set_f1(task, f1_block,				&alto2_cpu_device::f1_kwd_block_0, 0);

	set_f1(task, f1_task_10,			0, 0);
	set_f1(task, f1_kwd_strobe,			0, &alto2_cpu_device::f1_strobe_1);
	set_f1(task, f1_kwd_load_kstat,		0, &alto2_cpu_device::f1_load_kstat_1);
	set_f1(task, f1_kwd_increcno,		0, &alto2_cpu_device::f1_increcno_1);
	set_f1(task, f1_kwd_clrstat,		0, &alto2_cpu_device::f1_clrstat_1);
	set_f1(task, f1_kwd_load_kcom,		0, &alto2_cpu_device::f1_load_kcom_1);
	set_f1(task, f1_kwd_load_kadr,		0, &alto2_cpu_device::f1_load_kadr_1);
	set_f1(task, f1_kwd_load_kdata,		0, &alto2_cpu_device::f1_load_kdata_1);

	set_f2(task, f2_kwd_init,			0, &alto2_cpu_device::f2_init_1);
	set_f2(task, f2_kwd_rwc,			0, &alto2_cpu_device::f2_rwc_1);
	set_f2(task, f2_kwd_recno,			0, &alto2_cpu_device::f2_recno_1);
	set_f2(task, f2_kwd_xfrdat,			0, &alto2_cpu_device::f2_xfrdat_1);
	set_f2(task, f2_kwd_swrnrdy,		0, &alto2_cpu_device::f2_swrnrdy_1);
	set_f2(task, f2_kwd_nfer,			0, &alto2_cpu_device::f2_nfer_1);
	set_f2(task, f2_kwd_strobon,		0, &alto2_cpu_device::f2_strobon_1);
	set_f2(task, f2_task_17,			0, 0);
}

void alto2_cpu_device::exit_kwd()
{
	// nothing to do yet
}


