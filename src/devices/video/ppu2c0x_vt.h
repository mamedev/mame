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
	ppu_vt03_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	ppu_vt03_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	auto read_bg() { return m_read_bg.bind(); }
	auto read_sp() { return m_read_sp.bind(); }
	auto read_onespace() { return m_read_onespace.bind(); }
	auto read_onespace_with_relative() { return m_read_onespace_with_relative.bind(); }

	void set_palette_mode(vtxx_pal_mode pmode) { m_pal_mode = pmode; }

	u8 extended_modes_enable_r(offs_t offset);
	u8 extended_modes2_enable_r(offs_t offset);
	u8 videobank0_0_r(offs_t offset);
	u8 videobank0_1_r(offs_t offset);
	u8 videobank0_2_r(offs_t offset);
	u8 videobank0_3_r(offs_t offset);
	u8 videobank0_4_r(offs_t offset);
	u8 videobank0_5_r(offs_t offset);
	u8 videobank1_r(offs_t offset);
	u8 unk_2019_r(offs_t offset);
	u8 videobank0_extra_r(offs_t offset);
	u8 unk_201b_r(offs_t offset);
	u8 gun_x_r(offs_t offset);
	u8 gun_y_r(offs_t offset);
	u8 gun2_x_r(offs_t offset);
	u8 gun2_y_r(offs_t offset);

	void extended_modes_enable_w(offs_t offset, u8 data);
	void extended_modes2_enable_w(offs_t offset, u8 data);
	void videobank0_0_w(offs_t offset, u8 data);
	void videobank0_1_w(offs_t offset, u8 data);
	void videobank0_2_w(offs_t offset, u8 data);
	void videobank0_3_w(offs_t offset, u8 data);
	void videobank0_4_w(offs_t offset, u8 data);
	void videobank0_5_w(offs_t offset, u8 data);
	void videobank1_w(offs_t offset, u8 data);
	void gun_reset_w(offs_t offset, u8 data);
	void videobank0_extra_w(offs_t offset, u8 data);

	virtual u8 palette_read(offs_t offset) override;
	virtual void palette_write(offs_t offset, u8 data) override;

	u8 get_videobank0_reg(int reg) { return m_videobank0[reg]; }
	void set_videobank0_reg(int reg, u8 data) { m_videobank0[reg] = data; }
	u8 get_videobank1() { return m_videobank1; }
	u8 get_videobank0_extra() { return m_videobank0_extra; }
	u8 get_extended_modes_enable() { return m_extended_modes_enable; }
	u8 get_extended_modes2_enable() { return m_extended_modes2_enable; }

	u8 get_newvid_1c() { return m_newvid_1c; }
	u8 get_newvid_1d() { return m_newvid_1d; }
	bool is_v3xx_extended_mode() { return (m_newvid_1e == 0x00) ? false : true; }

	u16 get_newmode_tilebase() { return m_tilebases_2x[0] | (m_tilebases_2x[1] << 8); }
	u16 get_newmode_spritebase() { return m_tilebases_2x[2] | (m_tilebases_2x[3] << 8); }
	u8 vt3xx_extended_palette_r(offs_t offset) { return m_vt3xx_palette[offset]; }
	void vt3xx_extended_palette_w(offs_t offset, u8 data) { /*logerror("%s: extended palette write %04x %02x\n", machine().describe_context(), offset, data);*/ m_vt3xx_palette[offset] = data; }

	u8 get_m_read_bg4_bg3();
	u8 get_speva2_speva0();

	bool get_is_pal() { return m_is_pal; }
	bool get_is_50hz() { return m_is_50hz; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u32 palette_entries() const noexcept override { return (0x40 * 8) + (0x1000 * 8); }

	virtual void read_tile_plane_data(int address, int color) override;
	virtual void shift_tile_plane_data(u8 &pix) override;
	virtual void draw_tile_pixel(u8 pix, int color, u32 back_pen, u32 *&dest) override;
	inline void draw_tile_pixel_inner(u8 pen, u32 *dest);
	virtual void draw_back_pen(u32* dst, int back_pen) override;

	virtual void read_sprite_plane_data(int address) override;
	virtual void make_sprite_pixel_data(u8 &pixel_data, bool flipx) override;
	virtual void draw_sprite_pixel(int sprite_xpos, int color, int pixel, u8 pixel_data, bitmap_rgb32 &bitmap) override;
	virtual void read_extra_sprite_bits(int sprite_index) override;

	bool m_is_pal;
	bool m_is_50hz;

	u32 m_vtpens_rgb555[0x8000*8];
	u32 m_vtpens_rgb444[0x1000*8];

	u8 m_videobank0[6];
	u8 m_videobank0_extra;
	u8 m_videobank1;
	u8 m_extended_modes_enable;
	u8 m_extended_modes2_enable;

	devcb_read8 m_read_bg;
	devcb_read8 m_read_sp;
	devcb_read8 m_read_onespace;
	devcb_read8 m_read_onespace_with_relative;

	int32_t m_read_bg4_bg3;

	int m_whichpixel;

	u8 m_newvid_1b;
	u8 m_newvid_1c;
	u8 m_newvid_1d;
	u8 m_newvid_1e;
	u8 m_tilebases_2x[4];

	u8 m_vt3xx_palette[0x400];

	static constexpr unsigned YUV444_COLOR = (0x40 * 8);
private:

	u8 m_extra_sprite_bits;

	vtxx_pal_mode m_pal_mode = PAL_MODE_VT0x;

	void init_vt03_palette_tables(int palmode);
	void init_vtxx_rgb555_palette_tables();
	void init_vtxx_rgb444_palette_tables();

};

class ppu_vt03pal_device : public ppu_vt03_device {
public:
	ppu_vt03pal_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);
};

class ppu_vt32_device : public ppu_vt03_device {
public:
	ppu_vt32_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);
	ppu_vt32_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	void m_newvid_1b_w(u8 data);
	void m_newvid_1c_w(u8 data);
	void m_newvid_1d_w(u8 data);

private:
	virtual void draw_background(u8 *line_priority) override;
};

class ppu_vt32pal_device : public ppu_vt32_device {
public:
	ppu_vt32pal_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);
};


class ppu_vt3xx_device : public ppu_vt03_device {
public:
	ppu_vt3xx_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock);

	u8 extvidreg_201c_r(offs_t offset);
	u8 extvidreg_201d_r(offs_t offset);
	u8 extvidreg_201e_r(offs_t offset);
	u8 tilebases_202x_r(offs_t offset);

	void extvidreg_201c_w(offs_t offset, u8 data);
	void extvidreg_201d_w(offs_t offset, u8 data);
	void extvidreg_201e_w(offs_t offset, u8 data);
	void tilebases_202x_w(offs_t offset, u8 data);
	void lcdc_regs_w(offs_t offset, u8 data);

	u8 spritehigh_2008_r() { return m_2008_spritehigh; }
	void spritehigh_2008_w(u8 data) { m_2008_spritehigh = data; logerror("%s: spritehigh_2008_w %02x\n", machine().describe_context(), data); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	virtual void read_tile_plane_data(int address, int color) override;
	virtual void shift_tile_plane_data(u8& pix) override;
	virtual void draw_sprites(u8 *line_priority) override;
	u8 vt3xx_palette_r(offs_t offset);
	void vt3xx_palette_w(offs_t offset, u8 data);
	virtual void write_to_spriteram_with_increment(u8 data) override;
	u8 get_pixel_data(u8* spritepatternbuf, int bpp, int pixel);
	rgb_t get_pen_value(int pixel_data, int bpp, int pal);

	offs_t recalculate_offsets_8x8x4packed_tile(int address, int va34);
	offs_t recalculate_offsets_8x8x8packed_tile(int address, int va34);
	offs_t recalculate_offsets_16x16x8packed_hires_tile(int address, int va34);

	void draw_sprites_standard_res(u8* line_priority);
	void draw_sprites_high_res(u8* line_priority);

	void draw_extended_sprite_pixel_low(bitmap_rgb32& bitmap, int pixel_data, int pixel, int xpos, int pal, int bpp, u8* line_priority);
	void draw_extended_sprite_pixel_high(bitmap_rgb32& bitmap, int pixel_data, int pixel, int xpos, int pal, int bpp, u8* line_priority);

	u8 m_204x_screenregs[0xa];
	u8 m_2008_spritehigh;
};


DECLARE_DEVICE_TYPE(PPU_VT03,    ppu_vt03_device)
DECLARE_DEVICE_TYPE(PPU_VT03PAL, ppu_vt03pal_device)

DECLARE_DEVICE_TYPE(PPU_VT32, ppu_vt32_device)
DECLARE_DEVICE_TYPE(PPU_VT32PAL, ppu_vt32pal_device)

DECLARE_DEVICE_TYPE(PPU_VT3XX,    ppu_vt3xx_device)


#endif // MAME_VIDEO_PPU2C0X_VT_H
