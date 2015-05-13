// license:BSD-3-Clause
// copyright-holders:Luca Elia, Mirko Buffoni, Takahiro Nogi
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/tnzs.h"

/***************************************************************************

  The New Zealand Story doesn't have a color PROM. It uses 1024 bytes of RAM
  to dynamically create the palette. Each couple of bytes defines one
  color (15 bits per pixel; the top bit of the second byte is unused).
  Since the graphics use 4 bitplanes, hence 16 colors, this makes for 32
  different color codes.

***************************************************************************/


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Arkanoid has a two 512x8 palette PROMs. The two bytes joined together
  form 512 xRRRRRGGGGGBBBBB color values.

***************************************************************************/

PALETTE_INIT_MEMBER(tnzs_state,arknoid2)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i, col;

	for (i = 0; i < palette.entries(); i++)
	{
		col = (color_prom[i] << 8) + color_prom[i + 512];
		palette.set_pen_color(i, pal5bit(col >> 10), pal5bit(col >> 5), pal5bit(col >> 0));
	}
}


UINT32 tnzs_state::screen_update_tnzs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x1f0, cliprect);

	m_seta001->set_fg_yoffsets( -0x12, 0x0e );
	m_seta001->set_bg_yoffsets( 0x1, -0x1 );

	m_seta001->draw_sprites(screen, bitmap, cliprect, 0x800, 0 );
	return 0;
}

void tnzs_state::screen_eof_tnzs(screen_device &screen, bool state)
{
	// rising edge
	if (state)
		m_seta001->tnzs_eof();
}
