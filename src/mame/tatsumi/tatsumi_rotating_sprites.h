// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Angelo Salese

#ifndef MAME_TATSUMI_TATSUMI_ROTATING_SPRITES_H
#define MAME_TATSUMI_TATSUMI_ROTATING_SPRITES_H

#pragma once

#include "emupal.h"

class tatsumi_rotating_sprites_device : public device_t, public device_gfx_interface
{
public:
	tatsumi_rotating_sprites_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_rom_clut_offset(int rom_clut_offset) { m_rom_clut_offset = rom_clut_offset - m_rom_clut_size; }
	void set_sprite_palette_base(int sprite_palette_base) { m_sprite_palette_base = sprite_palette_base; }

	void draw_sprites(bitmap_rgb32& bitmap, const rectangle& cliprect, int write_priority_only, int rambank);
	void draw_sprites(bitmap_ind8& bitmap, const rectangle& cliprect, int write_priority_only, int rambank);

	void update_cluts();

	DECLARE_GFXDECODE_MEMBER(gfxinfo);

protected:
	tatsumi_rotating_sprites_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void common_init();

	int m_rom_clut_size;

	required_device<palette_device> m_fakepalette;
	required_device<palette_device> m_basepalette;

private:
	std::unique_ptr<uint8_t[]> m_shadow_pen_array;

	bitmap_rgb32 m_temp_bitmap;

	int m_rom_clut_offset;
	int m_sprite_palette_base;

	void mycopyrozbitmap_core(bitmap_ind8 &bitmap, const bitmap_rgb32 &srcbitmap,
			int dstx, int dsty, int srcwidth, int srcheight, int incxx, int incxy, int incyx, int incyy,
			const rectangle &clip, int transparent_color);
	void mycopyrozbitmap_core(bitmap_rgb32 &bitmap, const bitmap_rgb32 &srcbitmap,
			int dstx, int dsty, int srcwidth, int srcheight, int incxx, int incxy, int incyx, int incyy,
			const rectangle &clip, int transparent_color);

	template<class BitmapClass> void draw_sprites_main(BitmapClass &bitmap, const rectangle &cliprect, int write_priority_only, int rambank);
	template<class BitmapClass> inline void roundupt_drawgfxzoomrotate( BitmapClass &dest_bmp, const rectangle &clip,
		gfx_element *gfx, uint32_t code,uint32_t color,int flipx,int flipy,uint32_t ssx,uint32_t ssy,
		int scalex, int scaley, int rotate, int write_priority_only );

	required_shared_ptr<uint16_t> m_spriteram;
	required_region_ptr<uint8_t> m_sprites_l_rom;
	required_region_ptr<uint8_t> m_sprites_h_rom;
};

class tatsumi_rotating_sprites_bigpal_device : public tatsumi_rotating_sprites_device
{
public:
	tatsumi_rotating_sprites_bigpal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_GFXDECODE_MEMBER(gfxinfo_big);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(TATSUMI_ROTATING_SPRITES, tatsumi_rotating_sprites_device)
DECLARE_DEVICE_TYPE(TATSUMI_ROTATING_SPRITES_BIGPAL, tatsumi_rotating_sprites_bigpal_device)

#endif // MAME_TATSUMI_TATSUMI_ROTATING_SPRITES_H
