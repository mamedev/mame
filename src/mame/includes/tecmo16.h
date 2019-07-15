// license:BSD-3-Clause
// copyright-holders:Hau, Nicola Salmoria
#ifndef MAME_INCLUDES_TECMO16_H
#define MAME_INCLUDES_TECMO16_H

#pragma once

#include "video/bufsprite.h"
#include "video/tecmo_spr.h"
#include "video/tecmo_mix.h"
#include "emupal.h"
#include "screen.h"

class tecmo16_state : public driver_device
{
public:
	tecmo16_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_mixer(*this, "mixer"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2"),
		m_charram(*this, "charram"),
		m_spriteram(*this, "spriteram")
	{ }

	void ginkun(machine_config &config);
	void fstarfrc(machine_config &config);
	void riot(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<tecmo_spr_device> m_sprgen;
	required_device<tecmo_mix_device> m_mixer;

	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_colorram;
	required_shared_ptr<uint16_t> m_videoram2;
	required_shared_ptr<uint16_t> m_colorram2;
	required_shared_ptr<uint16_t> m_charram;
	required_device<buffered_spriteram16_device> m_spriteram;

	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	bitmap_ind16 m_sprite_bitmap;
	bitmap_ind16 m_tile_bitmap_bg;
	bitmap_ind16 m_tile_bitmap_fg;
	bitmap_ind16 m_tile_bitmap_tx;
	int m_flipscreen;
	int m_game_is_riot;
	uint16_t m_scroll_x_w;
	uint16_t m_scroll_y_w;
	uint16_t m_scroll2_x_w;
	uint16_t m_scroll2_y_w;
	uint16_t m_scroll_char_x_w;
	uint16_t m_scroll_char_y_w;

	DECLARE_WRITE16_MEMBER(videoram_w);
	DECLARE_WRITE16_MEMBER(colorram_w);
	DECLARE_WRITE16_MEMBER(videoram2_w);
	DECLARE_WRITE16_MEMBER(colorram2_w);
	DECLARE_WRITE16_MEMBER(charram_w);
	DECLARE_WRITE16_MEMBER(flipscreen_w);
	DECLARE_WRITE16_MEMBER(scroll_x_w);
	DECLARE_WRITE16_MEMBER(scroll_y_w);
	DECLARE_WRITE16_MEMBER(scroll2_x_w);
	DECLARE_WRITE16_MEMBER(scroll2_y_w);
	DECLARE_WRITE16_MEMBER(scroll_char_x_w);
	DECLARE_WRITE16_MEMBER(scroll_char_y_w);

	TILE_GET_INFO_MEMBER(fg_get_tile_info);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(tx_get_tile_info);

	virtual void video_start() override;
	DECLARE_VIDEO_START(ginkun);
	DECLARE_VIDEO_START(riot);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void save_state();

	void fstarfrc_map(address_map &map);
	void ginkun_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_TECMO16_H
