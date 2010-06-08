/*************************************************************************

    Atari Vindicators hardware

*************************************************************************/

#include "machine/atarigen.h"

class vindictr_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, vindictr_state(machine)); }

	vindictr_state(running_machine &machine) { }

	atarigen_state	atarigen;

	UINT8			playfield_tile_bank;
	UINT16			playfield_xscroll;
	UINT16			playfield_yscroll;
};


/*----------- defined in video/vindictr.c -----------*/

WRITE16_HANDLER( vindictr_paletteram_w );

VIDEO_START( vindictr );
VIDEO_UPDATE( vindictr );

void vindictr_scanline_update(screen_device &screen, int scanline);
