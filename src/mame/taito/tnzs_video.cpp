// license:BSD-3-Clause
// copyright-holders:Luca Elia, Mirko Buffoni, Takahiro Nogi
/***************************************************************************

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "tnzs_video.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Arkanoid has a two 512x8 palette PROMs. The two bytes joined together
  form 512 xRRRRRGGGGGBBBBB color values.

***************************************************************************/

void tnzs_video_state_base::prompalette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int const col = (color_prom[i] << 8) + color_prom[i + 512];
		palette.set_pen_color(i, pal5bit(col >> 10), pal5bit(col >> 5), pal5bit(col >> 0));
	}
}


uint32_t tnzs_video_state_base::screen_update_tnzs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x1f0, cliprect);

	m_spritegen->draw_sprites(screen, bitmap, cliprect, 0x800);
	return 0;
}

void tnzs_video_state_base::screen_vblank_tnzs(int state)
{
	// rising edge
	if (state)
		m_spritegen->tnzs_eof();
}
