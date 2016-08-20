// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII display vertical task
 *
 *****************************************************************************/
#include "alto2cpu.h"

/*
 * Copied from ALTOCODE24.MU
 *
 *	;Display Vertical Task
 *
 *	DVT:	MAR← L← DASTART+1;
 *		CBA← L, L← 0;
 *		CURDATA← L;
 *		SLC← L;
 *		T← MD;			CAUSE A VERTICAL FIELD INTERRUPT
 *		L← NWW OR T;
 *		MAR← CURLOC;		SET UP THE CURSOR
 *		NWW← L, T← 0-1;
 *		L← MD XOR T;		HARDWARE EXPECTS X COMPLEMENTED
 *		T← MD, EVENFIELD;
 *		CURX← L, :DVT1;
 *
 *	DVT1:	L← BIAS-T-1, TASK, :DVT2;	BIAS THE Y COORDINATE 
 *	DVT11:	L← BIAS-T, TASK;
 * 
 *	DVT2:	YPOS← L, :DVT;
 */

/**
 * @brief f1_dvt_block early: disable the display word task
 */
void alto2_cpu_device::f1_early_dvt_block()
{
//  m_task_wakeup &= ~(1 << m_task);
	LOG((this,LOG_DVT,2,"    BLOCK %s\n", task_name(m_task)));
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
}

void alto2_cpu_device::exit_dvt()
{
	// nothing to do yet
}

void alto2_cpu_device::reset_dvt()
{
	// nothing to do yet
}
