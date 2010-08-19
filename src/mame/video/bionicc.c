/***************************************************************************

    Bionic Commando Video Hardware

    This board handles tile/tile and tile/sprite priority with a PROM. Its
    working is complicated and hardcoded in the driver.

    The PROM is a 256x4 chip, with address inputs wired as follows:

    A0 bg opaque
    A1 \
    A2 |  fg pen
    A3 |
    A4 /
    A5 fg has priority over sprites (bit 5 of tile attribute)
    A6 fg has not priority over bg (bits 6 & 7 of tile attribute both set)
    A7 sprite opaque

    The output selects the active layer, it can be:
    0  bg
    1  fg
    2  sprite

***************************************************************************/

#include "emu.h"
#include "includes/bionicc.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	bionicc_state *state = machine->driver_data<bionicc_state>();

	int attr = state->bgvideoram[2 * tile_index + 1];
	SET_TILE_INFO(
			1,
			(state->bgvideoram[2 * tile_index] & 0xff) + ((attr & 0x07) << 8),
			(attr & 0x18) >> 3,
			TILE_FLIPXY((attr & 0xc0) >> 6));
}

static TILE_GET_INFO( get_fg_tile_info )
{
	bionicc_state *state = machine->driver_data<bionicc_state>();

	int attr = state->fgvideoram[2 * tile_index + 1];
	int flags;

	if ((attr & 0xc0) == 0xc0)
	{
		tileinfo->category = 1;
		tileinfo->group = 0;
		flags = 0;
	}
	else
	{
		tileinfo->category = 0;
		tileinfo->group = (attr & 0x20) >> 5;
		flags = TILE_FLIPXY((attr & 0xc0) >> 6);
	}

	SET_TILE_INFO(
			2,
			(state->fgvideoram[2 * tile_index] & 0xff) + ((attr & 0x07) << 8),
			(attr & 0x18) >> 3,
			flags);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	bionicc_state *state = machine->driver_data<bionicc_state>();

	int attr = state->txvideoram[tile_index + 0x400];
	SET_TILE_INFO(
			0,
			(state->txvideoram[tile_index] & 0xff) + ((attr & 0x00c0) << 2),
			attr & 0x3f,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( bionicc )
{
	bionicc_state *state = machine->driver_data<bionicc_state>();

	state->tx_tilemap = tilemap_create(machine, get_tx_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,  8, 8, 64, 64);

	tilemap_set_transparent_pen(state->tx_tilemap, 3);
	tilemap_set_transmask(state->fg_tilemap, 0, 0xffff, 0x8000); /* split type 0 is completely transparent in front half */
	tilemap_set_transmask(state->fg_tilemap, 1, 0xffc1, 0x803e); /* split type 1 has pens 1-5 opaque in front half */
	tilemap_set_transparent_pen(state->bg_tilemap, 15);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( bionicc_bgvideoram_w )
{
	bionicc_state *state = space->machine->driver_data<bionicc_state>();

	COMBINE_DATA(&state->bgvideoram[offset]);
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

WRITE16_HANDLER( bionicc_fgvideoram_w )
{
	bionicc_state *state = space->machine->driver_data<bionicc_state>();

	COMBINE_DATA(&state->fgvideoram[offset]);
	tilemap_mark_tile_dirty(state->fg_tilemap, offset / 2);
}

WRITE16_HANDLER( bionicc_txvideoram_w )
{
	bionicc_state *state = space->machine->driver_data<bionicc_state>();

	COMBINE_DATA(&state->txvideoram[offset]);
	tilemap_mark_tile_dirty(state->tx_tilemap, offset & 0x3ff);
}

WRITE16_HANDLER( bionicc_paletteram_w )
{
	bionicc_state *state = space->machine->driver_data<bionicc_state>();
	int r, g, b, bright;
	data = COMBINE_DATA(&state->paletteram[offset]);

	bright = (data & 0x0f);

	r = ((data >> 12) & 0x0f) * 0x11;
	g = ((data >> 8 ) & 0x0f) * 0x11;
	b = ((data >> 4 ) & 0x0f) * 0x11;

	if ((bright & 0x08) == 0)
	{
		r = r * (0x07 + bright) / 0x0e;
		g = g * (0x07 + bright) / 0x0e;
		b = b * (0x07 + bright) / 0x0e;
	}

	palette_set_color (space->machine, offset, MAKE_RGB(r, g, b));
}

WRITE16_HANDLER( bionicc_scroll_w )
{
	bionicc_state *state = space->machine->driver_data<bionicc_state>();

	data = COMBINE_DATA(&state->scroll[offset]);

	switch (offset)
	{
		case 0:
			tilemap_set_scrollx(state->fg_tilemap, 0, data);
			break;
		case 1:
			tilemap_set_scrolly(state->fg_tilemap, 0, data);
			break;
		case 2:
			tilemap_set_scrollx(state->bg_tilemap, 0, data);
			break;
		case 3:
			tilemap_set_scrolly(state->bg_tilemap, 0, data);
			break;
	}
}

WRITE16_HANDLER( bionicc_gfxctrl_w )
{
	bionicc_state *state = space->machine->driver_data<bionicc_state>();

	if (ACCESSING_BITS_8_15)
	{
		flip_screen_set(space->machine, data & 0x0100);

		tilemap_set_enable(state->bg_tilemap, data & 0x2000);	/* guess */
		tilemap_set_enable(state->fg_tilemap, data & 0x1000);	/* guess */

		coin_counter_w(space->machine, 0, data & 0x8000);
		coin_counter_w(space->machine, 1, data & 0x4000);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT16 *buffered_spriteram = machine->generic.buffered_spriteram.u16;
//  bionicc_state *state = machine->driver_data<bionicc_state>();
	int offs;
	const gfx_element *gfx = machine->gfx[3];

	for (offs = (machine->generic.spriteram_size - 8) / 2; offs >= 0; offs -= 4)
	{
		int tile_number = buffered_spriteram[offs] & 0x7ff;
		if( tile_number != 0x7ff )
		{
			int attr = buffered_spriteram[offs + 1];
			int color = (attr & 0x3c) >> 2;
			int flipx = attr & 0x02;
			int flipy = 0;
			int sx = (INT16)buffered_spriteram[offs + 3];	/* signed */
			int sy = (INT16)buffered_spriteram[offs + 2];	/* signed */

			if (sy > 512 - 16)
				sy -= 512;

			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transpen( bitmap, cliprect,gfx,
				tile_number,
				color,
				flipx,flipy,
				sx,sy,15);
		}
	}
}

VIDEO_UPDATE( bionicc )
{
	bionicc_state *state = screen->machine->driver_data<bionicc_state>();

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 1 | TILEMAP_DRAW_LAYER1, 0);	/* nothing in FRONT */
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0 | TILEMAP_DRAW_LAYER1, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0 | TILEMAP_DRAW_LAYER0, 0);
	tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);
	return 0;
}

VIDEO_EOF( bionicc )
{
	address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	buffer_spriteram16_w(space, 0, 0, 0xffff);
}
