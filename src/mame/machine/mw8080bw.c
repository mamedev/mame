/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/

#include "emu.h"
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
	mw8080bw_state *state = machine.driver_data<mw8080bw_state>();
	UINT8 next_counter;
	int next_vpos;
	int next_vblank;

	/* compute vector and set the interrupt line */
	int vpos = machine.primary_screen->vpos();
	UINT8 counter = vpos_to_vysnc_chain_counter(vpos);
	UINT8 vector = 0xc7 | ((counter & 0x40) >> 2) | ((~counter & 0x40) >> 3);
	device_set_input_line_and_vector(state->m_maincpu, 0, HOLD_LINE, vector);

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
	state->m_interrupt_timer->adjust(machine.primary_screen->time_until_pos(next_vpos));
}


static void mw8080bw_create_interrupt_timer( running_machine &machine )
{
	mw8080bw_state *state = machine.driver_data<mw8080bw_state>();
	state->m_interrupt_timer = machine.scheduler().timer_alloc(FUNC(mw8080bw_interrupt_callback));
}


static void mw8080bw_start_interrupt_timer( running_machine &machine )
{
	mw8080bw_state *state = machine.driver_data<mw8080bw_state>();
	int vpos = vysnc_chain_counter_to_vpos(MW8080BW_INT_TRIGGER_COUNT_1, MW8080BW_INT_TRIGGER_VBLANK_1);
	state->m_interrupt_timer->adjust(machine.primary_screen->time_until_pos(vpos));
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

MACHINE_START( mw8080bw )
{
	mw8080bw_state *state = machine.driver_data<mw8080bw_state>();

	mw8080bw_create_interrupt_timer(machine);

	state->m_maincpu = machine.device("maincpu");
	state->m_samples = machine.device<samples_device>("samples");
	state->m_samples1 = machine.device<samples_device>("samples1");
	state->m_samples2 = machine.device<samples_device>("samples2");
	state->m_sn = machine.device("snsnd");
	state->m_sn1 = machine.device("sn1");
	state->m_sn2 = machine.device("sn2");
	state->m_discrete = machine.device("discrete");
	state->m_mb14241 = machine.device("mb14241");
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
