// license:BSD-3-Clause
// copyright-holders:David Graves, Angelo Salese, David Haywood, Tomasz Slanina
#ifndef MAME_INCLUDES_LEGIONNA_H
#define MAME_INCLUDES_LEGIONNA_H

#pragma once

#include "sound/okim6295.h"
#include "audio/seibu.h"
#include "machine/gen_latch.h"
#include "machine/seibucop/seibucop.h"
#include "video/seibu_crtc.h"
#include "emupal.h"

class legionna_state : public driver_device, public seibu_sound_common
{
public:
	legionna_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_spriteram(*this, "spriteram")
		, m_swappal(*this, "swappal")
		, m_layer_disable(0)
		, m_back_gfx_bank(0)
		, m_fore_gfx_bank(0)
		, m_mid_gfx_bank(0)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_seibu_sound(*this, "seibu_sound")
		, m_oki(*this, "oki")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_crtc(*this, "crtc")
		, m_raiden2cop(*this, "raiden2cop")
	{
		memset(scrollvals, 0, sizeof(uint16_t)*6);
	}

	void cupsocs(machine_config &config);
	void heatbrl(machine_config &config);
	void cupsoc(machine_config &config);
	void grainbow(machine_config &config);
	void legionna(machine_config &config);
	void godzilla(machine_config &config);
	void denjinmk(machine_config &config);

	void init_legiongfx();
	void init_godzilla();
	void init_cupsoc_debug();
	void init_cupsoc();
	void init_cupsocs();
	void init_olysoc92();

private:
	required_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_swappal;
	std::unique_ptr<uint16_t[]> m_back_data;
	std::unique_ptr<uint16_t[]> m_fore_data;
	std::unique_ptr<uint16_t[]> m_mid_data;
	std::unique_ptr<uint16_t[]> m_textram;
	std::unique_ptr<uint16_t[]> m_scrollram16;
	std::unique_ptr<uint16_t[]> m_paletteram;
	uint16_t m_layer_disable;
	std::unique_ptr<uint16_t[]> m_layer_config;
	int m_sprite_xoffs;
	int m_sprite_yoffs;
	tilemap_t *m_background_layer;
	tilemap_t *m_foreground_layer;
	tilemap_t *m_midground_layer;
	tilemap_t *m_text_layer;
	int m_has_extended_banking;
	int m_has_extended_priority;
	uint16_t m_sprite_pri_mask[4];
	uint16_t m_back_gfx_bank;
	uint16_t m_fore_gfx_bank;
	uint16_t m_mid_gfx_bank;
	uint16_t scrollvals[6];
	DECLARE_WRITE16_MEMBER(tilemap_enable_w);
	DECLARE_WRITE16_MEMBER(tile_scroll_w);
	DECLARE_WRITE16_MEMBER(tile_scroll_base_w);
	DECLARE_WRITE16_MEMBER(tile_vreg_1a_w);
	DECLARE_WRITE16_MEMBER(videowrite_cb_w);
	DECLARE_WRITE16_MEMBER(wordswapram_w);
	DECLARE_WRITE16_MEMBER(legionna_background_w);
	DECLARE_WRITE16_MEMBER(legionna_midground_w);
	DECLARE_WRITE16_MEMBER(legionna_foreground_w);
	DECLARE_WRITE16_MEMBER(legionna_text_w);
	u8 denjinmk_sound_comms_r(offs_t offset);
	DECLARE_WRITE8_MEMBER(godzilla_oki_bank_w);
	DECLARE_WRITE16_MEMBER(denjinmk_setgfxbank);
	DECLARE_WRITE16_MEMBER(heatbrl_setgfxbank);
	DECLARE_WRITE16_MEMBER(grainbow_layer_config_w);
	DECLARE_WRITE16_MEMBER(palette_swap_w);

	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_mid_tile_info_denji);
	TILE_GET_INFO_MEMBER(get_mid_tile_info_cupsoc);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info_denji);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	DECLARE_VIDEO_START(legionna);
	DECLARE_VIDEO_START(heatbrl);
	DECLARE_VIDEO_START(godzilla);
	DECLARE_VIDEO_START(denjinmk);
	DECLARE_VIDEO_START(grainbow);
	DECLARE_VIDEO_START(cupsoc);
	uint32_t screen_update_legionna(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_heatbrl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_godzilla(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_grainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void descramble_legionnaire_gfx(uint8_t* src);
	void common_video_start();
	void common_video_allocate_ptr();
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<seibu_sound_device> m_seibu_sound;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<seibu_crtc_device> m_crtc;
	optional_device<raiden2cop_device> m_raiden2cop;
	void cupsoc_map(address_map &map);
	void cupsocs_map(address_map &map);
	void denjinmk_map(address_map &map);
	void godzilla_map(address_map &map);
	void grainbow_map(address_map &map);
	void heatbrl_map(address_map &map);
	void legionna_cop_map(address_map &map);
	void legionna_map(address_map &map);
	void godzilla_sound_io_map(address_map &map);
};

#endif // MAME_INCLUDES_LEGIONNA_H
