/*****************************************************************************
 *
 *   Xerox AltoII display word task
 *
 *   Copyright © Jürgen Buchmüller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 *
 *****************************************************************************/
#include "alto2cpu.h"

/**
 * @brief block the display word task
 */
void alto2_cpu_device::f1_early_dwt_block()
{
	m_dsp.dwt_blocks = 1;

	/* clear the wakeup for the display word task */
	m_task_wakeup &= ~(1 << m_task);
	LOG((LOG_DWT,2,"	BLOCK %s\n", task_name(m_task)));

	/* wakeup the display horizontal task, if it didn't block itself */
	if (!m_dsp.dht_blocks)
		m_task_wakeup |= 1 << task_dht;
}

/**
 * @brief load the display data register
 */
void alto2_cpu_device::f2_late_dwt_load_ddr()
{
	LOG((LOG_DWT,2,"	DDR← BUS (%#o)\n", m_bus));
	m_dsp.fifo[m_dsp.fifo_wr] = m_bus;
	m_dsp.fifo_wr = (m_dsp.fifo_wr + 1) % ALTO2_DISPLAY_FIFO;
	if (FIFO_STOPWAKE_0() == 0)
		m_task_wakeup &= ~(1 << task_dwt);
	LOG((LOG_DWT,2, "	DWT push %04x into FIFO[%02o]%s\n",
		m_bus, (m_dsp.fifo_wr - 1) & (ALTO2_DISPLAY_FIFO - 1),
		FIFO_STOPWAKE_0() == 0 ? " STOPWAKE" : ""));
}

void alto2_cpu_device::init_dwt(int task)
{
	set_f1(task, f1_block,			&alto2_cpu_device::f1_early_dwt_block, 0);
	set_f2(task, f2_dwt_load_ddr,	0, &alto2_cpu_device::f2_late_dwt_load_ddr);
}

void alto2_cpu_device::exit_dwt()
{
	// nothing to do yet
}
