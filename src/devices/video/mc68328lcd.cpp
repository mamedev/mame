// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************

    Generic LCD emulation for use with MC68328/MC68EZ328 devices

******************************************************************************/

#include "emu.h"

#include "mc68328lcd.h"

#include "screen.h"

DEFINE_DEVICE_TYPE(MC68328_LCD, mc68328_lcd_device, "mc68328_lcd", "MC68328-compatible LCD Controller")

mc68328_lcd_device::mc68328_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MC68328_LCD, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
{
}

void mc68328_lcd_device::device_start()
{
	save_item(NAME(m_lcd_first_line));
	save_item(NAME(m_lcd_line_pulse));
	save_item(NAME(m_lcd_shift_clk));
	save_item(NAME(m_lcd_data));
	save_item(NAME(m_lcd_scan_x));
	save_item(NAME(m_lcd_scan_y));
	save_item(NAME(m_bus_width));
	save_item(NAME(m_bpp));

	palette_init();
}

void mc68328_lcd_device::device_reset()
{
	m_lcd_first_line = true;
	m_lcd_line_pulse = false;
	m_lcd_shift_clk = false;
	m_lcd_data = 0;
	m_lcd_scan_x = 0;
	m_lcd_scan_y = 0;
	m_bus_width = 4;
	m_bpp = 1;
}

void mc68328_lcd_device::palette_init()
{
	constexpr u8 LCD_OFF_R = 0xbd;
	constexpr u8 LCD_OFF_G = 0xbd;
	constexpr u8 LCD_OFF_B = 0xaa;
	constexpr u8 LCD_ON_R = 0x40;
	constexpr u8 LCD_ON_G = 0x40;
	constexpr u8 LCD_ON_B = 0x40;
	for (int i = 0; i < 16; i++)
	{
		const float lerp_factor = i / 15.f;
		const u8 blend_r = (u8)(LCD_OFF_R * (1.f - lerp_factor) + LCD_ON_R * lerp_factor);
		const u8 blend_g = (u8)(LCD_OFF_G * (1.f - lerp_factor) + LCD_ON_G * lerp_factor);
		const u8 blend_b = (u8)(LCD_OFF_B * (1.f - lerp_factor) + LCD_ON_B * lerp_factor);
		set_pen_color(i, blend_r, blend_g, blend_b);
	}
}

DECLARE_WRITE_LINE_MEMBER(mc68328_lcd_device::flm_w)
{
	m_lcd_first_line = state;
}

DECLARE_WRITE_LINE_MEMBER(mc68328_lcd_device::llp_w)
{
	const int old = m_lcd_line_pulse;
	m_lcd_line_pulse = state;
	if (!state && old)
	{
		if (m_lcd_first_line)
		{
			m_lcd_scan_y = 0;
		}
		else
		{
			m_lcd_scan_y++;
		}
		m_lcd_scan_x = 0;
	}
}

DECLARE_WRITE_LINE_MEMBER(mc68328_lcd_device::lsclk_w)
{
	const int old = m_lcd_shift_clk;
	m_lcd_shift_clk = state;
	if (state && !old)
	{
		for (u8 i = 0; i < m_bus_width && m_lcd_scan_x < m_lcd_bitmap.width() && m_lcd_scan_y < m_lcd_bitmap.height(); i += m_bpp)
		{
			u8 value = 0;
			switch (m_bpp)
			{
				case 1:
					value = BIT(m_lcd_data, (m_bus_width - 1) - i) * 15;
					break;
				case 2:
					value = ((m_lcd_data >> ((m_bus_width - 2) - i)) & 3) * 5;
					break;
				case 4:
					value = m_lcd_data & 15;
					break;
			}
			if (m_lcd_scan_x < m_lcd_bitmap.width() && m_lcd_scan_y < m_lcd_bitmap.height())
			{
				m_lcd_bitmap.pix(m_lcd_scan_y, m_lcd_scan_x) = pen_color(value);
			}
			m_lcd_scan_x++;
		}
	}
}

void mc68328_lcd_device::ld_w(u8 data)
{
	m_lcd_data = data;
}

void mc68328_lcd_device::lcd_info_changed(double refresh_hz, int width, int height, u8 bus_width, u8 bpp)
{
	if (has_screen())
	{
		screen().set_refresh_hz(refresh_hz);
	}
	m_lcd_bitmap.resize(width, height);
	m_bus_width = bus_width;
	m_bpp = bpp;
	m_lcd_scan_x = 0;
	m_lcd_scan_y = 0;
}

void mc68328_lcd_device::video_update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(pen_color(0));
	if (m_lcd_bitmap.valid())
	{
		u32 *src = &m_lcd_bitmap.pix(0);
		u32 *dst = &bitmap.pix(0);
		const int word_count = std::min(bitmap.width() * bitmap.height(), m_lcd_bitmap.width() * m_lcd_bitmap.height());
		std::copy_n(src, word_count, dst);
	}
}
