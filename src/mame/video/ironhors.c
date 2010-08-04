/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/ironhors.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

PALETTE_INIT( ironhors )
{
	static const int resistances[4] = { 2000, 1000, 470, 220 };
	double rweights[4], gweights[4], bweights[4];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			4, resistances, rweights, 1000, 0,
			4, resistances, gweights, 1000, 0,
			4, resistances, bweights, 1000, 0);

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0x000] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x000] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x000] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x000] >> 3) & 0x01;
		r = combine_4_weights(rweights, bit0, bit1, bit2, bit3);

		/* green component */
		bit0 = (color_prom[i + 0x100] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x100] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x100] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x100] >> 3) & 0x01;
		g = combine_4_weights(gweights, bit0, bit1, bit2, bit3);

		/* blue component */
		bit0 = (color_prom[i + 0x200] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x200] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x200] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x200] >> 3) & 0x01;
		b = combine_4_weights(bweights, bit0, bit1, bit2, bit3);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table,*/
	color_prom += 0x300;

	/* characters use colors 0x10-0x1f of each 0x20 color bank,
       while sprites use colors 0-0x0f */
	for (i = 0; i < 0x200; i++)
	{
		int j;

		for (j = 0; j < 8; j++)
		{
			UINT8 ctabentry = (j << 5) | ((~i & 0x100) >> 4) | (color_prom[i] & 0x0f);
			colortable_entry_set_value(machine->colortable, ((i & 0x100) << 3) | (j << 8) | (i & 0xff), ctabentry);
		}
	}
}

WRITE8_HANDLER( ironhors_videoram_w )
{
	ironhors_state *state = space->machine->driver_data<ironhors_state>();
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( ironhors_colorram_w )
{
	ironhors_state *state = space->machine->driver_data<ironhors_state>();
	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( ironhors_charbank_w )
{
	ironhors_state *state = space->machine->driver_data<ironhors_state>();
	if (state->charbank != (data & 0x03))
	{
		state->charbank = data & 0x03;
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}

	state->spriterambank = data & 0x08;

	/* other bits unknown */
}

WRITE8_HANDLER( ironhors_palettebank_w )
{
	ironhors_state *state = space->machine->driver_data<ironhors_state>();
	if (state->palettebank != (data & 0x07))
	{
		state->palettebank = data & 0x07;
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}

	coin_counter_w(space->machine, 0, data & 0x10);
	coin_counter_w(space->machine, 1, data & 0x20);

	/* bit 6 unknown - set after game over */

	if (data & 0x88)
		popmessage("ironhors_palettebank_w %02x",data);
}

WRITE8_HANDLER( ironhors_flipscreen_w )
{
	if (flip_screen_get(space->machine) != (~data & 0x08))
	{
		flip_screen_set(space->machine, ~data & 0x08);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}

	/* other bits are used too, but unknown */
}

static TILE_GET_INFO( get_bg_tile_info )
{
	ironhors_state *state = machine->driver_data<ironhors_state>();
	int code = state->videoram[tile_index] + ((state->colorram[tile_index] & 0x40) << 2) +
		((state->colorram[tile_index] & 0x20) << 4) + (state->charbank << 10);
	int color = (state->colorram[tile_index] & 0x0f) + 16 * state->palettebank;
	int flags = ((state->colorram[tile_index] & 0x10) ? TILE_FLIPX : 0) |
		((state->colorram[tile_index] & 0x20) ? TILE_FLIPY : 0);

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( ironhors )
{
	ironhors_state *state = machine->driver_data<ironhors_state>();
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_scroll_rows(state->bg_tilemap, 32);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	ironhors_state *state = machine->driver_data<ironhors_state>();
	int offs;
	UINT8 *sr;

	if (state->spriterambank != 0)
		sr = state->spriteram;
	else
		sr = state->spriteram2;

	for (offs = 0; offs < state->spriteram_size; offs += 5)
	{
		int sx = sr[offs + 3];
		int sy = sr[offs + 2];
		int flipx = sr[offs + 4] & 0x20;
		int flipy = sr[offs + 4] & 0x40;
		int code = (sr[offs] << 2) + ((sr[offs + 1] & 0x03) << 10) + ((sr[offs + 1] & 0x0c) >> 2);
		int color = ((sr[offs + 1] & 0xf0) >> 4) + 16 * state->palettebank;
	//  int mod = flip_screen_get(machine) ? -8 : 8;

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		switch (sr[offs + 4] & 0x0c)
		{
			case 0x00:	/* 16x16 */
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
						code/4,
						color,
						flipx,flipy,
						sx,sy,0);
				break;

			case 0x04:	/* 16x8 */
				{
					if (flip_screen_get(machine)) sy += 8; // this fixes the train wheels' position

					drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
							code & ~1,
							color,
							flipx,flipy,
							flipx?sx+8:sx,sy,0);
					drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
							code | 1,
							color,
							flipx,flipy,
							flipx?sx:sx+8,sy,0);
				}
				break;

			case 0x08:	/* 8x16 */
				{
					drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
							code & ~2,
							color,
							flipx,flipy,
							sx,flipy?sy+8:sy,0);
					drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
							code | 2,
							color,
							flipx,flipy,
							sx,flipy?sy:sy+8,0);
				}
				break;

			case 0x0c:	/* 8x8 */
				{
					drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
							code,
							color,
							flipx,flipy,
							sx,sy,0);
				}
				break;
		}
	}
}

VIDEO_UPDATE( ironhors )
{
	ironhors_state *state = screen->machine->driver_data<ironhors_state>();
	int row;

	for (row = 0; row < 32; row++)
		tilemap_set_scrollx(state->bg_tilemap, row, state->scroll[row]);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}

static TILE_GET_INFO( farwest_get_bg_tile_info )
{
	ironhors_state *state = machine->driver_data<ironhors_state>();
	int code = state->videoram[tile_index] + ((state->colorram[tile_index] & 0x40) << 2) +
		((state->colorram[tile_index] & 0x20) << 4) + (state->charbank << 10);
	int color = (state->colorram[tile_index] & 0x0f) + 16 * state->palettebank;
	int flags = 0;//((state->colorram[tile_index] & 0x10) ? TILE_FLIPX : 0) |  ((state->colorram[tile_index] & 0x20) ? TILE_FLIPY : 0);

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( farwest )
{
	ironhors_state *state = machine->driver_data<ironhors_state>();
	state->bg_tilemap = tilemap_create(machine, farwest_get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_scroll_rows(state->bg_tilemap, 32);
}

static void farwest_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	ironhors_state *state = machine->driver_data<ironhors_state>();
	int offs;
	UINT8 *sr = state->spriteram2;
	UINT8 *sr2 = state->spriteram;

	for (offs = 0; offs < state->spriteram_size; offs += 4)
	{
		int sx = sr[offs + 2];
		int sy = sr[offs + 1];
		int flipx = sr[offs + 3] & 0x20;
		int flipy = sr[offs + 3] & 0x40;
		int code = (sr[offs] << 2) + ((sr2[offs] & 0x03) << 10) + ((sr2[offs] & 0x0c) >> 2);
		int color = ((sr2[offs] & 0xf0) >> 4) + 16 * state->palettebank;

	//  int mod = flip_screen_get() ? -8 : 8;

//      if (flip_screen_get())
		{
		//  sx = 240 - sx;
			sy = 240 - sy;
		//  flipx = !flipx;
		//  flipy = !flipy;
		}

		switch (sr[offs + 3] & 0x0c)
		{
			case 0x00:	/* 16x16 */
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
						code/4,
						color,
						flipx,flipy,
						sx,sy,0);
				break;

			case 0x04:	/* 16x8 */
				{
					if (flip_screen_get(machine)) sy += 8; // this fixes the train wheels' position

					drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
							code & ~1,
							color,
							flipx,flipy,
							flipx?sx+8:sx,sy,0);
					drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
							code | 1,
							color,
							flipx,flipy,
							flipx?sx:sx+8,sy,0);
				}
				break;

			case 0x08:	/* 8x16 */
				{
					drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
							code & ~2,
							color,
							flipx,flipy,
							sx,flipy?sy+8:sy,0);
					drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
							code | 2,
							color,
							flipx,flipy,
							sx,flipy?sy:sy+8,0);
				}
				break;

			case 0x0c:	/* 8x8 */
				{
					drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
							code,
							color,
							flipx,flipy,
							sx,sy,0);
				}
				break;
		}
	}
}

VIDEO_UPDATE( farwest)
{
	ironhors_state *state = screen->machine->driver_data<ironhors_state>();
	int row;

	for (row = 0; row < 32; row++)
		tilemap_set_scrollx(state->bg_tilemap, row, state->scroll[row]);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	farwest_draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}

