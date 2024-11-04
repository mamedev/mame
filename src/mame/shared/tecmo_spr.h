// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Tecmo Sprites */
#ifndef MAME_SHARED_TECMO_SPR_H
#define MAME_SHARED_TECMO_SPR_H

#pragma once


class tecmo_spr_device : public device_t, public device_gfx_interface
{
public:
	typedef device_delegate<uint32_t (uint8_t pri)> pri_cb_delegate;

	// constructors/destructors
	tecmo_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T> tecmo_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: tecmo_spr_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// config
	template <typename... T> void set_pri_callback(T &&... args) { m_pri_cb.set(std::forward<T>(args)...); }
	void set_bootleg(bool bootleg) { m_bootleg = bootleg; }
	void set_yoffset(int yoffset) { m_yoffset = yoffset; }

	// gaiden.cpp / spbactn.cpp / tecmo16.cpp sprites
	void gaiden_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, const uint16_t *spriteram, int sprite_sizey, int spr_offset_y, bool flip_screen);

	// tecmo.cpp sprites
	void draw_sprites_8bit(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, const uint8_t *spriteram, int size, int video_type, bool flip_screen);

	// wc90.cpp sprites
	void draw_wc90_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, const uint8_t *spriteram, int size);

	// tbowl.cpp sprites
	void tbowl_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int xscroll, const uint8_t *spriteram);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	pri_cb_delegate m_pri_cb;

	bool m_bootleg; // for Gals Pinball / Hot Pinball
	int m_yoffset;
};

DECLARE_DEVICE_TYPE(TECMO_SPRITE, tecmo_spr_device)


#endif // MAME_SHARED_TECMO_SPR_H
