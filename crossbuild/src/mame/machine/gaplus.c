/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "driver.h"
#include "machine/namcoio.h"
#include "sound/samples.h"

UINT8 *gaplus_customio_3;

/************************************************************************************
*                                                                                   *
*           Gaplus custom I/O chips (preliminary)                                   *
*                                                                                   *
************************************************************************************/

WRITE8_HANDLER( gaplus_customio_3_w )
{
	if ((offset == 0x09) && (data >= 0x0f))
		sample_start(0,0,0);
    gaplus_customio_3[offset] = data;
}


READ8_HANDLER( gaplus_customio_3_r )
{
    int mode;

    mode = gaplus_customio_3[8];
    if (mode == 2)
    {
        switch (offset)
        {
            case 0:
                return readinputport( 4 ); /* cabinet & test mode */
                break;
            case 2:
                return 0x0f;
                break;
            default:
                return gaplus_customio_3[offset];
        }
    }
    else
    {
        switch (offset)
        {
            case 0:
                return readinputport( 4 ); /* cabinet & test mode */
                break;
            case 1:
                return 0x0f;
                break;
            case 2:
                return 0x0e;
                break;
            case 3:
                return 0x01;
                break;
            default:
                return gaplus_customio_3[offset];
        }
    }
}
