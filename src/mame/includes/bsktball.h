// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Basketball hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define BSKTBALL_NOTE_DATA      NODE_01
#define BSKTBALL_CROWD_DATA     NODE_02
#define BSKTBALL_NOISE_EN       NODE_03
#define BSKTBALL_BOUNCE_EN      NODE_04


class bsktball_state : public driver_device
{
public:
	bsktball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_motion(*this, "motion"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_motion;
	required_device<discrete_device> m_discrete;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* misc */
	UINT32   m_nmi_on;
//  int      m_i256v;

	/* input-related */
	int m_ld1;
	int m_ld2;
	int m_dir0;
	int m_dir1;
	int m_dir2;
	int m_dir3;
	int m_last_p1_horiz;
	int m_last_p1_vert;
	int m_last_p2_horiz;
	int m_last_p2_vert;
	DECLARE_WRITE8_MEMBER(bsktball_nmion_w);
	DECLARE_WRITE8_MEMBER(bsktball_ld1_w);
	DECLARE_WRITE8_MEMBER(bsktball_ld2_w);
	DECLARE_READ8_MEMBER(bsktball_in0_r);
	DECLARE_WRITE8_MEMBER(bsktball_led1_w);
	DECLARE_WRITE8_MEMBER(bsktball_led2_w);
	DECLARE_WRITE8_MEMBER(bsktball_videoram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(bsktball);
	UINT32 screen_update_bsktball(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(bsktball_scanline);
	DECLARE_WRITE8_MEMBER(bsktball_bounce_w);
	DECLARE_WRITE8_MEMBER(bsktball_note_w);
	DECLARE_WRITE8_MEMBER(bsktball_noise_reset_w);
	void draw_sprites(  bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

/*----------- defined in audio/bsktball.c -----------*/

DISCRETE_SOUND_EXTERN( bsktball );
