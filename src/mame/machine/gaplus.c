/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "sound/samples.h"
#include "includes/gaplus.h"

UINT8 *gaplus_customio_3;

/************************************************************************************
*                                                                                   *
*           Gaplus custom I/O chips (preliminary)                                   *
*                                                                                   *
************************************************************************************/

WRITE8_HANDLER( gaplus_customio_3_w )
{
	running_device *samples = devtag_get_device(space->machine, "samples");
	if ((offset == 0x09) && (data >= 0x0f))
		sample_start(samples,0,0,0);

	gaplus_customio_3[offset] = data;
}


READ8_HANDLER( gaplus_customio_3_r )
{
	int mode = gaplus_customio_3[8];

	switch (offset)
	{
		case 0:
			return input_port_read(space->machine, "IN2");		/* cabinet & test mode */
		case 1:
			return (mode == 2) ? gaplus_customio_3[offset] : 0x0f;
		case 2:
			return (mode == 2) ? 0x0f : 0x0e;
		case 3:
			return (mode == 2) ? gaplus_customio_3[offset] : 0x01;
		default:
			return gaplus_customio_3[offset];
	}
}
