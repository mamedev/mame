// license:BSD-3-Clause
// copyright-holders:Mike Coates
#ifndef MAME_INCLUDES_CIRCUS_H
#define MAME_INCLUDES_CIRCUS_H

#pragma once

#include "machine/timer.h"
#include "sound/discrete.h"
#include "sound/samples.h"
#include "emupal.h"
#include "tilemap.h"

class circus_state : public driver_device
{
public:
	circus_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int m_clown_x;
	int m_clown_y;
	int m_clown_z;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_device<discrete_sound_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* game id */
	int m_game_id;
	DECLARE_READ8_MEMBER(circus_paddle_r);
	DECLARE_WRITE8_MEMBER(circus_videoram_w);
	DECLARE_WRITE8_MEMBER(circus_clown_x_w);
	DECLARE_WRITE8_MEMBER(circus_clown_y_w);
	DECLARE_WRITE8_MEMBER(circus_clown_z_w);
	void init_ripcord();
	void init_circus();
	void init_robotbwl();
	void init_crash();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_circus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_robotbwl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_crash(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ripcord(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(crash_scanline);
	void draw_line( bitmap_ind16 &bitmap, const rectangle &cliprect, int x1, int y1, int x2, int y2, int dotted );
	void draw_sprite_collision( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void circus_draw_fg( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void robotbwl_draw_box( bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y );
	void robotbwl_draw_scoreboard( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void robotbwl_draw_bowling_alley( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void robotbwl_draw_ball( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void crash_draw_car( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void robotbwl(machine_config &config);
	void ripcord(machine_config &config);
	void crash(machine_config &config);
	void circus(machine_config &config);
	void circus_map(address_map &map);
};
/*----------- defined in audio/circus.c -----------*/

DISCRETE_SOUND_EXTERN( circus_discrete );
DISCRETE_SOUND_EXTERN( robotbwl_discrete );
DISCRETE_SOUND_EXTERN( crash_discrete );
extern const char *const circus_sample_names[];
extern const char *const crash_sample_names[];
extern const char *const ripcord_sample_names[];
extern const char *const robotbwl_sample_names[];

#endif // MAME_INCLUDES_CIRCUS_H
