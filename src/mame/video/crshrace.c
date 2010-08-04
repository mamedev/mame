#include "emu.h"
#include "video/konicdev.h"
#include "includes/crshrace.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info1 )
{
	crshrace_state *state = machine->driver_data<crshrace_state>();
	int code = state->videoram1[tile_index];

	SET_TILE_INFO(1, (code & 0xfff) + (state->roz_bank << 12), code >> 12, 0);
}

static TILE_GET_INFO( get_tile_info2 )
{
	crshrace_state *state = machine->driver_data<crshrace_state>();
	int code = state->videoram2[tile_index];

	SET_TILE_INFO(0, code, 0, 0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( crshrace )
{
	crshrace_state *state = machine->driver_data<crshrace_state>();

	state->tilemap1 = tilemap_create(machine, get_tile_info1, tilemap_scan_rows, 16, 16, 64, 64);
	state->tilemap2 = tilemap_create(machine, get_tile_info2, tilemap_scan_rows, 8, 8, 64, 64);

	tilemap_set_transparent_pen(state->tilemap1, 0x0f);
	tilemap_set_transparent_pen(state->tilemap2, 0xff);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( crshrace_videoram1_w )
{
	crshrace_state *state = space->machine->driver_data<crshrace_state>();

	COMBINE_DATA(&state->videoram1[offset]);
	tilemap_mark_tile_dirty(state->tilemap1, offset);
}

WRITE16_HANDLER( crshrace_videoram2_w )
{
	crshrace_state *state = space->machine->driver_data<crshrace_state>();

	COMBINE_DATA(&state->videoram2[offset]);
	tilemap_mark_tile_dirty(state->tilemap2, offset);
}

WRITE16_HANDLER( crshrace_roz_bank_w )
{
	crshrace_state *state = space->machine->driver_data<crshrace_state>();

	if (ACCESSING_BITS_0_7)
	{
		if (state->roz_bank != (data & 0xff))
		{
			state->roz_bank = data & 0xff;
			tilemap_mark_all_tiles_dirty(state->tilemap1);
		}
	}
}


WRITE16_HANDLER( crshrace_gfxctrl_w )
{
	crshrace_state *state = space->machine->driver_data<crshrace_state>();

	if (ACCESSING_BITS_0_7)
	{
		state->gfxctrl = data & 0xdf;
		state->flipscreen = data & 0x20;
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	crshrace_state *state = machine->driver_data<crshrace_state>();
	UINT16 *buffered_spriteram = machine->generic.buffered_spriteram.u16;
	UINT16 *buffered_spriteram_2 = machine->generic.buffered_spriteram2.u16;
	int offs;

	offs = 0;
	while (offs < 0x0400 && (buffered_spriteram[offs] & 0x4000) == 0)
	{
		int attr_start;
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color;
		/* table hand made by looking at the ship explosion in aerofgt attract mode */
		/* it's almost a logarithmic scale but not exactly */
		static const int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		attr_start = 4 * (buffered_spriteram[offs++] & 0x03ff);

		ox = buffered_spriteram[attr_start + 1] & 0x01ff;
		xsize = (buffered_spriteram[attr_start + 1] & 0x0e00) >> 9;
		zoomx = (buffered_spriteram[attr_start + 1] & 0xf000) >> 12;
		oy = buffered_spriteram[attr_start + 0] & 0x01ff;
		ysize = (buffered_spriteram[attr_start + 0] & 0x0e00) >> 9;
		zoomy = (buffered_spriteram[attr_start + 0] & 0xf000) >> 12;
		flipx = buffered_spriteram[attr_start + 2] & 0x4000;
		flipy = buffered_spriteram[attr_start + 2] & 0x8000;
		color = (buffered_spriteram[attr_start + 2] & 0x1f00) >> 8;
		map_start = buffered_spriteram[attr_start + 3] & 0x7fff;

		zoomx = 16 - zoomtable[zoomx] / 8;
		zoomy = 16 - zoomtable[zoomy] / 8;

		if (buffered_spriteram[attr_start + 2] & 0x20ff) color = mame_rand(machine);

		for (y = 0; y <= ysize; y++)
		{
			int sx,sy;

			if (flipy) sy = ((oy + zoomy * (ysize - y) + 16) & 0x1ff) - 16;
			else sy = ((oy + zoomy * y + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx) sx = ((ox + zoomx * (xsize - x) + 16) & 0x1ff) - 16;
				else sx = ((ox + zoomx * x + 16) & 0x1ff) - 16;

				code = buffered_spriteram_2[map_start & 0x7fff];
				map_start++;

				if (state->flipscreen)
					drawgfxzoom_transpen(bitmap,cliprect,machine->gfx[2],
							code,
							color,
							!flipx,!flipy,
							304-sx,208-sy,
							0x1000 * zoomx,0x1000 * zoomy,15);
				else
					drawgfxzoom_transpen(bitmap,cliprect,machine->gfx[2],
							code,
							color,
							flipx,flipy,
							sx,sy,
							0x1000 * zoomx,0x1000 * zoomy,15);
			}
		}
	}
}


static void draw_bg( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	crshrace_state *state = machine->driver_data<crshrace_state>();
	tilemap_draw(bitmap, cliprect, state->tilemap2, 0, 0);
}


static void draw_fg(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	crshrace_state *state = machine->driver_data<crshrace_state>();
	k053936_zoom_draw(state->k053936, bitmap, cliprect, state->tilemap1, 0, 0, 1);
}


VIDEO_UPDATE( crshrace )
{
	crshrace_state *state = screen->machine->driver_data<crshrace_state>();

	if (state->gfxctrl & 0x04)	/* display disable? */
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
		return 0;
	}

	bitmap_fill(bitmap, cliprect, 0x1ff);

	switch (state->gfxctrl & 0xfb)
	{
		case 0x00:	/* high score screen */
			draw_sprites(screen->machine, bitmap, cliprect);
			draw_bg(screen->machine, bitmap, cliprect);
			draw_fg(screen->machine, bitmap, cliprect);
			break;
		case 0x01:
		case 0x02:
			draw_bg(screen->machine, bitmap, cliprect);
			draw_fg(screen->machine, bitmap, cliprect);
			draw_sprites(screen->machine, bitmap, cliprect);
			break;
		default:
			popmessage("gfxctrl = %02x", state->gfxctrl);
			break;
	}
	return 0;
}

VIDEO_EOF( crshrace )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	buffer_spriteram16_w(space, 0, 0, 0xffff);
	buffer_spriteram16_2_w(space, 0, 0, 0xffff);
}
