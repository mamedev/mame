// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Pierpaolo Prazzoli
/*****************************************************************************************

 Speed Attack video hardware emulation

*****************************************************************************************/
#include "emu.h"
#include "includes/speedatk.h"


void speedatk_state::speedatk_palette(palette_device &palette) const
{
	uint8_t const *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x10; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x20;

	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}

void speedatk_state::video_start()
{
	save_item(NAME(m_crtc_vreg));
	save_item(NAME(m_crtc_index));
	save_item(NAME(m_flip_scr));
}

WRITE8_MEMBER(speedatk_state::m6845_w)
{
	if(offset == 0)
	{
		m_crtc_index = data;
		m_crtc->address_w(data);
	}
	else
	{
		m_crtc_vreg[m_crtc_index] = data;
		m_crtc->register_w(data);
	}
}

uint32_t speedatk_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;
	int count;
	uint16_t tile;
	uint8_t color, region;

	bitmap.fill(0, cliprect);

	count = (m_crtc_vreg[0x0c]<<8)|(m_crtc_vreg[0x0d] & 0xff);

	if(m_flip_scr) { count = 0x3ff - count; }

	for(y=0;y<m_crtc_vreg[6];y++)
	{
		for(x=0;x<m_crtc_vreg[1];x++)
		{
			tile = m_videoram[count] + ((m_colorram[count] & 0xe0) << 3);
			color = m_colorram[count] & 0x1f;
			region = (m_colorram[count] & 0x10) >> 4;

			m_gfxdecode->gfx(region)->opaque(bitmap,cliprect,tile,color,m_flip_scr,m_flip_scr,x*8,y*8);

			count = (m_flip_scr) ? count-1 : count+1;
			count&=0x3ff;
		}
	}

	return 0;
}
