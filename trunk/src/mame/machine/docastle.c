/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/docastle.h"

/*
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
READ8_HANDLER( docastle_shared0_r )
{
	docastle_state *state = space->machine().driver_data<docastle_state>();
	if (offset == 8) logerror("CPU #0 shared0r  clock = %d\n", (UINT32)state->m_maincpu->total_cycles());
	return state->m_buffer0[offset];
}


READ8_HANDLER( docastle_shared1_r )
{
	docastle_state *state = space->machine().driver_data<docastle_state>();
	if (offset == 8) logerror("CPU #1 shared1r  clock = %d\n", (UINT32)state->m_slave->total_cycles());
	return state->m_buffer1[offset];
}


WRITE8_HANDLER( docastle_shared0_w )
{
	docastle_state *state = space->machine().driver_data<docastle_state>();
	if (offset == 8) logerror("CPU #1 shared0w %02x %02x %02x %02x %02x %02x %02x %02x %02x clock = %d\n",
		state->m_buffer0[0], state->m_buffer0[1], state->m_buffer0[2], state->m_buffer0[3],
		state->m_buffer0[4], state->m_buffer0[5], state->m_buffer0[6], state->m_buffer0[7],
		data, (UINT32)state->m_slave->total_cycles());

	state->m_buffer0[offset] = data;

	if (offset == 8)
		/* awake the master CPU */
		space->machine().scheduler().trigger(500);
}


WRITE8_HANDLER( docastle_shared1_w )
{
	docastle_state *state = space->machine().driver_data<docastle_state>();
	state->m_buffer1[offset] = data;

	if (offset == 8)
	{
		logerror("CPU #0 shared1w %02x %02x %02x %02x %02x %02x %02x %02x %02x clock = %d\n",
				state->m_buffer1[0], state->m_buffer1[1], state->m_buffer1[2], state->m_buffer1[3],
				state->m_buffer1[4], state->m_buffer1[5], state->m_buffer1[6], state->m_buffer1[7],
				data, (UINT32)state->m_maincpu->total_cycles());

		/* freeze execution of the master CPU until the slave has used the shared memory */
		device_spin_until_trigger(&space->device(), 500);
	}
}



WRITE8_HANDLER( docastle_nmitrigger_w )
{
	docastle_state *state = space->machine().driver_data<docastle_state>();
	device_set_input_line(state->m_slave, INPUT_LINE_NMI, PULSE_LINE);
}
