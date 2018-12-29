// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_AQUARIUM_H
#define MAME_INCLUDES_AQUARIUM_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/mb3773.h"
#include "sound/okim6295.h"
#include "video/excellent_spr.h"
#include "emupal.h"
#include "screen.h"

class aquarium_state : public driver_device
{
public:
	aquarium_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_mid_videoram(*this, "mid_videoram"),
		m_bak_videoram(*this, "bak_videoram"),
		m_txt_videoram(*this, "txt_videoram"),
		m_scroll(*this, "scroll"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_watchdog(*this, "watchdog")
	{ }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_mid_videoram;
	required_shared_ptr<uint16_t> m_bak_videoram;
	required_shared_ptr<uint16_t> m_txt_videoram;
	required_shared_ptr<uint16_t> m_scroll;

	/* video-related */
	tilemap_t  *m_txt_tilemap;
	tilemap_t  *m_mid_tilemap;
	tilemap_t  *m_bak_tilemap;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<excellent_spr_device> m_sprgen;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<mb3773_device> m_watchdog;

	DECLARE_WRITE8_MEMBER(aquarium_watchdog_w);
	DECLARE_WRITE8_MEMBER(aquarium_z80_bank_w);
	DECLARE_READ8_MEMBER(aquarium_oki_r);
	DECLARE_WRITE8_MEMBER(aquarium_oki_w);
	DECLARE_WRITE16_MEMBER(aquarium_txt_videoram_w);
	DECLARE_WRITE16_MEMBER(aquarium_mid_videoram_w);
	DECLARE_WRITE16_MEMBER(aquarium_bak_videoram_w);
	void init_aquarium();
	TILE_GET_INFO_MEMBER(get_aquarium_txt_tile_info);
	TILE_GET_INFO_MEMBER(get_aquarium_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_aquarium_bak_tile_info);
	virtual void video_start() override;
	uint32_t screen_update_aquarium(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint8_t aquarium_snd_bitswap( uint8_t scrambled_data );
	void mix_sprite_bitmap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority_mask, int priority_value);
	bitmap_ind16 m_temp_sprite_bitmap;
	void aquarium(machine_config &config);
	void main_map(address_map &map);
	void snd_map(address_map &map);
	void snd_portmap(address_map &map);
};

#endif // MAME_INCLUDES_AQUARIUM_H
