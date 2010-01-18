/*************************************************************************

    Atari Blasteroids hardware

*************************************************************************/

#include "machine/atarigen.h"

typedef struct _blstroid_state blstroid_state;
struct _blstroid_state
{
	atarigen_state	atarigen;

	UINT16 *		priorityram;
};


/*----------- defined in video/blstroid.c -----------*/

VIDEO_START( blstroid );
VIDEO_UPDATE( blstroid );

void blstroid_scanline_update(running_device *screen, int scanline);
