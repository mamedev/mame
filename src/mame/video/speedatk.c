// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Pierpaolo Prazzoli
/*****************************************************************************************

 Speed Attack video hardware emulation

*****************************************************************************************/
#include "emu.h"
#include "includes/speedatk.h"


PALETTE_INIT_MEMBER(speedatk_state, speedatk)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x10; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
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
		m_crtc->address_w(space,0,data);
	}
	else
	{
		m_crtc_vreg[m_crtc_index] = data;
		m_crtc->register_w(space,0,data);
	}
}

UINT32 speedatk_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;
	int count;
	UINT16 tile;
	UINT8 color, region;

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
