/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "sound/samples.h"
#include "includes/gaplus.h"


/************************************************************************************
*                                                                                   *
*           Gaplus custom I/O chips (preliminary)                                   *
*                                                                                   *
************************************************************************************/

WRITE8_MEMBER(gaplus_state::gaplus_customio_3_w)
{
	samples_device *samples = machine().device<samples_device>("samples");
	if ((offset == 0x09) && (data >= 0x0f))
		samples->start(0,0);

	m_customio_3[offset] = data;
}


READ8_MEMBER(gaplus_state::gaplus_customio_3_r)
{
	int mode = m_customio_3[8];

	switch (offset)
	{
		case 0:
			return input_port_read(machine(), "IN2");		/* cabinet & test mode */
		case 1:
			return (mode == 2) ? m_customio_3[offset] : 0x0f;
		case 2:
			return (mode == 2) ? 0x0f : 0x0e;
		case 3:
			return (mode == 2) ? m_customio_3[offset] : 0x01;
		default:
			return m_customio_3[offset];
	}
}
