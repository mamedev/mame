// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia
#ifndef MAME_VIDEO_JALBLEND_H
#define MAME_VIDEO_JALBLEND_H

#pragma once

#include "emupal.h"

class jaleco_blend_device : public device_t
{
public:
	jaleco_blend_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~jaleco_blend_device() {}

	void set(int color, uint8_t val);
	rgb_t func(rgb_t dest, rgb_t addMe, uint8_t alpha);
	void drawgfx(palette_device &palette,bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							uint32_t code,uint32_t color,int flipx,int flipy,int offsx,int offsy,
							int transparent_color);
	void drawgfx(palette_device &palette,bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							uint32_t code,uint32_t color,int flipx,int flipy,int offsx,int offsy,
							int transparent_color);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	/* each palette entry contains a fourth 'alpha' value */
	std::unique_ptr<uint8_t[]> m_table;

	template<class _BitmapClass>
	void drawgfx_common(palette_device &palette,_BitmapClass &dest_bmp,const rectangle &clip,gfx_element *gfx,
							uint32_t code,uint32_t color,int flipx,int flipy,int offsx,int offsy,
							int transparent_color);
};

DECLARE_DEVICE_TYPE(JALECO_BLEND, jaleco_blend_device)

#endif // MAME_VIDEO_JALBLEND_H
