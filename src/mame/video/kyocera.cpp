// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"
#include "includes/kyocera.h"

#include "screen.h"


void kc85_state::kc85_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

void tandy200_state::tandy200_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

uint32_t kc85_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (uint8_t i = 0; i < 10; i++)
		m_lcdc[i]->screen_update(screen, bitmap, cliprect);

	return 0;
}

uint32_t tandy200_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_lcdc->screen_update(screen, bitmap, cliprect);

	return 0;
}

void tandy200_state::tandy200_lcdc(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x1fff).ram();
}

void kc85_state::kc85_video(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_LCD));
	screen.set_refresh_hz(44);
	screen.set_screen_update(FUNC(kc85_state::screen_update));
	screen.set_size(240, 64);
	screen.set_visarea(0, 240-1, 0, 64-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(kc85_state::kc85_palette), 2);

	HD44102(config, m_lcdc[0], 0, SCREEN_TAG,   0,  0);
	HD44102(config, m_lcdc[1], 0, SCREEN_TAG,  50,  0);
	HD44102(config, m_lcdc[2], 0, SCREEN_TAG, 100,  0);
	HD44102(config, m_lcdc[3], 0, SCREEN_TAG, 150,  0);
	HD44102(config, m_lcdc[4], 0, SCREEN_TAG, 200,  0);
	HD44102(config, m_lcdc[5], 0, SCREEN_TAG,   0, 32);
	HD44102(config, m_lcdc[6], 0, SCREEN_TAG,  50, 32);
	HD44102(config, m_lcdc[7], 0, SCREEN_TAG, 100, 32);
	HD44102(config, m_lcdc[8], 0, SCREEN_TAG, 150, 32);
	HD44102(config, m_lcdc[9], 0, SCREEN_TAG, 200, 32);

//  MCFG_HD44103_MASTER_ADD("m11", SCREEN_TAG, CAP_P(18), RES_K(100), HD44103_FS_HIGH, HD44103_DUTY_1_32)
//  MCFG_HD44103_SLAVE_ADD( "m12", "m11", SCREEN_TAG, HD44103_FS_HIGH, HD44103_DUTY_1_32)
}

void tandy200_state::tandy200_video(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_LCD));
	screen.set_refresh_hz(80);
	screen.set_screen_update(FUNC(tandy200_state::screen_update));
	screen.set_size(240, 128);
	screen.set_visarea(0, 240-1, 0, 128-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(tandy200_state::tandy200_palette), 2);

	HD61830(config, m_lcdc, XTAL(4'915'200)/2/2);
	m_lcdc->set_addrmap(0, &tandy200_state::tandy200_lcdc);
	m_lcdc->set_screen(SCREEN_TAG);
}
