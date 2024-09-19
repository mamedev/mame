// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni,Nicola Salmoria,Bryan McPhail,David Haywood,R. Belmont,Alex Marshall,Angelo Salese,Luca Elia
// thanks-to:Richard Bush
#ifndef MAME_NMK_NMK16SPR_H
#define MAME_NMK_NMK16SPR_H

#pragma once

#include "screen.h"

class nmk_16bit_sprite_device : public device_t
{
public:
	typedef device_delegate<void (u32 &colour, u32 &pri_mask)> colpri_cb_delegate;
	typedef device_delegate<void (u16 attr, int &flipx, int &flipy, int &code)> ext_cb_delegate;

	nmk_16bit_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename... T> void set_colpri_callback(T &&... args) { m_colpri_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_ext_callback(T &&... args) { m_ext_cb.set(std::forward<T>(args)...); }
	void set_videoshift(int shift) { m_videoshift = shift; }
	void set_mask(int xmask, int ymask) { m_xmask = xmask, m_ymask = ymask; }
	void set_screen_size(int width, int height) { m_screen_width = width, m_screen_height = height; }
	void set_max_sprite_clock(u32 max) { m_max_sprite_clock = max; }

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, u16* spriteram, int size);
	void set_flip_screen(bool flip) { m_flip_screen = flip; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	struct sprite_t
	{
		u32 code = 0, colour = 0;
		int x = 0, y = 0;
		bool flipx = false, flipy = false;
		u32 pri_mask = 0;
	};
	colpri_cb_delegate m_colpri_cb;                  // callback for colour, priority
	ext_cb_delegate m_ext_cb;                        // callback for flipx, flipy or code bit modification
	bool m_flip_screen;                              // flip screen
	int m_videoshift;                                // x offset needs for more than 256 horizontal screen pixels
	int m_xmask, m_ymask;                            // x,y position masking
	int m_screen_width, m_screen_height;             // screen size related to flipscreen
	u32 m_max_sprite_clock;                          // max sprite cycles, related to screen total size?
	std::unique_ptr<sprite_t[]> m_spritelist;        // sprite list caches
};

DECLARE_DEVICE_TYPE(NMK_16BIT_SPRITE, nmk_16bit_sprite_device)

#endif // MAME_NMK_NMK16SPR_H
