/**********************************************************************

    RCA CDP1863 CMOS 8-Bit Programmable Frequency Generator emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                _RESET   1 |*    \_/     | 16  Vdd
                 CLK 2   2 |             | 15  OE
                 CLK 1   3 |             | 14  OUT
                   STR   4 |   CDP1863   | 13  DO7
                   DI0   5 |             | 12  DI6
                   DI1   6 |             | 11  DI5
                   DI2   7 |             | 10  DI4
                   Vss   8 |_____________| 9   DI3

**********************************************************************/

#ifndef __CDP1863__
#define __CDP1863__

#include "devlegcy.h"

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

DECLARE_LEGACY_SOUND_DEVICE(CDP1863, cdp1863);

#define MDRV_CDP1863_ADD(_tag, _clock1, _clock2) \
	MDRV_SOUND_ADD(_tag, CDP1863, _clock1) \
	MDRV_DEVICE_CONFIG_DATA32(cdp1863_config, clock2, _clock2)

#define CDP1863_INTERFACE(name) \
	const cdp1863_interface (name) =

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _cdp1863_config cdp1863_config;
struct _cdp1863_config
{
	int clock2;				/* the clock 2 (pin 2) of the chip */
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* load tone latch */
WRITE8_DEVICE_HANDLER( cdp1863_str_w );

/* output enable */
WRITE_LINE_DEVICE_HANDLER( cdp1863_oe_w );

/* clock setters */
void cdp1863_set_clk1(running_device *device, int frequency) ATTR_NONNULL(1);
void cdp1863_set_clk2(running_device *device, int frequency) ATTR_NONNULL(1);

#endif
