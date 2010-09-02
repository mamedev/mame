/**********************************************************************************

  FUNWORLD / TAB.
  Video Hardware.

  Written by Roberto Fresca.


  Games running on this hardware:

  * Jolly Card (austrian),                            TAB-Austria,        1985.
  * Jolly Card (3x3 deal),                            TAB-Austria,        1985.
  * Jolly Card Professional 2.0 (MZS Tech),           MZS Tech,           1993.
  * Jolly Card Professional 2.0 (Spale Soft),         Spale Soft,         2000.
  * Jolly Card (Evona Electronic),                    Evona Electronic    1998.
  * Jolly Card (croatian, set 1),                     TAB-Austria,        1985.
  * Jolly Card (croatian, set 2),                     Soft Design,        1993.
  * Jolly Card (italian, blue TAB board, encrypted),  bootleg,            199?.
  * Jolly Card (italian, encrypted bootleg),          bootleg,            1990.
  * Super Joly 2000 - 3x,                             M.P.                1985.
  * Jolly Card (austrian, Funworld, bootleg),         Inter Games,        1986.
  * Big Deal (hungarian, set 1),                      Funworld,           1986.
  * Big Deal (hungarian, set 2),                      Funworld,           1986.
  * Jolly Card (austrian, Funworld),                  Funworld,           1986.
  * Cuore 1 (italian),                                C.M.C.,             1996.
  * Elephant Family (italian, new),                   C.M.C.,             1997.
  * Elephant Family (italian, old),                   C.M.C.,             1996.
  * Pool 10 (italian, set 1),                         C.M.C.,             1996.
  * Pool 10 (italian, set 2),                         C.M.C.,             1996.
  * Pool 10 (italian, set 3),                         C.M.C.,             1996.
  * Pool 10 (italian, set 4),                         C.M.C.,             1997.
  * Tortuga Family (italian),                         C.M.C.,             1997.
  * Pot Game (italian),                               C.M.C.,             1996.
  * Bottle 10 (italian, set 1),                       C.M.C.,             1996.
  * Bottle 10 (italian, set 2),                       C.M.C.,             1996.
  * Royal Card (austrian, set 1),                     TAB-Austria,        1991.
  * Royal Card (austrian, set 2),                     TAB-Austria,        1991.
  * Royal Card (austrian/polish, set 3),              TAB-Austria,        1991.
  * Royal Card (austrian, set 4),                     TAB-Austria,        1991.
  * Royal Card (austrian, set 5),                     TAB-Austria,        1991.
  * Royal Card (austrian, set 6),                     TAB-Austria,        1991.
  * Royal Card (slovak, encrypted),                   Evona Electronic,   1991.
  * Royal Card Professional 2.0,                      Digital Dreams,     1993.
  * Lucky Lady (3x3 deal),                            TAB-Austria,        1991.
  * Lucky Lady (4x1 aces),                            TAB-Austria,        1991.
  * Magic Card II (bulgarian),                        Impera,             1996.
  * Magic Card II (green TAB or Impera board),        Impera,             1996.
  * Magic Card II (blue TAB board, encrypted),        Impera,             1996.
  * Royal Vegas Joker Card (slow deal),               Funworld,           1993.
  * Royal Vegas Joker Card (fast deal),               Soft Design,        1993.
  * Royal Vegas Joker Card (fast deal, english gfx),  Soft Design,        1993.
  * Royal Vegas Joker Card (fast deal, Mile),         Mile,               1993.
  * Jolly Joker (98bet, set 1).                       Impera,             198?.
  * Jolly Joker (98bet, set 2).                       Impera,             198?.
  * Jolly Joker (40bet, croatian hack),               Impera,             198?.
  * Multi Win (Ver.0167, encrypted),                  Funworld,           1992.
  * Joker Card (Ver.A267BC, encrypted),               Vesely Svet,        1993.
  * Mongolfier New (italian),                         bootleg,            199?.
  * Soccer New (italian),                             bootleg,            199?.
  * Saloon (french, encrypted),                       unknown,            199?.
  * Fun World Quiz (austrian),                        Funworld,           198?.


***********************************************************************************

  TAB/Impera/FunWorld color system circuitry
  ------------------------------------------

  74HC174 - Hex D-type flip-flops with reset; positive-edge trigger.
  N82S147 - 4K-bit TTL Bipolar PROM.
  74LS374 - 3-STATE Octal D-Type transparent latches and edge-triggered flip-flops.

                   N82S147         74LS374       RESNET        PULL-DOWN
   74HC174        .-------.       .-------.
  .-------.   (1)-|01   20|--VCC--|20   02|------[(1K)]---+              .-----.
  |       |   (1)-|02   06|-------|03   05|------[(470)]--+--+-----------| RED |
  |16: VCC|   (1)-|03   07|-------|04   06|------[(220)]--+  |           '-----'
  |       |   (1)-|04   08|-------|07     |                  '--[(100)]--GND
  |     02|-------|05   09|-------|08   09|------[(1K)]---+              .------.
  |     05|-------|16   11|-------|13   12|------[(470)]--+--+-----------| BLUE |
  |     07|-------|17   12|-------|14   15|------[(220)]--+  |           '------'
  |     10|-------|18   13|-------|17     |                  '--[(100)]--GND
  |     12|-------|19   14|-------|18   16|------[(470)]--+              .-------.
  |     13|---+---|15   10|---+---|10   19|------[(220)]--+--+-----------| GREEN |
  |15 08  |   |   |       |   |   |   01  |                  |           '-------'
  '-+--+--'   |   '-------'   |   '----+--'                  '--[(100)]--GND
    |  |      |               |        |
    |  '------+------GND------'        |
    '----------------------------------'

  (1): Connected either to:
       - A custom 40-pin GFX IC
       - 2x HYxxx devices (TAB blue PCB).
       - A little board with 4x 74LS138 or 74LS137 (Impera green PCB).

  NOTE: The 74LS374 could be replaced by a 74HCT373.

***********************************************************************************/


#include "emu.h"
#include "video/resnet.h"
#include "includes/funworld.h"

static tilemap_t *bg_tilemap;
UINT8 *funworld_colorram;
UINT8 *funworld_videoram;


PALETTE_INIT(funworld)
{
	int i;
	static const int resistances_rb[3] = { 1000, 470, 220 };
	static const int resistances_g [2] = { 470, 220 };
	double weights_r[3], weights_b[3], weights_g[2];

	compute_resistor_weights(0,	255,	-1.0,
			3,	resistances_rb,	weights_r,	100,	0,
			3,	resistances_rb,	weights_b,	100,	0,
			2,	resistances_g,	weights_g,	100,	0);


	for (i = 0; i < machine->total_colors(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(weights_r, bit0, bit1, bit2);
		/* blue component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		b = combine_3_weights(weights_b, bit0, bit1, bit2);
		/* green component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		g = combine_2_weights(weights_g, bit0, bit1);

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}


WRITE8_HANDLER( funworld_videoram_w )
{
	funworld_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( funworld_colorram_w )
{
	funworld_colorram[offset] = data;
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
/*  - bits -
    7654 3210
    xxxx ----   tiles color.
    ---- xxxx   unused.
*/
	int offs = tile_index;
	int attr = funworld_videoram[offs] + (funworld_colorram[offs] << 8);
	int code = attr & 0xfff;
	int color = funworld_colorram[offs] >> 4;	// 4 bits for color.

	SET_TILE_INFO(0, code, color, 0);
}


VIDEO_START(funworld)
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 4, 8, 96, 29);
}

VIDEO_START(magicrd2)
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 4, 8, 112, 34);
}


VIDEO_UPDATE(funworld)
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}
