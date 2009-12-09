/*************************************************************************

    Atari Toobin' hardware

*************************************************************************/

#include "machine/atarigen.h"

typedef struct _toobin_state toobin_state;
struct _toobin_state
{
	atarigen_state	atarigen;

	UINT16 *		interrupt_scan;

	double 			brightness;
	bitmap_t *		pfbitmap;
};


/*----------- defined in video/toobin.c -----------*/

WRITE16_HANDLER( toobin_paletteram_w );
WRITE16_HANDLER( toobin_intensity_w );
WRITE16_HANDLER( toobin_xscroll_w );
WRITE16_HANDLER( toobin_yscroll_w );
WRITE16_HANDLER( toobin_slip_w );

VIDEO_START( toobin );
VIDEO_UPDATE( toobin );
