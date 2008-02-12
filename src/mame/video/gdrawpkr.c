/**********************************************************************************

    GAMING DRAW POKER (CEI)
    Driver by Roberto Fresca.

    Video Hardware

***********************************************************************************/

#include "driver.h"
#include "video/crtc6845.h"

static crtc6845_t *crtc6845;
static tilemap *bg_tilemap;

WRITE8_HANDLER( gdrawpkr_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( gdrawpkr_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
/*  - bits -
    7654 3210
    --xx xx--   tiles color.
    ---- --x-   tiles bank.
    xx-- ---x   seems unused. */

	int attr = colorram[tile_index];
	int code = videoram[tile_index];
	int bank = (attr & 0x02) >> 1;	/* bit 1 switch the gfx banks */
	int color = (attr & 0x3c);	/* bits 2-3-4-5 for color */

	if (attr == 0x3a)	/* Is the palette wrong? */
		color = 0x3b;	/* 0x3b is the best match */

	SET_TILE_INFO(bank, code, color, 0);
}

WRITE8_HANDLER( gdrawpkr_crtc6845_address_w )
{
	crtc6845_address_w(crtc6845, data);
}

READ8_HANDLER( gdrawpkr_crtc6845_register_r )
{
	return crtc6845_register_r(crtc6845);
}

WRITE8_HANDLER( gdrawpkr_crtc6845_register_w )
{
	crtc6845_register_w(crtc6845, data);
}

VIDEO_START( gdrawpkr )
{
	crtc6845 = crtc6845_config(NULL);
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 31);
}

VIDEO_UPDATE( gdrawpkr )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}

PALETTE_INIT( gdrawpkr )
{
/*  prom bits
    7654 3210
    ---- ---x   red component.
    ---- --x-   green component.
    ---- -x--   blue component.
    ---- x---   unknown, aparently unused.
    xxxx ----   unused.
*/
	int i;

	/* 00000BGR */
	if (color_prom == 0) return;

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		r = bit0 * 0xff;

		/* green component */
		bit1 = (color_prom[i] >> 1) & 0x01;
		g = bit1 * 0xff;

		/* blue component */
		bit2 = (color_prom[i] >> 2) & 0x01;
		b = bit2 * 0xff;


		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}
