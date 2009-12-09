/*************************************************************************

    Atari Batman hardware

*************************************************************************/

#include "machine/atarigen.h"

typedef struct _batman_state batman_state;
struct _batman_state
{
	atarigen_state	atarigen;

	UINT16 			latch_data;

	UINT8 			alpha_tile_bank;
};


/*----------- defined in video/batman.c -----------*/

VIDEO_START( batman );
VIDEO_UPDATE( batman );

void batman_scanline_update(const device_config *screen, int scanline);
