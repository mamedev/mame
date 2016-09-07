// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz
/***************************************************************************

    gba_lcd.h

    File to handle emulation of the video hardware of the Game Boy Advance

    By R. Belmont, Ryan Holtz

***************************************************************************/

#pragma once

#ifndef __GBA_LCD_H__
#define __GBA_LCD_H__

#include "emu.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type GBA_LCD;


//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_GBA_LCD_ADD(_tag) \
		MCFG_DEVICE_ADD(_tag, GBA_LCD, 0)

#define MCFG_GBA_LCD_INT_HBLANK(_devcb) \
	devcb = &gba_lcd_device::set_int_hblank_callback(*device, DEVCB_##_devcb);

#define MCFG_GBA_LCD_INT_VBLANK(_devcb) \
	devcb = &gba_lcd_device::set_int_vblank_callback(*device, DEVCB_##_devcb);

#define MCFG_GBA_LCD_INT_VCOUNT(_devcb) \
	devcb = &gba_lcd_device::set_int_vcount_callback(*device, DEVCB_##_devcb);

#define MCFG_GBA_LCD_DMA_HBLANK(_devcb) \
	devcb = &gba_lcd_device::set_dma_hblank_callback(*device, DEVCB_##_devcb);

#define MCFG_GBA_LCD_DMA_VBLANK(_devcb) \
	devcb = &gba_lcd_device::set_dma_vblank_callback(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

template <unsigned COUNT, unsigned BASE>
class gba_registers
{
protected:
	static constexpr unsigned REG_BASE = BASE;

	// 32-bit Register
	UINT32 &WORD(unsigned x) { return m_regs[(x - REG_BASE) / 4]; }
	const UINT32 &WORD(unsigned x) const { return m_regs[(x - REG_BASE) / 4]; }

	// 16-bit Register, Upper Half-Word
	UINT16 HWHI(unsigned x) const { return UINT16(WORD(x) >> 16); }

	// 16-bit Register, Lower Half-Word
	UINT16 HWLO(unsigned x) const { return UINT16(WORD(x)); }

	UINT32 &WORD_SET(unsigned x, UINT32 y) { return WORD(x) |= y; }
	UINT32 &HWHI_SET(unsigned x, UINT16 y) { return WORD(x) |= UINT32(y) << 16; }
	UINT32 &HWLO_SET(unsigned x, UINT16 y) { return WORD(x) |= UINT32(y); }

	UINT32 &WORD_RESET(unsigned x, UINT32 y) { return WORD(x) &= ~y; }
	UINT32 &HWHI_RESET(unsigned x, UINT16 y) { return WORD(x) &= ~(UINT32(y) << 16); }
	UINT32 &HWLO_RESET(unsigned x, UINT16 y) { return WORD(x) &= ~UINT32(y); }

	UINT32 m_regs[COUNT];
};


class gba_lcd_device
		: public device_t
		, public device_video_interface
		, protected gba_registers<0x060 / 4, 0x000>
{
public:
	gba_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ32_MEMBER(video_r);
	DECLARE_WRITE32_MEMBER(video_w);
	DECLARE_READ32_MEMBER(gba_pram_r);
	DECLARE_WRITE32_MEMBER(gba_pram_w);
	DECLARE_READ32_MEMBER(gba_vram_r);
	DECLARE_WRITE32_MEMBER(gba_vram_w);
	DECLARE_READ32_MEMBER(gba_oam_r);
	DECLARE_WRITE32_MEMBER(gba_oam_w);
	DECLARE_PALETTE_INIT(gba);
	TIMER_CALLBACK_MEMBER(perform_hbl);
	TIMER_CALLBACK_MEMBER(perform_scan);

	template<class _Object> static devcb_base &set_int_hblank_callback(device_t &device, _Object object)
	{
		return downcast<gba_lcd_device &>(device).m_int_hblank_cb.set_callback(object);
	}

	template<class _Object> static devcb_base &set_int_vblank_callback(device_t &device, _Object object)
	{
		return downcast<gba_lcd_device &>(device).m_int_vblank_cb.set_callback(object);
	}

	template<class _Object> static devcb_base &set_int_vcount_callback(device_t &device, _Object object)
	{
		return downcast<gba_lcd_device &>(device).m_int_vcount_cb.set_callback(object);
	}

	template<class _Object> static devcb_base &set_dma_hblank_callback(device_t &device, _Object object)
	{
		return downcast<gba_lcd_device &>(device).m_dma_hblank_cb.set_callback(object);
	}

	template<class _Object> static devcb_base &set_dma_vblank_callback(device_t &device, _Object object)
	{
		return downcast<gba_lcd_device &>(device).m_dma_vblank_cb.set_callback(object);
	}

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	struct internal_reg
	{
		INT32 status;
		bool  update;
	};
	internal_reg m_bg2x, m_bg2y, m_bg3x, m_bg3y;

	UINT8 bg_video_mode();

	enum class dispcnt : UINT16
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

	enum class dispstat : UINT16
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

	enum class bgcnt : UINT16
	{
		mosaic_en     = 0x0040,
		palette_256   = 0x0080,
		wraparound_en = 0x2000
	};
	bool is_set(UINT16 bgxcnt, bgcnt flag);

	UINT8  bg_priority(UINT16 bgxcnt);
	UINT32 bg_char_base(UINT16 bgxcnt);
	UINT32 bg_screen_base(UINT16 bgxcnt);
	void   bg_screen_size(UINT16 bgxcnt, bool text, int &width, int &height);

	enum class size_type
	{
		bg_h = 0,
		bg_v,
		obj_h,
		obj_v
	};
	UINT16 mosaic_size(size_type type);

	enum class sfx : UINT16
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
	UINT8 color_sfx_target(target id);

	UINT16 tile_number(UINT16 vram_data) { return vram_data & 0x03ff; }
	bool   tile_hflip(UINT16 vram_data) { return vram_data & 0x0400; }
	bool   tile_vflip(UINT16 vram_data) { return vram_data & 0x0800; }

	void update_mask(UINT8 *mask, int y);
	void draw_roz_bitmap_scanline(UINT32 *scanline, int ypos, dispcnt bg_enable, UINT32 ctrl, INT32 X, INT32 Y, INT32 PA, INT32 PB, INT32 PC, INT32 PD, internal_reg &currentx, internal_reg &currenty, int depth);
	void draw_roz_scanline(UINT32 *scanline, int ypos, dispcnt bg_enable, UINT32 ctrl, INT32 X, INT32 Y, INT32 PA, INT32 PB, INT32 PC, INT32 PD, internal_reg &currentx, internal_reg &currenty);
	void draw_bg_scanline(UINT32 *scanline, int ypos, dispcnt bg_enable, UINT32 ctrl, UINT32 hofs, UINT32 vofs);
	void draw_oam_window(UINT32 *scanline, int y);
	void draw_oam(UINT32 *scanline, int y);
	void draw_scanline(int y);

	bool is_in_window_h(int x, int window);
	bool is_in_window_v(int y, int window);

	UINT32 alpha_blend(UINT32 color0, UINT32 color1);
	UINT32 increase_brightness(UINT32 color);
	UINT32 decrease_brightness(UINT32 color);

	devcb_write_line m_int_hblank_cb;   /* H-Blank interrupt callback function */
	devcb_write_line m_int_vblank_cb;   /* V-Blank interrupt callback function */
	devcb_write_line m_int_vcount_cb;   /* V-Counter Match interrupt callback function */
	devcb_write_line m_dma_hblank_cb;   /* H-Blank DMA request callback function */
	devcb_write_line m_dma_vblank_cb;   /* V-Blank DMA request callback function */

	std::unique_ptr<UINT32[]> m_pram;
	std::unique_ptr<UINT32[]> m_vram;
	std::unique_ptr<UINT32[]> m_oam;

	emu_timer *m_scan_timer, *m_hbl_timer;

	bitmap_ind16 m_bitmap;

	UINT32 m_scanline[6][240];

	// constants
	static constexpr UINT32 TRANSPARENT_PIXEL = 0x80000000;
};

#endif /* GBA_LCD_H_ */
