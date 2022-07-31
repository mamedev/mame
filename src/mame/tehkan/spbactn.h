// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_SPBACTN_H
#define MAME_INCLUDES_SPBACTN_H

#pragma once

#include "machine/gen_latch.h"
#include "tecmo_spr.h"
#include "tecmo_mix.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class spbactn_state : public driver_device
{
public:
	spbactn_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_mixer(*this, "mixer"),
		m_soundlatch(*this, "soundlatch"),
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_spvideoram(*this, "spvideoram"),
		m_extraram(*this, "extraram"),
		m_extraram2(*this, "extraram2")
	{ }

	void spbactn(machine_config &config);
	void spbactnp(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<tecmo_spr_device> m_sprgen;
	required_device<tecmo_mix_device> m_mixer;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_bgvideoram;
	required_shared_ptr<uint16_t> m_fgvideoram;
	required_shared_ptr<uint16_t> m_spvideoram;
	optional_shared_ptr<uint8_t> m_extraram;
	optional_shared_ptr<uint8_t> m_extraram2;

	tilemap_t    *m_bg_tilemap = nullptr;
	tilemap_t    *m_fg_tilemap = nullptr;

	tilemap_t    *m_extra_tilemap = nullptr;

	void main_irq_ack_w(uint16_t data);

	void bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void extraram_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_extra_tile_info);

	bitmap_ind16 m_tile_bitmap_bg;
	bitmap_ind16 m_tile_bitmap_fg;
	bitmap_ind16 m_sprite_bitmap;

	void spbatnp_90006_w(uint16_t data);
	void spbatnp_9000a_w(uint16_t data);
	void spbatnp_9000c_w(uint16_t data);
	void spbatnp_9000e_w(uint16_t data);

	void spbatnp_90124_w(uint16_t data);
	void spbatnp_9012c_w(uint16_t data);

	DECLARE_VIDEO_START(spbactn);
	DECLARE_VIDEO_START(spbactnp);

	//virtual void video_start();
	uint32_t screen_update_spbactn(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spbactnp(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	int draw_video(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, bool alt_sprites);

	// temp hack
	uint16_t temp_read_handler_r()
	{
		return 0xffff;
	}

	void spbactn_map(address_map &map);
	void spbactn_sound_map(address_map &map);
	void spbactnp_extra_map(address_map &map);
	void spbactnp_map(address_map &map);
};

#endif // MAME_INCLUDES_SPBACTN_H
