/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/

#include "driver.h"
#include "includes/mw8080bw.h"

/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static UINT8 vpos_to_vysnc_chain_counter( int vpos )
{
	/* convert from a vertical position to the actual values on the vertical sync counters */
	UINT8 counter;
	int vblank = (vpos >= MW8080BW_VBSTART);

	if (vblank)
		counter = vpos - MW8080BW_VBSTART + MW8080BW_VCOUNTER_START_VBLANK;
	else
		counter = vpos + MW8080BW_VCOUNTER_START_NO_VBLANK;

	return counter;
}


static int vysnc_chain_counter_to_vpos( UINT8 counter, int vblank )
{
	/* convert from the vertical sync counters to an actual vertical position */
	int vpos;

	if (vblank)
		vpos = counter - MW8080BW_VCOUNTER_START_VBLANK + MW8080BW_VBSTART;
	else
		vpos = counter - MW8080BW_VCOUNTER_START_NO_VBLANK;

	return vpos;
}


static TIMER_CALLBACK( mw8080bw_interrupt_callback )
{
	mw8080bw_state *state = (mw8080bw_state *)machine->driver_data;
	UINT8 next_counter;
	int next_vpos;
	int next_vblank;

	/* compute vector and set the interrupt line */
	int vpos = video_screen_get_vpos(machine->primary_screen);
	UINT8 counter = vpos_to_vysnc_chain_counter(vpos);
	UINT8 vector = 0xc7 | ((counter & 0x40) >> 2) | ((~counter & 0x40) >> 3);
	cpu_set_input_line_and_vector(state->maincpu, 0, HOLD_LINE, vector);

	/* set up for next interrupt */
	if (counter == MW8080BW_INT_TRIGGER_COUNT_1)
	{
		next_counter = MW8080BW_INT_TRIGGER_COUNT_2;
		next_vblank = MW8080BW_INT_TRIGGER_VBLANK_2;
	}
	else
	{
		next_counter = MW8080BW_INT_TRIGGER_COUNT_1;
		next_vblank = MW8080BW_INT_TRIGGER_VBLANK_1;
	}

	next_vpos = vysnc_chain_counter_to_vpos(next_counter, next_vblank);
	timer_adjust_oneshot(state->interrupt_timer, video_screen_get_time_until_pos(machine->primary_screen, next_vpos, 0), 0);
}


static void mw8080bw_create_interrupt_timer( running_machine *machine )
{
	mw8080bw_state *state = (mw8080bw_state *)machine->driver_data;
	state->interrupt_timer = timer_alloc(machine, mw8080bw_interrupt_callback, NULL);
}


static void mw8080bw_start_interrupt_timer( running_machine *machine )
{
	mw8080bw_state *state = (mw8080bw_state *)machine->driver_data;
	int vpos = vysnc_chain_counter_to_vpos(MW8080BW_INT_TRIGGER_COUNT_1, MW8080BW_INT_TRIGGER_VBLANK_1);
	timer_adjust_oneshot(state->interrupt_timer, video_screen_get_time_until_pos(machine->primary_screen, vpos, 0), 0);
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

MACHINE_START( mw8080bw )
{
	mw8080bw_state *state = (mw8080bw_state *)machine->driver_data;

	mw8080bw_create_interrupt_timer(machine);

	state->maincpu = devtag_get_device(machine, "maincpu");
	state->samples = devtag_get_device(machine, "samples");
	state->samples1 = devtag_get_device(machine, "samples1");
	state->samples2 = devtag_get_device(machine, "samples2");
	state->sn = devtag_get_device(machine, "snsnd");
	state->sn1 = devtag_get_device(machine, "sn1");
	state->sn2 = devtag_get_device(machine, "sn2");
	state->discrete = devtag_get_device(machine, "discrete");
	state->speaker = devtag_get_device(machine, "speaker");
	state->mb14241 = devtag_get_device(machine, "mb14241");
}


/*************************************
 *
 *  Machine reset
 *
 *************************************/

MACHINE_RESET( mw8080bw )
{
	mw8080bw_start_interrupt_timer(machine);
}
