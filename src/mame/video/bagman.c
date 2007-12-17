/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "video/resnet.h"


UINT8 *bagman_video_enable;

static tilemap *bg_tilemap;


WRITE8_HANDLER( bagman_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( bagman_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Bagman has two 32 bytes palette PROMs, connected to the RGB output this
  way:

  bit 7 -- 220 ohm resistor  -- \
        -- 470 ohm resistor  -- | -- 470 ohm pulldown resistor -- BLUE

        -- 220 ohm resistor  -- \
        -- 470 ohm resistor  -- | -- 470 ohm pulldown resistor -- GREEN
        -- 1  kohm resistor  -- /

        -- 220 ohm resistor  -- \
        -- 470 ohm resistor  -- | -- 470 ohm pulldown resistor -- RED
  bit 0 -- 1  kohm resistor  -- /

***************************************************************************/
PALETTE_INIT( bagman )
{
	int i;
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double weights_r[3], weights_g[3], weights_b[2];


	compute_resistor_weights(0,	255,	-1.0,
			3,	resistances_rg,	weights_r,	470,	0,
			3,	resistances_rg,	weights_g,	470,	0,
			2,	resistances_b,	weights_b,	470,	0);


	for (i = 0; i < machine->drv->total_colors; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(weights_r, bit0, bit1, bit2);
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(weights_g, bit0, bit1, bit2);
		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(weights_b, bit0, bit1);

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

WRITE8_HANDLER( bagman_flipscreen_w )
{
	if (flip_screen != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int gfxbank = (machine->gfx[2] && (colorram[tile_index] & 0x10)) ? 2 : 0;
	int code = videoram[tile_index] + 8 * (colorram[tile_index] & 0x20);
	int color = colorram[tile_index] & 0x0f;

	SET_TILE_INFO(gfxbank, code, color, 0);
}

VIDEO_START( bagman )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = spriteram_size - 4;offs >= 0;offs -= 4)
	{
		int sx,sy,flipx,flipy;


		sx = spriteram[offs + 3];
		sy = 240 - spriteram[offs + 2];
		flipx = spriteram[offs] & 0x40;
		flipy = spriteram[offs] & 0x80;
		if (flip_screen_x)
		{
			sx = 240 - sx +1;	/* compensate misplacement */
			flipx = !flipx;
		}
		if (flip_screen_y)
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		if (spriteram[offs + 2] && spriteram[offs + 3])
			drawgfx(bitmap,machine->gfx[1],
					(spriteram[offs] & 0x3f) + 2 * (spriteram[offs + 1] & 0x20),
					spriteram[offs + 1] & 0x1f,
					flipx,flipy,
					sx,sy+1,	/* compensate misplacement */
					cliprect,TRANSPARENCY_PEN,0);
	}
}


VIDEO_UPDATE( bagman )
{
	if (*bagman_video_enable == 0)
		return 0;

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
