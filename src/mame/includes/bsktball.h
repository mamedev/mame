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
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_motion;
	required_device<discrete_device> m_discrete;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* misc */
	uint32_t   m_nmi_on;
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
	void bsktball_nmion_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bsktball_ld1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bsktball_ld2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bsktball_in0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bsktball_led1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bsktball_led2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bsktball_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_bsktball(palette_device &palette);
	uint32_t screen_update_bsktball(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void bsktball_scanline(timer_device &timer, void *ptr, int32_t param);
	void bsktball_bounce_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bsktball_note_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bsktball_noise_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void draw_sprites(  bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

/*----------- defined in audio/bsktball.c -----------*/

DISCRETE_SOUND_EXTERN( bsktball );
