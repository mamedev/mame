/**********************************************************************

    Mostek MK3801 Serial Timer Interrupt Controller (Z80-STI) emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   TAO   1 |*    \_/     | 40  Vcc
                   TBO   2 |             | 39  RC
                   TCO   3 |             | 38  SI
                   TDO   4 |             | 37  SO
                   TCK   5 |             | 36  TC
                   _M1   6 |             | 35  A0
                  _RES   7 |             | 34  A1
                    I0   8 |             | 33  A2
                    I1   9 |             | 32  A3
                    I2  10 |   MK3801    | 31  _WR
                    I3  11 |   Z80-STI   | 30  _CE
                    I4  12 |             | 29  _RD
                    I5  13 |             | 28  D7
                    I6  14 |             | 27  D6
                    I7  15 |             | 26  D5
                   IEI  16 |             | 25  D4
                  _INT  17 |             | 24  D3
                   IEO  18 |             | 23  D2
                 _IORQ  19 |             | 22  D1
                   Vss  20 |_____________| 21  D0

**********************************************************************/

#ifndef __Z80STI__
#define __Z80STI__

#include "emu.h"
#include "devcb.h"

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define Z80STI DEVICE_GET_INFO_NAME( z80sti )

#define MDRV_Z80STI_ADD(_tag, _clock, _config) \
	MDRV_DEVICE_ADD((_tag), Z80STI, _clock)	\
	MDRV_DEVICE_CONFIG(_config)

#define Z80STI_INTERFACE(name) const z80sti_interface (name) =

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _z80sti_interface z80sti_interface;
struct _z80sti_interface
{
	int	rx_clock;			/* serial receive clock */
	int	tx_clock;			/* serial transmit clock */

	/* this gets called on each change of the _INT pin (pin 17) */
	devcb_write_line		out_int_func;

	/* this is called on each read of the GPIO pins */
	devcb_read8				in_gpio_func;

	/* this is called on each write of the GPIO pins */
	devcb_write8			out_gpio_func;

	/* this gets called for each read of the SI pin (pin 38) */
	devcb_read_line			in_si_func;

	/* this gets called for each change of the SO pin (pin 37) */
	devcb_write_line		out_so_func;

	/* this gets called for each change of the TAO pin (pin 1) */
	devcb_write_line		out_tao_func;

	/* this gets called for each change of the TBO pin (pin 2) */
	devcb_write_line		out_tbo_func;

	/* this gets called for each change of the TCO pin (pin 3) */
	devcb_write_line		out_tco_func;

	/* this gets called for each change of the TDO pin (pin 4) */
	devcb_write_line		out_tdo_func;
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* device interface */
DEVICE_GET_INFO( z80sti );

/* register access */
READ8_DEVICE_HANDLER( z80sti_r );
WRITE8_DEVICE_HANDLER( z80sti_w );

/* receive clock */
WRITE_LINE_DEVICE_HANDLER( z80sti_rc_w );

/* transmit clock */
WRITE_LINE_DEVICE_HANDLER( z80sti_tc_w );

/* GPIP input lines */
WRITE_LINE_DEVICE_HANDLER( z80sti_i0_w );
WRITE_LINE_DEVICE_HANDLER( z80sti_i1_w );
WRITE_LINE_DEVICE_HANDLER( z80sti_i2_w );
WRITE_LINE_DEVICE_HANDLER( z80sti_i3_w );
WRITE_LINE_DEVICE_HANDLER( z80sti_i4_w );
WRITE_LINE_DEVICE_HANDLER( z80sti_i5_w );
WRITE_LINE_DEVICE_HANDLER( z80sti_i6_w );
WRITE_LINE_DEVICE_HANDLER( z80sti_i7_w );

#endif
