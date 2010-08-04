/*************************************************************************

    Atari Blasteroids hardware

*************************************************************************/

#include "machine/atarigen.h"

class blstroid_state : public atarigen_state
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, blstroid_state(machine)); }

	blstroid_state(running_machine &machine)
		: atarigen_state(machine) { }

	UINT16 *		priorityram;
};


/*----------- defined in video/blstroid.c -----------*/

VIDEO_START( blstroid );
VIDEO_UPDATE( blstroid );

void blstroid_scanline_update(screen_device &screen, int scanline);
