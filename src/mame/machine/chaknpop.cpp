// license:BSD-3-Clause
// copyright-holders:BUT
/*
 *  Chack'n Pop (C) 1983 TAITO Corp.
 *  simulate 68705 MCU
 */

#include "emu.h"
#include "includes/chaknpop.h"


READ8_MEMBER(chaknpop_state::mcu_status_r)
{
	// bit 0 = when 1, MCU is ready to receive data from main CPU
	// bit 1 = when 1, MCU has sent data to the main CPU
	return
		((CLEAR_LINE == m_bmcu->host_semaphore_r()) ? 0x01 : 0x00) |
		((CLEAR_LINE != m_bmcu->mcu_semaphore_r()) ? 0x02 : 0x00);
}

