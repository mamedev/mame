/*************************************************************************

    Atari Escape hardware

*************************************************************************/

#include "machine/atarigen.h"

typedef struct _eprom_state eprom_state;
struct _eprom_state
{
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
