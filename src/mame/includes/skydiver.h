// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Skydiver hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define SKYDIVER_RANGE_DATA     NODE_01
#define SKYDIVER_NOTE_DATA      NODE_02
#define SKYDIVER_RANGE3_EN      NODE_03
#define SKYDIVER_NOISE_DATA     NODE_04
#define SKYDIVER_NOISE_RST      NODE_05
#define SKYDIVER_WHISTLE1_EN    NODE_06
#define SKYDIVER_WHISTLE2_EN    NODE_07
#define SKYDIVER_OCT1_EN        NODE_08
#define SKYDIVER_OCT2_EN        NODE_09
#define SKYDIVER_SOUND_EN       NODE_10


class skydiver_state : public driver_device
{
public:
	skydiver_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_videoram;

	int m_nmion;
	tilemap_t *m_bg_tilemap;
	int m_width;

	DECLARE_WRITE8_MEMBER(nmion_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_READ8_MEMBER(wram_r);
	DECLARE_WRITE8_MEMBER(wram_w);
	DECLARE_WRITE8_MEMBER(width_w);
	DECLARE_WRITE8_MEMBER(coin_lockout_w);
	DECLARE_WRITE8_MEMBER(start_lamp_1_w);
	DECLARE_WRITE8_MEMBER(start_lamp_2_w);
	DECLARE_WRITE8_MEMBER(lamp_s_w);
	DECLARE_WRITE8_MEMBER(lamp_k_w);
	DECLARE_WRITE8_MEMBER(lamp_y_w);
	DECLARE_WRITE8_MEMBER(lamp_d_w);
	DECLARE_WRITE8_MEMBER(_2000_201F_w);
	DECLARE_WRITE8_MEMBER(sound_enable_w);
	DECLARE_WRITE8_MEMBER(whistle_w);

	TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(skydiver);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(interrupt);
};

/*----------- defined in audio/skydiver.c -----------*/
DISCRETE_SOUND_EXTERN( skydiver );
