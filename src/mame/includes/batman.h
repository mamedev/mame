/*************************************************************************

    Atari Batman hardware

*************************************************************************/

#include "machine/atarigen.h"

class batman_state : public atarigen_state
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, batman_state(machine)); }

	batman_state(running_machine &machine)
		: atarigen_state(machine) { }

	UINT16			latch_data;

	UINT8			alpha_tile_bank;
};


/*----------- defined in video/batman.c -----------*/

VIDEO_START( batman );
VIDEO_UPDATE( batman );

void batman_scanline_update(screen_device &screen, int scanline);
