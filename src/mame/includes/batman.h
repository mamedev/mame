/*************************************************************************

    Atari Batman hardware

*************************************************************************/

#include "machine/atarigen.h"

class batman_state : public atarigen_state
{
public:
	batman_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }

	UINT16			m_latch_data;

	UINT8			m_alpha_tile_bank;
};


/*----------- defined in video/batman.c -----------*/

VIDEO_START( batman );
SCREEN_UPDATE( batman );

void batman_scanline_update(screen_device &screen, int scanline);
