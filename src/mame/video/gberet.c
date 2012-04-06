#include "emu.h"
#include "includes/gberet.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Green Beret has a 32 bytes palette PROM and two 256 bytes color lookup table
  PROMs (one for sprites, one for characters).
  The palette PROM is connected to the RGB output, this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT( gberet )
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

	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0x10;
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}

	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry;

		if (color_prom[i] & 0x0f)
			ctabentry = color_prom[i] & 0x0f;
		else
			ctabentry = 0;

		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}
}

WRITE8_MEMBER(gberet_state::gberet_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(gberet_state::gberet_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(gberet_state::gberet_scroll_w)
{
	int scroll;

	m_scrollram[offset] = data;

	scroll = m_scrollram[offset & 0x1f] | (m_scrollram[offset | 0x20] << 8);
	m_bg_tilemap->set_scrollx(offset & 0x1f, scroll);
}

WRITE8_MEMBER(gberet_state::gberet_sprite_bank_w)
{
	m_spritebank = data;
}

static TILE_GET_INFO( get_bg_tile_info )
{
	gberet_state *state = machine.driver_data<gberet_state>();
	int attr = state->m_colorram[tile_index];
	int code = state->m_videoram[tile_index] + ((attr & 0x40) << 2);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	tileinfo.group = color;
	tileinfo.category = (attr & 0x80) >> 7;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( gberet )
{
	gberet_state *state = machine.driver_data<gberet_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	colortable_configure_tilemap_groups(machine.colortable, state->m_bg_tilemap, machine.gfx[0], 0x10);
	state->m_bg_tilemap->set_scroll_rows(32);
}

static void gberet_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gberet_state *state = machine.driver_data<gberet_state>();
	int offs;
	UINT8 *sr;

	if (state->m_spritebank & 0x08)
		sr = state->m_spriteram2;
	else
		sr = state->m_spriteram;

	for (offs = 0; offs < 0xc0; offs += 4)
	{
		if (sr[offs + 3])
		{
			int attr = sr[offs + 1];
			int code = sr[offs + 0] + ((attr & 0x40) << 2);
			int color = attr & 0x0f;
			int sx = sr[offs + 2] - 2 * (attr & 0x80);
			int sy = sr[offs + 3];
			int flipx = attr & 0x10;
			int flipy = attr & 0x20;

			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transmask(bitmap, cliprect, machine.gfx[1], code, color, flipx, flipy, sx, sy,
				colortable_get_transpen_mask(machine.colortable, machine.gfx[1], color, 0));
		}
	}
}

SCREEN_UPDATE_IND16( gberet )
{
	gberet_state *state = screen.machine().driver_data<gberet_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0);
	gberet_draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

/* Green Beret (bootleg) */

WRITE8_MEMBER(gberet_state::gberetb_scroll_w)
{
	int scroll = data;

	if (offset)
		scroll |= 0x100;

	for (offset = 6; offset < 29; offset++)
		m_bg_tilemap->set_scrollx(offset, scroll + 64 - 8);
}

static void gberetb_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gberet_state *state = machine.driver_data<gberet_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = state->m_spriteram_size - 4; offs >= 0; offs -= 4)
	{
		if (spriteram[offs + 1])
		{
			int attr = spriteram[offs + 3];
			int code = spriteram[offs] + ((attr & 0x40) << 2);
			int color = attr & 0x0f;
			int sx = spriteram[offs + 2] - 2 * (attr & 0x80);
			int sy = 240 - spriteram[offs + 1];
			int flipx = attr & 0x10;
			int flipy = attr & 0x20;

			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transmask(bitmap, cliprect, machine.gfx[1], code, color, flipx, flipy, sx, sy,
				colortable_get_transpen_mask(machine.colortable, machine.gfx[1], color, 0));
		}
	}
}

SCREEN_UPDATE_IND16( gberetb )
{
	gberet_state *state = screen.machine().driver_data<gberet_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0);
	gberetb_draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
