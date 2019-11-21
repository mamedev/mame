// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"
#include "includes/pc8401a.h"
#include "rendlay.h"
#include "pc8500.lh"

/* PC-8401A */

void pc8401a_state::pc8401a_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(39, 108, 51));
	palette.set_pen_color(1, rgb_t(16, 37, 84));
}

void pc8401a_state::video_start()
{
}

/* PC-8500 */

void pc8500_state::video_start()
{
}

uint32_t pc8500_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  m_lcdc->screen_update(screen, bitmap, cliprect);

	/*
	if (strcmp(screen.tag(), SCREEN_TAG) == 0)
	{
	    sed1330_update(m_lcdc, &bitmap, cliprect);
	}
	else
	{
	    m_crtc->update(bitmap, cliprect);
	}
	*/

	return 0;
}

/* SED1330 Interface */

void pc8401a_state::pc8401a_lcdc(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x1fff).ram();
}

void pc8401a_state::pc8500_lcdc(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x3fff).ram();
}


/* Machine Drivers */

void pc8401a_state::pc8401a_video(machine_config &config)
{
//  config.set_default_layout(layout_pc8401a);

	PALETTE(config, "palette", FUNC(pc8401a_state::pc8401a_palette), 2);

	/* LCD */
	SCREEN(config, m_screen_lcd, SCREEN_TYPE_LCD);
	m_screen_lcd->set_refresh_hz(44);
	m_screen_lcd->set_screen_update(SED1330_TAG, FUNC(sed1330_device::screen_update));
	m_screen_lcd->set_size(480, 128);
	m_screen_lcd->set_visarea(0, 480-1, 0, 128-1);
	m_screen_lcd->set_palette("palette");

	SED1330(config, m_lcdc, 7.987_MHz_XTAL);
	m_lcdc->set_screen(m_screen_lcd);
	m_lcdc->set_addrmap(0, &pc8401a_state::pc8401a_lcdc);
}

void pc8500_state::pc8500_video(machine_config &config)
{
	config.set_default_layout(layout_pc8500);

	PALETTE(config, "palette", FUNC(pc8500_state::pc8401a_palette), 2 + 8);

	/* LCD */
	SCREEN(config, m_screen_lcd, SCREEN_TYPE_LCD);
	m_screen_lcd->set_refresh_hz(44);
	m_screen_lcd->set_screen_update(SED1330_TAG, FUNC(sed1330_device::screen_update));
	m_screen_lcd->set_size(480, 208);
	m_screen_lcd->set_visarea(0, 480-1, 0, 200-1);
	m_screen_lcd->set_palette("palette");

	SED1330(config, m_lcdc, 8000000);
	m_lcdc->set_screen(SCREEN_TAG);
	m_lcdc->set_addrmap(0, &pc8500_state::pc8500_lcdc);

	/* PC-8441A CRT */
	screen_device &screen(SCREEN(config, CRT_SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(pc8500_state::screen_update));
	screen.set_size(80*8, 24*8);
	screen.set_visarea(0, 80*8-1, 0, 24*8-1);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_refresh_hz(50);
	screen.set_palette("palette");

	MC6845(config, m_crtc, 400000);
	m_crtc->set_screen(CRT_SCREEN_TAG);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(6);
}
