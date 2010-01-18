/*****************************************************************************
 *
 *  Programmable Interval Timer 8253/8254
 *
 *****************************************************************************/

#ifndef __PIT8253_H_
#define __PIT8253_H_

#define PIT8253		DEVICE_GET_INFO_NAME(pit8253)
#define PIT8254		DEVICE_GET_INFO_NAME(pit8254)


typedef void (*pit8253_output_changed_func)(running_device *device, int state);
#define PIT8253_OUTPUT_CHANGED(name)	void name(running_device *device, int state )


struct pit8253_config
{
	struct
	{
		/* Input clock for this timer */
		double clockin;

		/* If specified, this gets called whenever the output for this timer changes */
		pit8253_output_changed_func			output_changed;

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

WRITE8_DEVICE_HANDLER( pit8253_gate_w );


/* In the 8253/8254 the CLKx input lines can be attached to a regular clock
   signal. Another option is to use the output from one timer as the input
   clock to another timer.

   The functions below should supply both functionalities. If the signal is
   a regular clock signal, use the pit8253_set_clockin function. If the
   CLKx input signal is the output of the different source, set the new_clockin
   to 0 with pit8253_set_clockin and call pit8253_set_clock_signal to change
   the state of the input CLKx signal.
 */
int pit8253_get_output(running_device *device, int timer);
void pit8253_set_clockin(running_device *device, int timer, double new_clockin);
void pit8253_set_clock_signal(running_device *device, int timer, int state);


#endif	/* __PIT8253_H_ */

