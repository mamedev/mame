// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"
#include "includes/pc8401a.h"
#include "rendlay.h"
#include "pc8500.lh"

/* PC-8401A */

PALETTE_INIT_MEMBER(pc8401a_state,pc8401a)
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

MACHINE_CONFIG_START(pc8401a_state::pc8401a_video)
//  config.set_default_layout(layout_pc8401a);

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(pc8401a_state,pc8401a)

	/* LCD */
	MCFG_SCREEN_ADD(SCREEN_TAG, LCD)
	MCFG_SCREEN_REFRESH_RATE(44)
	MCFG_SCREEN_UPDATE_DEVICE(SED1330_TAG, sed1330_device, screen_update)
	MCFG_SCREEN_SIZE(480, 128)
	MCFG_SCREEN_VISIBLE_AREA(0, 480-1, 0, 128-1)
	MCFG_SCREEN_PALETTE("palette")

	SED1330(config, m_lcdc, 0);
	m_lcdc->set_screen(SCREEN_TAG);
	m_lcdc->set_addrmap(0, &pc8401a_state::pc8401a_lcdc);
MACHINE_CONFIG_END

MACHINE_CONFIG_START(pc8500_state::pc8500_video)
	config.set_default_layout(layout_pc8500);

	MCFG_PALETTE_ADD("palette", 2+8)
	MCFG_PALETTE_INIT_OWNER(pc8401a_state,pc8401a)

	/* LCD */
	MCFG_SCREEN_ADD(SCREEN_TAG, LCD)
	MCFG_SCREEN_REFRESH_RATE(44)
	MCFG_SCREEN_UPDATE_DEVICE(SED1330_TAG, sed1330_device, screen_update)
	MCFG_SCREEN_SIZE(480, 208)
	MCFG_SCREEN_VISIBLE_AREA(0, 480-1, 0, 200-1)
	MCFG_SCREEN_PALETTE("palette")

	SED1330(config, m_lcdc, 0);
	m_lcdc->set_screen(SCREEN_TAG);
	m_lcdc->set_addrmap(0, &pc8500_state::pc8500_lcdc);

	/* PC-8441A CRT */
	MCFG_SCREEN_ADD(CRT_SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(pc8500_state, screen_update)
	MCFG_SCREEN_SIZE(80*8, 24*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*8-1, 0, 24*8-1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_PALETTE("palette")

	MC6845(config, m_crtc, 400000);
	m_crtc->set_screen(CRT_SCREEN_TAG);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(6);
MACHINE_CONFIG_END
