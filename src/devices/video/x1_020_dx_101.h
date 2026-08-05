// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
#ifndef MAME_VIDEO_X1_020_DX_101_H
#define MAME_VIDEO_X1_020_DX_101_H

#pragma once

#include "screen.h"

class x1_020_dx_101_device : public device_t, public device_gfx_interface, public device_video_interface
{
public:
	x1_020_dx_101_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configurations
	auto raster_irq_callback() { return m_raster_irq_cb.bind(); }
	auto flip_screen_callback() { return m_flip_screen_cb.bind(); }
	auto flip_screen_x_callback() { return m_flip_screen_x_cb.bind(); }
	auto flip_screen_y_callback() { return m_flip_screen_y_cb.bind(); }

	void vregs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t vregs_r(offs_t offset);
	uint16_t spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	int calculate_global_xoffset(bool nozoom_fixedpalette_fixedposition);
	int calculate_global_yoffset(bool nozoom_fixedpalette_fixedposition);
	void draw_sprites_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int scanline, int realscanline, int xoffset, uint32_t xzoom, bool xzoominverted);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void drawgfx_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int gfx, uint8_t const *const addr, uint32_t realcolor, bool flipx, bool flipy, int base_sx, uint32_t xzoom, bool shadow, int screenline, int line, bool opaque);
	inline void get_tile(uint16_t const *spriteram, bool is_16x16, int x, int y, int page, int &code, int &attr, bool &flipx, bool &flipy, int &color);

	TIMER_CALLBACK_MEMBER(raster_timer_done);

	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	// configuration
	devcb_write_line m_raster_irq_cb;
	devcb_write_line m_flip_screen_cb;
	devcb_write_line m_flip_screen_x_cb;
	devcb_write_line m_flip_screen_y_cb;

	// memory pointers
	memory_share_creator<uint16_t> m_spriteram;
	memory_share_creator<uint16_t> m_vregs;

	// live state
	std::unique_ptr<uint16_t[]> m_private_spriteram;
	std::unique_ptr<uint32_t[]> m_realtilenumber;

	uint16_t m_rasterposition = 0;
	uint16_t m_rasterenabled = 0;
	emu_timer *m_raster_timer = nullptr;
};

DECLARE_DEVICE_TYPE(X1_020_DX_101, x1_020_dx_101_device)

#endif // MAME_VIDEO_X1_020_DX_101_H
