// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************

    Generic LCD emulation for use with MC68328/MC68EZ328 devices

******************************************************************************/

#ifndef MAME_VIDEO_MC68328LCD_H
#define MAME_VIDEO_MC68328LCD_H

#pragma once

#include "screen.h"
#include "emupal.h"

class mc68328_lcd_device : public device_t
{
public:
	mc68328_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	mc68328_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 screen_width, u32 screen_height)
		: mc68328_lcd_device(mconfig, tag, owner, clock)
	{
		m_screen_width = screen_width;
		m_screen_height = screen_height;
	}

	DECLARE_WRITE_LINE_MEMBER(flm_w);
	DECLARE_WRITE_LINE_MEMBER(llp_w);
	DECLARE_WRITE_LINE_MEMBER(lsclk_w);
	void ld_w(u8 data);
	void lcd_info_changed(double refresh_hz, int width, int height, u8 bus_width, u8 bpp);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void init_palette(palette_device &palette) const;

	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	bitmap_rgb32 m_lcd_bitmap;
	bool m_lcd_first_line;
	bool m_lcd_line_pulse;
	bool m_lcd_shift_clk;
	u8 m_lcd_data;
	u16 m_lcd_scan_x;
	u16 m_lcd_scan_y;
	u8 m_bus_width;
	u8 m_bpp;
	u32 m_screen_width;
	u32 m_screen_height;
};

// device type definition
DECLARE_DEVICE_TYPE(MC68328_LCD_SCREEN, mc68328_lcd_device)

#endif // MAME_VIDEO_MC68328LCD_H
