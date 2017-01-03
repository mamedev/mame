// license:GPL-2.0+
// copyright-holders:Jarek Burczynski, Tomasz Slanina
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "includes/bigevglf.h"



READ8_MEMBER(bigevglf_state::bigevglf_mcu_status_r)
{
	int res = 0;

	if (!m_bmcu->get_main_sent())
		res |= 0x08;
	if (m_bmcu->get_mcu_sent())
		res |= 0x10;

	return res;
}
