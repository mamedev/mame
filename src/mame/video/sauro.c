/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"

UINT8 *tecfri_videoram;
UINT8 *tecfri_colorram;
UINT8 *tecfri_videoram2;
UINT8 *tecfri_colorram2;

static tilemap_t *bg_tilemap, *fg_tilemap;
static UINT8 palette_bank;

/* General */

WRITE8_HANDLER( tecfri_videoram_w )
{
	tecfri_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( tecfri_colorram_w )
{
	tecfri_colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( tecfri_videoram2_w )
{
	tecfri_videoram2[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( tecfri_colorram2_w )
{
	tecfri_colorram2[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( tecfri_scroll_bg_w )
{
	tilemap_set_scrollx(bg_tilemap, 0, data);
}

static TILE_GET_INFO( get_tile_info_bg )
{
	int code = tecfri_videoram[tile_index] + ((tecfri_colorram[tile_index] & 0x07) << 8);
	int color = ((tecfri_colorram[tile_index] >> 4) & 0x0f) | palette_bank;
	int flags = tecfri_colorram[tile_index] & 0x08 ? TILE_FLIPX : 0;

	SET_TILE_INFO(0, code, color, flags);
}

static TILE_GET_INFO( get_tile_info_fg )
{
	int code = tecfri_videoram2[tile_index] + ((tecfri_colorram2[tile_index] & 0x07) << 8);
	int color = ((tecfri_colorram2[tile_index] >> 4) & 0x0f) | palette_bank;
	int flags = tecfri_colorram2[tile_index] & 0x08 ? TILE_FLIPX : 0;

	SET_TILE_INFO(1, code, color, flags);
}

/* Sauro */

static const int scroll2_map[8] = {2, 1, 4, 3, 6, 5, 0, 7};
static const int scroll2_map_flip[8] = {0, 7, 2, 1, 4, 3, 6, 5};

WRITE8_HANDLER( sauro_palette_bank_w )
{
	palette_bank = (data & 0x03) << 4;
	tilemap_mark_all_tiles_dirty_all(space->machine);
}

WRITE8_HANDLER( sauro_scroll_fg_w )
{
	const int *map = (flip_screen_get(space->machine) ? scroll2_map_flip : scroll2_map);
	int scroll = (data & 0xf8) | map[data & 7];

	tilemap_set_scrollx(fg_tilemap, 0, scroll);
}

VIDEO_START( sauro )
{
	bg_tilemap = tilemap_create(machine, get_tile_info_bg, tilemap_scan_cols,
		 8, 8, 32, 32);

	fg_tilemap = tilemap_create(machine, get_tile_info_fg, tilemap_scan_cols,
		 8, 8, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 0);
	palette_bank = 0;
}

static void sauro_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	int offs,code,sx,sy,color,flipx;

	for (offs = 3;offs < machine->generic.spriteram_size - 1;offs += 4)
	{
		sy = spriteram[offs];
		if (sy == 0xf8) continue;

		code = spriteram[offs+1] + ((spriteram[offs+3] & 0x03) << 8);
		sx = spriteram[offs+2];
		sy = 236 - sy;
		color = ((spriteram[offs+3] >> 4) & 0x0f) | palette_bank;

		// I'm not really sure how this bit works
		if (spriteram[offs+3] & 0x08)
		{
			if (sx > 0xc0)
			{
				// Sign extend
				sx = (signed int)(signed char)sx;
			}
		}
		else
		{
			if (sx < 0x40) continue;
		}

		flipx = spriteram[offs+3] & 0x04;

		if (flip_screen_get(machine))
		{
			flipx = !flipx;
			sx = (235 - sx) & 0xff;  // The &0xff is not 100% percent correct
			sy = 240 - sy;
		}

		drawgfx_transpen(bitmap, cliprect,machine->gfx[2],
				code,
				color,
				flipx,flip_screen_get(machine),
				sx,sy,0);
	}
}

VIDEO_UPDATE( sauro )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	sauro_draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}

/* Tricky Doc */

VIDEO_START( trckydoc )
{
	bg_tilemap = tilemap_create(machine, get_tile_info_bg, tilemap_scan_cols,
		 8, 8, 32, 32);
}

static void trckydoc_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	int offs,code,sy,color,flipx,sx;

	/* Weird, sprites entries don't start on DWORD boundary */
	for (offs = 3;offs < machine->generic.spriteram_size - 1;offs += 4)
	{
		sy = spriteram[offs];

		if(spriteram[offs+3] & 0x08)
		{
			/* needed by the elevator cable (2nd stage), balls bouncing (3rd stage) and maybe other things */
			sy += 6;
		}

		code = spriteram[offs+1] + ((spriteram[offs+3] & 0x01) << 8);

		sx = spriteram[offs+2]-2;
		color = (spriteram[offs+3] >> 4) & 0x0f;

		sy = 236 - sy;

		/* similar to sauro but different bit is used .. */
		if (spriteram[offs+3] & 0x02)
		{
			if (sx > 0xc0)
			{
				/* Sign extend */
				sx = (signed int)(signed char)sx;
			}
		}
		else
		{
			if (sx < 0x40) continue;
		}

		flipx = spriteram[offs+3] & 0x04;

		if (flip_screen_get(machine))
		{
			flipx = !flipx;
			sx = (235 - sx) & 0xff;  /* The &0xff is not 100% percent correct */
			sy = 240 - sy;
		}

		drawgfx_transpen(bitmap, cliprect,machine->gfx[1],
				code,
				color,
				flipx,flip_screen_get(machine),
				sx,sy,0);
	}
}

VIDEO_UPDATE( trckydoc )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	trckydoc_draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
