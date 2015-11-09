// license:???
// copyright-holders:Steve Baines, Frank Palazzolo
/***************************************************************************

    Atari Star Wars hardware

    This file is Copyright Steve Baines.
    Modified by Frank Palazzolo for sound support

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/tms5220.h"
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
	UINT8 olddata = m_riot->porta_in_get();

	tms5220_device *tms5220 = machine().device<tms5220_device>("tms");
	return (olddata & 0xc0) | 0x10 | (tms5220->readyq_r() << 2);
}


WRITE8_MEMBER(starwars_state::r6532_porta_w)
{
	tms5220_device *tms5220 = machine().device<tms5220_device>("tms");
	/* handle 5220 read */
	tms5220->rsq_w((data & 2)>>1);
	/* handle 5220 write */
	tms5220->wsq_w((data & 1)>>0);
}


WRITE_LINE_MEMBER(starwars_state::snd_interrupt)
{
	m_audiocpu->set_input_line(M6809_IRQ_LINE, state);
}


/*************************************
 *
 *  Sound CPU to/from main CPU
 *
 *************************************/

TIMER_CALLBACK_MEMBER(starwars_state::sound_callback)
{
	m_riot->porta_in_set(0x40, 0x40);
	m_main_data = param;
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}


READ8_MEMBER(starwars_state::starwars_sin_r)
{
	m_riot->porta_in_set(0x00, 0x80);
	return m_sound_data;
}


WRITE8_MEMBER(starwars_state::starwars_sout_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(starwars_state::sound_callback), this), data);
}



/*************************************
 *
 *  Main CPU to/from source CPU
 *
 *************************************/

READ8_MEMBER(starwars_state::starwars_main_read_r)
{
	m_riot->porta_in_set(0x00, 0x40);
	return m_main_data;
}


READ8_MEMBER(starwars_state::starwars_main_ready_flag_r)
{
	return m_riot->porta_in_get() & 0xc0;    /* only upper two flag bits mapped */
}

TIMER_CALLBACK_MEMBER(starwars_state::main_callback )
{
	if (m_riot->porta_in_get() & 0x80)
		logerror("Sound data not read %x\n", m_sound_data);

	m_riot->porta_in_set(0x80, 0x80);
	m_sound_data = param;
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}

WRITE8_MEMBER(starwars_state::starwars_main_wr_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(starwars_state::main_callback), this), data);
}


WRITE8_MEMBER(starwars_state::starwars_soundrst_w)
{
	m_riot->porta_in_set(0x00, 0xc0);

	/* reset sound CPU here  */
	m_audiocpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
}
