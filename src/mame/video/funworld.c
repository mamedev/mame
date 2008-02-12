/**********************************************************************************


    FUNWORLD / TAB.

    Original preliminary driver:    Curt Coder, Peter Trauner.
    Rewrite and aditional work:     Roberto Fresca.



    Games running in this hardware:

    * Jolly Card (austrian),                            TAB-Austria,        1985.
    * Jolly Card (austrian, encrypted),                 TAB-Austria,        1985.
    * Jolly Card (3x3 deal),                            TAB-Austria,        1985.
    * Jolly Card Professional 2.0,                      Spale-Soft,         2000.
    * Jolly Card (croatian),                            Soft Design,        1993.
    * Jolly Card (italian, blue TAB board, encrypted),  bootleg,            199?.
    * Jolly Card (austrian, Funworld, bootleg),         Inter Games,        1986.
    * Big Deal (hungarian, set 1),                      Funworld,           1986.
    * Big Deal (hungarian, set 2),                      Funworld,           1986.
    * Jolly Card (austrian, Funworld),                  Funworld,           1986.
    * Cuore 1 (italian),                                C.M.C.,             1996.
    * Elephant Family (italian, new),                   C.M.C.,             1997.
    * Elephant Family (italian, old),                   C.M.C.,             1996.
    * Pool 10 (italian, set 1),                         C.M.C.,             1996.
    * Pool 10 (italian, set 2),                         C.M.C.,             1996.
    * Tortuga Family (italian),                         C.M.C.,             1997.
    * Royal Card (austrian, set 1),                     TAB-Austria,        1991.
    * Royal Card (austrian, set 2),                     TAB-Austria,        1991.
    * Royal Card (slovak, encrypted),                   Evona Electronic,   1991.
    * Lucky Lady (3x3 deal),                            TAB-Austria,        1991.
    * Lucky Lady (4x1 aces),                            TAB-Austria,        1991.
    * Magic Card II (bulgarian),                        Impera,             1996.
    * Magic Card II (green TAB or Impera board),        Impera,             1996.
    * Magic Card II (blue TAB board, encrypted),        Impera,             1996.
    * Royal Vegas Joker Card (slow deal),               Funworld,           1993.
    * Royal Vegas Joker Card (fast deal),               Soft Design,        1993.
    * Joker Card (Ver.A267BC, encrypted),               Vesely Svet,        1993.
    * Mongolfier New (italian),                         bootleg,            199?.
    * Soccer New (italian),                             bootleg,            199?.
    * Snooker 10 (Ver 1.11),                            Sandii',            1998.
    * Saloon (french, encrypted),                       unknown,            199?.


***********************************************************************************/


#include "driver.h"
#include "video/crtc6845.h"

static crtc6845_t *crtc6845;
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

WRITE8_HANDLER( funworld_crtc6845_address_w )
{
	crtc6845_address_w(crtc6845, data);
}

READ8_HANDLER( funworld_crtc6845_register_r )
{
	return crtc6845_register_r(crtc6845);
}

WRITE8_HANDLER( funworld_crtc6845_register_w )
{
	crtc6845_register_w(crtc6845, data);
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
	crtc6845 = crtc6845_config(NULL);
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows, 4, 8, 96, 29);
}

VIDEO_START(magiccrd)
{
	crtc6845 = crtc6845_config(NULL);
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows, 4, 8, 112, 34);
}

VIDEO_START(snookr10)
{
//	crtc6845 = crtc6845_config(NULL);
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows, 4, 8, 128, 32);
}

VIDEO_UPDATE(funworld)
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}
