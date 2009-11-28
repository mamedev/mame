#include "driver.h"
#include "nova2001.h"

UINT8 *nova2001_fg_videoram, *nova2001_bg_videoram;

static tilemap *bg_tilemap, *fg_tilemap;


/*************************************
 *
 *  Palette handling
 *
 *************************************/

PALETTE_INIT( nova2001 )
{
	int i;

	/* Color #1 is used for palette animation.          */

	/* To handle this, color entries 0-15 are based on  */
	/* the primary 16 colors, while color entries 16-31 */
	/* are based on the secondary set.                  */

	/* The only difference between 0-15 and 16-31 is that */
	/* color #1 changes each time */

	for (i = 0; i < 0x200; ++i)
	{
		int entry;
		int intensity,r,g,b;

		if ((i & 0xf) == 1)
		{
			entry = ((i & 0xf0) >> 4) | ((i & 0x100) >> 4);
		}
		else
		{
			entry = ((i & 0x0f) >> 0) | ((i & 0x100) >> 4);
		}

		intensity = (color_prom[entry] >> 0) & 0x03;
		/* red component */
		r = (((color_prom[entry] >> 0) & 0x0c) | intensity) * 0x11;
		/* green component */
		g = (((color_prom[entry] >> 2) & 0x0c) | intensity) * 0x11;
		/* blue component */
		b = (((color_prom[entry] >> 4) & 0x0c) | intensity) * 0x11;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

WRITE8_HANDLER( ninjakun_paletteram_w )
{
	int i;

	paletteram_BBGGRRII_w(space,offset,data);

	// expand the sprite palette to full length
	if (offset < 16)
	{
		paletteram_BBGGRRII_w(space, 0x200 + offset * 16 + 1, data);

		if (offset != 1)
		{
			for (i = 0; i < 16; i++)
			{
				paletteram_BBGGRRII_w(space, 0x200 + offset + i * 16, data);
			}
		}
	}
}



/*************************************
 *
 *  Callbacks for the TileMap code
 *
 *************************************/

static TILE_GET_INFO( nova2001_get_bg_tile_info )
{
	int code = nova2001_bg_videoram[tile_index];
	int color = nova2001_bg_videoram[tile_index + 0x400] & 0x0f;

	SET_TILE_INFO(2, code, color, 0);
}

static TILE_GET_INFO( nova2001_get_fg_tile_info )
{
	int attr = nova2001_fg_videoram[tile_index + 0x400];
	int code = nova2001_fg_videoram[tile_index];
	int color = attr & 0x0f;

	SET_TILE_INFO(1, code, color, 0);

	tileinfo->category = (attr & 0x10) >> 4;
}

static TILE_GET_INFO( ninjakun_get_bg_tile_info )
{
	int attr  = nova2001_bg_videoram[tile_index+0x400];
	int code = nova2001_bg_videoram[tile_index] + ((attr & 0xc0) << 2);
	int color = attr & 0x0f;

	SET_TILE_INFO(2, code, color, 0);
}

static TILE_GET_INFO( ninjakun_get_fg_tile_info )
{
	int attr = nova2001_fg_videoram[tile_index+0x400];
	int code = nova2001_fg_videoram[tile_index] + ((attr & 0x20) << 3);
	int color = attr & 0x0f;

	SET_TILE_INFO(1, code, color, 0);

	tileinfo->category = (attr & 0x10) >> 4;
}

static TILE_GET_INFO( pkunwar_get_bg_tile_info )
{
	int attr = nova2001_bg_videoram[tile_index + 0x400];
	int code = nova2001_bg_videoram[tile_index] + ((attr & 0x07) << 8);
	int color = (attr & 0xf0) >> 4;

	SET_TILE_INFO(1, code, color, 0);

	tileinfo->category = (attr & 0x08) >> 3;
}

static TILE_GET_INFO( raiders5_get_bg_tile_info )
{
	int attr = nova2001_bg_videoram[tile_index+0x400];
	int code = nova2001_bg_videoram[tile_index] + ((attr & 0x01) << 8);
	int color = (attr & 0xf0) >> 4;

	SET_TILE_INFO(2, code, color, 0);
}

static TILE_GET_INFO( raiders5_get_fg_tile_info )
{
	int code = nova2001_fg_videoram[tile_index];
	int color = (nova2001_fg_videoram[tile_index + 0x400] & 0xf0) >> 4;

	SET_TILE_INFO(1, code, color, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( nova2001 )
{
	bg_tilemap = tilemap_create(machine, nova2001_get_bg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	fg_tilemap = tilemap_create(machine, nova2001_get_fg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	tilemap_set_transparent_pen(fg_tilemap, 0);
	tilemap_set_scrolldx(bg_tilemap, 0, -7);
}

VIDEO_START( pkunwar )
{
	bg_tilemap = tilemap_create(machine, pkunwar_get_bg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	tilemap_set_transparent_pen(bg_tilemap, 0);
}

VIDEO_START( ninjakun )
{
	bg_tilemap = tilemap_create(machine, ninjakun_get_bg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	fg_tilemap = tilemap_create(machine, ninjakun_get_fg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	tilemap_set_transparent_pen(fg_tilemap, 0);
	tilemap_set_scrolldx(bg_tilemap, 7, 0);
}

VIDEO_START( raiders5 )
{
	bg_tilemap = tilemap_create(machine, raiders5_get_bg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	fg_tilemap = tilemap_create(machine, raiders5_get_fg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	tilemap_set_transparent_pen(fg_tilemap, 0);
	tilemap_set_scrolldx(bg_tilemap, 7, 0);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_HANDLER( nova2001_fg_videoram_w )
{
	nova2001_fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset & 0x3ff);
}

WRITE8_HANDLER( nova2001_bg_videoram_w )
{
	nova2001_bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( ninjakun_bg_videoram_w )
{
	int x = tilemap_get_scrollx(bg_tilemap, 0) >> 3;
	int y = tilemap_get_scrolly(bg_tilemap, 0) >> 3;

	// add scroll registers to address
	offset = ((offset + x + (y << 5)) & 0x3ff) + (offset & 0x400);

	nova2001_bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0x3ff);
}

READ8_HANDLER( ninjakun_bg_videoram_r )
{
	int x = tilemap_get_scrollx(bg_tilemap, 0) >> 3;
	int y = tilemap_get_scrolly(bg_tilemap, 0) >> 3;

	// add scroll registers to address
	offset = ((offset + x + (y << 5)) & 0x3ff) + (offset & 0x400);

	return nova2001_bg_videoram[offset];
}

WRITE8_HANDLER( nova2001_scroll_x_w )
{
	tilemap_set_scrollx(bg_tilemap, 0, data);
}

WRITE8_HANDLER( nova2001_scroll_y_w )
{
	tilemap_set_scrolly(bg_tilemap, 0, data);
}

WRITE8_HANDLER( nova2001_flipscreen_w )
{
	// inverted
	flip_screen_set(space->machine, ~data & 1);
}

WRITE8_HANDLER( pkunwar_flipscreen_w )
{
	flip_screen_set(space->machine, data & 1);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static void nova2001_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	const gfx_element *gfx = machine->gfx[0];
	int offs;

	for (offs = 0; offs < 0x800; offs += 32)
	{
		int attr = spriteram[offs+3];
		int flipx = attr & 0x10;
		int flipy = attr & 0x20;
		int sx = spriteram[offs+1] - ((attr & 0x40) << 2);	// high bit shown in schematics, not used by game
		int sy = spriteram[offs+2];
		int tile = spriteram[offs+0];
		int color = attr & 0x0f;

		if (attr & 0x80)	// disable bit shown in schematics, not used by game
		{
			continue;
		}

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, gfx,
				tile,
				color,
				flipx, flipy,
				sx, sy, 0);
	}
}

static void pkunwar_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	const gfx_element *gfx = machine->gfx[0];
	int offs;

	for (offs = 0; offs < 0x800; offs += 32)
	{
		int attr = spriteram[offs+3];
		int flipx = spriteram[offs+0] & 0x01;
		int flipy = spriteram[offs+0] & 0x02;
		int sx = spriteram[offs+1];
		int sy = spriteram[offs+2];
		int tile = ((spriteram[offs+0] & 0xfc) >> 2) + ((attr & 0x07) << 6);
		int color = (attr & 0xf0) >> 4;

		if (attr & 0x08)	// deducted by comparison, not used by game
		{
			continue;
		}

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, gfx,
				tile,
				color,
				flipx, flipy,
				sx, sy, 0);

		// there's no X MSB, so draw with wraparound (fixes title screen)
		drawgfx_transpen(bitmap, cliprect, gfx,
				tile,
				color,
				flipx, flipy,
				sx - 256, sy, 0);
	}
}



VIDEO_UPDATE( nova2001 )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	nova2001_draw_sprites(screen->machine, bitmap, cliprect);

	// according to the schematics, fg category 0 should be drawn behind sprites,
	// but it doesn't look right that way
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 1, 0);

	return 0;
}

VIDEO_UPDATE( pkunwar )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0);

	pkunwar_draw_sprites(screen->machine, bitmap, cliprect);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 1, 0);

	return 0;
}

VIDEO_UPDATE( ninjakun )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	tilemap_draw(bitmap, cliprect, fg_tilemap, 1, 0);

	nova2001_draw_sprites(screen->machine, bitmap, cliprect);

	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);

	return 0;
}

VIDEO_UPDATE( raiders5 )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	pkunwar_draw_sprites(screen->machine, bitmap, cliprect);

	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);

	return 0;
}
