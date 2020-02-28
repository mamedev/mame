// license:BSD-3-Clause
// copyright-holders:David Haywood, Nicola Salmoria, Tomasz Slanina
#ifndef MAME_INCLUDES_DARKMIST_H
#define MAME_INCLUDES_DARKMIST_H

#pragma once

#include "audio/t5182.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class darkmist_state : public driver_device
{
public:
	darkmist_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_t5182(*this, "t5182"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritebank(*this, "spritebank"),
		m_scroll(*this, "scroll"),
		m_videoram(*this, "videoram"),
		m_workram(*this, "workram"),
		m_spriteram(*this, "spriteram"),
		m_bg_clut(*this, "bg_clut"),
		m_fg_clut(*this, "fg_clut"),
		m_spr_clut(*this, "spr_clut"),
		m_tx_clut(*this, "tx_clut"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void darkmist(machine_config &config);

	void init_darkmist();

private:
	required_device<cpu_device> m_maincpu;
	required_device<t5182_device> m_t5182;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_spritebank;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_workram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_region_ptr<uint8_t> m_bg_clut;
	required_region_ptr<uint8_t> m_fg_clut;
	required_region_ptr<uint8_t> m_spr_clut;
	required_region_ptr<uint8_t> m_tx_clut;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	int m_hw;
	tilemap_t *m_bgtilemap;
	tilemap_t *m_fgtilemap;
	tilemap_t *m_txtilemap;

	DECLARE_WRITE8_MEMBER(hw_w);
	DECLARE_WRITE8_MEMBER(tx_vram_w);

	TILE_GET_INFO_MEMBER(get_bgtile_info);
	TILE_GET_INFO_MEMBER(get_fgtile_info);
	TILE_GET_INFO_MEMBER(get_txttile_info);

	virtual void machine_start() override;
	virtual void video_start() override;
	void darkmist_palette(palette_device &palette) const;

	bitmap_ind16 m_temp_bitmap;
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mix_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* clut);
	void decrypt_fgbgtiles(uint8_t* rgn, int size);
	void decrypt_gfx();
	void decrypt_snd();

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void decrypted_opcodes_map(address_map &map);
	void memmap(address_map &map);
};

#endif // MAME_INCLUDES_DARKMIST_H
