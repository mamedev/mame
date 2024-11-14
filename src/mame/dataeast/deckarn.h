// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,David Haywood
#ifndef MAME_DATAEAST_DECKARN_H
#define MAME_DATAEAST_DECKARN_H

#pragma once

#include "screen.h"

class deco_karnovsprites_device : public device_t, public device_gfx_interface
{
public:
	typedef device_delegate<void (u32 &colour, u32 &pri_mask)> colpri_cb_delegate;

	deco_karnovsprites_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> deco_karnovsprites_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: deco_karnovsprites_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	template <typename... T> void set_colpri_callback(T &&... args) { m_colpri_cb.set(std::forward<T>(args)...); }

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, const u16 *spriteram, int size);
	void set_flip_screen(bool flip) { m_flip_screen = flip; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	colpri_cb_delegate m_colpri_cb;
	bool m_flip_screen;
};

DECLARE_DEVICE_TYPE(DECO_KARNOVSPRITES, deco_karnovsprites_device)

#endif // MAME_DATAEAST_DECKARN_H
