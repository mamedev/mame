// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Tecmo Sprites */
#ifndef MAME_VIDEO_TECMO_SPR_H
#define MAME_VIDEO_TECMO_SPR_H

#pragma once


class tecmo_spr_device : public device_t
{
public:
	tecmo_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void set_bootleg(bool bootleg) { m_bootleg = bootleg; }
	void set_yoffset(int yoffset) { m_yoffset = yoffset; }

	// gaiden.cpp / spbactn.cpp / tecmo16.cpp sprites
	void gaiden_draw_sprites(screen_device &screen, gfx_element *gfx, const rectangle &cliprect, uint16_t* spriteram, int sprite_sizey, int spr_offset_y, int flip_screen, bitmap_ind16 &sprite_bitmap);

	// tecmo.cpp sprites
	void draw_sprites_8bit(screen_device &screen, bitmap_ind16 &bitmap, gfx_element *gfx, const rectangle &cliprect, uint8_t* spriteram, int size, int video_type, int flip_screen);

	// wc90.cpp sprites
	void draw_wc90_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, uint8_t* spriteram, int size, int priority);

	// tbowl.cpp sprites
	void tbowl_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, int xscroll, uint8_t* spriteram);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	bool m_bootleg; // for Gals Pinball / Hot Pinball
	int m_yoffset;
};

DECLARE_DEVICE_TYPE(TECMO_SPRITE, tecmo_spr_device)


#endif // MAME_VIDEO_TECMO_SPR_H
