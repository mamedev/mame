/*************************************************************************

    Atari Toobin' hardware

*************************************************************************/

#include "machine/atarigen.h"

class toobin_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, toobin_state(machine)); }

	toobin_state(running_machine &machine) { }

	atarigen_state	atarigen;

	UINT16 *		interrupt_scan;

	double			brightness;
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
