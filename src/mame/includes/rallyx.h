// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_RALLYX_H
#define MAME_INCLUDES_RALLYX_H

#pragma once

#include "audio/timeplt.h"
#include "sound/namco.h"
#include "sound/samples.h"
#include "emupal.h"
#include "screen.h"

class rallyx_state : public driver_device
{
public:
	struct jungler_star
	{
		int x, y, color;
	};

	static constexpr unsigned JUNGLER_MAX_STARS = 1000;

	rallyx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_radarattr(*this, "radarattr"),
		m_maincpu(*this, "maincpu"),
		m_namco_sound(*this, "namco"),
		m_samples(*this, "samples"),
		m_timeplt_audio(*this, "timeplt_audio"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
	{ }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_radarattr;
	uint8_t *  m_spriteram;
	uint8_t *  m_spriteram2;
	uint8_t *  m_radarx;
	uint8_t *  m_radary;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;

	/* misc */
	int      m_last_bang;
	int      m_spriteram_base;
	int      m_stars_enable;
	int      m_total_stars;
	uint8_t    m_drawmode_table[4];
	struct jungler_star m_stars[JUNGLER_MAX_STARS];

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<namco_device> m_namco_sound;
	optional_device<samples_device> m_samples;
	optional_device<timeplt_audio_device> m_timeplt_audio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	bool    m_main_irq_mask;
	DECLARE_WRITE8_MEMBER(rallyx_interrupt_vector_w);
	DECLARE_WRITE_LINE_MEMBER(bang_w);
	DECLARE_WRITE_LINE_MEMBER(irq_mask_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_mask_w);
	DECLARE_WRITE_LINE_MEMBER(sound_on_w);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_w);
	DECLARE_WRITE_LINE_MEMBER(coin_lockout_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	DECLARE_WRITE8_MEMBER(rallyx_videoram_w);
	DECLARE_WRITE8_MEMBER(rallyx_scrollx_w);
	DECLARE_WRITE8_MEMBER(rallyx_scrolly_w);
	DECLARE_WRITE_LINE_MEMBER(stars_enable_w);
	TILEMAP_MAPPER_MEMBER(fg_tilemap_scan);
	TILE_GET_INFO_MEMBER(rallyx_bg_get_tile_info);
	TILE_GET_INFO_MEMBER(rallyx_fg_get_tile_info);
	TILE_GET_INFO_MEMBER(locomotn_bg_get_tile_info);
	TILE_GET_INFO_MEMBER(locomotn_fg_get_tile_info);
	DECLARE_MACHINE_START(rallyx);
	DECLARE_VIDEO_START(rallyx);
	void rallyx_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(jungler);
	void jungler_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(locomotn);
	DECLARE_VIDEO_START(commsega);
	uint32_t screen_update_rallyx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_jungler(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_locomotn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(rallyx_vblank_irq);
	DECLARE_WRITE_LINE_MEMBER(jungler_vblank_irq);
	inline void rallyx_get_tile_info( tile_data &tileinfo, int tile_index, int ram_offs);
	inline void locomotn_get_tile_info(tile_data &tileinfo,int tile_index,int ram_offs);
	void calculate_star_field();
	void rallyx_video_start_common();
	void plot_star( bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y, int color );
	void draw_stars( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void rallyx_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void locomotn_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void rallyx_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect, bool transpen );
	void jungler_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect, bool transpen );
	void locomotn_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect, bool transpen );
	void commsega(machine_config &config);
	void locomotn(machine_config &config);
	void tactcian(machine_config &config);
	void rallyx(machine_config &config);
	void jungler(machine_config &config);
	void io_map(address_map &map);
	void jungler_map(address_map &map);
	void rallyx_map(address_map &map);
};

#endif // MAME_INCLUDES_RALLYX_H
