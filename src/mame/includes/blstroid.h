/*************************************************************************

    Atari Blasteroids hardware

*************************************************************************/

#include "machine/atarigen.h"

class blstroid_state : public atarigen_state
{
public:
	blstroid_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }

	UINT16 *		priorityram;
};


/*----------- defined in video/blstroid.c -----------*/

VIDEO_START( blstroid );
VIDEO_UPDATE( blstroid );

void blstroid_scanline_update(screen_device &screen, int scanline);
