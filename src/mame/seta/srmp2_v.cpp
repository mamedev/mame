// license:BSD-3-Clause
// copyright-holders:Yochizo, Takahiro Nogi
/***************************************************************************

Functions to emulate the video hardware of the machine.

***************************************************************************/


#include "emu.h"
#include "srmp2.h"

void srmp2_state::srmp2_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < palette.entries(); i++)
	{
		int const col = (color_prom[i] << 8) + color_prom[i + palette.entries()];
		palette.set_pen_color(i ^ 0x0f, pal5bit(col >> 10), pal5bit(col >> 5), pal5bit(col >> 0));
	}
}


void srmp2_state::srmp3_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < palette.entries(); i++)
	{
		int const col = (color_prom[i] << 8) + color_prom[i + palette.entries()];
		palette.set_pen_color(i,pal5bit(col >> 10), pal5bit(col >> 5), pal5bit(col >> 0));
	}
}

SETA001_SPRITE_GFXBANK_CB_MEMBER(srmp2_state::srmp3_gfxbank_callback)
{
	return (code & 0x3fff) + ((code & 0x2000) ? (m_gfx_bank<<13) : 0);
}


uint32_t srmp2_state::screen_update_srmp2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x1ff, cliprect);

	m_seta001->set_colorbase(m_color_bank<<5);

	m_seta001->draw_sprites(screen,bitmap,cliprect,0x1000);
	return 0;
}

uint32_t srmp2_state::screen_update_srmp3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x1f0, cliprect);

	m_seta001->draw_sprites(screen,bitmap,cliprect,0x1000);
	return 0;
}
