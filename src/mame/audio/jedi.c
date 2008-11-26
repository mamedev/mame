/***************************************************************************

    Atari Return of the Jedi hardware

    driver by Dan Boris

***************************************************************************/

#include "driver.h"
#include "cpu/m6502/m6502.h"
#include "sound/5220intf.h"
#include "sound/pokey.h"
#include "jedi.h"



/*************************************
 *
 *  Start
 *
 *************************************/

static SOUND_START( jedi )
{
	jedi_state *state = machine->driver_data;

	/* set up save state */
	state_save_register_global(state->audio_latch);
	state_save_register_global(state->audio_ack_latch);
	state_save_register_global(state->speech_strobe_state);
}



/*************************************
 *
 *  Reset
 *
 *************************************/

static SOUND_RESET( jedi )
{
	jedi_state *state = machine->driver_data;

	/* init globals */
	state->audio_latch = 0;
	state->audio_ack_latch = 0;
	*state->audio_comm_stat = 0;
	*state->speech_data = 0;
	state->speech_strobe_state = 0;
}



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

static WRITE8_HANDLER( irq_ack_w )
{
	cpu_set_input_line(space->machine->cpu[1], M6502_IRQ_LINE, CLEAR_LINE);
}



/*************************************
 *
 *  Main CPU -> Sound CPU communications
 *
 *************************************/

WRITE8_HANDLER( jedi_audio_reset_w )
{
	cpu_set_input_line(space->machine->cpu[1], INPUT_LINE_RESET, (data & 1) ? CLEAR_LINE : ASSERT_LINE);
}


static TIMER_CALLBACK( delayed_audio_latch_w )
{
	jedi_state *state = machine->driver_data;

	state->audio_latch = param;
	*state->audio_comm_stat |= 0x80;
}


WRITE8_HANDLER( jedi_audio_latch_w )
{
	timer_call_after_resynch(space->machine, NULL, data, delayed_audio_latch_w);
}


static READ8_HANDLER( audio_latch_r )
{
	jedi_state *state = space->machine->driver_data;

	*state->audio_comm_stat &= ~0x80;
	return state->audio_latch;
}


CUSTOM_INPUT( jedi_audio_comm_stat_r )
{
	jedi_state *state = field->port->machine->driver_data;
	return *state->audio_comm_stat >> 6;
}



/*************************************
 *
 *  Sound CPU -> Main CPU communications
 *
 *************************************/

READ8_HANDLER( jedi_audio_ack_latch_r )
{
	jedi_state *state = space->machine->driver_data;

	*state->audio_comm_stat &= ~0x40;
	return state->audio_ack_latch;
}


static WRITE8_HANDLER( audio_ack_latch_w )
{
	jedi_state *state = space->machine->driver_data;

	state->audio_ack_latch = data;
	*state->audio_comm_stat |= 0x40;
}



/*************************************
 *
 *  Speech access
 *
 *************************************/

static WRITE8_HANDLER( speech_strobe_w )
{
	jedi_state *state = space->machine->driver_data;
	int new_speech_strobe_state = (~offset >> 8) & 1;

	if ((new_speech_strobe_state != state->speech_strobe_state) && new_speech_strobe_state)
		tms5220_data_w(space, 0, *state->speech_data);
	state->speech_strobe_state = new_speech_strobe_state;
}


static READ8_HANDLER( speech_ready_r )
{
	return (!tms5220_ready_r()) << 7;
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

static ADDRESS_MAP_START( audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x080f) AM_MIRROR(0x07c0) AM_READWRITE(pokey1_r, pokey1_w)
	AM_RANGE(0x0810, 0x081f) AM_MIRROR(0x07c0) AM_READWRITE(pokey2_r, pokey2_w)
	AM_RANGE(0x0820, 0x082f) AM_MIRROR(0x07c0) AM_READWRITE(pokey3_r, pokey3_w)
	AM_RANGE(0x0830, 0x083f) AM_MIRROR(0x07c0) AM_READWRITE(pokey4_r, pokey4_w)
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x00ff) AM_READWRITE(SMH_NOP, irq_ack_w)
	AM_RANGE(0x1100, 0x1100) AM_MIRROR(0x00ff) AM_READWRITE(SMH_NOP, SMH_RAM) AM_BASE_MEMBER(jedi_state, speech_data)
	AM_RANGE(0x1200, 0x13ff) AM_READWRITE(SMH_NOP, speech_strobe_w)
	AM_RANGE(0x1400, 0x1400) AM_MIRROR(0x00ff) AM_READWRITE(SMH_NOP, audio_ack_latch_w)
	AM_RANGE(0x1500, 0x1500) AM_MIRROR(0x00ff) AM_READWRITE(SMH_NOP, speech_reset_w)
	AM_RANGE(0x1600, 0x17ff) AM_NOP
	AM_RANGE(0x1800, 0x1800) AM_MIRROR(0x03ff) AM_READWRITE(audio_latch_r, SMH_NOP)
	AM_RANGE(0x1c00, 0x1c00) AM_MIRROR(0x03fe) AM_READWRITE(speech_ready_r, SMH_NOP)
	AM_RANGE(0x1c01, 0x1c01) AM_MIRROR(0x03fe) AM_RAM_WRITE(SMH_NOP) AM_BASE_MEMBER(jedi_state, audio_comm_stat)
	AM_RANGE(0x2000, 0x7fff) AM_NOP
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_DRIVER_START( jedi_audio )

	MDRV_CPU_ADD("audio", M6502, JEDI_AUDIO_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(audio_map,0)

	MDRV_SOUND_START(jedi)
	MDRV_SOUND_RESET(jedi)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD("pokey1", POKEY, JEDI_POKEY_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.30)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.30)

	MDRV_SOUND_ADD("pokey2", POKEY, JEDI_POKEY_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.30)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.30)

	MDRV_SOUND_ADD("pokey3", POKEY, JEDI_POKEY_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.30)

	MDRV_SOUND_ADD("pokey4", POKEY, JEDI_POKEY_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.30)

	MDRV_SOUND_ADD("tms", TMS5220, JEDI_TMS5220_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)
MACHINE_DRIVER_END
