// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni,Nicola Salmoria,Bryan McPhail,David Haywood,R. Belmont,Alex Marshall,Angelo Salese,Luca Elia
// thanks-to:Richard Bush
#ifndef MAME_VIDEO_NMK16SPR_H
#define MAME_VIDEO_NMK16SPR_H

#pragma once

#include "screen.h"

class nmk_16bit_sprite_device : public device_t
{
public:
	nmk_16bit_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	typedef device_delegate<void (u32 &colour, u32 &pri_mask)> nmk16_spr_colpri_cb_delegate;
	typedef device_delegate<void (u16 attr, int &flipx, int &flipy, int &code)> nmk16_spr_ext_cb_delegate;

	// configuration
	template <typename... T> void set_colpri_callback(T &&... args) { m_colpri_cb = nmk_16bit_sprite_device::nmk16_spr_colpri_cb_delegate(std::forward<T>(args)...); }
	template <typename... T> void set_ext_callback(T &&... args) { m_ext_cb = nmk_16bit_sprite_device::nmk16_spr_ext_cb_delegate(std::forward<T>(args)...); }
	void set_videoshift(int shift) { m_videoshift = shift; }
	void set_mask(int xmask, int ymask) { m_xmask = xmask, m_ymask = ymask; }
	void set_screen_size(int width, int height) { m_screen_width = width, m_screen_height = height; }

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, u16* spriteram, int size);
	void set_flip_screen(bool flip) { m_flip_screen = flip; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	struct sprite_t
	{
		int height;
		u32 code, colour;
		int x, y;
		bool flipx, flipy;
		u32 pri_mask;
	};
	nmk16_spr_colpri_cb_delegate m_colpri_cb;
	nmk16_spr_ext_cb_delegate m_ext_cb;
	bool m_flip_screen;
	int m_videoshift;
	int m_xmask, m_ymask;
	int m_screen_width, m_screen_height;
	std::unique_ptr<struct sprite_t[]> m_spritelist;
};

DECLARE_DEVICE_TYPE(NMK_16BIT_SPRITE, nmk_16bit_sprite_device)

#endif // MAME_VIDEO_NMK16SPR_H
