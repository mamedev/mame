/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/shaolins.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Shao-lin's Road has three 256x4 palette PROMs (one per gun) and two 256x4
  lookup table PROMs (one for characters, one for sprites).
  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
PALETTE_INIT( shaolins )
{
	static const int resistances[4] = { 2200, 1000, 470, 220 };
	double rweights[4], gweights[4], bweights[4];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			4, resistances, rweights, 470, 0,
			4, resistances, gweights, 470, 0,
			4, resistances, bweights, 470, 0);

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x100);

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

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
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
			colortable_entry_set_value(machine.colortable, ((i & 0x100) << 3) | (j << 8) | (i & 0xff), ctabentry);
		}
	}
}

WRITE8_MEMBER(shaolins_state::shaolins_videoram_w)
{

	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(shaolins_state::shaolins_colorram_w)
{

	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(shaolins_state::shaolins_palettebank_w)
{

	if (m_palettebank != (data & 0x07))
	{
		m_palettebank = data & 0x07;
		machine().tilemap().mark_all_dirty();
	}
}

WRITE8_MEMBER(shaolins_state::shaolins_scroll_w)
{
	int col;

	for (col = 4; col < 32; col++)
		m_bg_tilemap->set_scrolly(col, data + 1);
}

WRITE8_MEMBER(shaolins_state::shaolins_nmi_w)
{

	m_nmi_enable = data;

	if (flip_screen_get(machine()) != (data & 0x01))
	{
		flip_screen_set(machine(), data & 0x01);
		machine().tilemap().mark_all_dirty();
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	shaolins_state *state = machine.driver_data<shaolins_state>();
	int attr = state->m_colorram[tile_index];
	int code = state->m_videoram[tile_index] + ((attr & 0x40) << 2);
	int color = (attr & 0x0f) + 16 * state->m_palettebank;
	int flags = (attr & 0x20) ? TILE_FLIPY : 0;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( shaolins )
{
	shaolins_state *state = machine.driver_data<shaolins_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	state->m_bg_tilemap->set_scroll_cols(32);
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	shaolins_state *state = machine.driver_data<shaolins_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = state->m_spriteram_size - 32; offs >= 0; offs -= 32 ) /* max 24 sprites */
	{
		if (spriteram[offs] && spriteram[offs + 6]) /* stop rogue sprites on high score screen */
		{
			int code = spriteram[offs + 8];
			int color = (spriteram[offs + 9] & 0x0f) | (state->m_palettebank << 4);
			int flipx = !(spriteram[offs + 9] & 0x40);
			int flipy = spriteram[offs + 9] & 0x80;
			int sx = 240 - spriteram[offs + 6];
			int sy = 248 - spriteram[offs + 4];

			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 248 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transmask(bitmap, cliprect,machine.gfx[1],
				code, color,
				flipx, flipy,
				sx, sy,
				colortable_get_transpen_mask(machine.colortable, machine.gfx[1], color, state->m_palettebank << 5));
		}
	}
}

SCREEN_UPDATE_IND16( shaolins )
{
	shaolins_state *state = screen.machine().driver_data<shaolins_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
