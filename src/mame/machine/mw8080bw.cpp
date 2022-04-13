// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Tormod Tjaberg, Mirko Buffoni,Lee Taylor, Valerio Verrando, Zsolt Vasvari
// thanks-to:Michael Strutts, Marco Cassili
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

uint8_t mw8080bw_state::vpos_to_vysnc_chain_counter( int vpos )
{
	/* convert from a vertical position to the actual values on the vertical sync counters */
	uint8_t counter;
	int vblank = (vpos >= MW8080BW_VBSTART);

	if (vblank)
		counter = vpos - MW8080BW_VBSTART + MW8080BW_VCOUNTER_START_VBLANK;
	else
		counter = vpos + MW8080BW_VCOUNTER_START_NO_VBLANK;

	return counter;
}


int mw8080bw_state::vysnc_chain_counter_to_vpos( uint8_t counter, int vblank )
{
	/* convert from the vertical sync counters to an actual vertical position */
	int vpos;

	if (vblank)
		vpos = counter - MW8080BW_VCOUNTER_START_VBLANK + MW8080BW_VBSTART;
	else
		vpos = counter - MW8080BW_VCOUNTER_START_NO_VBLANK;

	return vpos;
}


TIMER_CALLBACK_MEMBER(mw8080bw_state::interrupt_trigger)
{
	int vpos = m_screen->vpos();
	uint8_t counter = vpos_to_vysnc_chain_counter(vpos);

	if (m_int_enable)
	{
		m_maincpu->set_input_line(0, ASSERT_LINE);
		m_interrupt_time = machine().time();
	}
	else
		m_maincpu->set_input_line(0, CLEAR_LINE);

	/* set up for next interrupt */
	uint8_t next_counter;
	int next_vblank;
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

	int next_vpos = vysnc_chain_counter_to_vpos(next_counter, next_vblank);
	m_interrupt_timer->adjust(m_screen->time_until_pos(next_vpos));
}


WRITE_LINE_MEMBER(mw8080bw_state::int_enable_w)
{
	m_int_enable = state;
}


IRQ_CALLBACK_MEMBER(mw8080bw_state::interrupt_vector)
{
	int vpos = m_screen->vpos();
	// MAME scheduling quirks cause this to happen more often than you might think, in fact far too often
	if (machine().time() < m_interrupt_time)
		vpos++;
	uint8_t counter = vpos_to_vysnc_chain_counter(vpos);
	uint8_t vector = 0xc7 | ((counter & 0x40) >> 2) | ((~counter & 0x40) >> 3);

	m_maincpu->set_input_line(0, CLEAR_LINE);
	return vector;
}


void mw8080bw_state::mw8080bw_create_interrupt_timer(  )
{
	m_interrupt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mw8080bw_state::interrupt_trigger),this));
}


void mw8080bw_state::mw8080bw_start_interrupt_timer(  )
{
	int vpos = vysnc_chain_counter_to_vpos(MW8080BW_INT_TRIGGER_COUNT_1, MW8080BW_INT_TRIGGER_VBLANK_1);
	m_interrupt_timer->adjust(m_screen->time_until_pos(vpos));

	m_interrupt_time = attotime::zero;
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

void mw8080bw_state::machine_start()
{
	mw8080bw_create_interrupt_timer();
}


/*************************************
 *
 *  Machine reset
 *
 *************************************/

MACHINE_RESET_MEMBER(mw8080bw_state,mw8080bw)
{
	mw8080bw_start_interrupt_timer();
}
