/*************************************************************************

    Atari Escape hardware

*************************************************************************/

#include "machine/atarigen.h"

class eprom_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, eprom_state(machine)); }

	eprom_state(running_machine &machine) { }

	atarigen_state	atarigen;
	int 			screen_intensity;
	int 			video_disable;
	UINT16 *		sync_data;
};


/*----------- defined in video/eprom.c -----------*/

VIDEO_START( eprom );
VIDEO_UPDATE( eprom );

VIDEO_START( guts );
VIDEO_UPDATE( guts );

void eprom_scanline_update(running_device *screen, int scanline);
