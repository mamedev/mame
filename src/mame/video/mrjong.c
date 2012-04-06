/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/mrjong.h"


/***************************************************************************

  Convert the color PROMs. (from video/pengo.c)

***************************************************************************/

PALETTE_INIT( mrjong )
{
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x10);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x10; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* characters/sprites */
	for (i = 0; i < 0x80; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}
}


/***************************************************************************

  Display control parameter.

***************************************************************************/

WRITE8_MEMBER(mrjong_state::mrjong_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(mrjong_state::mrjong_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(mrjong_state::mrjong_flipscreen_w)
{
	if (flip_screen_get(machine()) != BIT(data, 2))
	{
		flip_screen_set(machine(), BIT(data, 2));
		machine().tilemap().mark_all_dirty();
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	mrjong_state *state = machine.driver_data<mrjong_state>();
	int code = state->m_videoram[tile_index] | ((state->m_colorram[tile_index] & 0x20) << 3);
	int color = state->m_colorram[tile_index] & 0x1f;
	int flags = ((state->m_colorram[tile_index] & 0x40) ? TILE_FLIPX : 0) | ((state->m_colorram[tile_index] & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( mrjong )
{
	mrjong_state *state = machine.driver_data<mrjong_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows_flip_xy, 8, 8, 32, 32);
}

/*
Note: First 0x40 entries in the videoram are actually spriteram
*/
static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	mrjong_state *state = machine.driver_data<mrjong_state>();
	int offs;

	for (offs = (0x40 - 4); offs >= 0; offs -= 4)
	{
		int sprt;
		int color;
		int sx, sy;
		int flipx, flipy;

		sprt = (((state->m_videoram[offs + 1] >> 2) & 0x3f) | ((state->m_videoram[offs + 3] & 0x20) << 1));
		flipx = (state->m_videoram[offs + 1] & 0x01) >> 0;
		flipy = (state->m_videoram[offs + 1] & 0x02) >> 1;
		color = (state->m_videoram[offs + 3] & 0x1f);

		sx = 224 - state->m_videoram[offs + 2];
		sy = state->m_videoram[offs + 0];
		if (flip_screen_get(machine))
		{
			sx = 208 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, machine.gfx[1],
				sprt,
				color,
				flipx, flipy,
				sx, sy, 0);
	}
}

SCREEN_UPDATE_IND16( mrjong )
{
	mrjong_state *state = screen.machine().driver_data<mrjong_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
