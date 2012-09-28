/***************************************************************************

    Atari Star Wars hardware

    This file is Copyright Steve Baines.
    Modified by Frank Palazzolo for sound support

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/tms5220.h"
#include "includes/starwars.h"





SOUND_START( starwars )
{
	starwars_state *state = machine.driver_data<starwars_state>();
	state->m_riot = machine.device("riot");
}


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
	UINT8 olddata = riot6532_porta_in_get(machine().device("riot"));

	return (olddata & 0xc0) | 0x10 | (tms5220_readyq_r(machine().device("tms")) << 2);
}


WRITE8_MEMBER(starwars_state::r6532_porta_w)
{
	device_t *device = machine().device("tms");
	/* handle 5220 read */
	tms5220_rsq_w(device, (data & 2)>>1);
	/* handle 5220 write */
	tms5220_wsq_w(device, (data & 1)>>0);
}


WRITE_LINE_MEMBER(starwars_state::snd_interrupt)
{
	machine().device("audiocpu")->execute().set_input_line(M6809_IRQ_LINE, state);
}


const riot6532_interface starwars_riot6532_intf =
{
	DEVCB_DRIVER_MEMBER(starwars_state,r6532_porta_r),
	DEVCB_DEVICE_HANDLER("tms", tms5220_status_r),
	DEVCB_DRIVER_MEMBER(starwars_state,r6532_porta_w),
	DEVCB_DEVICE_HANDLER("tms", tms5220_data_w),
	DEVCB_DRIVER_LINE_MEMBER(starwars_state,snd_interrupt)
};




/*************************************
 *
 *  Sound CPU to/from main CPU
 *
 *************************************/

static TIMER_CALLBACK( sound_callback )
{
	starwars_state *state = machine.driver_data<starwars_state>();
	riot6532_porta_in_set(state->m_riot, 0x40, 0x40);
	state->m_main_data = param;
	machine.scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}


READ8_MEMBER(starwars_state::starwars_sin_r)
{
	riot6532_porta_in_set(m_riot, 0x00, 0x80);
	return m_sound_data;
}


WRITE8_MEMBER(starwars_state::starwars_sout_w)
{
	machine().scheduler().synchronize(FUNC(sound_callback), data);
}



/*************************************
 *
 *  Main CPU to/from source CPU
 *
 *************************************/

READ8_MEMBER(starwars_state::starwars_main_read_r)
{
	riot6532_porta_in_set(m_riot, 0x00, 0x40);
	return m_main_data;
}


READ8_MEMBER(starwars_state::starwars_main_ready_flag_r)
{
	return riot6532_porta_in_get(m_riot) & 0xc0;	/* only upper two flag bits mapped */
}

static TIMER_CALLBACK( main_callback )
{
	starwars_state *state = machine.driver_data<starwars_state>();
	if (riot6532_porta_in_get(state->m_riot) & 0x80)
		logerror("Sound data not read %x\n",state->m_sound_data);

	riot6532_porta_in_set(state->m_riot, 0x80, 0x80);
	state->m_sound_data = param;
	machine.scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}

WRITE8_MEMBER(starwars_state::starwars_main_wr_w)
{
	machine().scheduler().synchronize(FUNC(main_callback), data);
}


WRITE8_MEMBER(starwars_state::starwars_soundrst_w)
{
	riot6532_porta_in_set(m_riot, 0x00, 0xc0);

	/* reset sound CPU here  */
	machine().device("audiocpu")->execute().set_input_line(INPUT_LINE_RESET, PULSE_LINE);
}
