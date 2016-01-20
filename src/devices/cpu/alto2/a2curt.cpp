// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII cursor task
 *
 *****************************************************************************/
#include "alto2cpu.h"

/**
 * @brief disable the cursor task and set the curt_blocks flag
 */
void alto2_cpu_device::f1_early_curt_block()
{
	m_dsp.curt_blocks = true;
	m_task_wakeup &= ~(1 << m_task);
	LOG((this,LOG_CURT,2,"   BLOCK %s\n", task_name(m_task)));
}

/**
 * @brief f2_load_xpreg late: load the x position register from BUS[6-15]
 */
void alto2_cpu_device::f2_late_load_xpreg()
{
	m_dsp.xpreg = X_RDBITS(m_bus,16,6,15);
	LOG((this,LOG_CURT, 9,"  XPREG<- BUS[6-15] (%#o)\n", m_dsp.xpreg));
}

/**
 * @brief f2_load_csr late: load the cursor shift register from BUS[0-15]
 *
 * Shift CSR to xpreg % 16 position to make it easier to
 * to handle the word xor in unload_word().
 * <PRE>
 * xpreg % 16   cursor bits
 *              [ first word   ][  second word ]
 * ----------------------------------------------
 *     0        xxxxxxxxxxxxxxxx0000000000000000
 *     1        0xxxxxxxxxxxxxxxx000000000000000
 *     2        00xxxxxxxxxxxxxxxx00000000000000
 * ...
 *    14        00000000000000xxxxxxxxxxxxxxxx00
 *    15        000000000000000xxxxxxxxxxxxxxxx0
 * </PRE>
 */
void alto2_cpu_device::f2_late_load_csr()
{
	m_dsp.csr = m_bus;
	LOG((this,LOG_CURT, m_dsp.csr ? 2 : 9,"  CSR<- BUS (%#o)\n", m_dsp.csr));
}

/**
 * @brief curt_activate: called by the CPU when the cursor task becomes active
 */
void alto2_cpu_device::activate_curt()
{
	m_task_wakeup &= ~(1 << m_task);
	m_dsp.curt_wakeup = false;

	int x = 01777 - m_dsp.xpreg;
	UINT32 bits = m_dsp.csr << (16 - (x & 15));
	m_dsp.cursor0 = static_cast<UINT16>(bits >> 16);
	m_dsp.cursor1 = static_cast<UINT16>(bits);
	m_dsp.curxpos = x / 16;
}

/** @brief initialize the cursor task F1 and F2 functions */
void alto2_cpu_device::init_curt(int task)
{
	set_f1(task, f1_block,              &alto2_cpu_device::f1_early_curt_block, nullptr);
	set_f2(task, f2_curt_load_xpreg,    nullptr, &alto2_cpu_device::f2_late_load_xpreg);
	set_f2(task, f2_curt_load_csr,      nullptr, &alto2_cpu_device::f2_late_load_csr);
	m_active_callback[task] = &alto2_cpu_device::activate_curt;
}

void alto2_cpu_device::exit_curt()
{
	// nothing to do yet
}

void alto2_cpu_device::reset_curt()
{
	m_dsp.curt_blocks = false;
	m_dsp.xpreg = 0;
	m_dsp.csr = 0;
	m_dsp.curxpos = 0;
	m_dsp.cursor0 = m_dsp.cursor1 = 0;
}
