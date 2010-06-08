/***************************************************************************

    Intel 8257 Programmable DMA Controller emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************
                            _____   _____
                 _I/OR   1 |*    \_/     | 40  A7
                 _I/OW   2 |             | 39  A6
                 _MEMR   3 |             | 38  A5
                 _MEMW   4 |             | 37  A4
                  MARK   5 |             | 36  TC
                 READY   6 |             | 35  A3
                  HLDA   7 |             | 34  A2
                 ADSTB   8 |             | 33  A1
                   AEN   9 |             | 32  A0
                   HRQ  10 |     8257    | 31  Vcc
                   _CS  11 |             | 30  D0
                   CLK  12 |             | 29  D1
                 RESET  13 |             | 28  D2
                _DACK2  14 |             | 27  D3
                _DACK3  15 |             | 26  D4
                  DRQ3  16 |             | 25  _DACK0
                  DRQ2  17 |             | 24  _DACK1
                  DRQ1  18 |             | 23  D5
                  DRQ0  19 |             | 22  D6
                   GND  20 |_____________| 21  D7

***************************************************************************/

#ifndef __I8257__
#define __I8257__

#include "devlegcy.h"


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

DECLARE_LEGACY_DEVICE(I8257, i8257);

#define MDRV_I8257_ADD(_tag, _clock, _config) \
	MDRV_DEVICE_ADD(_tag, I8257, _clock) \
	MDRV_DEVICE_CONFIG(_config)

#define I8257_INTERFACE(_name) \
	const i8257_interface (_name) =

#define I8257_NUM_CHANNELS		(4)

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _i8257_interface i8257_interface;
struct _i8257_interface
{
	devcb_write_line	out_hrq_func;
	devcb_write_line	out_tc_func;
	devcb_write_line	out_mark_func;

	/* accessors to main memory */
	devcb_read8			in_memr_func;
	devcb_write8		out_memw_func;

	/* channel accesors */
	devcb_read8			in_ior_func[I8257_NUM_CHANNELS];
	devcb_write8		out_iow_func[I8257_NUM_CHANNELS];
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* register access */
READ8_DEVICE_HANDLER( i8257_r );
WRITE8_DEVICE_HANDLER( i8257_w );

/* hold acknowledge */
WRITE_LINE_DEVICE_HANDLER( i8257_hlda_w );

/* ready */
WRITE_LINE_DEVICE_HANDLER( i8257_ready_w );

/* data request */
WRITE_LINE_DEVICE_HANDLER( i8257_drq0_w );
WRITE_LINE_DEVICE_HANDLER( i8257_drq1_w );
WRITE_LINE_DEVICE_HANDLER( i8257_drq2_w );
WRITE_LINE_DEVICE_HANDLER( i8257_drq3_w );

#endif
