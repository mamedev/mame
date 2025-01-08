// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************

    Generic LCD emulation for use with MC68328/MC68EZ328 devices

******************************************************************************/

#ifndef MAME_VIDEO_MC68328LCD_H
#define MAME_VIDEO_MC68328LCD_H

#pragma once

class mc68328_lcd_device : public device_t,
						   public device_palette_interface,
						   public device_video_interface
{
public:
	mc68328_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void flm_w(int state);
	void llp_w(int state);
	void lsclk_w(int state);
	void ld_w(u8 data);
	void lcd_info_changed(double refresh_hz, int width, int height, u8 bus_width, u8 bpp);

	void video_update(bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_palette_interface overrides
	virtual uint32_t palette_entries() const noexcept override { return 16; }

	void palette_init();

	bitmap_rgb32 m_lcd_bitmap;
	bool m_lcd_first_line;
	bool m_lcd_line_pulse;
	bool m_lcd_shift_clk;
	u8 m_lcd_data;
	u16 m_lcd_scan_x;
	u16 m_lcd_scan_y;
	u8 m_bus_width;
	u8 m_bpp;
};

// device type definition
DECLARE_DEVICE_TYPE(MC68328_LCD, mc68328_lcd_device)

#endif // MAME_VIDEO_MC68328LCD_H
