/*************************************************************************

    Atari Gauntlet hardware

*************************************************************************/

#include "machine/atarigen.h"

class gauntlet_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, gauntlet_state(machine)); }

	gauntlet_state(running_machine &machine) { }

	atarigen_state	atarigen;
	UINT16			sound_reset_val;
	UINT8			vindctr2_screen_refresh;
	UINT8			playfield_tile_bank;
	UINT8			playfield_color_bank;
};


/*----------- defined in video/gauntlet.c -----------*/

WRITE16_HANDLER( gauntlet_xscroll_w );
WRITE16_HANDLER( gauntlet_yscroll_w );

VIDEO_START( gauntlet );
VIDEO_UPDATE( gauntlet );
