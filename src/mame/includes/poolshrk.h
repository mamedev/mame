/*************************************************************************

    Atari Pool Shark hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */

class poolshrk_state : public driver_device
{
public:
	poolshrk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_playfield_ram(*this, "playfield_ram"),
		m_hpos_ram(*this, "hpos_ram"),
		m_vpos_ram(*this, "vpos_ram"){ }

	int m_da_latch;
	required_shared_ptr<UINT8> m_playfield_ram;
	required_shared_ptr<UINT8> m_hpos_ram;
	required_shared_ptr<UINT8> m_vpos_ram;
	tilemap_t* m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(poolshrk_da_latch_w);
	DECLARE_WRITE8_MEMBER(poolshrk_led_w);
	DECLARE_WRITE8_MEMBER(poolshrk_watchdog_w);
	DECLARE_READ8_MEMBER(poolshrk_input_r);
	DECLARE_READ8_MEMBER(poolshrk_irq_reset_r);
	DECLARE_DRIVER_INIT(poolshrk);
	TILE_GET_INFO_MEMBER(get_tile_info);
};


/*----------- defined in audio/poolshrk.c -----------*/

WRITE8_DEVICE_HANDLER( poolshrk_scratch_sound_w );
WRITE8_DEVICE_HANDLER( poolshrk_score_sound_w );
WRITE8_DEVICE_HANDLER( poolshrk_click_sound_w );
WRITE8_DEVICE_HANDLER( poolshrk_bump_sound_w );

DISCRETE_SOUND_EXTERN( poolshrk );


/*----------- defined in video/poolshrk.c -----------*/

VIDEO_START( poolshrk );
SCREEN_UPDATE_IND16( poolshrk );

