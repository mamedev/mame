// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************

    Generic LCD emulation for use with MC68328/MC68EZ328 devices

******************************************************************************/

#include "emu.h"
#include "mc68328lcd.h"

DEFINE_DEVICE_TYPE(MC68328_LCD_SCREEN, mc68328_lcd_device, "mc68328_lcd", "Generic MC68328-compatible LCD Screen")

mc68328_lcd_device::mc68328_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MC68328_LCD_SCREEN, tag, owner, clock)
	, m_screen(*this, "screen")
	, m_palette(*this, "palette")
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

void mc68328_lcd_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(m_screen_width, m_screen_height);
	m_screen->set_visarea(0, m_screen_width - 1, 0, m_screen_height - 1);
	m_screen->set_screen_update(FUNC(mc68328_lcd_device::screen_update));

	PALETTE(config, m_palette, FUNC(mc68328_lcd_device::init_palette), 16);
}

void mc68328_lcd_device::init_palette(palette_device &palette) const
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
		palette.set_pen_color(i, blend_r, blend_g, blend_b);
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
				m_lcd_bitmap.pix(m_lcd_scan_y, m_lcd_scan_x) = m_palette->pen_color(value);
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
	m_screen->set_refresh_hz(refresh_hz);
	m_screen->set_size(width, height);
	m_screen->set_visarea(0, width - 1, 0, height - 1);
	m_lcd_bitmap.resize(width, height);
	m_bus_width = bus_width;
	m_bpp = bpp;
	m_lcd_scan_x = 0;
	m_lcd_scan_y = 0;
}

u32 mc68328_lcd_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->pen_color(0));
	if (m_lcd_bitmap.valid())
	{
		u32 *src = &m_lcd_bitmap.pix(0);
		u32 *dst = &bitmap.pix(0);
		const int word_count = std::min(bitmap.width() * bitmap.height(), m_lcd_bitmap.width() * m_lcd_bitmap.height());
		std::copy_n(src, word_count, dst);
	}
	return 0;
}
