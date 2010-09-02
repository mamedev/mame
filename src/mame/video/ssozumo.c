/***************************************************************************

Syusse Oozumou
(c) 1984 Technos Japan (Licensed by Data East)

Driver by Takahiro Nogi (nogi@kt.rim.or.jp) 1999/10/04

***************************************************************************/

#include "emu.h"
#include "includes/ssozumo.h"

/**************************************************************************/

PALETTE_INIT( ssozumo )
{
	int	bit0, bit1, bit2, bit3, r, g, b;
	int	i;

	for (i = 0 ; i < 64 ; i++)
	{
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[64] >> 0) & 0x01;
		bit1 = (color_prom[64] >> 1) & 0x01;
		bit2 = (color_prom[64] >> 2) & 0x01;
		bit3 = (color_prom[64] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}
}

WRITE8_HANDLER( ssozumo_videoram_w )
{
	ssozumo_state *state = space->machine->driver_data<ssozumo_state>();

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( ssozumo_colorram_w )
{
	ssozumo_state *state = space->machine->driver_data<ssozumo_state>();

	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( ssozumo_videoram2_w )
{
	ssozumo_state *state = space->machine->driver_data<ssozumo_state>();

	state->videoram2[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( ssozumo_colorram2_w )
{
	ssozumo_state *state = space->machine->driver_data<ssozumo_state>();

	state->colorram2[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( ssozumo_paletteram_w )
{
	ssozumo_state *state = space->machine->driver_data<ssozumo_state>();
	int	bit0, bit1, bit2, bit3, val;
	int	r, g, b;
	int	offs2;

	state->paletteram[offset] = data;
	offs2 = offset & 0x0f;

	val = state->paletteram[offs2];
	bit0 = (val >> 0) & 0x01;
	bit1 = (val >> 1) & 0x01;
	bit2 = (val >> 2) & 0x01;
	bit3 = (val >> 3) & 0x01;
	r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	val = state->paletteram[offs2 | 0x10];
	bit0 = (val >> 0) & 0x01;
	bit1 = (val >> 1) & 0x01;
	bit2 = (val >> 2) & 0x01;
	bit3 = (val >> 3) & 0x01;
	g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	val = state->paletteram[offs2 | 0x20];
	bit0 = (val >> 0) & 0x01;
	bit1 = (val >> 1) & 0x01;
	bit2 = (val >> 2) & 0x01;
	bit3 = (val >> 3) & 0x01;
	b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	palette_set_color(space->machine, offs2 + 64, MAKE_RGB(r, g, b));
}

WRITE8_HANDLER( ssozumo_scroll_w )
{
	ssozumo_state *state = space->machine->driver_data<ssozumo_state>();

	tilemap_set_scrolly(state->bg_tilemap, 0, data);
}

WRITE8_HANDLER( ssozumo_flipscreen_w )
{
	flip_screen_set(space->machine, data & 0x80);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	ssozumo_state *state = machine->driver_data<ssozumo_state>();
	int code = state->videoram[tile_index] + ((state->colorram[tile_index] & 0x08) << 5);
	int color = (state->colorram[tile_index] & 0x30) >> 4;
	int flags = ((tile_index % 32) >= 16) ? TILE_FLIPY : 0;

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	ssozumo_state *state = machine->driver_data<ssozumo_state>();
	int code = state->videoram2[tile_index] + 256 * (state->colorram2[tile_index] & 0x07);
	int color = (state->colorram2[tile_index] & 0x30) >> 4;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( ssozumo )
{
	ssozumo_state *state = machine->driver_data<ssozumo_state>();

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols_flip_x,
		 16, 16, 16, 32);

	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols_flip_x,
		 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 0);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	ssozumo_state *state = machine->driver_data<ssozumo_state>();
	UINT8 *spriteram = state->spriteram;
	int offs;

	for (offs = 0; offs < state->spriteram_size; offs += 4)
	{
		if (spriteram[offs] & 0x01)
		{
			int code = spriteram[offs + 1] + ((spriteram[offs] & 0xf0) << 4);
			int color = (spriteram[offs] & 0x08) >> 3;
			int flipx = spriteram[offs] & 0x04;
			int flipy = spriteram[offs] & 0x02;
			int sx = 239 - spriteram[offs + 3];
			int sy = (240 - spriteram[offs + 2]) & 0xff;

			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transpen(bitmap, cliprect,
				machine->gfx[2],
				code, color,
				flipx, flipy,
				sx, sy, 0);
		}
	}
}

VIDEO_UPDATE( ssozumo )
{
	ssozumo_state *state = screen->machine->driver_data<ssozumo_state>();

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
