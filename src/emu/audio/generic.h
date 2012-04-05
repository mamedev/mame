/***************************************************************************

    generic.h

    Generic simple sound functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __SOUND_GENERIC_H__
#define __SOUND_GENERIC_H__



/***************************************************************************

    Function prototypes

***************************************************************************/

int generic_sound_init(running_machine &machine);

/* If you're going to use soundlatchX_clear_w, and the cleared value is
   something other than 0x00, use this function from machine_init. Note
   that this one call effects all 4 latches */
void soundlatch_setclearedvalue(running_machine &machine, int value);


#endif	/* __SOUND_GENERIC_H__ */
