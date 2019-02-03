// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#ifndef MAME_INCLUDES_DEADANG_H
#define MAME_INCLUDES_DEADANG_H

#pragma once

#include "audio/seibu.h"
#include "machine/timer.h"
#include "sound/ym2151.h"
#include "emupal.h"
#include "screen.h"

class deadang_state : public driver_device
{
public:
	deadang_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_scroll_ram(*this, "scroll_ram"),
		m_videoram(*this, "videoram"),
		m_video_data(*this, "video_data"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_seibu_sound(*this, "seibu_sound"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_adpcm1(*this, "adpcm1"),
		m_adpcm2(*this, "adpcm2")
	{ }

	void deadang(machine_config &config);

	void init_deadang();
	void init_ghunter();

	DECLARE_WRITE16_MEMBER(foreground_w);

	TILEMAP_MAPPER_MEMBER(bg_scan);
	TILE_GET_INFO_MEMBER(get_pf1_tile_info);
	TILE_GET_INFO_MEMBER(get_pf3_tile_info);
	TILE_GET_INFO_MEMBER(get_pf2_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);

protected:
	virtual void video_start() override;

	void main_map(address_map &map);

	required_shared_ptr<uint16_t> m_scroll_ram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_video_data;
	required_shared_ptr<uint16_t> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<seibu_sound_device> m_seibu_sound;

	int m_tilebank;
	int m_oldtilebank;

	tilemap_t *m_pf3_layer;
	tilemap_t *m_pf2_layer;
	tilemap_t *m_pf1_layer;
	tilemap_t *m_text_layer;

	DECLARE_WRITE16_MEMBER(text_w);
	DECLARE_WRITE16_MEMBER(bank_w);

	void sound_decrypted_opcodes_map(address_map &map);
	void sound_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<seibu_adpcm_device> m_adpcm1;
	optional_device<seibu_adpcm_device> m_adpcm2;

	DECLARE_READ16_MEMBER(ghunter_trackball_low_r);
	DECLARE_READ16_MEMBER(ghunter_trackball_high_r);


	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(main_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(sub_scanline);
	void sub_map(address_map &map);
};

class popnrun_state : public deadang_state
{
public:
	popnrun_state(const machine_config &mconfig, device_type type, const char *tag) :
		deadang_state(mconfig, type, tag)
	{ }

	void popnrun(machine_config &config);

	void init_popnrun();

protected:
	virtual void video_start() override;

private:
	TILE_GET_INFO_MEMBER(get_popnrun_text_tile_info);
	DECLARE_WRITE16_MEMBER(popnrun_text_w);
	void popnrun_main_map(address_map &map);
	void popnrun_sub_map(address_map &map);
	void popnrun_sound_map(address_map &map);

	uint32_t popnrun_screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void popnrun_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

#endif // MAME_INCLUDES_DEADANG_H
