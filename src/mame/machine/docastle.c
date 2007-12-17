/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "includes/docastle.h"



static UINT8 buffer0[9],buffer1[9];



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
	if (offset == 8) logerror("CPU #0 shared0r  clock = %d\n",activecpu_gettotalcycles());
	return buffer0[offset];
}


READ8_HANDLER( docastle_shared1_r )
{
	if (offset == 8) logerror("CPU #1 shared1r  clock = %d\n",activecpu_gettotalcycles());
	return buffer1[offset];
}


WRITE8_HANDLER( docastle_shared0_w )
{
	if (offset == 8) logerror("CPU #1 shared0w %02x %02x %02x %02x %02x %02x %02x %02x %02x clock = %d\n",
		buffer0[0],buffer0[1],buffer0[2],buffer0[3],buffer0[4],buffer0[5],buffer0[6],buffer0[7],data,activecpu_gettotalcycles());

	buffer0[offset] = data;

	if (offset == 8)
		/* awake the master CPU */
		cpu_trigger(500);
}


WRITE8_HANDLER( docastle_shared1_w )
{
	buffer1[offset] = data;

	if (offset == 8)
	{
		logerror("CPU #0 shared1w %02x %02x %02x %02x %02x %02x %02x %02x %02x clock = %d\n",
				buffer1[0],buffer1[1],buffer1[2],buffer1[3],buffer1[4],buffer1[5],buffer1[6],buffer1[7],data,activecpu_gettotalcycles());

		/* freeze execution of the master CPU until the slave has used the shared memory */
		cpu_spinuntil_trigger(500);
	}
}



WRITE8_HANDLER( docastle_nmitrigger_w )
{
	cpunum_set_input_line(1,INPUT_LINE_NMI,PULSE_LINE);
}
