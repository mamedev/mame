/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/gyruss.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Gyruss has one 32x8 palette PROM and two 256x4 lookup table PROMs
  (one for characters, one for sprites).
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

PALETTE_INIT( gyruss )
{
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double weights_rg[3], weights_b[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3, resistances_rg, weights_rg, 470, 0,
			2, resistances_b,  weights_b,  470, 0,
			0, 0, 0, 0, 0);

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 32);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(weights_rg, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(weights_rg, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(weights_b, bit0, bit1);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 32;

	/* sprites map to the lower 16 palette entries */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}

	/* characters map to the upper 16 palette entries */
	for (i = 0x100; i < 0x140; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine.colortable, i, ctabentry + 0x10);
	}
}



WRITE8_MEMBER(gyruss_state::gyruss_spriteram_w)
{
	machine().primary_screen->update_now();
	m_spriteram[offset] = data;
}


static TILE_GET_INFO( gyruss_get_tile_info )
{
	gyruss_state *state = machine.driver_data<gyruss_state>();
	int code = ((state->m_colorram[tile_index] & 0x20) << 3) | state->m_videoram[tile_index];
	int color = state->m_colorram[tile_index] & 0x0f;
	int flags = TILE_FLIPYX(state->m_colorram[tile_index] >> 6);

	tileinfo.group = (state->m_colorram[tile_index] & 0x10) ? 0 : 1;

	SET_TILE_INFO(2, code, color, flags);
}


VIDEO_START( gyruss )
{
	gyruss_state *state = machine.driver_data<gyruss_state>();
	state->m_tilemap = tilemap_create(machine, gyruss_get_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_tilemap->set_transmask(0, 0x00, 0);	/* opaque */
	state->m_tilemap->set_transmask(1, 0x0f, 0);  /* transparent */
}



READ8_MEMBER(gyruss_state::gyruss_scanline_r)
{
	/* reads 1V - 128V */
	return machine().primary_screen->vpos();
}


static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element **gfx )
{
	gyruss_state *state = machine.driver_data<gyruss_state>();
	int offs;

	for (offs = 0xbc; offs >= 0; offs -= 4)
	{
		int x = state->m_spriteram[offs];
		int y = 241 - state->m_spriteram[offs + 3];

		int gfx_bank = state->m_spriteram[offs + 1] & 0x01;
		int code = ((state->m_spriteram[offs + 2] & 0x20) << 2) | ( state->m_spriteram[offs + 1] >> 1);
		int color = state->m_spriteram[offs + 2] & 0x0f;
		int flip_x = ~state->m_spriteram[offs + 2] & 0x40;
		int flip_y =  state->m_spriteram[offs + 2] & 0x80;

		drawgfx_transpen(bitmap, cliprect, gfx[gfx_bank], code, color, flip_x, flip_y, x, y, 0);
	}
}


SCREEN_UPDATE_IND16( gyruss )
{
	gyruss_state *state = screen.machine().driver_data<gyruss_state>();

	if (cliprect.min_y == screen.visible_area().min_y)
	{
		screen.machine().tilemap().mark_all_dirty();
		screen.machine().tilemap().set_flip_all((*state->m_flipscreen & 0x01) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
	}

	state->m_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	draw_sprites(screen.machine(), bitmap, cliprect, screen.machine().gfx);
	state->m_tilemap->draw(bitmap, cliprect, 0, 0);

	return 0;
}
