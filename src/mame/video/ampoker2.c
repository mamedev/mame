/********************************************************************************


    AMERICAN POKER 2 - VIDEO HARDWARE
    ---------------------------------

    Company:    Novomatic.
    Year:       1990.

    Original preliminary driver:   Curt Coder.
    Rewrite and additional work:   Roberto Fresca, with a lot of help of Grull Osgo.


    --- Supported Sets ---

    Old name | New name | Relation | Description
    --------------------------------------------
    ampokr2b   ampoker2   parent     American Poker II.
      ----     ampkr2b1   clone      American Poker II (bootleg, set 1).
    ampokr2a   ampkr2b2   clone      American Poker II (bootleg, set 2).
    ampoker2   ampkr2b3   clone      American Poker II (bootleg, set 3).
    ampokr2c   pkrdewin   clone      Poker De Win.
      ----     ampkr95    clone      American Poker 95.
      ----     sigmapkr   parent     Sigma Poker.
      ----     sigma2k    parent     Sigma Poker 2000.


********************************************************************************/


#include "driver.h"

static tilemap *bg_tilemap;


PALETTE_INIT( ampoker2 )
{
	int i;

/*    - bits -
      76543210
      RRRGGGBB
*/
	for (i = 0; i < machine->config->total_colors; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 0) & 0x01;
		bit2 = (*color_prom >> 1) & 0x01;
		b =  ( 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2);

		/* green component */
		bit0 = (*color_prom >> 2) & 0x01;
		bit1 = (*color_prom >> 3) & 0x01;
		bit2 = (*color_prom >> 4) & 0x01;
		g =  ( 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2);

		/* red component */
		bit0 = (*color_prom >> 5) & 0x01;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		r = ( 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2);

		palette_set_color(machine, i, MAKE_RGB(r, g, b));

		color_prom++;
	}
}

WRITE8_HANDLER( ampoker2_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int offs = tile_index * 2;
	int attr = videoram[offs + 1];
	int code = videoram[offs];
	int color = attr;
	code=code+(256*(color & 0x03));	/* code = color.bit1 + color.bit0 + code */
	color= color >> 1;				/* color = color - bit0 (bit1..bit7) */

	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( s2k_get_bg_tile_info )
{
	int offs = tile_index * 2;
	int attr = videoram[offs + 1];
	int code = videoram[offs];
	int color = attr;
	code=code+(256*(color & 0x0f));	/* the game use 2 extra bits */
	color= color >> 1;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START(ampoker2)
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 64, 32);
}

VIDEO_START(sigma2k)
{
	bg_tilemap = tilemap_create(s2k_get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 64, 32);
}

VIDEO_UPDATE(ampoker2)
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}
