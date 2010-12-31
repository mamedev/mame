/**********************************************************************

    RCA CDP1852 Byte-Wide Input/Output Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
              CSI/_CSI   1 |*    \_/     | 24  Vdd
                  MODE   2 |             | 23  _SR/SR
                   DI0   3 |             | 22  DI7
                   DO0   4 |             | 21  DO7
                   DI1   5 |             | 20  DI6
                   DO1   6 |   CDP1852   | 19  DO6
                   DI2   7 |             | 18  DI5
                   DO2   8 |             | 17  DO5
                   DI3   9 |             | 16  DI4
                   DO3  10 |             | 15  DO4
                 CLOCK  11 |             | 14  _CLEAR
                   Vss  12 |_____________| 13  CS2

**********************************************************************/

#ifndef __CDP1852__
#define __CDP1852__

#include "devlegcy.h"


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define CDP1852_CLOCK_HIGH	0

DECLARE_LEGACY_DEVICE(CDP1852, cdp1852);

#define MCFG_CDP1852_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, CDP1852, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define CDP1852_INTERFACE(_name) \
	const cdp1852_interface (_name)=

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

enum _cdp1852_mode {
	CDP1852_MODE_INPUT = 0,
	CDP1852_MODE_OUTPUT
};
typedef enum _cdp1852_mode cdp1852_mode;

typedef struct _cdp1852_interface cdp1852_interface;
struct _cdp1852_interface
{
	int mode;				/* operation mode */

	/* this gets called for every external data read */
	devcb_read8				in_data_func;

	/* this gets called for every external data write */
	devcb_write8			out_data_func;

	/* this gets called for every change of the SR pin (pin 23) */
	devcb_write_line		out_sr_func;
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* data access */
READ8_DEVICE_HANDLER( cdp1852_data_r );
WRITE8_DEVICE_HANDLER( cdp1852_data_w );

#endif
