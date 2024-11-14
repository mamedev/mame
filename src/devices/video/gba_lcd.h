// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz
/***************************************************************************

    gba_lcd.h

    File to handle emulation of the video hardware of the Game Boy Advance

    By R. Belmont, Ryan Holtz

***************************************************************************/
#ifndef MAME_VIDEO_GBA_LCD_H
#define MAME_VIDEO_GBA_LCD_H

#pragma once

#include "emupal.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(GBA_LCD, gba_lcd_device)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

template <unsigned COUNT, unsigned BASE>
class gba_registers
{
protected:
	static constexpr unsigned REG_BASE = BASE;

	// 32-bit Register
	uint32_t &WORD(unsigned x) { return m_regs[(x - REG_BASE) / 4]; }
	const uint32_t &WORD(unsigned x) const { return m_regs[(x - REG_BASE) / 4]; }

	// 16-bit Register, Upper Half-Word
	uint16_t HWHI(unsigned x) const { return uint16_t(WORD(x) >> 16); }

	// 16-bit Register, Lower Half-Word
	uint16_t HWLO(unsigned x) const { return uint16_t(WORD(x)); }

	uint32_t &WORD_SET(unsigned x, uint32_t y) { return WORD(x) |= y; }
	uint32_t &HWHI_SET(unsigned x, uint16_t y) { return WORD(x) |= uint32_t(y) << 16; }
	uint32_t &HWLO_SET(unsigned x, uint16_t y) { return WORD(x) |= uint32_t(y); }

	uint32_t &WORD_RESET(unsigned x, uint32_t y) { return WORD(x) &= ~y; }
	uint32_t &HWHI_RESET(unsigned x, uint16_t y) { return WORD(x) &= ~(uint32_t(y) << 16); }
	uint32_t &HWLO_RESET(unsigned x, uint16_t y) { return WORD(x) &= ~uint32_t(y); }

	uint32_t m_regs[COUNT];
};


class gba_lcd_device
		: public device_t
		, public device_video_interface
		, protected gba_registers<0x060 / 4, 0x000>
{
public:
	gba_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t video_r(offs_t offset, uint32_t mem_mask = ~0);
	void video_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t gba_pram_r(offs_t offset);
	void gba_pram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t gba_vram_r(offs_t offset);
	void gba_vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t gba_oam_r(offs_t offset);
	void gba_oam_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	TIMER_CALLBACK_MEMBER(perform_hbl);
	TIMER_CALLBACK_MEMBER(perform_scan);

	auto int_hblank_callback() { return m_int_hblank_cb.bind(); }
	auto int_vblank_callback() { return m_int_vblank_cb.bind(); }
	auto int_vcount_callback() { return m_int_vcount_cb.bind(); }
	auto dma_hblank_callback() { return m_dma_hblank_cb.bind(); }
	auto dma_vblank_callback() { return m_dma_vblank_cb.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	struct internal_reg
	{
		int32_t status;
		bool  update;
	};
	internal_reg m_bg2x, m_bg2y, m_bg3x, m_bg3y;

	uint8_t bg_video_mode();

	enum class dispcnt : uint16_t
	{
		alt_frame_sel = 0x0010,
		vram_map_1d   = 0x0040,
		forced_blank  = 0x0080,
		bg0_en        = 0x0100,
		bg1_en        = 0x0200,
		bg2_en        = 0x0400,
		bg3_en        = 0x0800,
		obj_en        = 0x1000,
		win0_en       = 0x2000,
		win1_en       = 0x4000,
		obj_win_en    = 0x8000
	};
	bool is_set(dispcnt flag);

	enum class dispstat : uint16_t
	{
		vblank        = 0x0001,
		hblank        = 0x0002,
		vcount        = 0x0004,
		vblank_irq_en = 0x0008,
		hblank_irq_en = 0x0010,
		vcount_irq_en = 0x0020
	};
	void set(dispstat flag);
	void clear(dispstat flag);
	bool is_set(dispstat flag);

	enum class bgcnt : uint16_t
	{
		mosaic_en     = 0x0040,
		palette_256   = 0x0080,
		wraparound_en = 0x2000
	};
	bool is_set(uint16_t bgxcnt, bgcnt flag);

	uint8_t  bg_priority(uint16_t bgxcnt);
	uint32_t bg_char_base(uint16_t bgxcnt);
	uint32_t bg_screen_base(uint16_t bgxcnt);
	void   bg_screen_size(uint16_t bgxcnt, bool text, int &width, int &height);

	enum class size_type
	{
		bg_h = 0,
		bg_v,
		obj_h,
		obj_v
	};
	uint16_t mosaic_size(size_type type);

	enum class sfx : uint16_t
	{
		none    = 0x0000,
		alpha   = 0x0040,
		lighten = 0x0080,
		darken  = 0x00c0
	};
	sfx color_sfx();

	enum class target
	{
		first = 0,
		second
	};
	uint8_t color_sfx_target(target id);

	uint16_t tile_number(uint16_t vram_data) { return vram_data & 0x03ff; }
	bool   tile_hflip(uint16_t vram_data) { return vram_data & 0x0400; }
	bool   tile_vflip(uint16_t vram_data) { return vram_data & 0x0800; }

	void update_mask(uint8_t *mask, int y);
	void draw_roz_bitmap_scanline(uint32_t *scanline, int ypos, dispcnt bg_enable, uint32_t ctrl, int32_t X, int32_t Y, int32_t PA, int32_t PB, int32_t PC, int32_t PD, internal_reg &currentx, internal_reg &currenty, int depth);
	void draw_roz_scanline(uint32_t *scanline, int ypos, dispcnt bg_enable, uint32_t ctrl, int32_t X, int32_t Y, int32_t PA, int32_t PB, int32_t PC, int32_t PD, internal_reg &currentx, internal_reg &currenty);
	void draw_bg_scanline(uint32_t *scanline, int ypos, dispcnt bg_enable, uint32_t ctrl, uint32_t hofs, uint32_t vofs);
	void draw_oam_window(uint32_t *scanline, int y);
	void draw_oam(uint32_t *scanline, int y);
	void draw_scanline(int y);

	bool is_in_window_h(int x, int window);
	bool is_in_window_v(int y, int window);

	uint32_t alpha_blend(uint32_t color0, uint32_t color1);
	uint32_t increase_brightness(uint32_t color);
	uint32_t decrease_brightness(uint32_t color);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gba_palette(palette_device &palette) const;

	devcb_write_line m_int_hblank_cb;   /* H-Blank interrupt callback function */
	devcb_write_line m_int_vblank_cb;   /* V-Blank interrupt callback function */
	devcb_write_line m_int_vcount_cb;   /* V-Counter Match interrupt callback function */
	devcb_write_line m_dma_hblank_cb;   /* H-Blank DMA request callback function */
	devcb_write_line m_dma_vblank_cb;   /* V-Blank DMA request callback function */

	std::unique_ptr<uint32_t[]> m_pram;
	std::unique_ptr<uint32_t[]> m_vram;
	std::unique_ptr<uint32_t[]> m_oam;

	emu_timer *m_scan_timer, *m_hbl_timer;

	bitmap_ind16 m_bitmap;

	uint32_t m_scanline[6][240];

	// constants
	static constexpr uint32_t TRANSPARENT_PIXEL = 0x80000000;
};

#endif // MAME_VIDEO_GBA_LCD_H
