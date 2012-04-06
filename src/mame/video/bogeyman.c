#include "emu.h"
#include "includes/bogeyman.h"


PALETTE_INIT( bogeyman )
{
	int i;

	/* first 16 colors are RAM */

	for (i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[256] >> 0) & 0x01;
		bit2 = (color_prom[256] >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[256] >> 2) & 0x01;
		bit2 = (color_prom[256] >> 3) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i + 16, MAKE_RGB(r,g,b));
		color_prom++;
	}
}

WRITE8_MEMBER(bogeyman_state::bogeyman_videoram_w)
{

	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(bogeyman_state::bogeyman_colorram_w)
{

	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(bogeyman_state::bogeyman_videoram2_w)
{

	m_videoram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(bogeyman_state::bogeyman_colorram2_w)
{

	m_colorram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(bogeyman_state::bogeyman_paletteram_w)
{
	/* RGB output is inverted */
	paletteram_BBGGGRRR_w(space, offset, ~data);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	bogeyman_state *state = machine.driver_data<bogeyman_state>();
	int attr = state->m_colorram[tile_index];
	int gfxbank = ((((attr & 0x01) << 8) + state->m_videoram[tile_index]) / 0x80) + 3;
	int code = state->m_videoram[tile_index] & 0x7f;
	int color = (attr >> 1) & 0x07;

	SET_TILE_INFO(gfxbank, code, color, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	bogeyman_state *state = machine.driver_data<bogeyman_state>();
	int attr = state->m_colorram2[tile_index];
	int tile = state->m_videoram2[tile_index] | ((attr & 0x03) << 8);
	int gfxbank = tile / 0x200;
	int code = tile & 0x1ff;

	SET_TILE_INFO(gfxbank, code, state->m_colbank, 0);
}

VIDEO_START( bogeyman )
{
	bogeyman_state *state = machine.driver_data<bogeyman_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 16, 16);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_fg_tilemap->set_transparent_pen(0);
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	bogeyman_state *state = machine.driver_data<bogeyman_state>();
	int offs;

	for (offs = 0; offs < state->m_spriteram_size; offs += 4)
	{
		int attr = state->m_spriteram[offs];

		if (attr & 0x01)
		{
			int code = state->m_spriteram[offs + 1] + ((attr & 0x40) << 2);
			int color = (attr & 0x08) >> 3;
			int flipx = !(attr & 0x04);
			int flipy = attr & 0x02;
			int sx = state->m_spriteram[offs + 3];
			int sy = (240 - state->m_spriteram[offs + 2]) & 0xff;
			int multi = attr & 0x10;

			if (multi) sy -= 16;

			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transpen(bitmap, cliprect,
				machine.gfx[2],
				code, color,
				flipx, flipy,
				sx, sy, 0);

			if (multi)
			{
				drawgfx_transpen(bitmap,cliprect,
					machine.gfx[2],
					code + 1, color,
					flipx, flipy,
					sx, sy + (flip_screen_get(machine) ? -16 : 16), 0);
			}
		}
	}
}

SCREEN_UPDATE_IND16( bogeyman )
{
	bogeyman_state *state = screen.machine().driver_data<bogeyman_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
