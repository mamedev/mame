// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Canyon Bomber hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define CANYON_MOTOR1_DATA      NODE_01
#define CANYON_MOTOR2_DATA      NODE_02
#define CANYON_EXPLODE_DATA     NODE_03
#define CANYON_WHISTLE1_EN      NODE_04
#define CANYON_WHISTLE2_EN      NODE_05
#define CANYON_ATTRACT1_EN      NODE_06
#define CANYON_ATTRACT2_EN      NODE_07



class canyon_state : public driver_device
{
public:
	canyon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;

	required_device<discrete_device> m_discrete;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	DECLARE_READ8_MEMBER(canyon_switches_r);
	DECLARE_READ8_MEMBER(canyon_options_r);
	DECLARE_WRITE8_MEMBER(canyon_led_w);
	DECLARE_WRITE8_MEMBER(canyon_videoram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(canyon);
	UINT32 screen_update_canyon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(canyon_motor_w);
	DECLARE_WRITE8_MEMBER(canyon_explode_w);
	DECLARE_WRITE8_MEMBER(canyon_attract_w);
	DECLARE_WRITE8_MEMBER(canyon_whistle_w);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_bombs( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


/*----------- defined in audio/canyon.c -----------*/
DISCRETE_SOUND_EXTERN( canyon );
