// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

******************************************************************************/

#ifndef MAME_VIDEO_PPU2C0X_SH6578_H
#define MAME_VIDEO_PPU2C0X_SH6578_H

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

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	// draws with the dot renderer through its addressing, pattern and pixel hooks; its timing still follows the screen
	// (with the 2C02's own timing the SH6578 multicarts show a grey screen) and PPUDATA has no 2C02 pipeline
	virtual bool keeps_own_timing() const override { return false; }
	virtual bool has_data_pipeline() const override { return false; }
	virtual u16 dot_nt_addr() const override;
	virtual u16 dot_at_addr() const override { return dot_nt_addr() + 1; }
	virtual u8 dot_at_decode(u8 data) override;
	virtual u16 dot_pattern_addr(bool high) const override;
	virtual u8 dot_bg_pattern(u16 addr, bool high, u8 bus_data, u8 attr, u8 &extra) override;
	virtual u8 dot_sp_pattern(u16 addr, bool high, u8 bus_data, u8 attr, u8 &extra) override;
	virtual bool dot_bg_opaque(u8 bg_pix, u8 bg_pal) const override;
	virtual bool dot_pixel_rgb(int x, u8 bg_pix, u8 bg_pal, bool bg_opaque, u8 sp_pix, u8 sp_pal, bool sprite_wins, u32 *dest) override;
	u8 dot_bg_index(u8 bg_pix, u8 bg_pal) const;

	virtual void draw_sprite_pixel(int sprite_xpos, int color, int pixel, uint8_t pixel_data, bitmap_rgb32 &bitmap) override;
	//virtual void draw_sprites(uint8_t *line_priority) override;
	virtual void draw_background(uint8_t *line_priority) override;
	virtual int apply_sprite_pattern_page(int index1, int size) override;
	virtual void read_sprite_plane_data(int address) override;

	void ppu_internal_map(address_map &map) ATTR_COLD;

private:
	void scanline_increment_fine_ycounter() override;

	void read_tile_plane_data(int address, int color) override;
	void draw_tile(uint8_t *line_priority, int color_byte, int color_bits, int address, int start_x, uint32_t back_pen, uint32_t *&dest) override;

	uint8_t m_extplanebuf[2];
	uint8_t m_colsel_pntstart;
	uint8_t m_next_tile_hi = 0;     // dot renderer: the colour byte of the tile being fetched (tile number bits 8-11)
};

class ppu_sh6578pal_device : public ppu_sh6578_device
{
public:
	ppu_sh6578pal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(PPU_SH6578,    ppu_sh6578_device)
DECLARE_DEVICE_TYPE(PPU_SH6578PAL, ppu_sh6578pal_device)

#endif // MAME_VIDEO_PPU2C0X_SH6578_H
