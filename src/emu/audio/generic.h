/***************************************************************************

    generic.h

    Generic simple sound functions.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __SOUND_GENERIC_H__
#define __SOUND_GENERIC_H__

#include "memory.h"



/***************************************************************************

    Function prototypes

***************************************************************************/

int generic_sound_init(void);

/* latch readers */
READ8_HANDLER( soundlatch_r );
READ8_HANDLER( soundlatch2_r );
READ8_HANDLER( soundlatch3_r );
READ8_HANDLER( soundlatch4_r );
READ16_HANDLER( soundlatch_word_r );
READ16_HANDLER( soundlatch2_word_r );
READ16_HANDLER( soundlatch3_word_r );
READ16_HANDLER( soundlatch4_word_r );

/* latch writers */
WRITE8_HANDLER( soundlatch_w );
WRITE8_HANDLER( soundlatch2_w );
WRITE8_HANDLER( soundlatch3_w );
WRITE8_HANDLER( soundlatch4_w );
WRITE16_HANDLER( soundlatch_word_w );
WRITE16_HANDLER( soundlatch2_word_w );
WRITE16_HANDLER( soundlatch3_word_w );
WRITE16_HANDLER( soundlatch4_word_w );

/* latch clearers */
WRITE8_HANDLER( soundlatch_clear_w );
WRITE8_HANDLER( soundlatch2_clear_w );
WRITE8_HANDLER( soundlatch3_clear_w );
WRITE8_HANDLER( soundlatch4_clear_w );

/* If you're going to use soundlatchX_clear_w, and the cleared value is
   something other than 0x00, use this function from machine_init. Note
   that this one call effects all 4 latches */
void soundlatch_setclearedvalue(int value);


#endif	/* __SOUND_GENERIC_H__ */
