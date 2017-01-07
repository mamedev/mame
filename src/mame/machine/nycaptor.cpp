// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "includes/nycaptor.h"


READ8_MEMBER(nycaptor_state::nycaptor_mcu_status_r1)
{
	/* bit 1 = when 1, mcu has sent data to the main cpu */
	return m_bmcu->get_mcu_sent() ? 2 : 0;
}

READ8_MEMBER(nycaptor_state::nycaptor_mcu_status_r2)
{
	/* bit 0 = when 1, mcu is ready to receive data from main cpu */
	return m_bmcu->get_main_sent() ? 0 : 1;
}
