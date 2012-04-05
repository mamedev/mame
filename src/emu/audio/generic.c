/***************************************************************************

    generic.c

    Generic simple sound functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct _generic_audio_private
{
	UINT16		latch_clear_value;
	UINT16		latched_value[4];
	UINT8		latch_read[4];
};



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    generic_sound_init - initialize globals and
    register for save states
-------------------------------------------------*/

int generic_sound_init(running_machine &machine)
{
	generic_audio_private *state;

	state = machine.generic_audio_data = auto_alloc_clear(machine, generic_audio_private);

	/* register globals with the save state system */
	state_save_register_global_array(machine, state->latched_value);
	state_save_register_global_array(machine, state->latch_read);

	return 0;
}



/***************************************************************************

    Many games use a master-slave CPU setup. Typically, the main CPU writes
    a command to some register, and then writes to another register to trigger
    an interrupt on the slave CPU (the interrupt might also be triggered by
    the first write). The slave CPU, notified by the interrupt, goes and reads
    the command.

***************************************************************************/

/*-------------------------------------------------
    latch_callback - time-delayed callback to
    set a latch value
-------------------------------------------------*/

static TIMER_CALLBACK( latch_callback )
{
	generic_audio_private *state = machine.generic_audio_data;
	UINT16 value = param >> 8;
	int which = param & 0xff;

	/* if the latch hasn't been read and the value is changed, log a warning */
	if (!state->latch_read[which] && state->latched_value[which] != value)
		logerror("Warning: sound latch %d written before being read. Previous: %02x, new: %02x\n", which, state->latched_value[which], value);

	/* store the new value and mark it not read */
	state->latched_value[which] = value;
	state->latch_read[which] = 0;
}


/*-------------------------------------------------
    latch_w - handle a write to a given latch
-------------------------------------------------*/

INLINE void latch_w(address_space *space, int which, UINT16 value)
{
	space->machine().scheduler().synchronize(FUNC(latch_callback), which | (value << 8));
}


/*-------------------------------------------------
    latch_r - handle a read from a given latch
-------------------------------------------------*/

INLINE UINT16 latch_r(address_space *space, int which)
{
	generic_audio_private *state = space->machine().generic_audio_data;
	state->latch_read[which] = 1;
	return state->latched_value[which];
}


/*-------------------------------------------------
    latch_clear - clear a given latch
-------------------------------------------------*/

INLINE void latch_clear(address_space *space, int which)
{
	generic_audio_private *state = space->machine().generic_audio_data;
	state->latched_value[which] = state->latch_clear_value;
}


/*-------------------------------------------------
    soundlatch_w - global write handlers for
    writing to sound latches
-------------------------------------------------*/

WRITE8_MEMBER( driver_device::soundlatch_w )		{ latch_w(&space, 0, data); }
WRITE16_MEMBER( driver_device::soundlatch_word_w )  { latch_w(&space, 0, data); }
WRITE8_MEMBER( driver_device::soundlatch2_w )		{ latch_w(&space, 1, data); }
WRITE16_MEMBER( driver_device::soundlatch2_word_w ) { latch_w(&space, 1, data); }
WRITE8_MEMBER( driver_device::soundlatch3_w )		{ latch_w(&space, 2, data); }
WRITE16_MEMBER( driver_device::soundlatch3_word_w ) { latch_w(&space, 2, data); }
WRITE8_MEMBER( driver_device::soundlatch4_w )		{ latch_w(&space, 3, data); }
WRITE16_MEMBER( driver_device::soundlatch4_word_w ) { latch_w(&space, 3, data); }


/*-------------------------------------------------
    soundlatch_r - global read handlers for
    reading from sound latches
-------------------------------------------------*/

READ8_MEMBER( driver_device::soundlatch_r ) 		{ return latch_r(&space, 0); }
READ16_MEMBER( driver_device::soundlatch_word_r )   { return latch_r(&space, 0); }
READ8_MEMBER( driver_device::soundlatch2_r )		{ return latch_r(&space, 1); }
READ16_MEMBER( driver_device::soundlatch2_word_r )  { return latch_r(&space, 1); }
READ8_MEMBER( driver_device::soundlatch3_r )		{ return latch_r(&space, 2); }
READ16_MEMBER( driver_device::soundlatch3_word_r )  { return latch_r(&space, 2); }
READ8_MEMBER( driver_device::soundlatch4_r )		{ return latch_r(&space, 3); }
READ16_MEMBER( driver_device::soundlatch4_word_r )  { return latch_r(&space, 3); }


/*-------------------------------------------------
    soundlatch_clear_w - global write handlers
    for clearing sound latches
-------------------------------------------------*/

WRITE8_MEMBER( driver_device::soundlatch_clear_w )  { latch_clear(&space, 0); }
WRITE8_MEMBER( driver_device::soundlatch2_clear_w ) { latch_clear(&space, 1); }
WRITE8_MEMBER( driver_device::soundlatch3_clear_w ) { latch_clear(&space, 2); }
WRITE8_MEMBER( driver_device::soundlatch4_clear_w ) { latch_clear(&space, 3); }


/*-------------------------------------------------
    soundlatch_setclearedvalue - set the "clear"
    value for all sound latches
-------------------------------------------------*/

void soundlatch_setclearedvalue(running_machine &machine, int value)
{
	generic_audio_private *state = machine.generic_audio_data;
	assert_always(machine.phase() == MACHINE_PHASE_INIT, "Can only call soundlatch_setclearedvalue at init time!");
	state->latch_clear_value = value;
}
