/***************************************************************************

    Zilog Z80 Parallel Input/Output Controller implementation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************
                            _____   _____
                    D2   1 |*    \_/     | 40  D3
                    D7   2 |             | 39  D4
                    D6   3 |             | 38  D5
                   _CE   4 |             | 37  _M1
                  C/_D   5 |             | 36  _IORQ
                  B/_A   6 |             | 35  RD
                   PA7   7 |             | 34  PB7
                   PA6   8 |             | 33  PB6
                   PA5   9 |             | 32  PB5
                   PA4  10 |    Z8420    | 31  PB4
                   GND  11 |         	 | 30  PB3
                   PA3  12 |             | 29  PB2
                   PA2  13 |             | 28  PB1
                   PA1  14 |             | 27  PB0
                   PA0  15 |             | 26  +5V
                 _ASTB  16 |             | 25  CLK
                 _BSTB  17 |             | 24  IEI
                  ARDY  18 |             | 23  _INT
                    D0  19 |             | 22  IEO
                    D1  20 |_____________| 21  BRDY

***************************************************************************/

#ifndef __Z80PIO__
#define __Z80PIO__

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define Z80PIO DEVICE_GET_INFO_NAME(z80pio)

#define MDRV_Z80PIO_ADD(_tag, _clock, _intrf) \
	MDRV_DEVICE_ADD(_tag, Z80PIO, _clock) \
	MDRV_DEVICE_CONFIG(_intrf)

#define Z80PIO_INTERFACE(_name) \
	const z80pio_interface (_name) =

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _z80pio_interface z80pio_interface;
struct _z80pio_interface
{
	devcb_write_line	out_int_func;

	devcb_read8			in_pa_func;
	devcb_write8		out_pa_func;
	devcb_write_line	out_ardy_func;

	devcb_read8			in_pb_func;
	devcb_write8		out_pb_func;
	devcb_write_line	out_brdy_func;
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO( z80pio );

/* control register access */
READ8_DEVICE_HANDLER( z80pio_c_r );
WRITE8_DEVICE_HANDLER( z80pio_c_w );

/* data register access */
READ8_DEVICE_HANDLER( z80pio_d_r );
WRITE8_DEVICE_HANDLER( z80pio_d_w );

/* register access */
READ8_DEVICE_HANDLER( z80pio_cd_ba_r );
WRITE8_DEVICE_HANDLER( z80pio_cd_ba_w );

READ8_DEVICE_HANDLER( z80pio_ba_cd_r );
WRITE8_DEVICE_HANDLER( z80pio_ba_cd_w );

/* port access */
READ8_DEVICE_HANDLER( z80pio_pa_r );
WRITE8_DEVICE_HANDLER( z80pio_pa_w );

READ8_DEVICE_HANDLER( z80pio_pb_r );
WRITE8_DEVICE_HANDLER( z80pio_pb_w );

/* strobe */
WRITE_LINE_DEVICE_HANDLER( z80pio_astb_w );
WRITE_LINE_DEVICE_HANDLER( z80pio_bstb_w );

#endif
