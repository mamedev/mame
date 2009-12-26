/***************************************************************************

    Intel 8237 Programmable DMA Controller emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************
                            _____   _____
                 _I/OR   1 |*    \_/     | 40  A7
                 _I/OW   2 |             | 39  A6
                 _MEMR   3 |             | 38  A5
                 _MEMW   4 |             | 37  A4
                         5 |             | 36  _EOP
                 READY   6 |             | 35  A3
                  HLDA   7 |             | 34  A2
                 ADSTB   8 |             | 33  A1
                   AEN   9 |             | 32  A0
                   HRQ  10 |     8237    | 31  Vcc
                   _CS  11 |             | 30  DB0
                   CLK  12 |             | 29  DB1
                 RESET  13 |             | 28  DB2
                 DACK2  14 |             | 27  DB3
                 DACK3  15 |             | 26  DB4
                 DREQ3  16 |             | 25  DACK0
                 DREQ2  17 |             | 24  DACK1
                 DREQ1  18 |             | 23  DB5
                 DREQ0  19 |             | 22  DB6
                   GND  20 |_____________| 21  DB7

***************************************************************************/

#ifndef __I8237__
#define __I8237__

#include "devcb.h"

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define I8237 DEVICE_GET_INFO_NAME(i8237)

#define MDRV_I8237_ADD(_tag, _clock, _config) \
	MDRV_DEVICE_ADD(_tag, I8237, _clock) \
	MDRV_DEVICE_CONFIG(_config)

#define I8237_INTERFACE(_name) \
	const i8237_interface (_name) =

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _i8237_interface i8237_interface;
struct _i8237_interface
{
	devcb_write_line	out_hrq_func;
	devcb_write_line	out_eop_func;

	/* accessors to main memory */
	devcb_read8			in_memr_func;
	devcb_write8		out_memw_func;

	/* channel accessors */
	devcb_read8			in_ior_func[4];
	devcb_write8		out_iow_func[4];
	devcb_write_line	out_dack_func[4];
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* register access */
READ8_DEVICE_HANDLER( i8237_r );
WRITE8_DEVICE_HANDLER( i8237_w );

/* hold acknowledge */
WRITE_LINE_DEVICE_HANDLER( i8237_hlda_w );

/* ready */
WRITE_LINE_DEVICE_HANDLER( i8237_ready_w );

/* data request */
WRITE_LINE_DEVICE_HANDLER( i8237_dreq0_w );
WRITE_LINE_DEVICE_HANDLER( i8237_dreq1_w );
WRITE_LINE_DEVICE_HANDLER( i8237_dreq2_w );
WRITE_LINE_DEVICE_HANDLER( i8237_dreq3_w );

/* end of process */
WRITE_LINE_DEVICE_HANDLER( i8237_eop_w );

DEVICE_GET_INFO( i8237 );

#endif
