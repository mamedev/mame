// license:BSD-3-Clause
// copyright-holders:JJ Stacino
/////////////////////////////////////////////////////////////////////////////////
///// Hector video
/////////////////////////////////////////////////////////////////////////////////


#include "emu.h"
#include "hec2hrp.h"

#include "screen.h"


void hec2hrp_state::init_palette(palette_device &palette)
{
	m_hector_color[0] = 0; // black
	m_hector_color[1] = 1; // red
	m_hector_color[2] = 7; // white
	m_hector_color[3] = 3; // yellow

	// Full brightness
	m_palette->set_pen_color( 0,rgb_t(000,000,000)); // black
	m_palette->set_pen_color( 1,rgb_t(255,000,000)); // red
	m_palette->set_pen_color( 2,rgb_t(000,255,000)); // green
	m_palette->set_pen_color( 3,rgb_t(255,255,000)); // yellow
	m_palette->set_pen_color( 4,rgb_t(000,000,255)); // blue
	m_palette->set_pen_color( 5,rgb_t(255,000,255)); // magenta
	m_palette->set_pen_color( 6,rgb_t(000,255,255)); // cyan
	m_palette->set_pen_color( 7,rgb_t(255,255,255)); // white

	// Half brightness
	m_palette->set_pen_color( 8,rgb_t(000,000,000));  // black
	m_palette->set_pen_color( 9,rgb_t(128,000,000));  // red
	m_palette->set_pen_color( 10,rgb_t(000,128,000)); // green
	m_palette->set_pen_color( 11,rgb_t(128,128,000)); // yellow
	m_palette->set_pen_color( 12,rgb_t(000,000,128)); // blue
	m_palette->set_pen_color( 13,rgb_t(128,000,128)); // magenta
	m_palette->set_pen_color( 14,rgb_t(000,128,128)); // cyan
	m_palette->set_pen_color( 15,rgb_t(128,128,128)); // white
}

void hec2hrp_state::hector_hr(bitmap_ind16 &bitmap, uint8_t *page, int ymax, int yram)
{
	int sy = 0;
	int ma = 0;
	for (int y = 0; y <= ymax; y++)
	{
		uint16_t *pix = &bitmap.pix(sy++);
		for (int x = ma; x < ma + yram; x++)
		{
			uint8_t gfx = *(page + x);
			*pix++ = m_hector_color[BIT(gfx, 0, 2)];
			*pix++ = m_hector_color[BIT(gfx, 2, 2)];
			*pix++ = m_hector_color[BIT(gfx, 4, 2)];
			*pix++ = m_hector_color[BIT(gfx, 6, 2)];
		}
		ma+=yram;
	}
}

uint32_t hec2hrp_state::screen_update_interact(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.set_visible_area(0, 113, 0, 75);
	hector_hr(bitmap, m_vram, 77, 32);
	return 0;
}

void hec2hrp_state::hector_80c(bitmap_ind16 &bitmap, uint8_t *page, int ymax, int yram)
{
	int sy = 0;
	int ma = 0;
	for (int y = 0; y <= ymax; y++)
	{
		uint16_t *pix = &bitmap.pix(sy++);
		for (int x = ma; x < ma + yram; x++)
		{
			uint8_t gfx = *(page + x);
			for (u8 i = 0; i < 8; i++)
				*pix++ = BIT(gfx, i) ? 7 : 0;
		}
		ma += yram;
	}
}

uint32_t hec2hrp_state::screen_update_hec2hrp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_hector_flag_hr)
	{
		if (!m_hector_flag_80c)
		{
			screen.set_visible_area(0, 243, 0, 227);
			hector_hr(bitmap , m_hector_vram, 227, 64);
		}
		else
		{
			screen.set_visible_area(0, 243*2, 0, 227);
			hector_80c(bitmap , m_hector_vram, 227, 64);
		}
	}
	else
	{
		screen.set_visible_area(0, 113, 0, 75);
		hector_hr(bitmap, m_vram, 77, 32);
	}
	return 0;
}

