/***************************************************************************

    Z80 PIO (Z8420) implementation

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
                   PA4  10 |   Z80-PIO   | 31  PB4
                   GND  11 |             | 30  PB3
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

#ifndef __Z80PIO_H__
#define __Z80PIO_H__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _z80pio_interface z80pio_interface;
struct _z80pio_interface
{
	void (*intr)(const device_config *device, int which);    /* callback when change interrupt status */
	read8_device_func portAread;    /* port A read callback */
	read8_device_func portBread;    /* port B read callback */
	write8_device_func portAwrite;  /* port A write callback */
	write8_device_func portBwrite;  /* port B write callback */
	void (*rdyA)(int data);     /* portA ready active callback (do not support yet)*/
	void (*rdyB)(int data);     /* portB ready active callback (do not support yet)*/
};



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_Z80PIO_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, Z80PIO) \
	MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_Z80PIO_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag, Z80PIO)



/***************************************************************************
    CONTROL REGISTER READ/WRITE
***************************************************************************/

WRITE8_DEVICE_HANDLER( z80pio_c_w );
READ8_DEVICE_HANDLER( z80pio_c_r );


/***************************************************************************
    DATA REGISTER READ/WRITE
***************************************************************************/

WRITE8_DEVICE_HANDLER( z80pio_d_w );
READ8_DEVICE_HANDLER( z80pio_d_r );


/***************************************************************************
    PORT I/O
***************************************************************************/

WRITE8_DEVICE_HANDLER( z80pio_p_w );
READ8_DEVICE_HANDLER( z80pio_p_r );


/***************************************************************************
    STROBE STATE MANAGEMENT
***************************************************************************/

void z80pio_astb_w(const device_config *device, int state);
void z80pio_bstb_w(const device_config *device, int state);


/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/
READ8_DEVICE_HANDLER(z80pio_r);
WRITE8_DEVICE_HANDLER(z80pio_w);


/* ----- device interface ----- */

#define Z80PIO DEVICE_GET_INFO_NAME(z80pio)
DEVICE_GET_INFO( z80pio );

#endif
