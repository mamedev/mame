/**********************************************************************************


    FUNWORLD.


    Original preliminary driver:    Curt Coder, Peter Trauner.
    Rewrite and aditional work:     Roberto Fresca.


    Games running in this hardware:

    * Jolly Card (Austria),                     TAB-Austria,        1985.
    * Jolly Card (Austria, encrypted),          TAB-Austria,        1985.
    * Jolly Card (Croatia),                     Soft Design,        1993.
    * Jolly Card (Italia, encrypted),           bootleg,            199?.
    * Jolly Card (Austria, Fun World, bootleg), Inter Games,        1995.
    * Big Deal (Hungary, set 1),                Fun World,          1990.
    * Big Deal (Hungary, set 2),                Fun World,          1990.
    * Jolly Card (Austria, Fun World),          Fun World,          1990.
    * Cuore 1 (Italia),                         C.M.C.,             1996.
    * Elephant Family (Italia, new),            C.M.C.,             1997.
    * Elephant Family (Italia, old),            C.M.C.,             1996.
    * Pool 10 (Italia, set 1),                  C.M.C.,             1996.
    * Pool 10 (Italia, set 2),                  C.M.C.,             1996.
    * Tortuga Family (Italia),                  C.M.C.,             1997.
    * Royal Card (Austria, set 1),              TAB-Austria,        1991.
    * Royal Card (Austria, set 2),              TAB-Austria,        1991.
    * Royal Card (Slovakia, encrypted),         Evona Electronic,   1991.
    * Magic Card II (Bulgaria, bootleg),        Impera,             1996.
    * Joker Card (Ver.A267BC, encrypted),       Vesely Svet,        1993.
    * Mongolfier New (Italia),                  bootleg,            199?.
    * Soccer New (Italia),                      bootleg,            199?.
    * Snooker 10 (Ver 1.11),                    Sandiy,             1998.
    * Saloon (France, encrypted),               unknown,            199?.


***********************************************************************************/


#include "driver.h"

static tilemap *bg_tilemap;


PALETTE_INIT(funworld)
{
	int i;

	/* RRRBBBGG */
	if (color_prom == 0) return;

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

WRITE8_HANDLER( funworld_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( funworld_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

/**** normal hardware limit ****
    - bits -
    7654 3210
    xxxx xx--   tiles color.
    xxx- x-xx   tiles color (title).
    xxxx -xxx   tiles color (background).
*/

static TILE_GET_INFO( get_bg_tile_info )
{
//  - bits -
//  7654 3210
//  xxxx ----   tiles color.
//  ---- xxxx   unused.

	int offs = tile_index;
	int attr = videoram[offs] + (colorram[offs] << 8);
	int code = attr & 0xfff;
	int color = colorram[offs] >> 4;	// 4 bits for color.

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START(funworld)
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		 4, 8, 96, 29);
}

VIDEO_START(magiccrd)
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		 4, 8, 112, 34);
}

VIDEO_UPDATE(funworld)
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}
