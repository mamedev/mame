// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/docastle.h"

#define LOG 0

/*
THIS IS A GIANT HACK! It can be accurately emulated once our Z80 core supports WAIT.

Communication between the two CPUs happens through a single bidirectional latch.
Whenever CPU 0 reads or writes it, its WAIT input is asserted. It is implicitly
cleared by CPU 1, when it accesses the latch. This enforces synchronization
between the two CPUs.
This is currently impossible to reproduce accurately in MAME, because it would
require putting on hold CPU 0 while it is reading the latch, and resume its
execution only when CPU 1 has written it.
Instead, we take advantage of how the two CPUs access the latch, and treat it as
if it was a small shared buffer. The order of operations is:
1) CPU 0 triggers NMI on CPU 1
2) CPU 0 writes 9 bytes to the buffer
3) at this point we suspend execution of CPU 0, to give CPU 1 time to read the 9
   bytes and write its own 9 bytes
4) resume execution of CPU 0.
*/
READ8_MEMBER(docastle_state::docastle_shared0_r)
{
	if (offset == 8 && LOG)
		logerror("CPU #0 shared0r  clock = %d\n", (UINT32)m_maincpu->total_cycles());

	return m_buffer0[offset];
}


READ8_MEMBER(docastle_state::docastle_shared1_r)
{
	if (offset == 8 && LOG)
		logerror("CPU #1 shared1r  clock = %d\n", (UINT32)m_slave->total_cycles());

	return m_buffer1[offset];
}


WRITE8_MEMBER(docastle_state::docastle_shared0_w)
{
	if (offset == 8 && LOG)
		logerror("CPU #1 shared0w %02x %02x %02x %02x %02x %02x %02x %02x %02x clock = %d\n",
		m_buffer0[0], m_buffer0[1], m_buffer0[2], m_buffer0[3],
		m_buffer0[4], m_buffer0[5], m_buffer0[6], m_buffer0[7],
		data, (UINT32)m_slave->total_cycles());

	m_buffer0[offset] = data;

	/* awake the master CPU */
	if (offset == 8)
		machine().scheduler().trigger(500);
}


WRITE8_MEMBER(docastle_state::docastle_shared1_w)
{
	m_buffer1[offset] = data;

	if (offset == 8 && LOG)
		logerror("CPU #0 shared1w %02x %02x %02x %02x %02x %02x %02x %02x %02x clock = %d\n",
		m_buffer1[0], m_buffer1[1], m_buffer1[2], m_buffer1[3],
		m_buffer1[4], m_buffer1[5], m_buffer1[6], m_buffer1[7],
		data, (UINT32)m_maincpu->total_cycles());

	/* freeze execution of the master CPU until the slave has used the shared memory */
	if (offset == 8)
		space.device().execute().spin_until_trigger(500);
}



WRITE8_MEMBER(docastle_state::docastle_nmitrigger_w)
{
	m_slave->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}
