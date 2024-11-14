// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia
#ifndef MAME_JALECO_JALBLEND_H
#define MAME_JALECO_JALBLEND_H

#pragma once

#include "emupal.h"

class jaleco_blend_device : public device_t
{
public:
	jaleco_blend_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	~jaleco_blend_device() {}

	rgb_t func(rgb_t dest, rgb_t addMe);
	rgb_t func(rgb_t dest, rgb_t addMe, u8 alpha);
	void drawgfx(
			palette_device &palette, gfx_element *gfx,
			bitmap_ind16 &dest_bmp, const rectangle &clip,
			u32 code, u32 color,
			bool flipx, bool flipy, int offsx, int offsy,
			u8 transparent_color);
	void drawgfx(
			palette_device &palette, gfx_element *gfx,
			bitmap_rgb32 &dest_bmp, const rectangle &clip,
			u32 code, u32 color,
			bool flipx, bool flipy, int offsx, int offsy,
			u8 transparent_color);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	template<class BitmapClass>
	void drawgfx_common(palette_device &palette,BitmapClass &dest_bmp,const rectangle &clip,gfx_element *gfx,
			u32 code,u32 color,bool flipx,bool flipy,int offsx,int offsy,
			u8 transparent_color);
};

DECLARE_DEVICE_TYPE(JALECO_BLEND, jaleco_blend_device)

#endif // MAME_JALECO_JALBLEND_H
