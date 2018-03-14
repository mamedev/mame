// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"
#include "includes/kyocera.h"

#include "rendlay.h"
#include "screen.h"


PALETTE_INIT_MEMBER(kc85_state,kc85)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

PALETTE_INIT_MEMBER(tandy200_state,tandy200)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

uint32_t kc85_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_lcdc0->screen_update(screen, bitmap, cliprect);
	m_lcdc1->screen_update(screen, bitmap, cliprect);
	m_lcdc2->screen_update(screen, bitmap, cliprect);
	m_lcdc3->screen_update(screen, bitmap, cliprect);
	m_lcdc4->screen_update(screen, bitmap, cliprect);
	m_lcdc5->screen_update(screen, bitmap, cliprect);
	m_lcdc6->screen_update(screen, bitmap, cliprect);
	m_lcdc7->screen_update(screen, bitmap, cliprect);
	m_lcdc8->screen_update(screen, bitmap, cliprect);
	m_lcdc9->screen_update(screen, bitmap, cliprect);

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

MACHINE_CONFIG_START(kc85_state::kc85_video)
	MCFG_SCREEN_ADD(SCREEN_TAG, LCD)
	MCFG_SCREEN_REFRESH_RATE(44)
	MCFG_SCREEN_UPDATE_DRIVER(kc85_state, screen_update)
	MCFG_SCREEN_SIZE(240, 64)
	MCFG_SCREEN_VISIBLE_AREA(0, 240-1, 0, 64-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(kc85_state,kc85)

	MCFG_HD44102_ADD(HD44102_0_TAG, SCREEN_TAG,   0,  0)
	MCFG_HD44102_ADD(HD44102_1_TAG, SCREEN_TAG,  50,  0)
	MCFG_HD44102_ADD(HD44102_2_TAG, SCREEN_TAG, 100,  0)
	MCFG_HD44102_ADD(HD44102_3_TAG, SCREEN_TAG, 150,  0)
	MCFG_HD44102_ADD(HD44102_4_TAG, SCREEN_TAG, 200,  0)
	MCFG_HD44102_ADD(HD44102_5_TAG, SCREEN_TAG,   0, 32)
	MCFG_HD44102_ADD(HD44102_6_TAG, SCREEN_TAG,  50, 32)
	MCFG_HD44102_ADD(HD44102_7_TAG, SCREEN_TAG, 100, 32)
	MCFG_HD44102_ADD(HD44102_8_TAG, SCREEN_TAG, 150, 32)
	MCFG_HD44102_ADD(HD44102_9_TAG, SCREEN_TAG, 200, 32)

//  MCFG_HD44103_MASTER_ADD("m11", SCREEN_TAG, CAP_P(18), RES_K(100), HD44103_FS_HIGH, HD44103_DUTY_1_32)
//  MCFG_HD44103_SLAVE_ADD( "m12", "m11", SCREEN_TAG, HD44103_FS_HIGH, HD44103_DUTY_1_32)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(tandy200_state::tandy200_video)
	MCFG_SCREEN_ADD(SCREEN_TAG, LCD)
	MCFG_SCREEN_REFRESH_RATE(80)
	MCFG_SCREEN_UPDATE_DRIVER(tandy200_state, screen_update)
	MCFG_SCREEN_SIZE(240, 128)
	MCFG_SCREEN_VISIBLE_AREA(0, 240-1, 0, 128-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(tandy200_state,tandy200)

	MCFG_DEVICE_ADD(HD61830_TAG, HD61830, XTAL(4'915'200)/2/2)
	MCFG_DEVICE_ADDRESS_MAP(0, tandy200_lcdc)
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)
MACHINE_CONFIG_END
