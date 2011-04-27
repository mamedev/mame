/*************************************************************************

    Atari Pool Shark hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */

class poolshrk_state : public driver_device
{
public:
	poolshrk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_da_latch;
	UINT8* m_playfield_ram;
	UINT8* m_hpos_ram;
	UINT8* m_vpos_ram;
	tilemap_t* m_bg_tilemap;
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

