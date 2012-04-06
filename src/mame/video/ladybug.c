/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/ladybug.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Lady Bug has a 32 bytes palette PROM and a 32 bytes sprite color lookup
  table PROM.
  The palette PROM is connected to the RGB output this way:

  bit 7 -- inverter -- 220 ohm resistor  -- BLUE
        -- inverter -- 220 ohm resistor  -- GREEN
        -- inverter -- 220 ohm resistor  -- RED
        -- inverter -- 470 ohm resistor  -- BLUE
        -- unused
        -- inverter -- 470 ohm resistor  -- GREEN
        -- unused
  bit 0 -- inverter -- 470 ohm resistor  -- RED

***************************************************************************/

static void palette_init_common( running_machine &machine, const UINT8 *color_prom, int colortable_size,
								int r_bit0, int r_bit1, int g_bit0, int g_bit1, int b_bit0, int b_bit1 )
{
	static const int resistances[2] = { 470, 220 };
	double rweights[2], gweights[2], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			2, resistances, rweights, 470, 0,
			2, resistances, gweights, 470, 0,
			2, resistances, bweights, 470, 0);

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, colortable_size);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1;
		int r, g, b;

		/* red component */
		bit0 = (~color_prom[i] >> r_bit0) & 0x01;
		bit1 = (~color_prom[i] >> r_bit1) & 0x01;
		r = combine_2_weights(rweights, bit0, bit1);

		/* green component */
		bit0 = (~color_prom[i] >> g_bit0) & 0x01;
		bit1 = (~color_prom[i] >> g_bit1) & 0x01;
		g = combine_2_weights(gweights, bit0, bit1);

		/* blue component */
		bit0 = (~color_prom[i] >> b_bit0) & 0x01;
		bit1 = (~color_prom[i] >> b_bit1) & 0x01;
		b = combine_2_weights(bweights, bit0, bit1);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* characters */
	for (i = 0; i < 0x20; i++)
	{
		UINT8 ctabentry = ((i << 3) & 0x18) | ((i >> 2) & 0x07);
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}

	/* sprites */
	for (i = 0x20; i < 0x40; i++)
	{
		UINT8 ctabentry = color_prom[(i - 0x20) >> 1];

		ctabentry = BITSWAP8((color_prom[i - 0x20] >> 0) & 0x0f, 7,6,5,4,0,1,2,3);
		colortable_entry_set_value(machine.colortable, i + 0x00, ctabentry);

		ctabentry = BITSWAP8((color_prom[i - 0x20] >> 4) & 0x0f, 7,6,5,4,0,1,2,3);
		colortable_entry_set_value(machine.colortable, i + 0x20, ctabentry);
	}
}


PALETTE_INIT( ladybug )
{
	palette_init_common(machine, color_prom, 0x20, 0, 5, 2, 6, 4, 7);
}

PALETTE_INIT( sraider )
{
	int i;

	/* the resistor net may be probably different than Lady Bug */
	palette_init_common(machine, color_prom, 0x41, 3, 0, 5, 4, 7, 6);

	/* star colors */
	for (i = 0x20; i < 0x40; i++)
	{
		int bit0, bit1;
		int r, g, b;

		/* red component */
		bit0 = ((i - 0x20) >> 3) & 0x01;
		bit1 = ((i - 0x20) >> 4) & 0x01;
		b = 0x47 * bit0 + 0x97 * bit1;

		/* green component */
		bit0 = ((i - 0x20) >> 1) & 0x01;
		bit1 = ((i - 0x20) >> 2) & 0x01;
		g = 0x47 * bit0 + 0x97 * bit1;

		/* blue component */
		bit0 = ((i - 0x20) >> 0) & 0x01;
		r = 0x47 * bit0;

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	for (i = 0x60; i < 0x80; i++)
		colortable_entry_set_value(machine.colortable, i, (i - 0x60) + 0x20);

	/* stationary part of grid */
	colortable_entry_set_value(machine.colortable, 0x81, 0x40);
}

WRITE8_MEMBER(ladybug_state::ladybug_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(ladybug_state::ladybug_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(ladybug_state::ladybug_flipscreen_w)
{
	if (flip_screen_get(machine()) != (data & 0x01))
	{
		flip_screen_set(machine(), data & 0x01);
		machine().tilemap().mark_all_dirty();
	}
}

WRITE8_MEMBER(ladybug_state::sraider_io_w)
{

	// bit7 = flip
	// bit6 = grid red
	// bit5 = grid green
	// bit4 = grid blue
	// bit3 = enable stars
	// bit210 = stars speed/dir

	if (flip_screen_get(machine()) != (data & 0x80))
	{
		flip_screen_set(machine(), data & 0x80);
		machine().tilemap().mark_all_dirty();
	}

	m_grid_color = data & 0x70;

	redclash_set_stars_enable(machine(), (data & 0x08) >> 3);

	/*
     * There must be a subtle clocking difference between
     * Space Raider and the other games using this star generator,
     * hence the -1 here
     */

	redclash_set_stars_speed(machine(), (data & 0x07) - 1);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	ladybug_state *state = machine.driver_data<ladybug_state>();
	int code = state->m_videoram[tile_index] + 32 * (state->m_colorram[tile_index] & 0x08);
	int color = state->m_colorram[tile_index] & 0x07;

	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( get_grid_tile_info )
{
	if (tile_index < 512)
		SET_TILE_INFO(3, tile_index, 0, 0);
	else
	{
		int temp = tile_index / 32;
		tile_index = (31 - temp) * 32 + (tile_index % 32);
		SET_TILE_INFO(4, tile_index, 0, 0);
	}
}

VIDEO_START( ladybug )
{
	ladybug_state *state = machine.driver_data<ladybug_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_bg_tilemap->set_scroll_rows(32);
	state->m_bg_tilemap->set_transparent_pen(0);
}

VIDEO_START( sraider )
{
	ladybug_state *state = machine.driver_data<ladybug_state>();

	state->m_grid_tilemap = tilemap_create(machine, get_grid_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_grid_tilemap->set_scroll_rows(32);
	state->m_grid_tilemap->set_transparent_pen(0);

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_bg_tilemap->set_scroll_rows(32);
	state->m_bg_tilemap->set_transparent_pen(0);
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	ladybug_state *state = machine.driver_data<ladybug_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = state->m_spriteram_size - 2 * 0x40; offs >= 2 * 0x40; offs -= 0x40)
	{
		int i = 0;

		while (i < 0x40 && spriteram[offs + i] != 0)
			i += 4;

		while (i > 0)
		{
/*
 abccdddd eeeeeeee fffghhhh iiiiiiii

 a enable?
 b size (0 = 8x8, 1 = 16x16)
 cc flip
 dddd y offset
 eeeeeeee sprite code (shift right 2 bits for 16x16 sprites)
 fff unknown
 g sprite bank
 hhhh color
 iiiiiiii x position
*/
			i -= 4;

			if (spriteram[offs + i] & 0x80)
			{
				if (spriteram[offs + i] & 0x40)	/* 16x16 */
					drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
							(spriteram[offs + i + 1] >> 2) + 4 * (spriteram[offs + i + 2] & 0x10),
							spriteram[offs + i + 2] & 0x0f,
							spriteram[offs + i] & 0x20,spriteram[offs + i] & 0x10,
							spriteram[offs + i + 3],
							offs / 4 - 8 + (spriteram[offs + i] & 0x0f),0);
				else	/* 8x8 */
					drawgfx_transpen(bitmap,cliprect,machine.gfx[2],
							spriteram[offs + i + 1] + 16 * (spriteram[offs + i + 2] & 0x10),
							spriteram[offs + i + 2] & 0x0f,
							spriteram[offs + i] & 0x20,spriteram[offs + i] & 0x10,
							spriteram[offs + i + 3],
							offs / 4 + (spriteram[offs + i] & 0x0f),0);
			}
		}
	}
}

SCREEN_UPDATE_IND16( ladybug )
{
	ladybug_state *state = screen.machine().driver_data<ladybug_state>();
	int offs;

	// clear the bg bitmap
	bitmap.fill(0, cliprect);

	for (offs = 0; offs < 32; offs++)
	{
		int sx = offs % 4;
		int sy = offs / 4;

		if (flip_screen_get(screen.machine()))
			state->m_bg_tilemap->set_scrollx(offs, -state->m_videoram[32 * sx + sy]);
		else
			state->m_bg_tilemap->set_scrollx(offs, state->m_videoram[32 * sx + sy]);
	}

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}

SCREEN_VBLANK( sraider )	/* update starfield position */
{
	// falling edge
	if (!vblank_on)
		redclash_update_stars_state(screen.machine());
}

SCREEN_UPDATE_IND16( sraider )
{
	ladybug_state *state = screen.machine().driver_data<ladybug_state>();

	// this part is boilerplate from ladybug, not sure if hardware does this,
	// since it's not used

	int offs;
	int i;

	for (offs = 0; offs < 32; offs++)
	{
		int sx = offs % 4;
		int sy = offs / 4;

		if (flip_screen_get(screen.machine()))
			state->m_bg_tilemap->set_scrollx(offs, -state->m_videoram[32 * sx + sy]);
		else
			state->m_bg_tilemap->set_scrollx(offs, state->m_videoram[32 * sx + sy]);
	}

	// clear the bg bitmap
	bitmap.fill(0, cliprect);

	// draw the stars
	if (flip_screen_get(screen.machine()))
		redclash_draw_stars(screen.machine(), bitmap, cliprect, 0x60, 1, 0x27, 0xff);
	else
		redclash_draw_stars(screen.machine(), bitmap, cliprect, 0x60, 1, 0x00, 0xd8);

	// draw the gridlines
	colortable_palette_set_color(screen.machine().colortable, 0x40, MAKE_RGB(state->m_grid_color & 0x40 ? 0xff : 0,
		            														 state->m_grid_color & 0x20 ? 0xff : 0,
		            														 state->m_grid_color & 0x10 ? 0xff : 0));
	state->m_grid_tilemap->draw(bitmap, cliprect, 0, flip_screen_get(screen.machine()));

	for (i = 0; i < 0x100; i++)
	{
		if (state->m_grid_data[i] != 0)
		{
			UINT8 x = i;
			int height = cliprect.max_y - cliprect.min_y + 1;

			if (flip_screen_get(screen.machine()))
				x = ~x;

			bitmap.plot_box(x, cliprect.min_y, 1, height, 0x81);
		}
	}

	// now the chars
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, flip_screen_get(screen.machine()));

	// now the sprites
	draw_sprites(screen.machine(), bitmap, cliprect);

	return 0;
}
