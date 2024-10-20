// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_NAMCO_RALLYX_H
#define MAME_NAMCO_RALLYX_H

#pragma once

#include "timeplt_a.h"

#include "sound/namco.h"
#include "sound/samples.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class rallyx_state : public driver_device
{
public:
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

	void commsega(machine_config &config);
	void jungler(machine_config &config);
	void locomotn(machine_config &config);
	void rallyx(machine_config &config);
	void tactcian(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_radarattr;
	uint8_t *  m_spriteram = nullptr;
	uint8_t *  m_spriteram2 = nullptr;
	uint8_t *  m_radarx = nullptr;
	uint8_t *  m_radary = nullptr;

	// video-related
	tilemap_t  *m_bg_tilemap = nullptr;
	tilemap_t  *m_fg_tilemap = nullptr;

	// misc
	static constexpr unsigned JUNGLER_MAX_STARS = 1000;

	struct jungler_star
	{
		uint16_t x = 0, y = 0;
		uint8_t color = 0;
	};

	bool      m_last_bang = 0;
	uint8_t   m_spriteram_base = 0;
	bool      m_stars_enable = 0;
	uint16_t  m_total_stars = 0;
	uint8_t   m_drawmode_table[4];
	struct jungler_star m_stars[JUNGLER_MAX_STARS];
	bool      m_main_irq_mask = 0;
	uint8_t   m_interrupt_vector = 0;

	// devices
	required_device<cpu_device> m_maincpu;
	optional_device<namco_device> m_namco_sound;
	optional_device<samples_device> m_samples;
	optional_device<timeplt_audio_device> m_timeplt_audio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	void rallyx_interrupt_vector_w(uint8_t data);
	IRQ_CALLBACK_MEMBER(interrupt_vector_r);
	void bang_w(int state);
	void irq_mask_w(int state);
	void nmi_mask_w(int state);
	void sound_on_w(int state);
	void coin_lockout_w(int state);
	void coin_counter_1_w(int state);
	void coin_counter_2_w(int state);
	void videoram_w(offs_t offset, uint8_t data);
	void scrollx_w(uint8_t data);
	void scrolly_w(uint8_t data);
	void stars_enable_w(int state);
	void rallyx_vblank_irq(int state);
	void jungler_vblank_irq(int state);

	TILEMAP_MAPPER_MEMBER(fg_tilemap_scan);
	TILE_GET_INFO_MEMBER(rallyx_bg_get_tile_info);
	TILE_GET_INFO_MEMBER(rallyx_fg_get_tile_info);
	TILE_GET_INFO_MEMBER(locomotn_bg_get_tile_info);
	TILE_GET_INFO_MEMBER(locomotn_fg_get_tile_info);

	DECLARE_VIDEO_START(rallyx);
	DECLARE_VIDEO_START(jungler);
	DECLARE_VIDEO_START(locomotn);
	DECLARE_VIDEO_START(commsega);

	void rallyx_palette(palette_device &palette) const;
	void jungler_palette(palette_device &palette) const;
	uint32_t screen_update_rallyx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_jungler(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_locomotn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void rallyx_get_tile_info( tile_data &tileinfo, int tile_index, int ram_offs);
	inline void locomotn_get_tile_info(tile_data &tileinfo,int tile_index,int ram_offs);
	void calculate_star_field();
	void video_start_common();
	void plot_star( bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y, int color );
	void draw_stars( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void rallyx_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void locomotn_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void rallyx_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect, bool transpen );
	void jungler_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect, bool transpen );
	void locomotn_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect, bool transpen );

	void io_map(address_map &map) ATTR_COLD;
	void jungler_map(address_map &map) ATTR_COLD;
	void rallyx_map(address_map &map) ATTR_COLD;
};

#endif // MAME_NAMCO_RALLYX_H
