// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
/***************************************************************************

    Atari Star Wars hardware

    This file is Copyright Steve Baines.
    Modified by Frank Palazzolo for sound support

***************************************************************************/

#include "emu.h"
#include "includes/starwars.h"


/*************************************
 *
 *  RIOT interfaces
 *
 *************************************/

READ8_MEMBER(starwars_state::r6532_porta_r)
{
	/* Configured as follows:           */
	/* d7 (in)  Main Ready Flag         */
	/* d6 (in)  Sound Ready Flag        */
	/* d5 (out) Mute Speech             */
	/* d4 (in)  Not Sound Self Test     */
	/* d3 (out) Hold Main CPU in Reset? */
	/*          + enable delay circuit? */
	/* d2 (in)  TMS5220 Not Ready       */
	/* d1 (out) TMS5220 Not Read        */
	/* d0 (out) TMS5220 Not Write       */
	/* Note: bit 4 is always set to avoid sound self test */
	uint8_t olddata = m_riot->porta_in_get();

	return (olddata & 0xc0) | 0x10 | (m_tms->readyq_r() << 2);
}


WRITE8_MEMBER(starwars_state::r6532_porta_w)
{
	/* handle 5220 read */
	m_tms->rsq_w((data & 2)>>1);
	/* handle 5220 write */
	m_tms->wsq_w((data & 1)>>0);
}


/*************************************
 *
 *  Sound CPU to/from main CPU
 *
 *************************************/

WRITE_LINE_MEMBER(starwars_state::boost_interleave_hack)
{
	if (state)
		machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}


READ8_MEMBER(starwars_state::starwars_main_ready_flag_r)
{
	return m_riot->porta_in_get() & 0xc0;    /* only upper two flag bits mapped */
}


WRITE8_MEMBER(starwars_state::starwars_soundrst_w)
{
	m_soundlatch->acknowledge_w();
	m_mainlatch->acknowledge_w();

	/* reset sound CPU here  */
	m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}
