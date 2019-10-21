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
#include "tilemap.h"

class aquarium_state : public driver_device
{
public:
	aquarium_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_mid_videoram(*this, "mid_videoram"),
		m_bak_videoram(*this, "bak_videoram"),
		m_txt_videoram(*this, "txt_videoram"),
		m_scroll(*this, "scroll"),
		m_audiobank(*this, "bank1"),
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

	void init_aquarium();

	void aquarium(machine_config &config);

protected:
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<u16> m_mid_videoram;
	required_shared_ptr<u16> m_bak_videoram;
	required_shared_ptr<u16> m_txt_videoram;
	required_shared_ptr<u16> m_scroll;
	required_memory_bank m_audiobank;

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

	void watchdog_w(u8 data);
	void z80_bank_w(u8 data);
	u8 oki_r();
	void oki_w(u8 data);

	void expand_gfx(int low, int hi);
	void txt_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void mid_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bak_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_txt_tile_info);
	TILE_GET_INFO_MEMBER(get_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_bak_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u8 snd_bitswap(u8 scrambled_data);
	void aquarium_colpri_cb(u32 &colour, u32 &pri_mask);

	void main_map(address_map &map);
	void snd_map(address_map &map);
	void snd_portmap(address_map &map);
};

#endif // MAME_INCLUDES_AQUARIUM_H
