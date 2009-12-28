/***************************************************************************

    Kyugo hardware games

***************************************************************************/

#include "driver.h"
#include "includes/kyugo.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	kyugo_state *state = (kyugo_state *)machine->driver_data;
	int code = state->fgvideoram[tile_index];
	SET_TILE_INFO(0,
				  code,
				  2 * state->color_codes[code >> 3] + state->fgcolor,
				  0);
}


static TILE_GET_INFO( get_bg_tile_info )
{
	kyugo_state *state = (kyugo_state *)machine->driver_data;
	int code = state->bgvideoram[tile_index];
	int attr = state->bgattribram[tile_index];
	SET_TILE_INFO(1,
				  code | ((attr & 0x03) << 8),
				  (attr >> 4) | (state->bgpalbank << 4),
				  TILE_FLIPYX((attr & 0x0c) >> 2));
}


/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( kyugo )
{
	kyugo_state *state = (kyugo_state *)machine->driver_data;

	state->color_codes = memory_region(machine, "proms") + 0x300;

	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 0);

	tilemap_set_scrolldx(state->fg_tilemap,   0, 224);
	tilemap_set_scrolldx(state->bg_tilemap, -32, 32);
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_HANDLER( kyugo_fgvideoram_w )
{
	kyugo_state *state = (kyugo_state *)space->machine->driver_data;

	state->fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}


WRITE8_HANDLER( kyugo_bgvideoram_w )
{
	kyugo_state *state = (kyugo_state *)space->machine->driver_data;

	state->bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}


WRITE8_HANDLER( kyugo_bgattribram_w )
{
	kyugo_state *state = (kyugo_state *)space->machine->driver_data;

	state->bgattribram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}


READ8_HANDLER( kyugo_spriteram_2_r )
{
	kyugo_state *state = (kyugo_state *)space->machine->driver_data;

	// only the lower nibble is connected
	return state->spriteram_2[offset] | 0xf0;
}


WRITE8_HANDLER( kyugo_scroll_x_lo_w )
{
	kyugo_state *state = (kyugo_state *)space->machine->driver_data;
	state->scroll_x_lo = data;
}


WRITE8_HANDLER( kyugo_gfxctrl_w )
{
	kyugo_state *state = (kyugo_state *)space->machine->driver_data;

	/* bit 0 is scroll MSB */
	state->scroll_x_hi = data & 0x01;

	/* bit 5 is front layer color (Son of Phoenix only) */
	if (state->fgcolor != ((data & 0x20) >> 5))
	{
		state->fgcolor = (data & 0x20) >> 5;

		tilemap_mark_all_tiles_dirty(state->fg_tilemap);
	}

	/* bit 6 is background palette bank */
	if (state->bgpalbank != ((data & 0x40) >> 6))
	{
		state->bgpalbank = (data & 0x40) >> 6;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	if (data & 0x9e)
		popmessage("%02x",data);
}


WRITE8_HANDLER( kyugo_scroll_y_w )
{
	kyugo_state *state = (kyugo_state *)space->machine->driver_data;
	state->scroll_y = data;
}


WRITE8_HANDLER( kyugo_flipscreen_w )
{
	kyugo_state *state = (kyugo_state *)space->machine->driver_data;

	if (state->flipscreen != (data & 0x01))
	{
		state->flipscreen = (data & 0x01);
		tilemap_set_flip_all(space->machine, (state->flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY): 0));
	}
}


/*************************************
 *
 *  Video update
 *
 *************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	kyugo_state *state = (kyugo_state *)machine->driver_data;

	/* sprite information is scattered through memory */
	/* and uses a portion of the text layer memory (outside the visible area) */
	UINT8 *spriteram_area1 = &state->spriteram_1[0x28];
	UINT8 *spriteram_area2 = &state->spriteram_2[0x28];
	UINT8 *spriteram_area3 = &state->fgvideoram[0x28];

	int n;

	for (n = 0; n < 12 * 2; n++)
	{
		int offs, y, sy, sx, color;

		offs = 2 * (n % 12) + 64 * (n / 12);

		sx = spriteram_area3[offs + 1] + 256 * (spriteram_area2[offs + 1] & 1);
		if (sx > 320)
			sx -= 512;

		sy = 255 - spriteram_area1[offs] + 2;
		if (sy > 0xf0)
			sy -= 256;

		if (state->flipscreen)
			sy = 240 - sy;

		color = spriteram_area1[offs + 1] & 0x1f;

		for (y = 0; y < 16; y++)
		{
			int code, attr, flipx, flipy;

			code = spriteram_area3[offs + 128 * y];
			attr = spriteram_area2[offs + 128 * y];

			code = code | ((attr & 0x01) << 9) | ((attr & 0x02) << 7);

			flipx =  attr & 0x08;
			flipy =  attr & 0x04;

			if (state->flipscreen)
			{
				flipx = !flipx;
				flipy = !flipy;
			}


			drawgfx_transpen( bitmap, cliprect,machine->gfx[2],
					 code,
					 color,
					 flipx,flipy,
					 sx,state->flipscreen ? sy - 16*y : sy + 16*y, 0 );
		}
	}
}


VIDEO_UPDATE( kyugo )
{
	kyugo_state *state = (kyugo_state *)screen->machine->driver_data;

	if (state->flipscreen)
		tilemap_set_scrollx(state->bg_tilemap, 0, -(state->scroll_x_lo + (state->scroll_x_hi * 256)));
	else
		tilemap_set_scrollx(state->bg_tilemap, 0,   state->scroll_x_lo + (state->scroll_x_hi * 256));

	tilemap_set_scrolly(state->bg_tilemap, 0, state->scroll_y);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}
