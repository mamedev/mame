/*****************************************************************************

    74123 monoflop emulator - see 74123.h for pin out and truth table

    Formulas came from the TI datasheet revised on March 1998

 *****************************************************************************/

#include "driver.h"
#include "machine/74123.h"
#include "machine/rescap.h"


#define	LOG		(0)


typedef struct _TTL74123_state TTL74123_state;

struct _TTL74123_state
{
	const TTL74123_interface *intf;
	int which;

	UINT8 A;			/* pin 1/9 */
	UINT8 B;			/* pin 2/10 */
	UINT8 clear;		/* pin 3/11 */
	emu_timer *timer;
};

static TTL74123_state chips[MAX_TTL74123];


static attotime compute_duration(TTL74123_state *chip)
{
	double duration;

	switch (chip->intf->connection_type)
	{
	case TTL74123_NOT_GROUNDED_NO_DIODE:
		duration = 0.28 * chip->intf->res * chip->intf->cap * (1.0 + (0.7 / chip->intf->res));
		break;

	case TTL74123_NOT_GROUNDED_DIODE:
		duration = 0.25 * chip->intf->res * chip->intf->cap * (1.0 + (0.7 / chip->intf->res));
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


static int timer_running(TTL74123_state *chip)
{
	return (attotime_compare(timer_timeleft(chip->timer), attotime_zero) > 0) &&
		   (attotime_compare(timer_timeleft(chip->timer), attotime_never) != 0);
}


static void set_output(TTL74123_state *chip)
{
	int output = timer_running(chip);

	chip->intf->output_changed_cb(output);

	if (LOG) logerror("74123 #%d:  Output: %d\n", chip->which, output);
}


static TIMER_CALLBACK( clear_callback )
{
	TTL74123_state *chip = ptr;

	set_output(chip);
}


void TTL74123_config(int which, const TTL74123_interface *intf)
{
	TTL74123_state *chip;

	assert_always(which < MAX_TTL74123, "Exceeded maximum number of 74123 chips");
	assert_always(intf, "No interface specified");
	assert_always((intf->connection_type == TTL74123_GROUNDED) && (intf->cap >= CAP_U(0.01)), "Only capacitors >= 0.01uF supported for GROUNDED type");
	assert_always(intf->cap >= CAP_P(1000), "Only capacitors >= 1000pF supported ");

	chip = &chips[which];

	chip->intf = intf;
	chip->which = which;
	chip->timer = timer_alloc(clear_callback, chip);

	/* start with the defaults */
	chip->A = intf->A;
    chip->B = intf->B;
	chip->clear = intf->clear;

	/* register for state saving */
	state_save_register_item("TTL74123", which, chip->A);
	state_save_register_item("TTL74123", which, chip->B);
	state_save_register_item("TTL74123", which, chip->clear);
}


void TTL74123_reset(int which)
{
	TTL74123_state *chip = &chips[which];

	set_output(chip);
}


static void start_pulse(TTL74123_state *chip)
{
	attotime duration = compute_duration(chip);

	if (timer_running(chip))
	{
		/* retriggering, but not if we are called to quickly */
		attotime delay_time = attotime_make(0, ATTOSECONDS_PER_SECOND * chip->intf->cap * 220);

		if (attotime_compare(timer_timeelapsed(chip->timer), delay_time) >= 0)
		{
			timer_adjust_oneshot(chip->timer, duration, 0);

			if (LOG) logerror("74123 #%d:  Retriggering pulse.  Duration: %f\n", chip->which, attotime_to_double(duration));
		}
		else
		{
			if (LOG) logerror("74123 #%d:  Retriggering failed.\n", chip->which);
		}
	}
	else
	{
		/* starting */
		timer_adjust_oneshot(chip->timer, duration, 0);

		set_output(chip);

		if (LOG) logerror("74123 #%d:  Starting pulse.  Duration: %f\n", chip->which, attotime_to_double(duration));
	}
}


void TTL74123_A_w(int which, int data)
{
	TTL74123_state *chip = &chips[which];

	/* start/regtrigger pulse if B=HI and falling edge on A (while clear is HI) */
	if (!data && chip->A && chip->B && chip->clear)
		start_pulse(chip);

	chip->A = data;
}


void TTL74123_B_w(int which, int data)
{
	TTL74123_state *chip = &chips[which];

	/* start/regtrigger pulse if A=LO and rising edge on B (while clear is HI) */
	if (data && !chip->B && !chip->A && chip->clear)
		start_pulse(chip);

	chip->B = data;
}


void TTL74123_clear_w(int which, int data)
{
	TTL74123_state *chip = &chips[which];

	/* clear the output if A=LO, B=HI and falling edge on clear */
	if (!data && chip->clear && chip->B && !chip->A && !chip->clear)
	{
		timer_adjust_oneshot(chip->timer, attotime_zero, 0);

		if (LOG) logerror("74123 #%d:  Cleared\n", which);
	}

	chip->clear = data;
}
