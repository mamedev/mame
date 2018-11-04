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

#define MCFG_PPU_VT03_ADD(_tag)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_VT03)

#define MCFG_PPU_VT03_READ_BG_CB(_devcb) \
	devcb = &ppu_vt03_device::set_read_bg_callback(*device, DEVCB_##_devcb);

#define MCFG_PPU_VT03_READ_SP_CB(_devcb) \
	devcb = &ppu_vt03_device::set_read_sp_callback(*device, DEVCB_##_devcb);

#define MCFG_PPU_VT03_MODIFY MCFG_DEVICE_MODIFY

#define MCFG_PPU_VT03_SET_PAL_MODE(pmode) \
	ppu_vt03_device::set_palette_mode(*device, pmode);

#define MCFG_PPU_VT03_SET_DESCRAMBLE(dsc) \
	ppu_vt03_device::set_201x_descramble(*device, dsc);


enum vtxx_pal_mode {
	PAL_MODE_VT0x,
	PAL_MODE_NEW_RGB,
	PAL_MODE_NEW_VG,
	PAL_MODE_NEW_RGB12,
};

class ppu_vt03_device : public ppu2c0x_device {
public:
	ppu_vt03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> static devcb_base &set_read_bg_callback(device_t &device, Object &&cb) { return downcast<ppu_vt03_device &>(device).m_read_bg.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_read_sp_callback(device_t &device, Object &&cb) { return downcast<ppu_vt03_device &>(device).m_read_sp.set_callback(std::forward<Object>(cb)); }

	static void set_palette_mode(device_t &device, vtxx_pal_mode pmode) { downcast<ppu_vt03_device &>(device).m_pal_mode = pmode; }
	static void set_201x_descramble(device_t &device, const uint8_t descramble[6]) { for (int i = 0; i < 6; i++) downcast<ppu_vt03_device &>(device).m_2012_2017_descramble[i] = descramble[i]; }

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;
	virtual DECLARE_READ8_MEMBER(palette_read) override;
	virtual DECLARE_WRITE8_MEMBER(palette_write) override;

	virtual void init_palette( palette_device &palette, int first_entry ) override;

	virtual void read_tile_plane_data(int address, int color) override;
	virtual void shift_tile_plane_data(uint8_t &pix) override;
	virtual void draw_tile_pixel(uint8_t pix, int color, uint16_t back_pen, uint16_t *&dest, const pen_t *color_table) override;

	virtual void read_sprite_plane_data(int address) override;
	virtual void make_sprite_pixel_data(uint8_t &pixel_data, int flipx) override;
	virtual void draw_sprite_pixel(int sprite_xpos, int color, int pixel, uint8_t pixel_data, bitmap_ind16& bitmap) override;
	virtual void read_extra_sprite_bits(int sprite_index) override;

	virtual void device_start() override;
	virtual void device_reset() override;

	void set_201x_reg(int reg, uint8_t data);
	uint8_t get_201x_reg(int reg);

	uint8_t get_va34();
	uint8_t get_m_read_bg4_bg3();
	uint8_t get_speva2_speva0();

private:
	devcb_read8 m_read_bg;
	devcb_read8 m_read_sp;

	std::unique_ptr<uint8_t[]> m_newpal;

	int m_read_bg4_bg3;
	int m_va34;

	uint8_t m_extplanebuf[2];
	uint8_t m_extra_sprite_bits;

	palette_device *m_palette;

	uint8_t m_201x_regs[0x20];
	
	uint8_t m_2012_2017_descramble[0x6];
	
	vtxx_pal_mode m_pal_mode = PAL_MODE_VT0x;

	void set_2010_reg(uint8_t data);

	void set_new_pen(int i);
};

DECLARE_DEVICE_TYPE(PPU_VT03,    ppu_vt03_device)

#endif // MAME_VIDEO_PPU_VT03_H
