// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

******************************************************************************/

#ifndef MAME_VIDEO_PPU_SH6578_H
#define MAME_VIDEO_PPU_SH6578_H

#pragma once

#include "video/ppu2c0x.h"

class ppu_sh6578_device : public ppu2c0x_device
{
public:
	ppu_sh6578_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t palette_read(offs_t offset) override;
	virtual void palette_write(offs_t offset, uint8_t data) override;
	virtual void write_extended(offs_t offset, uint8_t data);
	virtual uint8_t read_extended(offs_t offset);

protected:
	ppu_sh6578_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void ppu_internal_map(address_map &map) ATTR_COLD;

private:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void scanline_increment_fine_ycounter() override;

	void read_tile_plane_data(int address, int color) override;
	void draw_tile(uint8_t* line_priority, int color_byte, int color_bits, int address, int start_x, uint32_t back_pen, uint32_t*& dest) override;

	virtual void draw_sprite_pixel(int sprite_xpos, int color, int pixel, uint8_t pixel_data, bitmap_rgb32& bitmap) override;
	//virtual void draw_sprites(uint8_t* line_priority) override;
	virtual void draw_background(uint8_t* line_priority) override;
	virtual int apply_sprite_pattern_page(int index1, int size) override;
	virtual void read_sprite_plane_data(int address) override;

	uint8_t m_extplanebuf[2];
	uint8_t m_colsel_pntstart;
};

class ppu_sh6578pal_device : public ppu_sh6578_device
{
public:
	ppu_sh6578pal_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(PPU_SH6578,    ppu_sh6578_device)
DECLARE_DEVICE_TYPE(PPU_SH6578PAL, ppu_sh6578pal_device)

#endif // MAME_VIDEO_PPU_SH6578_H
