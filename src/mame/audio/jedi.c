/***************************************************************************

    Atari Return of the Jedi hardware

    driver by Dan Boris

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/tms5220.h"
#include "sound/pokey.h"
#include "includes/jedi.h"



/*************************************
 *
 *  Start
 *
 *************************************/

static SOUND_START( jedi )
{
	jedi_state *state = machine.driver_data<jedi_state>();

	/* set up save state */
	state->save_item(NAME(state->m_audio_latch));
	state->save_item(NAME(state->m_audio_ack_latch));
	state->save_item(NAME(state->m_speech_strobe_state));
}



/*************************************
 *
 *  Reset
 *
 *************************************/

static SOUND_RESET( jedi )
{
	jedi_state *state = machine.driver_data<jedi_state>();

	/* init globals */
	state->m_audio_latch = 0;
	state->m_audio_ack_latch = 0;
	*state->m_audio_comm_stat = 0;
	*state->m_speech_data = 0;
	state->m_speech_strobe_state = 0;
}



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

static WRITE8_HANDLER( irq_ack_w )
{
	cputag_set_input_line(space->machine(), "audiocpu", M6502_IRQ_LINE, CLEAR_LINE);
}



/*************************************
 *
 *  Main CPU -> Sound CPU communications
 *
 *************************************/

WRITE8_HANDLER( jedi_audio_reset_w )
{
	cputag_set_input_line(space->machine(), "audiocpu", INPUT_LINE_RESET, (data & 1) ? CLEAR_LINE : ASSERT_LINE);
}


static TIMER_CALLBACK( delayed_audio_latch_w )
{
	jedi_state *state = machine.driver_data<jedi_state>();

	state->m_audio_latch = param;
	*state->m_audio_comm_stat |= 0x80;
}


WRITE8_HANDLER( jedi_audio_latch_w )
{
	space->machine().scheduler().synchronize(FUNC(delayed_audio_latch_w), data);
}


static READ8_HANDLER( audio_latch_r )
{
	jedi_state *state = space->machine().driver_data<jedi_state>();

	*state->m_audio_comm_stat &= ~0x80;
	return state->m_audio_latch;
}


CUSTOM_INPUT_MEMBER(jedi_state::jedi_audio_comm_stat_r)
{
	return *m_audio_comm_stat >> 6;
}



/*************************************
 *
 *  Sound CPU -> Main CPU communications
 *
 *************************************/

READ8_HANDLER( jedi_audio_ack_latch_r )
{
	jedi_state *state = space->machine().driver_data<jedi_state>();

	*state->m_audio_comm_stat &= ~0x40;
	return state->m_audio_ack_latch;
}


static WRITE8_HANDLER( audio_ack_latch_w )
{
	jedi_state *state = space->machine().driver_data<jedi_state>();

	state->m_audio_ack_latch = data;
	*state->m_audio_comm_stat |= 0x40;
}



/*************************************
 *
 *  Speech access
 *
 *************************************/

static WRITE8_HANDLER( speech_strobe_w )
{
	jedi_state *state = space->machine().driver_data<jedi_state>();
	int new_speech_strobe_state = (~offset >> 8) & 1;

	if ((new_speech_strobe_state != state->m_speech_strobe_state) && new_speech_strobe_state)
	{
		device_t *tms = space->machine().device("tms");
		tms5220_data_w(tms, 0, *state->m_speech_data);
	}
	state->m_speech_strobe_state = new_speech_strobe_state;
}


static READ8_HANDLER( speech_ready_r )
{
	return (tms5220_readyq_r(space->machine().device("tms"))) << 7;
}


static WRITE8_HANDLER( speech_reset_w )
{
	/* not supported by the TMS5220 emulator */
}



/*************************************
 *
 *  Audio CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( audio_map, AS_PROGRAM, 8, jedi_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x080f) AM_MIRROR(0x07c0) AM_DEVREADWRITE_LEGACY("pokey1", pokey_r, pokey_w)
	AM_RANGE(0x0810, 0x081f) AM_MIRROR(0x07c0) AM_DEVREADWRITE_LEGACY("pokey2", pokey_r, pokey_w)
	AM_RANGE(0x0820, 0x082f) AM_MIRROR(0x07c0) AM_DEVREADWRITE_LEGACY("pokey3", pokey_r, pokey_w)
	AM_RANGE(0x0830, 0x083f) AM_MIRROR(0x07c0) AM_DEVREADWRITE_LEGACY("pokey4", pokey_r, pokey_w)
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x00ff) AM_READNOP AM_WRITE_LEGACY(irq_ack_w)
	AM_RANGE(0x1100, 0x1100) AM_MIRROR(0x00ff) AM_READNOP AM_WRITEONLY AM_BASE(m_speech_data)
	AM_RANGE(0x1200, 0x13ff) AM_READNOP AM_WRITE_LEGACY(speech_strobe_w)
	AM_RANGE(0x1400, 0x1400) AM_MIRROR(0x00ff) AM_READNOP AM_WRITE_LEGACY(audio_ack_latch_w)
	AM_RANGE(0x1500, 0x1500) AM_MIRROR(0x00ff) AM_READNOP AM_WRITE_LEGACY(speech_reset_w)
	AM_RANGE(0x1600, 0x17ff) AM_NOP
	AM_RANGE(0x1800, 0x1800) AM_MIRROR(0x03ff) AM_READ_LEGACY(audio_latch_r) AM_WRITENOP
	AM_RANGE(0x1c00, 0x1c00) AM_MIRROR(0x03fe) AM_READ_LEGACY(speech_ready_r) AM_WRITENOP
	AM_RANGE(0x1c01, 0x1c01) AM_MIRROR(0x03fe) AM_READONLY AM_WRITENOP AM_BASE(m_audio_comm_stat)
	AM_RANGE(0x2000, 0x7fff) AM_NOP
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( jedi_audio )

	MCFG_CPU_ADD("audiocpu", M6502, JEDI_AUDIO_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(audio_map)

	MCFG_SOUND_START(jedi)
	MCFG_SOUND_RESET(jedi)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("pokey1", POKEY, JEDI_POKEY_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)

	MCFG_SOUND_ADD("pokey2", POKEY, JEDI_POKEY_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)

	MCFG_SOUND_ADD("pokey3", POKEY, JEDI_POKEY_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)

	MCFG_SOUND_ADD("pokey4", POKEY, JEDI_POKEY_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)

	MCFG_SOUND_ADD("tms", TMS5220, JEDI_TMS5220_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END
