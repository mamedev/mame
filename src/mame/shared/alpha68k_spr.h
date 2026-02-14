// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Acho A. Tang, Nicola Salmoria
#ifndef MAME_SHARED_ALPHA68K_SPR_H
#define MAME_SHARED_ALPHA68K_SPR_H

#pragma once

#include "screen.h"


class alpha68k_sprite_device : public device_t, public device_gfx_interface, public device_video_interface
{
public:
	typedef device_delegate<void (u32 &tile, bool &fx, bool &fy, u8 &region, u32 &color)> tile_indirection_delegate;

	alpha68k_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> alpha68k_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: alpha68k_sprite_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// static configuration
	template <typename T> void set_spriteram_tag(T &&tag) { m_spriteram.set_tag(std::forward<T>(tag)); }
	template <typename... T> void set_tile_indirect_cb(T &&... args) { m_newtilecb.set(std::forward<T>(args)...); }
	void set_no_partial() { m_partialupdates = false; }
	void set_xpos_shift(u8 data) { m_xpos_shift = data; }

	u16 spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int group, u16 start_offset, u16 end_offset);
	void draw_sprites_all(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites_alt(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void tile_callback_noindirect(u32 &tile, bool &fx, bool &fy, u8 &region, u32 &color);
	void set_flip(bool flip);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	tile_indirection_delegate m_newtilecb;
	required_shared_ptr<u16> m_spriteram;
	bool m_flipscreen;
	bool m_partialupdates; // the original hardware needs this, the cloned hardware does not.
	u8 m_xpos_shift;
};


DECLARE_DEVICE_TYPE(ALPHA68K_SPR, alpha68k_sprite_device)

#endif // MAME_SHARED_ALPHA68K_SPR_H
