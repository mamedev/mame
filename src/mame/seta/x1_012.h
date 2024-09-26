// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_SETA_X1_012_H
#define MAME_SETA_X1_012_H

#pragma once

#include "screen.h"
#include "tilemap.h"

class x1_012_device : public device_t, public device_gfx_interface
{
public:
	x1_012_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> x1_012_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: x1_012_device(mconfig, tag, owner, u32(0))
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configuration
	template <typename... T> void set_tile_offset_callback(T &&... args) { m_tile_offset_callback.set(std::forward<T>(args)...); }
	template <typename T> void set_screen(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	void set_xoffsets(int flip, int noflip) { m_xoffsets[1] = flip; m_xoffsets[0] = noflip; }

	void vram_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	u16 vctrl_r(offs_t offset, u16 mem_mask = 0xffff);
	void vctrl_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);

	u16 vctrl(int index) const { return m_vctrl[index]; }
	void update_scroll(int vis_dimy, bool flip);

	void set_flip(u32 flip) { m_tilemap->set_flip(flip); }
	void mark_all_dirty() { m_tilemap->mark_all_dirty(); }

	void draw(screen_device &screen, bitmap_ind16 &dest, const rectangle &cliprect, u32 flags = TILEMAP_DRAW_ALL_CATEGORIES, u8 priority = 0, u8 priority_mask = 0xff);
	void draw_tilemap_palette_effect(bitmap_ind16 &bitmap, const rectangle &cliprect, bool flipscreen);

protected:
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	TILE_GET_INFO_MEMBER(get_tile_info);

	device_delegate<u16 (u16 code)> m_tile_offset_callback;

	required_shared_ptr<u16> m_vram;

	required_device<screen_device> m_screen;

	tilemap_t *m_tilemap;

	int m_xoffsets[2];

	u16 m_vctrl[3];
	u8 m_rambank;
};

DECLARE_DEVICE_TYPE(X1_012, x1_012_device)

#endif // MAME_SETA_X1_012_H

