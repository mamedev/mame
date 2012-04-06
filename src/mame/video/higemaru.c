#include "emu.h"
#include "includes/higemaru.h"

WRITE8_MEMBER(higemaru_state::higemaru_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(higemaru_state::higemaru_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

PALETTE_INIT( higemaru )
{
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x20);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* characters use colors 0-15 */
	for (i = 0; i < 0x80; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}

	/* sprites use colors 16-31 */
	for (i = 0x80; i < 0x180; i++)
	{
		UINT8 ctabentry = (color_prom[i + 0x80] & 0x0f) | 0x10;
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}
}

WRITE8_MEMBER(higemaru_state::higemaru_c800_w)
{
	if (data & 0x7c)
		logerror("c800 = %02x\n",data);

	/* bits 0 and 1 are coin counters */
	coin_counter_w(machine(), 0,data & 2);
	coin_counter_w(machine(), 1,data & 1);

	/* bit 7 flips screen */
	if (flip_screen_get(machine()) != (data & 0x80))
	{
		flip_screen_set(machine(), data & 0x80);
		m_bg_tilemap->mark_all_dirty();
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	higemaru_state *state = machine.driver_data<higemaru_state>();
	int code = state->m_videoram[tile_index] + ((state->m_colorram[tile_index] & 0x80) << 1);
	int color = state->m_colorram[tile_index] & 0x1f;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( higemaru )
{
	higemaru_state *state = machine.driver_data<higemaru_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	higemaru_state *state = machine.driver_data<higemaru_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = state->m_spriteram_size - 16; offs >= 0; offs -= 16)
	{
		int code,col,sx,sy,flipx,flipy;

		code = spriteram[offs] & 0x7f;
		col = spriteram[offs + 4] & 0x0f;
		sx = spriteram[offs + 12];
		sy = spriteram[offs + 8];
		flipx = spriteram[offs + 4] & 0x10;
		flipy = spriteram[offs + 4] & 0x20;
		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				code,
				col,
				flipx,flipy,
				sx,sy,15);

		/* draw again with wraparound */
		drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				code,
				col,
				flipx,flipy,
				sx - 256,sy,15);
	}
}

SCREEN_UPDATE_IND16( higemaru )
{
	higemaru_state *state = screen.machine().driver_data<higemaru_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
