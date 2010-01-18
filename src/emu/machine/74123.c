/*****************************************************************************

    74123 monoflop emulator - see 74123.h for pin out and truth table

    Formulas came from the TI datasheet revised on March 1998

 *****************************************************************************/

#include "emu.h"
#include "machine/74123.h"
#include "machine/rescap.h"


#define	LOG		(0)


typedef struct _ttl74123_t ttl74123_t;

struct _ttl74123_t
{
	const ttl74123_config *intf;

	UINT8 a;			/* pin 1/9 */
	UINT8 b;			/* pin 2/10 */
	UINT8 clear;		/* pin 3/11 */
	emu_timer *timer;
};

/* ----------------------------------------------------------------------- */

INLINE ttl74123_t *get_safe_token(running_device *device) {
	assert( device != NULL );
	assert( device->token != NULL );
	assert( device->type == TTL74123 );
	return ( ttl74123_t * ) device->token;
}


static attotime compute_duration(ttl74123_t *chip)
{
	double duration;

	switch (chip->intf->connection_type)
	{
	case TTL74123_NOT_GROUNDED_NO_DIODE:
		duration = 0.28 * chip->intf->res * chip->intf->cap * (1.0 + (700.0 / chip->intf->res));
		break;

	case TTL74123_NOT_GROUNDED_DIODE:
		duration = 0.25 * chip->intf->res * chip->intf->cap * (1.0 + (700.0 / chip->intf->res));
		break;

	case TTL74123_GROUNDED:
	default:
		if (chip->intf->cap < CAP_U(0.1))
			/* this is really a curve - a very flat one in the 0.1uF-.01uF range */
			duration = 0.32 * chip->intf->res * chip->intf->cap;
		else
			duration = 0.33 * chip->intf->res * chip->intf->cap;
		break;
	}

	return double_to_attotime(duration);
}


static int timer_running(ttl74123_t *chip)
{
	return (attotime_compare(timer_timeleft(chip->timer), attotime_zero) > 0) &&
		   (attotime_compare(timer_timeleft(chip->timer), attotime_never) != 0);
}


static TIMER_CALLBACK( output_callback )
{
	running_device *device = (running_device *)ptr;
	ttl74123_t *chip = get_safe_token(device);

	chip->intf->output_changed_cb(device, 0, param);
}


static void set_output(running_device *device)
{
	ttl74123_t *chip = get_safe_token(device);
	int output = timer_running(chip);

	timer_set( device->machine, attotime_zero, (void *) device, output, output_callback );

	if (LOG) logerror("74123 %s:  Output: %d\n", device->tag.cstr(), output);
}


static TIMER_CALLBACK( clear_callback )
{
	running_device *device = (running_device *)ptr;
	ttl74123_t *chip = get_safe_token(device);
	int output = timer_running(chip);

	chip->intf->output_changed_cb(device, 0, output);
}




static void start_pulse(running_device *device)
{
	ttl74123_t *chip = get_safe_token(device);

	attotime duration = compute_duration(chip);

	if (timer_running(chip))
	{
		/* retriggering, but not if we are called to quickly */
		attotime delay_time = attotime_make(0, ATTOSECONDS_PER_SECOND * chip->intf->cap * 220);

		if (attotime_compare(timer_timeelapsed(chip->timer), delay_time) >= 0)
		{
			timer_adjust_oneshot(chip->timer, duration, 0);

			if (LOG) logerror("74123 %s:  Retriggering pulse.  Duration: %f\n", device->tag.cstr(), attotime_to_double(duration));
		}
		else
		{
			if (LOG) logerror("74123 %s:  Retriggering failed.\n", device->tag.cstr());
		}
	}
	else
	{
		/* starting */
		timer_adjust_oneshot(chip->timer, duration, 0);

		set_output(device);

		if (LOG) logerror("74123 %s:  Starting pulse.  Duration: %f\n", device->tag.cstr(), attotime_to_double(duration));
	}
}


WRITE8_DEVICE_HANDLER( ttl74123_a_w )
{
	ttl74123_t *chip = get_safe_token(device);

	/* start/regtrigger pulse if B=HI and falling edge on A (while clear is HI) */
	if (!data && chip->a && chip->b && chip->clear)
		start_pulse(device);

	chip->a = data;
}


WRITE8_DEVICE_HANDLER( ttl74123_b_w )
{
	ttl74123_t *chip = get_safe_token(device);

	/* start/regtrigger pulse if A=LO and rising edge on B (while clear is HI) */
	if (data && !chip->b && !chip->a && chip->clear)
		start_pulse(device);

	chip->b = data;
}


WRITE8_DEVICE_HANDLER( ttl74123_clear_w )
{
	ttl74123_t *chip = get_safe_token(device);

	/* start/regtrigger pulse if B=HI and A=LO and rising edge on clear */
	if (data && !chip->a && chip->b && !chip->clear)
		start_pulse(device);
	else if (!data) 	/* clear the output  */
	{
		timer_adjust_oneshot(chip->timer, attotime_zero, 0);

		if (LOG) logerror("74123 #%s:  Cleared\n", device->tag.cstr() );
	}
	chip->clear = data;
}

WRITE8_DEVICE_HANDLER( ttl74123_reset_w )
{
	set_output(device);
}

/* ----------------------------------------------------------------------- */

/* device interface */

static DEVICE_START( ttl74123 )
{
	ttl74123_t *chip = get_safe_token(device);

	/* validate arguments */
	chip->intf = (ttl74123_config *)device->baseconfig().static_config;

	assert_always(chip->intf, "No interface specified");
	assert_always((chip->intf->connection_type != TTL74123_GROUNDED) || (chip->intf->cap >= CAP_U(0.01)), "Only capacitors >= 0.01uF supported for GROUNDED type");
	assert_always(chip->intf->cap >= CAP_P(1000), "Only capacitors >= 1000pF supported ");

	chip->timer = timer_alloc(device->machine, clear_callback, (void *) device);

	/* start with the defaults */
	chip->a = chip->intf->a;
    chip->b = chip->intf->b;
	chip->clear = chip->intf->clear;

	/* register for state saving */
	state_save_register_device_item(device, 0, chip->a);
	state_save_register_device_item(device, 0, chip->b);
	state_save_register_device_item(device, 0, chip->clear);
}


static DEVICE_RESET( ttl74123 )
{
	set_output(device);
}


DEVICE_GET_INFO( ttl74123 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(ttl74123_t);						break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;										break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(ttl74123);			break;
		case DEVINFO_FCT_STOP:							/* Nothing */										break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(ttl74123);			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "74123");							break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "TTL");								break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");								break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);							break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
