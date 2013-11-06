/*****************************************************************************
 *
 *   Portable Xerox AltoII parity task
 *
 *   Copyright: Juergen Buchmueller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 *
 *****************************************************************************/
#include "alto2.h"

/** @brief called by the CPU when the parity task becomes active */
void alto2_cpu_device::activate_part()
{
	/* TODO: what do we do here ? */
	m_task_wakeup &= ~(1 << m_task);
}

/**
 * @brief parity task slots initialization
 */
void alto2_cpu_device::init_part(int task)
{
	m_active_callback[task] = &alto2_cpu_device::activate_part;
}

