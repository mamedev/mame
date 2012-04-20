/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/tnzs.h"
#include "video/seta001.h"

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

PALETTE_INIT( arknoid2 )
{
	const UINT8 *color_prom = machine.root_device().memregion("proms")->base();
	int i, col;

	for (i = 0; i < machine.total_colors(); i++)
	{
		col = (color_prom[i] << 8) + color_prom[i + 512];
		palette_set_color_rgb(machine, i, pal5bit(col >> 10), pal5bit(col >> 5), pal5bit(col >> 0));
	}
}


SCREEN_UPDATE_IND16( tnzs )
{
	bitmap.fill(0x1f0, cliprect);

	screen.machine().device<seta001_device>("spritegen")->set_fg_yoffsets( -0x12, 0x0e );
	screen.machine().device<seta001_device>("spritegen")->set_bg_yoffsets( 0x1, -0x1 );

	screen.machine().device<seta001_device>("spritegen")->seta001_draw_sprites(screen.machine(), bitmap, cliprect, 0x800, 0 );
	return 0;
}

SCREEN_VBLANK( tnzs )
{
	// rising edge
	if (vblank_on)
		screen.machine().device<seta001_device>("spritegen")->tnzs_eof();
}
