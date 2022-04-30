// license:BSD-3-Clause
// copyright-holders:Uki
/*************************************************************************

    Momoko 120%

*************************************************************************/
#ifndef MAME_INCLUDES_MOMOKO_H
#define MAME_INCLUDES_MOMOKO_H

#pragma once

#include "emupal.h"

class momoko_state : public driver_device
{
public:
	momoko_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_bg_scrolly(*this, "bg_scrolly"),
		m_bg_scrollx(*this, "bg_scrollx"),
		m_bg_gfx(*this, "bg_gfx"),
		m_bg_map(*this, "bg_map"),
		m_bg_col_map(*this, "bg_col_map"),
		m_fg_map(*this, "fg_map"),
		m_proms(*this, "proms"),
		m_bgbank(*this, "bgbank"),
		m_io_fake(*this, "FAKE"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void momoko(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<u8> m_spriteram;
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_bg_scrolly;
	required_shared_ptr<u8> m_bg_scrollx;
	required_region_ptr<u8> m_bg_gfx;
	required_region_ptr<u8> m_bg_map;
	required_region_ptr<u8> m_bg_col_map;
	required_region_ptr<u8> m_fg_map;
	required_region_ptr<u8> m_proms;
	required_memory_bank m_bgbank;
	required_ioport m_io_fake;

	/* video-related */
	u8          m_fg_scrollx = 0;
	u8          m_fg_scrolly = 0;
	u8          m_fg_select = 0;
	u8          m_text_scrolly = 0;
	u8          m_text_mode = 0;
	u8          m_bg_select = 0;
	u8          m_bg_priority = 0;
	u8          m_bg_mask = 0;
	u8          m_fg_mask = 0;
	u8          m_flipscreen = 0;
	void bg_read_bank_w(u8 data);
	void fg_scrollx_w(u8 data);
	void fg_scrolly_w(u8 data);
	void fg_select_w(u8 data);
	void text_scrolly_w(u8 data);
	void text_mode_w(u8 data);
	void bg_scrollx_w(offs_t offset, u8 data);
	void bg_scrolly_w(offs_t offset, u8 data);
	void bg_select_w(u8 data);
	void bg_priority_w(u8 data);
	void flipscreen_w(u8 data);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bg_pri(bitmap_ind16 &bitmap, int chr, int col, int flipx, int flipy, int x, int y, int pri);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end, int flip);
	void draw_text_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	void draw_fg_romtilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	void draw_bg_romtilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip, bool high);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void momoko_map(address_map &map);
	void momoko_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_MOMOKO_H
