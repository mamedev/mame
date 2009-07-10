/**********************************************************************

    Intel 8255A Programmable Peripheral Interface emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
							_____   _____
				   PA3	 1 |*    \_/     | 40  PA4
				   PA2	 2 |			 | 39  PA5
				   PA1	 3 |			 | 38  PA6
				   PA0	 4 |			 | 37  PA7
				   _RD	 5 |			 | 36  WR
				   _CS	 6 |			 | 35  RESET
				   GND	 7 |			 | 34  D0
					A1	 8 |			 | 33  D1
					A0	 9 |			 | 32  D2
				   PC7	10 |    8255A	 | 31  D3
				   PC6	11 |			 | 30  D4
				   PC5	12 |			 | 29  D5
				   PC4	13 |			 | 28  D6
				   PC0	14 |			 | 27  D7
				   PC1	15 |			 | 26  Vcc
				   PC2	16 |			 | 25  PB7
				   PC3	17 |			 | 24  PB6
				   PB0	18 |			 | 23  PB5
				   PB1	19 |			 | 22  PB4
				   PB2  20 |_____________| 21  PB3

**********************************************************************/

#ifndef __I8255A__
#define __I8255A__

#include "devcb.h"

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define I8255A		DEVICE_GET_INFO_NAME(i8255a)

#define MDRV_I8255A_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, I8255A, 0) \
	MDRV_DEVICE_CONFIG(_intrf)

#define I8255A_INTERFACE(name) \
	const i8255a_interface (name)=

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _i8255a_interface i8255a_interface;
struct _i8255a_interface
{
	devcb_read8			in_pa_func;
	devcb_read8			in_pb_func;
	devcb_read8			in_pc_func;
	
	devcb_write8		out_pa_func;
	devcb_write8		out_pb_func;
	devcb_write8		out_pc_func;
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* device interface */
DEVICE_GET_INFO( i8255a );

/* register access */
READ8_DEVICE_HANDLER( i8255a_r );
WRITE8_DEVICE_HANDLER( i8255a_w );

/* port access */
READ8_DEVICE_HANDLER( i8255a_pa_r );
READ8_DEVICE_HANDLER( i8255a_pb_r );

/* handshaking signals */
WRITE_LINE_DEVICE_HANDLER( i8255a_pc2_w );
WRITE_LINE_DEVICE_HANDLER( i8255a_pc4_w );
WRITE_LINE_DEVICE_HANDLER( i8255a_pc6_w );

#endif
