// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII display word task
 *
 *****************************************************************************/
#include "alto2cpu.h"

//! PROM a38 bit O1 is STOPWAKE' (stop DWT if bit is zero)
#define FIFO_STOPWAKE(a38) (0 == (a38 & disp_a38_STOPWAKE) ? true : false)

/**
 * @brief block the display word task
 */
void alto2_cpu_device::f1_early_dwt_block()
{
	m_dsp.dwt_blocks = true;

	/* clear the wakeup for the display word task */
	m_task_wakeup &= ~(1 << m_task);
	LOG((this,LOG_DWT,2,"    BLOCK %s\n", task_name(m_task)));

	/* wakeup the display horizontal task, if it didn't block itself */
	if (!m_dsp.dht_blocks)
		m_task_wakeup |= 1 << task_dht;
}

/**
 * @brief load the display data register
 */
void alto2_cpu_device::f2_late_dwt_load_ddr()
{
	LOG((this,LOG_DWT,2,"    DDR<- BUS (%#o)\n", m_bus));
	m_dsp.fifo[m_dsp.wa] = m_bus;
	m_dsp.wa = (m_dsp.wa + 1) % ALTO2_DISPLAY_FIFO;
	UINT8 a38 = m_disp_a38[m_dsp.ra * 16 + m_dsp.wa];
	if (FIFO_STOPWAKE(a38))
		m_task_wakeup &= ~(1 << task_dwt);
	LOG((this,LOG_DWT,2, "   DWT push %04x into FIFO[%02o]%s\n",
		m_bus, (m_dsp.wa - 1) & (ALTO2_DISPLAY_FIFO - 1),
		FIFO_STOPWAKE(a38) ? " STOPWAKE" : ""));
}

void alto2_cpu_device::init_dwt(int task)
{
	set_f1(task, f1_block,          &alto2_cpu_device::f1_early_dwt_block, nullptr);
	set_f2(task, f2_dwt_load_ddr,   nullptr, &alto2_cpu_device::f2_late_dwt_load_ddr);
}

void alto2_cpu_device::exit_dwt()
{
	// nothing to do yet
}

void alto2_cpu_device::reset_dwt()
{
	m_dsp.dwt_blocks = false;
	memset(m_dsp.fifo, 0, sizeof(m_dsp.fifo));
	m_dsp.wa = 0;
	m_dsp.ra = 0;
}
