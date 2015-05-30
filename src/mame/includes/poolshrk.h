// license:???
// copyright-holders:Stefan Jokisch
/*************************************************************************

    Atari Pool Shark hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */

class poolshrk_state : public driver_device
{
public:
	poolshrk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_playfield_ram(*this, "playfield_ram"),
		m_hpos_ram(*this, "hpos_ram"),
		m_vpos_ram(*this, "vpos_ram"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	int m_da_latch;
	required_shared_ptr<UINT8> m_playfield_ram;
	required_shared_ptr<UINT8> m_hpos_ram;
	required_shared_ptr<UINT8> m_vpos_ram;
	required_device<discrete_device> m_discrete;
	tilemap_t* m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(poolshrk_da_latch_w);
	DECLARE_WRITE8_MEMBER(poolshrk_led_w);
	DECLARE_WRITE8_MEMBER(poolshrk_watchdog_w);
	DECLARE_READ8_MEMBER(poolshrk_input_r);
	DECLARE_READ8_MEMBER(poolshrk_irq_reset_r);
	DECLARE_DRIVER_INIT(poolshrk);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void video_start();
	DECLARE_PALETTE_INIT(poolshrk);
	UINT32 screen_update_poolshrk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(poolshrk_scratch_sound_w);
	DECLARE_WRITE8_MEMBER(poolshrk_score_sound_w);
	DECLARE_WRITE8_MEMBER(poolshrk_click_sound_w);
	DECLARE_WRITE8_MEMBER(poolshrk_bump_sound_w);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


/*----------- defined in audio/poolshrk.c -----------*/
DISCRETE_SOUND_EXTERN( poolshrk );
