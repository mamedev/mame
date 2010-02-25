/***************************************************************************

    Intel 8253/8254

    Programmable Interval Timer

                            _____   _____
                    D7   1 |*    \_/     | 24  VCC
                    D6   2 |             | 23  _WR
                    D5   3 |             | 22  _RD
                    D4   4 |             | 21  _CS
                    D3   5 |             | 20  A1
                    D2   6 |             | 19  A0
                    D1   7 |    8259A    | 18  CLK2
                    D0   8 |             | 17  OUT2
                  CLK0   9 |             | 16  GATE2
                  OUT0  10 |             | 15  CLK1
                 GATE0  11 |             | 14  GATE1
                   GND  12 |_____________| 13  OUT1

***************************************************************************/

#ifndef __PIT8253_H__
#define __PIT8253_H__

#include "devcb.h"


#define PIT8253		DEVICE_GET_INFO_NAME(pit8253)
#define PIT8254		DEVICE_GET_INFO_NAME(pit8254)

struct pit8253_config
{
	struct
	{
		double				clockin;		/* timer clock */
		devcb_read_line		in_gate_func;	/* gate signal */
		devcb_write_line	out_out_func;	/* out signal */
	} timer[3];
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_PIT8253_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, PIT8253, 0) \
	MDRV_DEVICE_CONFIG(_intrf)


#define MDRV_PIT8254_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, PIT8254, 0) \
	MDRV_DEVICE_CONFIG(_intrf)


/* device interface */
DEVICE_GET_INFO( pit8253 );
DEVICE_GET_INFO( pit8254 );

READ8_DEVICE_HANDLER( pit8253_r );
WRITE8_DEVICE_HANDLER( pit8253_w );

WRITE_LINE_DEVICE_HANDLER( pit8253_clk0_w );
WRITE_LINE_DEVICE_HANDLER( pit8253_clk1_w );
WRITE_LINE_DEVICE_HANDLER( pit8253_clk2_w );

WRITE_LINE_DEVICE_HANDLER( pit8253_gate0_w );
WRITE_LINE_DEVICE_HANDLER( pit8253_gate1_w );
WRITE_LINE_DEVICE_HANDLER( pit8253_gate2_w );

/* In the 8253/8254 the CLKx input lines can be attached to a regular clock
   signal. Another option is to use the output from one timer as the input
   clock to another timer.

   The functions below should supply both functionalities. If the signal is
   a regular clock signal, use the pit8253_set_clockin function. If the
   CLKx input signal is the output of the different source, set the new_clockin
   to 0 with pit8253_set_clockin and call pit8253_clkX_w to change
   the state of the input CLKx signal.
 */
int pit8253_get_output(running_device *device, int timer);
void pit8253_set_clockin(running_device *device, int timer, double new_clockin);


#endif	/* __PIT8253_H__ */
