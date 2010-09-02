/*************************************************************************

    Atari Vindicators hardware

*************************************************************************/

#include "machine/atarigen.h"

class vindictr_state : public atarigen_state
{
public:
	vindictr_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }

	UINT8			playfield_tile_bank;
	UINT16			playfield_xscroll;
	UINT16			playfield_yscroll;
};


/*----------- defined in video/vindictr.c -----------*/

WRITE16_HANDLER( vindictr_paletteram_w );

VIDEO_START( vindictr );
VIDEO_UPDATE( vindictr );

void vindictr_scanline_update(screen_device &screen, int scanline);
