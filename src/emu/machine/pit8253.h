/*****************************************************************************
 *
 *  Programmable Interval Timer 8253/8254
 *
 *****************************************************************************/

#ifndef __PIT8253_H_
#define __PIT8253_H_

#define PIT8253		DEVICE_GET_INFO_NAME(pit8253)
#define PIT8254		DEVICE_GET_INFO_NAME(pit8254)


typedef void (*pit8253_output_changed_func)(const device_config *device, int state);
#define PIT8253_OUTPUT_CHANGED(name)	void name(const device_config *device, int state )

typedef void (*pit8253_frequency_changed_func)(const device_config *device, double frequency);
#define PIT8253_FREQUENCY_CHANGED(name)	void name(const device_config *device, double frequency)


struct pit8253_config
{
	struct
	{
		/* Input clock for this timer */
		double clockin;

		/* If specified, this gets called whenever the output for this timer changes */
		pit8253_output_changed_func			output_changed;

		/* If specified, this gets called whenever the frequency of the output for this
		   timer changes. */
		pit8253_frequency_changed_func		frequency_changed;		
	} timer[3];
};


/* device interface */
DEVICE_GET_INFO( pit8253 );
DEVICE_GET_INFO( pit8254 );

READ8_DEVICE_HANDLER( pit8253_r );
WRITE8_DEVICE_HANDLER( pit8253_w );

WRITE8_DEVICE_HANDLER( pit8253_gate_w );


int pit8253_get_frequency(const device_config *device, int timer);
int pit8253_get_output(const device_config *device, int timer);
void pit8253_set_clockin(const device_config *device, int timer, double new_clockin);


#endif	/* __PIT8253_H_ */

