// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    VT video emulation

    The VT video is based on the ppu2c0x but with enhanced capabilities such
    as 16 colour sprites.

******************************************************************************/

#ifndef MAME_VIDEO_PPU2C0X_VT_H
#define MAME_VIDEO_PPU2C0X_VT_H

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

	uint8_t read_2010(offs_t offset);
	uint8_t read_2011(offs_t offset);
	uint8_t videobank0_0_r(offs_t offset);
	uint8_t videobank0_1_r(offs_t offset);
	uint8_t videobank0_2_r(offs_t offset);
	uint8_t videobank0_3_r(offs_t offset);
	uint8_t videobank0_4_r(offs_t offset);
	uint8_t videobank0_5_r(offs_t offset);
	uint8_t read_2018(offs_t offset);
	uint8_t read_2019(offs_t offset);
	uint8_t read_201a(offs_t offset);
	uint8_t read_201b(offs_t offset);
	uint8_t read_201c(offs_t offset);
	uint8_t read_201d(offs_t offset);
	uint8_t read_201e(offs_t offset);
	uint8_t read_201f(offs_t offset);

	void write_2010(offs_t offset, uint8_t data);
	void write_2011(offs_t offset, uint8_t data);
	void videobank0_0_w(offs_t offset, uint8_t data);
	void videobank0_1_w(offs_t offset, uint8_t data);
	void videobank0_2_w(offs_t offset, uint8_t data);
	void videobank0_3_w(offs_t offset, uint8_t data);
	void videobank0_4_w(offs_t offset, uint8_t data);
	void videobank0_5_w(offs_t offset, uint8_t data);
	void write_2018(offs_t offset, uint8_t data);
	void write_2019(offs_t offset, uint8_t data);
	void write_201a(offs_t offset, uint8_t data);

	virtual uint8_t palette_read(offs_t offset) override;
	virtual void palette_write(offs_t offset, uint8_t data) override;

	uint8_t get_extended_modes_enable() { return m_extended_modes_enable; }

	void set_201x_reg(int reg, uint8_t data);
	uint8_t get_201x_reg(int reg);
	uint8_t get_videobank0_reg(int reg) { return m_videobank0[reg]; }
	void set_videobank0_reg(int reg, uint8_t data) { m_videobank0[reg] = data; }

	uint8_t get_va34();
	uint8_t get_m_read_bg4_bg3();
	uint8_t get_speva2_speva0();

	bool get_is_pal() { return m_is_pal; }
	bool get_is_50hz() { return m_is_50hz; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u32 palette_entries() const noexcept override { return (0x40 * 8) + (0x1000 * 8); }

	virtual void read_tile_plane_data(int address, int color) override;
	virtual void shift_tile_plane_data(uint8_t &pix) override;
	virtual void draw_tile_pixel(uint8_t pix, int color, uint32_t back_pen, uint32_t *&dest) override;
	inline void draw_tile_pixel_inner(uint8_t pen, uint32_t *dest);
	virtual void draw_back_pen(uint32_t* dst, int back_pen) override;

	virtual void read_sprite_plane_data(int address) override;
	virtual void make_sprite_pixel_data(uint8_t &pixel_data, bool flipx) override;
	virtual void draw_sprite_pixel(int sprite_xpos, int color, int pixel, uint8_t pixel_data, bitmap_rgb32 &bitmap) override;
	virtual void read_extra_sprite_bits(int sprite_index) override;

	bool m_is_pal;
	bool m_is_50hz;

	uint32_t m_vtpens_rgb555[0x8000*8];
	uint32_t m_vtpens_rgb444[0x1000*8];

	uint8_t m_201x_regs[0x20];
	uint8_t m_videobank0[6];
	uint8_t m_extended_modes_enable;

private:
	devcb_read8 m_read_bg;
	devcb_read8 m_read_sp;

	int32_t m_read_bg4_bg3;
	bool m_va34;

	uint8_t m_extplanebuf[2];
	uint8_t m_extra_sprite_bits;

	vtxx_pal_mode m_pal_mode = PAL_MODE_VT0x;

	void init_vt03_palette_tables(int palmode);
	void init_vtxx_rgb555_palette_tables();
	void init_vtxx_rgb444_palette_tables();

	static constexpr unsigned YUV444_COLOR = (0x40 * 8);
};

class ppu_vt03pal_device : public ppu_vt03_device {
public:
	ppu_vt03pal_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);
};

class ppu_vt3xx_device : public ppu_vt03_device {
public:
	ppu_vt3xx_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

	uint8_t read_201c_newvid(offs_t offset);
	uint8_t read_201d_newvid(offs_t offset);
	uint8_t read_201e_newvid(offs_t offset);

	void write_201c_newvid(offs_t offset, uint8_t data);
	void write_201d_newvid(offs_t offset, uint8_t data);
	void write_201e_newvid(offs_t offset, uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_newvid_1c;
	uint8_t m_newvid_1d;
	uint8_t m_newvid_1e;
};


DECLARE_DEVICE_TYPE(PPU_VT03,    ppu_vt03_device)
DECLARE_DEVICE_TYPE(PPU_VT03PAL, ppu_vt03pal_device)

DECLARE_DEVICE_TYPE(PPU_VT3XX,    ppu_vt3xx_device)


#endif // MAME_VIDEO_PPU2C0X_VT_H
