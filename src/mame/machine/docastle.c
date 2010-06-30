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
	docastle_state *state = (docastle_state *)space->machine->driver_data;
	if (offset == 8) logerror("CPU #0 shared0r  clock = %d\n", (UINT32)state->maincpu->total_cycles());
	return state->buffer0[offset];
}


READ8_HANDLER( docastle_shared1_r )
{
	docastle_state *state = (docastle_state *)space->machine->driver_data;
	if (offset == 8) logerror("CPU #1 shared1r  clock = %d\n", (UINT32)state->slave->total_cycles());
	return state->buffer1[offset];
}


WRITE8_HANDLER( docastle_shared0_w )
{
	docastle_state *state = (docastle_state *)space->machine->driver_data;
	if (offset == 8) logerror("CPU #1 shared0w %02x %02x %02x %02x %02x %02x %02x %02x %02x clock = %d\n",
		state->buffer0[0], state->buffer0[1], state->buffer0[2], state->buffer0[3],
		state->buffer0[4], state->buffer0[5], state->buffer0[6], state->buffer0[7],
		data, (UINT32)state->slave->total_cycles());

	state->buffer0[offset] = data;

	if (offset == 8)
		/* awake the master CPU */
		cpuexec_trigger(space->machine, 500);
}


WRITE8_HANDLER( docastle_shared1_w )
{
	docastle_state *state = (docastle_state *)space->machine->driver_data;
	state->buffer1[offset] = data;

	if (offset == 8)
	{
		logerror("CPU #0 shared1w %02x %02x %02x %02x %02x %02x %02x %02x %02x clock = %d\n",
				state->buffer1[0], state->buffer1[1], state->buffer1[2], state->buffer1[3],
				state->buffer1[4], state->buffer1[5], state->buffer1[6], state->buffer1[7],
				data, (UINT32)state->maincpu->total_cycles());

		/* freeze execution of the master CPU until the slave has used the shared memory */
		cpu_spinuntil_trigger(space->cpu, 500);
	}
}



WRITE8_HANDLER( docastle_nmitrigger_w )
{
	docastle_state *state = (docastle_state *)space->machine->driver_data;
	cpu_set_input_line(state->slave, INPUT_LINE_NMI, PULSE_LINE);
}
