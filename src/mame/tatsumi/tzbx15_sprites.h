// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Angelo Salese

#ifndef MAME_TATSUMI_TZBX15_SPRITES_H
#define MAME_TATSUMI_TZBX15_SPRITES_H

#pragma once

#include "emupal.h"

class tzbx15_device : public device_t, public device_gfx_interface
{
public:
	void set_sprite_palette_base(int sprite_palette_base) { m_sprite_palette_base = sprite_palette_base; }
	template <typename T> void set_basepalette(T &&tag) { m_palette_base.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_spriteram(T &&tag) { m_spriteram.set_tag(std::forward<T>(tag)); }

	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int write_priority_only, int rambank);
	void draw_sprites(bitmap_ind8 &bitmap, const rectangle &cliprect, int write_priority_only, int rambank);

	void update_cluts();

protected:
	tzbx15_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
	tzbx15_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 clut_size);

private:

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void mycopyrozbitmap_core(bitmap_ind8 &bitmap, const bitmap_rgb32 &srcbitmap,
			int dstx, int dsty, int srcwidth, int srcheight, int incxx, int incxy, int incyx, int incyy,
			const rectangle &clip, int transparent_color);
	void mycopyrozbitmap_core(bitmap_rgb32 &bitmap, const bitmap_rgb32 &srcbitmap,
			int dstx, int dsty, int srcwidth, int srcheight, int incxx, int incxy, int incyx, int incyy,
			const rectangle &clip, int transparent_color);

	template<class BitmapClass> void draw_sprites_main(BitmapClass &bitmap, const rectangle &cliprect, int write_priority_only, int rambank);
	template<class BitmapClass> void roundupt_drawgfxzoomrotate(
			BitmapClass &dest_bmp, const rectangle &clip,
			gfx_element *gfx, uint32_t code, uint32_t color,
			int flipx, int flipy, uint32_t ssx, uint32_t ssy,
			int scalex, int scaley, int rotate,
			int write_priority_only);

	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	required_device<palette_device> m_palette_clut;
	required_device<palette_device> m_palette_base;
	required_shared_ptr<uint16_t> m_spriteram;
	required_region_ptr<uint8_t> m_sprites_l_rom;
	required_region_ptr<uint8_t> m_sprites_h_rom;

	std::unique_ptr<uint8_t[]> m_shadow_pen_array;
	bitmap_rgb32 m_temp_bitmap;

	// config
	int m_rom_clut_size;
	int m_rom_clut_offset;
	int m_sprite_palette_base;
};

class tzb215_device : public tzbx15_device
{
public:
	tzb215_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 clut_size);
	tzb215_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class tzb315_device : public tzbx15_device
{
public:
	tzb315_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 clut_size);
	tzb315_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(TZB215_SPRITES, tzb215_device)
DECLARE_DEVICE_TYPE(TZB315_SPRITES, tzb315_device)

#endif // MAME_TATSUMI_TZBX15_SPRITES_H
