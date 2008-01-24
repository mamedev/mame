/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/

#include "driver.h"
#include "mw8080bw.h"
#include "machine/mb14241.h"


/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static emu_timer *interrupt_timer;


static UINT8 vpos_to_vysnc_chain_counter(int vpos)
{
	/* convert from a vertical position to the actual values on the vertical sync counters */
	UINT8 counter;
	int vblank = (vpos >= MW8080BW_VBSTART);

	if (vblank)
	{
		counter = vpos - MW8080BW_VBSTART + MW8080BW_VCOUNTER_START_VBLANK;
	}
	else
	{
		counter = vpos + MW8080BW_VCOUNTER_START_NO_VBLANK;
	}


	return counter;
}


static int vysnc_chain_counter_to_vpos(UINT8 counter, int vblank)
{
	/* convert from the vertical sync counters to an actual vertical position */
	int vpos;

	if (vblank)
	{
		vpos = counter - MW8080BW_VCOUNTER_START_VBLANK + MW8080BW_VBSTART;
	}
	else
	{
		vpos = counter - MW8080BW_VCOUNTER_START_NO_VBLANK;
	}

	return vpos;
}


static TIMER_CALLBACK( mw8080bw_interrupt_callback )
{
	UINT8 next_counter;
	int next_vpos;
	int next_vblank;

	/* compute vector and set the interrupt line */
	int vpos = video_screen_get_vpos(0);
	UINT8 counter = vpos_to_vysnc_chain_counter(vpos);
	UINT8 vector = 0xc7 | ((counter & 0x40) >> 2) | ((~counter & 0x40) >> 3);
	cpunum_set_input_line_and_vector(machine, 0, 0, HOLD_LINE, vector);

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
	timer_adjust(interrupt_timer, video_screen_get_time_until_pos(0, next_vpos, 0), 0, attotime_zero);
}


static void mw8080bw_create_interrupt_timer(void)
{
	interrupt_timer = timer_alloc(mw8080bw_interrupt_callback, NULL);
}


static void mw8080bw_start_interrupt_timer(void)
{
	int vpos = vysnc_chain_counter_to_vpos(MW8080BW_INT_TRIGGER_COUNT_1, MW8080BW_INT_TRIGGER_VBLANK_1);
	timer_adjust(interrupt_timer, video_screen_get_time_until_pos(0, vpos, 0), 0, attotime_zero);
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

MACHINE_START( mw8080bw )
{
	mw8080bw_create_interrupt_timer();
	mb14241_init(0);
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

MACHINE_RESET( mw8080bw )
{
	mw8080bw_start_interrupt_timer();
}
