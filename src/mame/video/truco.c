// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Roberto Fresca
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/truco.h"

PALETTE_INIT_MEMBER(truco_state, truco)
{
	int i;

	for (i = 0;i < palette.entries();i++)
	{
		int r = ( i & 0x8 ) ? 0xff : 0x00;
		int g = ( i & 0x4 ) ? 0xff : 0x00;
		int b = ( i & 0x2 ) ? 0xff : 0x00;

		int dim = ( i & 0x1 );

		if ( dim ) {
			r >>= 1;
			g >>= 1;
			b >>= 1;
		}

		palette.set_pen_color(i,rgb_t(r,g,b));
	}
}

UINT32 truco_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram;
	UINT8       *vid = videoram;
	int x, y;

	for( y = 0; y < 192; y++ )
	{
		for( x = 0; x < 256; x++ )
		{
			int     pixel;

			if ( x & 1 )
				pixel = vid[x>>1] & 0x0f;
			else
				pixel = ( vid[x>>1] >> 4 ) & 0x0f;

			bitmap.pix16(y, x) = pixel;
		}

		vid += 0x80;
	}
	return 0;
}
