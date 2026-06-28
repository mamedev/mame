// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay,Brice Onken

// Sony NEWS laptop graphics subsystem

#include "emu.h"
#include "news_lcdfb.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NEWS_LCD,  news_lcd_device,  "news_lcd",  "Sony NEWS Laptop LCD Subsystem")


news_lcd_device::news_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NEWS_LCD, tag, owner, clock),
	m_lcd(*this, "lcd"),
	m_vram(*this, finder_base::DUMMY_TAG)
{
}

void news_lcd_device::map_lctc(address_map &map)
{
	map(0x0, 0x1f).rw(FUNC(news_lcd_device::lctc_r), FUNC(news_lcd_device::lctc_w));
}

void news_lcd_device::lcd_enable_w(u8 data)
{
	LOG("(%s) lcd_enable_w(%02x)\n", machine().describe_context(), data);
	m_lcd_enable = bool(data);
}

void news_lcd_device::lcd_dim_w(u8 data)
{
	LOG("(%s) lcd_dim_w(%02x)\n", machine().describe_context(), data);
	m_lcd_dim = BIT(data, 0);
}

void news_lcd_device::device_start()
{
	save_item(NAME(m_lcd_enable));
	save_item(NAME(m_lcd_dim));

	m_lcd_enable = false;
	m_lcd_dim = false;
}

void news_lcd_device::device_add_mconfig(machine_config &config)
{
	// LCD panel
	SCREEN(config, m_lcd, SCREEN_TYPE_LCD);
	m_lcd->set_raw(52416000, 1120, 0, 1120, 780, 0, 780);
	m_lcd->set_screen_update(FUNC(news_lcd_device::screen_update));

	// TODO: Hitachi HD64646FS LCD Timing Controller (software-compatible with Hitachi's CRTC)
}

u8 news_lcd_device::lctc_r(offs_t offset)
{
	LOG("(%s) lctc_r(0x%x)\n", machine().describe_context(), offset);
	return 0x0;
}

void news_lcd_device::lctc_w(offs_t offset, u8 data)
{
	LOG("(%s) lctc_w(0x%x, 0x%x)\n", machine().describe_context(), offset, data);
}

u32 news_lcd_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, [[maybe_unused]] const rectangle &cliprect)
{
	if (!m_lcd_enable)
	{
		return 0;
	}

	rgb_t constexpr black = rgb_t::black();
	rgb_t const white = m_lcd_dim ? rgb_t(191, 191, 191) : rgb_t::white();

	u32 const *pixel_pointer = m_vram;

	for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
	{
		for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 32)
		{
			u32 const pixel_data = *pixel_pointer++;

			for (int i = 0; i < 32; i++)
			{
				bitmap.pix(y, x + i) = BIT(pixel_data, 31 - i) ? black : white;
			}
		}
	}

	return 0;
}
