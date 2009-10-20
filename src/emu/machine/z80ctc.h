/***************************************************************************

    Z80 CTC (Z8430) implementation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __Z80CTC_H__
#define __Z80CTC_H__

#include "devcb.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define NOTIMER_0 (1<<0)
#define NOTIMER_1 (1<<1)
#define NOTIMER_2 (1<<2)
#define NOTIMER_3 (1<<3)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _z80ctc_interface z80ctc_interface;
struct _z80ctc_interface
{
	int notimer;				/* timer disablers */
	devcb_write_line intr;		/* callback when change interrupt status */
	devcb_write_line zc0;		/* ZC/TO0 callback */
	devcb_write_line zc1;		/* ZC/TO1 callback */
	devcb_write_line zc2;		/* ZC/TO2 callback */
};


#define Z80CTC_INTERFACE(name) \
	const z80ctc_interface (name)=



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_Z80CTC_ADD(_tag, _clock, _intrf) \
	MDRV_DEVICE_ADD(_tag, Z80CTC, _clock) \
	MDRV_DEVICE_CONFIG(_intrf)



/***************************************************************************
    INITIALIZATION/CONFIGURATION
***************************************************************************/

attotime z80ctc_getperiod(const device_config *device, int ch);



/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/

WRITE8_DEVICE_HANDLER( z80ctc_w );
READ8_DEVICE_HANDLER( z80ctc_r );



/***************************************************************************
    EXTERNAL TRIGGERS
***************************************************************************/

WRITE_LINE_DEVICE_HANDLER( z80ctc_trg0_w );
WRITE_LINE_DEVICE_HANDLER( z80ctc_trg1_w );
WRITE_LINE_DEVICE_HANDLER( z80ctc_trg2_w );
WRITE_LINE_DEVICE_HANDLER( z80ctc_trg3_w );



/* ----- device interface ----- */

#define Z80CTC DEVICE_GET_INFO_NAME(z80ctc)
DEVICE_GET_INFO( z80ctc );

#endif
