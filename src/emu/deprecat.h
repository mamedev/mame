/***************************************************************************

    deprecat.h

    Definition of derprecated and obsolte constructs that should not
    be used by new code, if at all possible.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __DEPRECAT_H__
#define __DEPRECAT_H__



/*************************************
 *
 *  Global access to the currently
 *  executing machine.
 *
 *  Please investigate if it is
 *  possible to use a passed in
 *  'machine' argument.
 *
 *************************************/

extern running_machine *Machine;



/*************************************
 *
 *  Video timing
 *
 *  These functions are no longer considered
 *  to be the authority on scanline timing.
 *  Please use the video_screen_* functions
 *  in video.c for newer drivers.
 *
 *************************************/

/* Recomputes the VBLANK timing after, e.g., a visible area change */
void cpu_compute_vblank_timing(running_machine *machine);

/* Returns the number of the video frame we are currently playing */
int cpu_getcurrentframe(void);

#endif	/* __DEPRECAT_H__ */
