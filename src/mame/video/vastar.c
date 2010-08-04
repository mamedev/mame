/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/vastar.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	vastar_state *state = machine->driver_data<vastar_state>();
	UINT8 *videoram = state->fgvideoram;
	int code, color;

	code = videoram[tile_index + 0x800] | (videoram[tile_index + 0x400] << 8);
	color = videoram[tile_index];
	SET_TILE_INFO(
			0,
			code,
			color & 0x3f,
			0);
}

static TILE_GET_INFO( get_bg1_tile_info )
{
	vastar_state *state = machine->driver_data<vastar_state>();
	UINT8 *videoram = state->bg1videoram;
	int code, color;

	code = videoram[tile_index + 0x800] | (videoram[tile_index] << 8);
	color = videoram[tile_index + 0xc00];
	SET_TILE_INFO(
			4,
			code,
			color & 0x3f,
			0);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	vastar_state *state = machine->driver_data<vastar_state>();
	UINT8 *videoram = state->bg2videoram;
	int code, color;

	code = videoram[tile_index + 0x800] | (videoram[tile_index] << 8);
	color = videoram[tile_index + 0xc00];
	SET_TILE_INFO(
			3,
			code,
			color & 0x3f,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( vastar )
{
	vastar_state *state = machine->driver_data<vastar_state>();

	state->fg_tilemap  = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,8,8,32,32);
	state->bg1_tilemap = tilemap_create(machine, get_bg1_tile_info,tilemap_scan_rows,8,8,32,32);
	state->bg2_tilemap = tilemap_create(machine, get_bg2_tile_info,tilemap_scan_rows,8,8,32,32);

	tilemap_set_transparent_pen(state->fg_tilemap,0);
	tilemap_set_transparent_pen(state->bg1_tilemap,0);
	tilemap_set_transparent_pen(state->bg2_tilemap,0);

	tilemap_set_scroll_cols(state->bg1_tilemap, 32);
	tilemap_set_scroll_cols(state->bg2_tilemap, 32);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( vastar_fgvideoram_w )
{
	vastar_state *state = space->machine->driver_data<vastar_state>();

	state->fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( vastar_bg1videoram_w )
{
	vastar_state *state = space->machine->driver_data<vastar_state>();

	state->bg1videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg1_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( vastar_bg2videoram_w )
{
	vastar_state *state = space->machine->driver_data<vastar_state>();

	state->bg2videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg2_tilemap,offset & 0x3ff);
}


READ8_HANDLER( vastar_bg1videoram_r )
{
	vastar_state *state = space->machine->driver_data<vastar_state>();

	return state->bg1videoram[offset];
}

READ8_HANDLER( vastar_bg2videoram_r )
{
	vastar_state *state = space->machine->driver_data<vastar_state>();

	return state->bg2videoram[offset];
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	vastar_state *state = machine->driver_data<vastar_state>();
	UINT8 *spriteram = state->spriteram1;
	UINT8 *spriteram_2 = state->spriteram2;
	UINT8 *spriteram_3 = state->spriteram3;
	int offs;

	for (offs = 0; offs < 0x40; offs += 2)
	{
		int code, sx, sy, color, flipx, flipy;


		code = ((spriteram_3[offs] & 0xfc) >> 2) + ((spriteram_2[offs] & 0x01) << 6)
				+ ((offs & 0x20) << 2);

		sx = spriteram_3[offs + 1];
		sy = spriteram[offs];
		color = spriteram[offs + 1] & 0x3f;
		flipx = spriteram_3[offs] & 0x02;
		flipy = spriteram_3[offs] & 0x01;

		if (flip_screen_get(machine))
		{
			flipx = !flipx;
			flipy = !flipy;
		}

		if (spriteram_2[offs] & 0x08)	/* double width */
		{
			if (!flip_screen_get(machine))
				sy = 224 - sy;

			drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
					code/2,
					color,
					flipx,flipy,
					sx,sy,0);
			/* redraw with wraparound */
			drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
					code/2,
					color,
					flipx,flipy,
					sx,sy+256,0);
		}
		else
		{
			if (!flip_screen_get(machine))
				sy = 240 - sy;

			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					code,
					color,
					flipx,flipy,
					sx,sy,0);
		}
	}
}

VIDEO_UPDATE( vastar )
{
	vastar_state *state = screen->machine->driver_data<vastar_state>();
	int i;


	for (i = 0;i < 32;i++)
	{
		tilemap_set_scrolly(state->bg1_tilemap,i,state->bg1_scroll[i]);
		tilemap_set_scrolly(state->bg2_tilemap,i,state->bg2_scroll[i]);
	}

	switch (*state->sprite_priority)
	{
	case 0:
		tilemap_draw(bitmap,cliprect, state->bg1_tilemap, TILEMAP_DRAW_OPAQUE,0);
		draw_sprites(screen->machine, bitmap,cliprect);
		tilemap_draw(bitmap,cliprect, state->bg2_tilemap, 0,0);
		tilemap_draw(bitmap,cliprect, state->fg_tilemap, 0,0);
		break;

	case 2:
		tilemap_draw(bitmap,cliprect, state->bg1_tilemap, TILEMAP_DRAW_OPAQUE,0);
		draw_sprites(screen->machine, bitmap,cliprect);
		tilemap_draw(bitmap,cliprect, state->bg1_tilemap, 0,0);
		tilemap_draw(bitmap,cliprect, state->bg2_tilemap, 0,0);
		tilemap_draw(bitmap,cliprect, state->fg_tilemap, 0,0);
		break;

	case 3:
		tilemap_draw(bitmap,cliprect, state->bg1_tilemap, TILEMAP_DRAW_OPAQUE,0);
		tilemap_draw(bitmap,cliprect, state->bg2_tilemap, 0,0);
		tilemap_draw(bitmap,cliprect, state->fg_tilemap, 0,0);
		draw_sprites(screen->machine, bitmap,cliprect);
		break;

	default:
		logerror("Unimplemented priority %X\n", *state->sprite_priority);
		break;
	}
	return 0;
}
