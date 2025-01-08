// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#ifndef MAME_VIDEO_K051316_H
#define MAME_VIDEO_K051316_H

#pragma once

#include "tilemap.h"


#define K051316_CB_MEMBER(_name)   void _name(int *code, int *color)


class k051316_device : public device_t, public device_gfx_interface
{
public:
	using zoom_delegate = device_delegate<void (int *code, int *color)>;

	k051316_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static const gfx_layout charlayout4;
	static const gfx_layout charlayout7;
	static const gfx_layout charlayout8;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);
	DECLARE_GFXDECODE_MEMBER(gfxinfo7);
	DECLARE_GFXDECODE_MEMBER(gfxinfo8);
	DECLARE_GFXDECODE_MEMBER(gfxinfo4_ram);

	// configuration
	template <typename... T> void set_zoom_callback(T &&... args) { m_k051316_cb.set(std::forward<T>(args)...); }
	void set_wrap(int wrap) { m_wrap = wrap; }
	void set_bpp(int bpp);
	void set_layermask(int mask) { m_layermask = mask; }
	void set_offsets(int x_offset, int y_offset)
	{
		m_dx = x_offset;
		m_dy = y_offset;
	}

	/*
	The callback is passed:
	- code (range 00-FF, contents of the first tilemap RAM byte)
	- color (range 00-FF, contents of the first tilemap RAM byte). Note that bit 6
	  seems to be hardcoded as flip X.
	The callback must put:
	- in code the resulting tile number
	- in color the resulting color index
	  structure (e.g. TILE_FLIPX)
	*/

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	u8 rom_r(offs_t offset);
	void ctrl_w(offs_t offset, u8 data);
	void zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, u32 priority);
	void wraparound_enable(int status);

	void mark_gfx_dirty(offs_t byteoffset) { gfx(0)->mark_dirty(byteoffset * m_pixels_per_byte / (16 * 16)); }
	void mark_tmap_dirty() { m_tmap->mark_all_dirty(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	// internal state
	std::vector<uint8_t> m_ram;
	uint8_t m_ctrlram[14];
	tilemap_t *m_tmap;

	optional_region_ptr<uint8_t> m_zoom_rom;

	int m_dx, m_dy;
	int m_wrap;
	int m_pixels_per_byte;
	int m_layermask;
	zoom_delegate m_k051316_cb;
	bool m_readout_enabled;
	bool m_flipx_enabled;
	bool m_flipy_enabled;

	TILE_GET_INFO_MEMBER(get_tile_info);
};

DECLARE_DEVICE_TYPE(K051316, k051316_device)

#endif // MAME_VIDEO_K051316_H
