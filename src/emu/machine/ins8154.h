/***************************************************************************

    National Semiconductor INS8154

    N-Channel 128-by-8 Bit RAM Input/Output (RAM I/O)

                            _____   _____
                   PB6   1 |*    \_/     | 40  VCC
                   PB5   2 |             | 39  PB7
                   PB4   3 |             | 38  NWDS
                   PB3   4 |             | 37  NRDS
                   PB2   5 |             | 36  NRST
                   PB1   6 |             | 35  _CS0
                   PB0   7 |             | 34  CS1
                   DB7   8 |             | 33  M/_IO
                   DB6   9 |             | 32  AD6
                   DB5  10 |   INS8154   | 31  AD5
                   DB4  11 |             | 30  AD4
                   DB3  12 |             | 29  AD3
                   DB2  13 |             | 28  AD2
                   DB1  14 |             | 27  AD1
                   DB0  15 |             | 26  AD0
                   PA7  16 |             | 25  INTR
                   PA6  17 |             | 24  PA0
                   PA5  18 |             | 23  PA1
                   PA4  19 |             | 22  PA2
                   GND  20 |_____________| 21  PA3

***************************************************************************/

#ifndef __INS8154_H__
#define __INS8154_H__

#include "devcb.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _ins8154_interface ins8154_interface;
struct _ins8154_interface
{
	devcb_read8			in_a_func;
	devcb_write8		out_a_func;
	devcb_read8			in_b_func;
	devcb_write8		out_b_func;
	devcb_write_line	out_irq_func;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO( ins8154 );

READ8_DEVICE_HANDLER( ins8154_r );
WRITE8_DEVICE_HANDLER( ins8154_w );

WRITE8_DEVICE_HANDLER( ins8154_porta_w );
WRITE8_DEVICE_HANDLER( ins8154_portb_w );


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define INS8154  DEVICE_GET_INFO_NAME(ins8154)

#define MDRV_INS8154_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, INS8154, 0) \
	MDRV_DEVICE_CONFIG(_intrf)


#endif /* __INS8154_H__ */
