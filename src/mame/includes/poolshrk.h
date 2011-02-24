/*************************************************************************

    Atari Pool Shark hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */

class poolshrk_state : public driver_device
{
public:
	poolshrk_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int da_latch;
	UINT8* playfield_ram;
	UINT8* hpos_ram;
	UINT8* vpos_ram;
	tilemap_t* bg_tilemap;
};


/*----------- defined in audio/poolshrk.c -----------*/

WRITE8_DEVICE_HANDLER( poolshrk_scratch_sound_w );
WRITE8_DEVICE_HANDLER( poolshrk_score_sound_w );
WRITE8_DEVICE_HANDLER( poolshrk_click_sound_w );
WRITE8_DEVICE_HANDLER( poolshrk_bump_sound_w );

DISCRETE_SOUND_EXTERN( poolshrk );


/*----------- defined in video/poolshrk.c -----------*/

VIDEO_START( poolshrk );
SCREEN_UPDATE( poolshrk );

