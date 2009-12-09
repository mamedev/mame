/*************************************************************************

    Atari Vindicators hardware

*************************************************************************/

#include "machine/atarigen.h"

typedef struct _vindictr_state vindictr_state;
struct _vindictr_state
{
	atarigen_state	atarigen;

	UINT8 			playfield_tile_bank;
	UINT16 			playfield_xscroll;
	UINT16 			playfield_yscroll;
};


/*----------- defined in video/vindictr.c -----------*/

WRITE16_HANDLER( vindictr_paletteram_w );

VIDEO_START( vindictr );
VIDEO_UPDATE( vindictr );

void vindictr_scanline_update(const device_config *screen, int scanline);
