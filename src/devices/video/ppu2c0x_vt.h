// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    VT video emulation

    The VT video is based on the ppu2c0x but with enhanced capabilities such
    as 16 colour sprites.

******************************************************************************/

#ifndef MAME_VIDEO_PPU_VT03_H
#define MAME_VIDEO_PPU_VT03_H

#pragma once

#include "video/ppu2c0x.h"

enum vtxx_pal_mode {
	PAL_MODE_VT0x,
	PAL_MODE_NEW_RGB,
	PAL_MODE_NEW_RGB12,
};

class ppu_vt03_device : public ppu2c0x_device {
public:
	ppu_vt03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	ppu_vt03_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	auto read_bg() { return m_read_bg.bind(); }
	auto read_sp() { return m_read_sp.bind(); }

	void set_palette_mode(vtxx_pal_mode pmode) { m_pal_mode = pmode; }
	void set_201x_descramble(uint8_t reg0, uint8_t reg1, uint8_t reg2, uint8_t reg3, uint8_t reg4, uint8_t reg5);

	uint8_t read_extended(offs_t offset);
	void write_extended(offs_t offset, uint8_t data);

	virtual uint8_t palette_read(offs_t offset) override;
	virtual void palette_write(offs_t offset, uint8_t data) override;

	void init_vt03_palette_tables(int palmode);
	void init_vtxx_rgb555_palette_tables();
	void init_vtxx_rgb444_palette_tables();

	virtual void read_tile_plane_data(int address, int color) override;
	virtual void shift_tile_plane_data(uint8_t &pix) override;
	virtual void draw_tile_pixel(uint8_t pix, int color, uint32_t back_pen, uint32_t *&dest) override;
	inline void draw_tile_pixel_inner(uint8_t pen, uint32_t *dest);
	virtual void draw_back_pen(uint32_t* dst, int back_pen) override;

	virtual void read_sprite_plane_data(int address) override;
	virtual void make_sprite_pixel_data(uint8_t &pixel_data, int flipx) override;
	virtual void draw_sprite_pixel(int sprite_xpos, int color, int pixel, uint8_t pixel_data, bitmap_rgb32 &bitmap) override;
	virtual void read_extra_sprite_bits(int sprite_index) override;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void set_201x_reg(int reg, uint8_t data);
	uint8_t get_201x_reg(int reg);

	uint8_t get_va34();
	uint8_t get_m_read_bg4_bg3();
	uint8_t get_speva2_speva0();

	bool get_is_pal() { return m_is_pal; }
	bool get_is_50hz() { return m_is_50hz; }

protected:
	bool m_is_pal;
	bool m_is_50hz;

	uint32_t m_vtpens[0x1000*8];
	uint32_t m_vtpens_rgb555[0x8000*8];
	uint32_t m_vtpens_rgb444[0x1000*8];

private:
	devcb_read8 m_read_bg;
	devcb_read8 m_read_sp;

	int m_read_bg4_bg3;
	int m_va34;

	uint8_t m_extplanebuf[2];
	uint8_t m_extra_sprite_bits;

	uint8_t m_201x_regs[0x20];

	uint8_t m_2012_2017_descramble[0x6];

	vtxx_pal_mode m_pal_mode = PAL_MODE_VT0x;

	void set_2010_reg(uint8_t data);
};

class ppu_vt03pal_device : public ppu_vt03_device {
public:
	ppu_vt03pal_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(PPU_VT03,    ppu_vt03_device)
DECLARE_DEVICE_TYPE(PPU_VT03PAL,    ppu_vt03pal_device)

#endif // MAME_VIDEO_PPU_VT03_H
