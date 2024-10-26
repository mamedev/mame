// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_IGS_IGS023_VIDEO_H
#define MAME_IGS_IGS023_VIDEO_H

#pragma once

#include "tilemap.h"

class igs023_video_device : public device_t, public device_gfx_interface
{
public:
	igs023_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto read_spriteram_callback() { return m_readspriteram_cb.bind(); }

	u16 videoram_r(offs_t offset);
	void videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 videoregs_r(offs_t offset);
	void videoregs_w(offs_t offset, u16 data, u16 mem_mask);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void get_sprites();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	struct sprite_t
	{
		int x = 0, y = 0;
		bool xgrow = false, ygrow = false;
		u32 xzoom = 0, yzoom = 0;
		u32 color = 0, offs = 0;
		u32 width = 0, height = 0;
		u8 flip = 0, pri = 0;
	};

	required_memory_region m_gfx_region;

	required_region_ptr<u16> m_adata;
	required_region_ptr<u16> m_bdata;

	devcb_read16 m_readspriteram_cb; // for reading spritelist from mainram

	std::unique_ptr<sprite_t[]> m_spritelist;
	sprite_t *m_sprite_ptr_pre;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;

	u32 m_aoffset;
	u8 m_abit;
	u32 m_boffset;

	std::unique_ptr<uint16_t []> m_videoregs;
	std::unique_ptr<uint16_t []> m_videoram;

	// working variables, don't need saving
	u16 *m_bg_videoram = nullptr;
	u16 *m_tx_videoram = nullptr;
	u16 *m_rowscrollram = nullptr;

	void pgm_draw_pix(int xdrawpos, int pri, u16 *dest, u8 *destpri, const rectangle &cliprect, u16 srcdat);
	void pgm_draw_pix_nopri(int xdrawpos, u16 *dest, u8 *destpri, const rectangle &cliprect, u16 srcdat);
	void pgm_draw_pix_pri(int xdrawpos, u16 *dest, u8 *destpri, const rectangle &cliprect, u16 srcdat);
	u8 get_sprite_pix();
	void draw_sprite_line(int wide, u16 *dest, u8 *destpri, const rectangle &cliprect, int xzoom, bool xgrow, int flip, int xpos, int pri, int realxsize, int palt, bool draw);
	void draw_sprite_new_zoomed(int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, u32 xzoom, bool xgrow, u32 yzoom, bool ygrow, int pri);
	void draw_sprite_line_basic(int wide, u16 *dest, u8 *destpri, const rectangle &cliprect, int flip, int xpos, int pri, int realxsize, int palt, bool draw);
	void draw_sprite_new_basic(int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri);
	void draw_sprites(bitmap_ind16& spritebitmap, const rectangle &cliprect, bitmap_ind8& priority_bitmap);

	void tx_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	DECLARE_GFXDECODE_MEMBER(gfxinfo);
};

DECLARE_DEVICE_TYPE(IGS023_VIDEO, igs023_video_device)

#endif // MAME_IGS_IGS023_VIDEO_H
