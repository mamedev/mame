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
	void set_gfx_region(int gfxregion) { m_gfxregion = gfxregion; }
	void set_bootleg(int bootleg) { m_bootleg = bootleg; }
	void set_yoffset(int yoffset) { m_yoffset = yoffset; }

	// gaiden.c / spbactn.c / tecmo16.c sprites
	void gaiden_draw_sprites(screen_device &screen, gfxdecode_device *gfxdecode, const rectangle &cliprect, uint16_t* spriteram, int sprite_sizey, int spr_offset_y, int flip_screen, bitmap_ind16 &sprite_bitmap);

	// tecmo.c sprites
	void draw_sprites_8bit(screen_device &screen, bitmap_ind16 &bitmap, gfxdecode_device *gfxdecode, const rectangle &cliprect, uint8_t* spriteram, int size, int video_type, int flip_screen);

	// wc90.c sprites
	void draw_wc90_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode, uint8_t* spriteram, int size, int priority);

	// tbowl.c sprites
	void tbowl_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode, int xscroll, uint8_t* spriteram);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	uint8_t m_gfxregion;
	int m_bootleg; // for Gals Pinball / Hot Pinball
	int m_yoffset;
};

DECLARE_DEVICE_TYPE(TECMO_SPRITE, tecmo_spr_device)


#define MCFG_TECMO_SPRITE_GFX_REGION(_region) \
	downcast<tecmo_spr_device &>(*device).set_gfx_region(_region);

#define MCFG_TECMO_SPRITE_BOOTLEG(_bootleg) \
	downcast<tecmo_spr_device &>(*device).set_bootleg(_bootleg);

#define MCFG_TECMO_SPRITE_YOFFSET(_yoffset) \
	downcast<tecmo_spr_device &>(*device).set_yoffset(_yoffset);

#endif // MAME_VIDEO_TECMO_SPR_H
