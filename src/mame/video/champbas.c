#include "emu.h"
#include "video/resnet.h"
#include "includes/champbas.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT( champbas )
{
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3, &resistances_rg[0], rweights, 0, 0,
			3, &resistances_rg[0], gweights, 0, 0,
			2, &resistances_b[0],  bweights, 0, 0);

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
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(bweights, bit0, bit1);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	color_prom += 0x20;

	for (i = 0; i < 0x200; i++)
	{
		UINT8 ctabentry = (color_prom[i & 0xff] & 0x0f) | ((i & 0x100) >> 4);
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}
}


PALETTE_INIT( exctsccr )
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

	/* characters / sprites (3bpp) */
	for (i = 0; i < 0x100; i++)
	{
		int swapped_i = BITSWAP8(i, 2, 7, 6, 5, 4, 3, 1, 0);
		UINT8 ctabentry = (color_prom[swapped_i] & 0x0f) | ((i & 0x80) >> 3);
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}

	/* sprites (4bpp) */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[0x100 + i] & 0x0f) | 0x10;
		colortable_entry_set_value(machine.colortable, i + 0x100, ctabentry);
	}
}



static TILE_GET_INFO( champbas_get_bg_tile_info )
{
	champbas_state *state = machine.driver_data<champbas_state>();
	int code = state->m_bg_videoram[tile_index] | (state->m_gfx_bank << 8);
	int color = (state->m_bg_videoram[tile_index + 0x400] & 0x1f) | 0x20;

	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( exctsccr_get_bg_tile_info )
{
	champbas_state *state = machine.driver_data<champbas_state>();
	int code = state->m_bg_videoram[tile_index] | (state->m_gfx_bank << 8);
	int color = state->m_bg_videoram[tile_index + 0x400] & 0x0f;

	SET_TILE_INFO(0, code, color, 0);
}



VIDEO_START( champbas )
{
	champbas_state *state = machine.driver_data<champbas_state>();
	state->m_bg_tilemap = tilemap_create(machine, champbas_get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

VIDEO_START( exctsccr )
{
	champbas_state *state = machine.driver_data<champbas_state>();
	state->m_bg_tilemap = tilemap_create(machine, exctsccr_get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}



WRITE8_MEMBER(champbas_state::champbas_bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(champbas_state::champbas_gfxbank_w)
{
	data &= 1;
	if (m_gfx_bank != data)
	{
		m_gfx_bank = data;
		m_bg_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(champbas_state::champbas_palette_bank_w)
{
	m_palette_bank = data & 1;
	m_bg_tilemap->set_palette_offset(m_palette_bank << 8);
}

WRITE8_MEMBER(champbas_state::champbas_flipscreen_w)
{
	flip_screen_set(machine(), ~data & 1);
}



static void champbas_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	champbas_state *state = machine.driver_data<champbas_state>();
	int offs;
	const gfx_element* const gfx = machine.gfx[1];

	for (offs = state->m_spriteram_size - 2; offs >= 0; offs -= 2)
	{
		int code = (state->m_spriteram[offs] >> 2) | (state->m_gfx_bank << 6);
		int color = (state->m_spriteram[offs + 1] & 0x1f) | (state->m_palette_bank << 6);
		int flipx = ~state->m_spriteram[offs] & 0x01;
		int flipy = ~state->m_spriteram[offs] & 0x02;
		int sx = state->m_spriteram_2[offs + 1] - 16;
		int sy = 255 - state->m_spriteram_2[offs];

		drawgfx_transmask(bitmap, cliprect,
				gfx,
				code, color,
				flipx, flipy,
				sx, sy,
				colortable_get_transpen_mask(machine.colortable, gfx, color, 0));

		// wraparound
		drawgfx_transmask(bitmap, cliprect,
				gfx,
				code, color,
				flipx, flipy,
				sx + 256, sy,
				colortable_get_transpen_mask(machine.colortable, gfx, color, 0));
	}
}

static void exctsccr_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	champbas_state *state = machine.driver_data<champbas_state>();
	int offs;
	UINT8 *obj1, *obj2;

	obj1 = state->m_bg_videoram;
	obj2 = &(state->m_spriteram[0x20]);

	for (offs = 0x0e; offs >= 0; offs -= 2)
	{
		int sx, sy, code, bank, flipx, flipy, color;

		sx = obj2[offs + 1] - 16;
		sy = 255 - obj2[offs];

		code = (obj1[offs] >> 2) & 0x3f;
		flipx = (~obj1[offs]) & 0x01;
		flipy = (~obj1[offs]) & 0x02;
		color = (obj1[offs + 1]) & 0x0f;
		bank = ((obj1[offs + 1] >> 4) & 1);

		drawgfx_transpen(bitmap,cliprect,
				machine.gfx[1],
				code + (bank << 6),
				color,
				flipx, flipy,
				sx,sy,0);
	}

	obj1 = state->m_spriteram_2;
	obj2 = state->m_spriteram;

	for (offs = 0x0e; offs >= 0; offs -= 2)
	{
		int sx, sy, code, flipx, flipy, color;

		sx = obj2[offs + 1] - 16;
		sy = 255 - obj2[offs];

		code = (obj1[offs] >> 2) & 0x3f;
		flipx = (~obj1[offs]) & 0x01;
		flipy = (~obj1[offs]) & 0x02;
		color = (obj1[offs + 1]) & 0x0f;

		drawgfx_transmask(bitmap,cliprect,
				machine.gfx[2],
				code,
				color,
				flipx, flipy,
				sx,sy,
				colortable_get_transpen_mask(machine.colortable, machine.gfx[2], color, 0x10));
	}
}



SCREEN_UPDATE_IND16( champbas )
{
	champbas_state *state = screen.machine().driver_data<champbas_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	champbas_draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE_IND16( exctsccr )
{
	champbas_state *state = screen.machine().driver_data<champbas_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	exctsccr_draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
