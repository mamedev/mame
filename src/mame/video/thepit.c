/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/thepit.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED


***************************************************************************/

PALETTE_INIT( thepit )
{
	int i;

	for (i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}

	/* allocate primary colors for the background and foreground
       this is wrong, but I don't know where to pick the colors from */
	for (i = 0; i < 8; i++)
		palette_set_color_rgb(machine, i + 32, pal1bit(i >> 2), pal1bit(i >> 1), pal1bit(i >> 0));
}


/***************************************************************************

 Super Mouse has 5 bits per gun (maybe 6 for green), exact weights are
 unknown.

***************************************************************************/

PALETTE_INIT( suprmous )
{
	int i;

	for (i = 0; i < 32; i++)
	{
		UINT8 b = BITSWAP8(color_prom[i + 0x00], 0, 1, 2, 3, 4, 5, 6, 7);
		UINT8 g = BITSWAP8(color_prom[i + 0x20], 0, 1, 2, 3, 4, 5, 6, 7);
		UINT8 r = (b>>5&7)<<2 | (g>>6&3);

		palette_set_color_rgb(machine, i, pal5bit(r), pal5bit(g), pal4bit(b));
	}

	/* allocate primary colors for the background and foreground
       this is wrong, but I don't know where to pick the colors from */
	for (i = 0; i < 8; i++)
		palette_set_color_rgb(machine, i + 32, pal1bit(i >> 2), pal1bit(i >> 1), pal1bit(i >> 0));
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( solid_get_tile_info )
{
	thepit_state *state = machine.driver_data<thepit_state>();
	UINT8 back_color = (state->m_colorram[tile_index] & 0x70) >> 4;
	int priority = (back_color != 0) && ((state->m_colorram[tile_index] & 0x80) == 0);
	tileinfo.pen_data = state->m_dummy_tile;
	tileinfo.palette_base = back_color + 32;
	tileinfo.category = priority;
}


static TILE_GET_INFO( get_tile_info )
{
	thepit_state *state = machine.driver_data<thepit_state>();
	UINT8 fore_color = state->m_colorram[tile_index] % machine.gfx[0]->total_colors;
	UINT8 code = state->m_videoram[tile_index];
	SET_TILE_INFO(2 * state->m_graphics_bank, code, fore_color, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( thepit )
{
	thepit_state *state = machine.driver_data<thepit_state>();
	state->m_solid_tilemap = tilemap_create(machine, solid_get_tile_info,tilemap_scan_rows,8,8,32,32);

	state->m_tilemap = tilemap_create(machine, get_tile_info,tilemap_scan_rows,8,8,32,32);
	state->m_tilemap->set_transparent_pen(0);

	state->m_solid_tilemap->set_scroll_cols(32);
	state->m_tilemap->set_scroll_cols(32);

	state->m_dummy_tile = auto_alloc_array_clear(machine, UINT8, 8*8);

	state->m_graphics_bank = 0;	/* only used in intrepid */
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(thepit_state::thepit_videoram_w)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(thepit_state::thepit_colorram_w)
{
	m_colorram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
	m_solid_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(thepit_state::thepit_flip_screen_x_w)
{
	int flip;

	m_flip_screen_x = data & 0x01;

	flip = m_flip_screen_x ? TILEMAP_FLIPX : 0;
	if (m_flip_screen_y)
		flip |= TILEMAP_FLIPY ;

	m_tilemap->set_flip(flip);
	m_solid_tilemap->set_flip(flip);

}


WRITE8_MEMBER(thepit_state::thepit_flip_screen_y_w)
{
	int flip;

	m_flip_screen_y = data & 0x01;

	flip = m_flip_screen_x ? TILEMAP_FLIPX : 0;
	if (m_flip_screen_y)
		flip |= TILEMAP_FLIPY ;

	m_tilemap->set_flip(flip);
	m_solid_tilemap->set_flip(flip);

}


WRITE8_MEMBER(thepit_state::intrepid_graphics_bank_w)
{
	if (m_graphics_bank != (data & 0x01))
	{
		m_graphics_bank = data & 0x01;

		m_tilemap->mark_all_dirty();
	}
}


READ8_MEMBER(thepit_state::thepit_input_port_0_r)
{
	/* Read either the real or the fake input ports depending on the
       horizontal flip switch. (This is how the real PCB does it) */
	if (m_flip_screen_x)
	{
		return input_port_read(machine(), "IN2");
	}
	else
	{
		return input_port_read(machine(), "IN0");
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static void draw_sprites(running_machine &machine,
						 bitmap_ind16 &bitmap,
						 const rectangle &cliprect,
						 int priority_to_draw)
{
	const rectangle spritevisiblearea(2*8+1, 32*8-1, 2*8, 30*8-1);
	const rectangle spritevisibleareaflipx(0*8, 30*8-2, 2*8, 30*8-1);

	thepit_state *state = machine.driver_data<thepit_state>();
	int offs;

	for (offs = state->m_spriteram_size - 4; offs >= 0; offs -= 4)
	{
		if (((state->m_spriteram[offs + 2] & 0x08) >> 3) == priority_to_draw)
		{
			UINT8 y, x, flipx, flipy;

			if ((state->m_spriteram[offs + 0] == 0) || (state->m_spriteram[offs + 3] == 0))
			{
				continue;
			}

			y = 240 - state->m_spriteram[offs];
			x = state->m_spriteram[offs + 3] + 1;

			flipx = state->m_spriteram[offs + 1] & 0x40;
			flipy = state->m_spriteram[offs + 1] & 0x80;

			if (state->m_flip_screen_y)
			{
				y = 240 - y;
				flipy = !flipy;
			}

			if (state->m_flip_screen_x)
			{
				x = 242 - x;
				flipx = !flipx;
			}

			/* sprites 0-3 are drawn one pixel down */
			if (offs < 16) y++;

			drawgfx_transpen(bitmap, state->m_flip_screen_x ? spritevisibleareaflipx : spritevisiblearea,
					machine.gfx[2 * state->m_graphics_bank + 1],
					state->m_spriteram[offs + 1] & 0x3f,
					state->m_spriteram[offs + 2],
					flipx, flipy, x, y, 0);
		}
	}
}


SCREEN_UPDATE_IND16( thepit )
{
	thepit_state *state = screen.machine().driver_data<thepit_state>();
	offs_t offs;

	for (offs = 0; offs < 32; offs++)
	{
		int xshift = state->m_flip_screen_x ? 128 : 0;
		int yshift = state->m_flip_screen_y ? -8 : 0;

		state->m_tilemap->set_scrollx(offs, xshift);
		state->m_solid_tilemap->set_scrollx(offs, xshift);

		state->m_tilemap->set_scrolly(offs, yshift + state->m_attributesram[offs << 1]);
		state->m_solid_tilemap->set_scrolly(offs, yshift + state->m_attributesram[offs << 1]);
	}

	/* low priority tiles */
	state->m_solid_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_tilemap->draw(bitmap, cliprect, 0, 0);

	/* low priority sprites */
	draw_sprites(screen.machine(), bitmap, cliprect, 0);

	/* high priority tiles */
	state->m_solid_tilemap->draw(bitmap, cliprect, 1, 1);

	/* high priority sprites */
	draw_sprites(screen.machine(), bitmap, cliprect, 1);

	return 0;
}
