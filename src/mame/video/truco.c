/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/truco.h"

void truco_state::palette_init()
{
	int i;

	for (i = 0;i < machine().total_colors();i++)
	{
		int	r = ( i & 0x8 ) ? 0xff : 0x00;
		int g = ( i & 0x4 ) ? 0xff : 0x00;
		int b = ( i & 0x2 ) ? 0xff : 0x00;

		int dim = ( i & 0x1 );

		if ( dim ) {
			r >>= 1;
			g >>= 1;
			b >>= 1;
		}

		palette_set_color(machine(),i,MAKE_RGB(r,g,b));
	}
}

SCREEN_UPDATE_IND16( truco )
{
	truco_state *state = screen.machine().driver_data<truco_state>();
	UINT8 *videoram = state->m_videoram;
	UINT8		*vid = videoram;
	int x, y;

	for( y = 0; y < 192; y++ )
	{
		for( x = 0; x < 256; x++ )
		{
			int		pixel;

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
