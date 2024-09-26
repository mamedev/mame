// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#ifndef MAME_DATAEAST_DECMXC06_H
#define MAME_DATAEAST_DECMXC06_H

#pragma once

#include "screen.h"

class deco_mxc06_device : public device_t, public device_video_interface, public device_gfx_interface
{
public:
	typedef device_delegate<void (u32 &colour, u32 &pri_mask)> colpri_cb_delegate;

	deco_mxc06_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T> deco_mxc06_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: deco_mxc06_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configuration
	template <typename... T> void set_colpri_callback(T &&... args) { m_colpri_cb.set(std::forward<T>(args)...); }

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u16* spriteram, int size);
	void draw_sprites_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u16* spriteram, int size);
	void set_flip_screen(bool flip) { m_flip_screen = flip; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	struct sprite_t
	{
		int height = 0;
		u32 code[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }, colour = 0;
		int x[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }, y[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		bool flipx = false, flipy = false;
		u32 pri_mask = 0;
	};
	colpri_cb_delegate m_colpri_cb;
	bool m_flip_screen = false;
	std::unique_ptr<struct sprite_t[]> m_spritelist;
};

DECLARE_DEVICE_TYPE(DECO_MXC06, deco_mxc06_device)

#endif // MAME_DATAEAST_DECMXC06_H
