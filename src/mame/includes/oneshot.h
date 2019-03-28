// license:BSD-3-Clause
// copyright-holders:David Haywood, Paul Priest
#ifndef MAME_INCLUDES_ONESHOT_H
#define MAME_INCLUDES_ONESHOT_H

#pragma once

#include "sound/okim6295.h"
#include "emupal.h"

class oneshot_state : public driver_device
{
public:
	oneshot_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_sprites(*this, "sprites"),
		m_bg_videoram(*this, "bg_videoram"),
		m_mid_videoram(*this, "mid_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_scroll(*this, "scroll"),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void maddonna(machine_config &config);
	void komocomo(machine_config &config);
	void oneshot(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_sprites;
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_mid_videoram;
	required_shared_ptr<uint16_t> m_fg_videoram;
	required_shared_ptr<uint16_t> m_scroll;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_mid_tilemap;
	tilemap_t  *m_fg_tilemap;

	/* misc */
	int m_gun_x_p1;
	int m_gun_y_p1;
	int m_gun_x_p2;
	int m_gun_y_p2;
	int m_gun_x_shift;
	int m_p1_wobble;
	int m_p2_wobble;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_READ16_MEMBER(oneshot_in0_word_r);
	DECLARE_READ16_MEMBER(oneshot_gun_x_p1_r);
	DECLARE_READ16_MEMBER(oneshot_gun_y_p1_r);
	DECLARE_READ16_MEMBER(oneshot_gun_x_p2_r);
	DECLARE_READ16_MEMBER(oneshot_gun_y_p2_r);
	DECLARE_WRITE16_MEMBER(oneshot_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(oneshot_mid_videoram_w);
	DECLARE_WRITE16_MEMBER(oneshot_fg_videoram_w);
	DECLARE_WRITE16_MEMBER(soundbank_w);
	TILE_GET_INFO_MEMBER(get_oneshot_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_oneshot_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_oneshot_fg_tile_info);
	uint32_t screen_update_oneshot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_maddonna(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_komocomo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_crosshairs( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void oneshot_map(address_map &map);
	void oneshot_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_ONESHOT_H
