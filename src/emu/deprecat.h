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

#include "mamecore.h"
#include "devintrf.h"


/*************************************
 *
 *  Old way of allowing "VBLANK"
 *  interrupts to fire more than once
 *  a frame.
 *
 *  These should be replaced with
 *  scanline based interrupts as
 *  it makes no sense to have more
 *  than one VBLANK interrupt
 *  per frame.
 *
 *************************************/

#define MDRV_CPU_VBLANK_INT_HACK(_func, _rate) \
	MDRV_DEVICE_CONFIG_DATAPTR(cpu_config, vblank_interrupt, _func) \
	MDRV_DEVICE_CONFIG_DATAPTR(cpu_config, vblank_interrupt_screen, NULL) \
	MDRV_DEVICE_CONFIG_DATA32(cpu_config, vblank_interrupts_per_frame, _rate)



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
int cpu_getiloops(const device_config *device);



/*************************************
 *
 *  Graphics decoding. Drivers should
 *  use the new gfx_element_mark_dirty()
 *  to explicitly mark tiles dirty, and
 *  gfx_element_get_data() to fetch a
 *  pointer to the data (and undirty
 *  the tile)
 *
 *************************************/

void decodechar(const gfx_element *gfx, UINT32 code, const UINT8 *src);



#endif	/* __DEPRECAT_H__ */
