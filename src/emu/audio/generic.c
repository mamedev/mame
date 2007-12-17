/***************************************************************************

    generic.c

    Generic simple sound functions.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "driver.h"
#include "generic.h"



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static UINT16 latch_clear_value = 0x00;
static UINT16 latched_value[4];
static UINT8 latch_read[4];



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    generic_sound_init - initialize globals and
    register for save states
-------------------------------------------------*/

int generic_sound_init(void)
{
	/* reset latches */
	latch_clear_value = 0;
	memset(latched_value, 0, sizeof(latched_value));
	memset(latch_read, 0, sizeof(latch_read));

	/* register globals with the save state system */
	state_save_register_global_array(latched_value);
	state_save_register_global_array(latch_read);

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
	UINT16 value = param >> 8;
	int which = param & 0xff;

	/* if the latch hasn't been read and the value is changed, log a warning */
	if (!latch_read[which] && latched_value[which] != value)
		logerror("Warning: sound latch %d written before being read. Previous: %02x, new: %02x\n", which, latched_value[which], value);

	/* store the new value and mark it not read */
	latched_value[which] = value;
	latch_read[which] = 0;
}


/*-------------------------------------------------
    latch_w - handle a write to a given latch
-------------------------------------------------*/

INLINE void latch_w(int which, UINT16 value)
{
	timer_call_after_resynch(which | (value << 8), latch_callback);
}


/*-------------------------------------------------
    latch_r - handle a read from a given latch
-------------------------------------------------*/

INLINE UINT16 latch_r(int which)
{
	latch_read[which] = 1;
	return latched_value[which];
}


/*-------------------------------------------------
    latch_clear - clear a given latch
-------------------------------------------------*/

INLINE void latch_clear(int which)
{
	latched_value[which] = latch_clear_value;
}


/*-------------------------------------------------
    soundlatch_w - global write handlers for
    writing to sound latches
-------------------------------------------------*/

WRITE8_HANDLER( soundlatch_w )        { latch_w(0, data); }
WRITE16_HANDLER( soundlatch_word_w )  { latch_w(0, data); }
WRITE8_HANDLER( soundlatch2_w )       { latch_w(1, data); }
WRITE16_HANDLER( soundlatch2_word_w ) { latch_w(1, data); }
WRITE8_HANDLER( soundlatch3_w )       { latch_w(2, data); }
WRITE16_HANDLER( soundlatch3_word_w ) { latch_w(2, data); }
WRITE8_HANDLER( soundlatch4_w )       { latch_w(3, data); }
WRITE16_HANDLER( soundlatch4_word_w ) { latch_w(3, data); }


/*-------------------------------------------------
    soundlatch_r - global read handlers for
    reading from sound latches
-------------------------------------------------*/

READ8_HANDLER( soundlatch_r )         { return latch_r(0); }
READ16_HANDLER( soundlatch_word_r )   { return latch_r(0); }
READ8_HANDLER( soundlatch2_r )        { return latch_r(1); }
READ16_HANDLER( soundlatch2_word_r )  { return latch_r(1); }
READ8_HANDLER( soundlatch3_r )        { return latch_r(2); }
READ16_HANDLER( soundlatch3_word_r )  { return latch_r(2); }
READ8_HANDLER( soundlatch4_r )        { return latch_r(3); }
READ16_HANDLER( soundlatch4_word_r )  { return latch_r(3); }


/*-------------------------------------------------
    soundlatch_clear_w - global write handlers
    for clearing sound latches
-------------------------------------------------*/

WRITE8_HANDLER( soundlatch_clear_w )  { latch_clear(0); }
WRITE8_HANDLER( soundlatch2_clear_w ) { latch_clear(1); }
WRITE8_HANDLER( soundlatch3_clear_w ) { latch_clear(2); }
WRITE8_HANDLER( soundlatch4_clear_w ) { latch_clear(3); }


/*-------------------------------------------------
    soundlatch_setclearedvalue - set the "clear"
    value for all sound latches
-------------------------------------------------*/

void soundlatch_setclearedvalue(int value)
{
	latch_clear_value = value;
}
