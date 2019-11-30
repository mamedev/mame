// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,David Haywood
#ifndef MAME_VIDEO_DECO_ZOOMSPR_H
#define MAME_VIDEO_DECO_ZOOMSPR_H

#pragma once


class deco_zoomspr_device : public device_t
{
public:
	deco_zoomspr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_gfxdecode(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }

	void dragngun_draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, const uint32_t *spritedata, uint32_t* dragngun_sprite_layout_0_ram, uint32_t* dragngun_sprite_layout_1_ram, uint32_t* dragngun_sprite_lookup_0_ram, uint32_t* dragngun_sprite_lookup_1_ram, uint32_t dragngun_sprite_ctrl, bitmap_ind8 &pri_bitmap, bitmap_rgb32 &temp_bitmap);


protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<gfxdecode_device> m_gfxdecode;

	void dragngun_drawgfxzoom(
		bitmap_rgb32 &dest_bmp, const rectangle &clip, gfx_element *gfx,
		uint32_t code, uint32_t color, int flipx, int flipy, int sx, int sy,
		int transparent_color,
		int scalex, int scaley, bitmap_ind8 *pri_buffer, uint32_t pri_mask, int sprite_screen_width, int  sprite_screen_height, uint8_t alpha, bitmap_ind8 &pri_bitmap, bitmap_rgb32 &temp_bitmap,
		int priority);

};


DECLARE_DEVICE_TYPE(DECO_ZOOMSPR, deco_zoomspr_device)

#endif // MAME_VIDEO_DECO_ZOOMSPR_H
