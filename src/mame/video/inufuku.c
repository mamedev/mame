/******************************************************************************

    Video Hardware for Video System Games.

    Quiz & Variety Sukusuku Inufuku
    (c)1998 Video System Co.,Ltd.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2003/08/09 -

    based on other Video System drivers

******************************************************************************/

#include "emu.h"
#include "includes/inufuku.h"


/******************************************************************************

    Memory handlers

******************************************************************************/

WRITE16_HANDLER( inufuku_palettereg_w )
{
	inufuku_state *state = (inufuku_state *)space->machine->driver_data;
	switch (offset)
	{
		case 0x02:	state->bg_palettebank = (data & 0xf000) >> 12;
				tilemap_mark_all_tiles_dirty(state->bg_tilemap);
				break;
		case 0x03:	state->tx_palettebank = (data & 0xf000) >> 12;
				tilemap_mark_all_tiles_dirty(state->tx_tilemap);
				break;
	}
}

WRITE16_HANDLER( inufuku_scrollreg_w )
{
	inufuku_state *state = (inufuku_state *)space->machine->driver_data;
	switch (offset)
	{
		case 0x00:	state->bg_scrollx = data + 1; break;
		case 0x01:	state->bg_scrolly = data + 0; break;
		case 0x02:	state->tx_scrollx = data - 3; break;
		case 0x03:	state->tx_scrolly = data + 1; break;
		case 0x04:	state->bg_raster = (data & 0x0200) ? 0 : 1; break;
	}
}


/******************************************************************************

    Sprite routines

******************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	inufuku_state *state = (inufuku_state *)machine->driver_data;
	int offs;

	for (offs = (state->spriteram1_size / 16) - 1; offs >= 0; offs--)
	{
		if ((state->spriteram1[offs] & 0x8000) == 0x0000)
		{
			int attr_start;
			int map_start;
			int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color;
			int priority, priority_mask;

			attr_start = 4 * (state->spriteram1[offs] & 0x03ff);

			/*
                attr_start + 0x0000
                ---- ---x xxxx xxxx oy
                ---- xxx- ---- ---- ysize
                xxxx ---- ---- ---- zoomy

                attr_start + 0x0001
                ---- ---x xxxx xxxx ox
                ---- xxx- ---- ---- xsize
                xxxx ---- ---- ---- zoomx

                attr_start + 0x0002
                -x-- ---- ---- ---- flipx
                x--- ---- ---- ---- flipy
                --xx xxxx ---- ---- color
                --xx ---- ---- ---- priority?
                ---- ---- xxxx xxxx unused?

                attr_start + 0x0003
                -xxx xxxx xxxx xxxx map start
                x--- ---- ---- ---- unused?
            */

			ox = (state->spriteram1[attr_start + 1] & 0x01ff) + 0;
			xsize = (state->spriteram1[attr_start + 1] & 0x0e00) >> 9;
			zoomx = (state->spriteram1[attr_start + 1] & 0xf000) >> 12;
			oy = (state->spriteram1[attr_start + 0] & 0x01ff) + 1;
			ysize = (state->spriteram1[attr_start + 0] & 0x0e00) >> 9;
			zoomy = (state->spriteram1[attr_start + 0] & 0xf000) >> 12;
			flipx = state->spriteram1[attr_start + 2] & 0x4000;
			flipy = state->spriteram1[attr_start + 2] & 0x8000;
			color = (state->spriteram1[attr_start + 2] & 0x3f00) >> 8;
			priority = (state->spriteram1[attr_start + 2] & 0x3000) >> 12;
			map_start = (state->spriteram1[attr_start + 3] & 0x7fff) << 1;

			switch (priority)
			{
				default:
				case 0:	priority_mask = 0x00; break;
				case 3:	priority_mask = 0xfe; break;
				case 2:	priority_mask = 0xfc; break;
				case 1:	priority_mask = 0xf0; break;
			}

			ox += (xsize * zoomx + 2) / 4;
			oy += (ysize * zoomy + 2) / 4;

			zoomx = 32 - zoomx;
			zoomy = 32 - zoomy;

			for (y = 0; y <= ysize; y++)
			{
				int sx, sy;

				if (flipy)
					sy = (oy + zoomy * (ysize - y) / 2 + 16) & 0x1ff;
				else
					sy = (oy + zoomy * y / 2 + 16) & 0x1ff;

				for (x = 0; x <= xsize; x++)
				{
					int code;

					if (flipx)
						sx = (ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff;
					else
						sx = (ox + zoomx * x / 2 + 16) & 0x1ff;

					code  = ((state->spriteram2[map_start] & 0x0007) << 16) + state->spriteram2[map_start + 1];

					pdrawgfxzoom_transpen(bitmap, cliprect, machine->gfx[2],
							code,
							color,
							flipx, flipy,
							sx - 16, sy - 16,
							zoomx << 11, zoomy << 11,
							machine->priority_bitmap,priority_mask, 15);

					map_start += 2;
				}
			}
		}
	}
}


/******************************************************************************

    Tilemap callbacks

******************************************************************************/

static TILE_GET_INFO( get_inufuku_bg_tile_info )
{
	inufuku_state *state = (inufuku_state *)machine->driver_data;
	SET_TILE_INFO(
			0,
			state->bg_videoram[tile_index],
			state->bg_palettebank,
			0);
}

static TILE_GET_INFO( get_inufuku_tx_tile_info )
{
	inufuku_state *state = (inufuku_state *)machine->driver_data;
	SET_TILE_INFO(
			1,
			state->tx_videoram[tile_index],
			state->tx_palettebank,
			0);
}

READ16_HANDLER( inufuku_bg_videoram_r )
{
	inufuku_state *state = (inufuku_state *)space->machine->driver_data;
	return state->bg_videoram[offset];
}

WRITE16_HANDLER( inufuku_bg_videoram_w )
{
	inufuku_state *state = (inufuku_state *)space->machine->driver_data;
	COMBINE_DATA(&state->bg_videoram[offset]);
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

READ16_HANDLER( inufuku_tx_videoram_r )
{
	inufuku_state *state = (inufuku_state *)space->machine->driver_data;
	return state->tx_videoram[offset];
}

WRITE16_HANDLER( inufuku_tx_videoram_w )
{
	inufuku_state *state = (inufuku_state *)space->machine->driver_data;
	COMBINE_DATA(&state->tx_videoram[offset]);
	tilemap_mark_tile_dirty(state->tx_tilemap, offset);
}


/******************************************************************************

    Start the video hardware emulation

******************************************************************************/

VIDEO_START( inufuku )
{
	inufuku_state *state = (inufuku_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_inufuku_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->tx_tilemap = tilemap_create(machine, get_inufuku_tx_tile_info, tilemap_scan_rows, 8, 8, 64, 64);

	tilemap_set_transparent_pen(state->bg_tilemap, 255);
	tilemap_set_transparent_pen(state->tx_tilemap, 255);
}


/******************************************************************************

    Display refresh

******************************************************************************/

VIDEO_UPDATE( inufuku )
{
	inufuku_state *state = (inufuku_state *)screen->machine->driver_data;
	int i;

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	bitmap_fill(screen->machine->priority_bitmap, NULL, 0);

	if (state->bg_raster)
	{
		tilemap_set_scroll_rows(state->bg_tilemap, 512);
		for (i = 0; i < 256; i++)
			tilemap_set_scrollx(state->bg_tilemap, (state->bg_scrolly + i) & 0x1ff, state->bg_rasterram[i]);
	}
	else
	{
		tilemap_set_scroll_rows(state->bg_tilemap, 1);
		tilemap_set_scrollx(state->bg_tilemap, 0, state->bg_scrollx);
	}
	tilemap_set_scrolly(state->bg_tilemap, 0, state->bg_scrolly);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	tilemap_set_scrollx(state->tx_tilemap, 0, state->tx_scrollx);
	tilemap_set_scrolly(state->tx_tilemap, 0, state->tx_scrolly);
	tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 4);

	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
