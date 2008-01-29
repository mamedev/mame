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

/* Returns the number of the video frame we are currently playing */
int cpu_getcurrentframe(void);



/*************************************
 *
 *  Core timing
 *
 *************************************/

/* Returns the number of times the interrupt handler will be called before
   the end of the current video frame. This is can be useful to interrupt
   handlers to synchronize their operation. If you call this from outside
   an interrupt handler, add 1 to the result, i.e. if it returns 0, it means
   that the interrupt handler will be called once. */
int cpu_getiloops(void);

/* Scales a given value by the ratio of fcount / fperiod */
int cpu_scalebyfcount(int value);


#endif	/* __DEPRECAT_H__ */
