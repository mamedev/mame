#include "emu.h"
#include "includes/skykid.h"


/***************************************************************************

    Convert the color PROMs.

    The palette PROMs are connected to the RGB output this way:

    bit 3   -- 220 ohm resistor  -- RED/GREEN/BLUE
            -- 470 ohm resistor  -- RED/GREEN/BLUE
            -- 1  kohm resistor  -- RED/GREEN/BLUE
    bit 0   -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT( skykid )
{
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* text palette */
	for (i = 0; i < 0x100; i++)
		colortable_entry_set_value(machine.colortable, i, i);

	/* tiles/sprites */
	for (i = 0x100; i < 0x500; i++)
	{
		UINT8 ctabentry = color_prom[i - 0x100];
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
static TILEMAP_MAPPER( tx_tilemap_scan )
{
	int offs;

	row += 2;
	col -= 2;
	if (col & 0x20)
		offs = row + ((col & 0x1f) << 5);
	else
		offs = col + (row << 5);

	return offs;
}

static TILE_GET_INFO( tx_get_tile_info )
{
	skykid_state *state = machine.driver_data<skykid_state>();
	int code = state->m_textram[tile_index];
	int attr = state->m_textram[tile_index + 0x400];
	tileinfo->category = code >> 4 & 0xf;

	/* the hardware has two character sets, one normal and one flipped. When
       screen is flipped, character flip is done by selecting the 2nd character set.
       We reproduce this here, but since the tilemap system automatically flips
       characters when screen is flipped, we have to flip them back. */
	SET_TILE_INFO(
			0,
			code | (flip_screen_get(machine) ? 0x100 : 0),
			attr & 0x3f,
			flip_screen_get(machine) ? (TILE_FLIPY | TILE_FLIPX) : 0);
}


static TILE_GET_INFO( bg_get_tile_info )
{
	skykid_state *state = machine.driver_data<skykid_state>();
	int code = state->m_videoram[tile_index];
	int attr = state->m_videoram[tile_index+0x800];

	SET_TILE_INFO(
			1,
			code + ((attr & 0x01) << 8),
			((attr & 0x7e) >> 1) | ((attr & 0x01) << 6),
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( skykid )
{
	skykid_state *state = machine.driver_data<skykid_state>();
	state->m_tx_tilemap = tilemap_create(machine, tx_get_tile_info,tx_tilemap_scan,  8,8,36,28);
	state->m_bg_tilemap = tilemap_create(machine, bg_get_tile_info,tilemap_scan_rows,     8,8,64,32);

	tilemap_set_transparent_pen(state->m_tx_tilemap, 0);

	state_save_register_global(machine, state->m_priority);
	state_save_register_global(machine, state->m_scroll_x);
	state_save_register_global(machine, state->m_scroll_y);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

READ8_HANDLER( skykid_videoram_r )
{
	skykid_state *state = space->machine().driver_data<skykid_state>();
	return state->m_videoram[offset];
}

WRITE8_HANDLER( skykid_videoram_w )
{
	skykid_state *state = space->machine().driver_data<skykid_state>();
	state->m_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap,offset & 0x7ff);
}

READ8_HANDLER( skykid_textram_r )
{
	skykid_state *state = space->machine().driver_data<skykid_state>();
	return state->m_textram[offset];
}

WRITE8_HANDLER( skykid_textram_w )
{
	skykid_state *state = space->machine().driver_data<skykid_state>();
	state->m_textram[offset] = data;
	tilemap_mark_tile_dirty(state->m_tx_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( skykid_scroll_x_w )
{
	skykid_state *state = space->machine().driver_data<skykid_state>();
	state->m_scroll_x = offset;
}

WRITE8_HANDLER( skykid_scroll_y_w )
{
	skykid_state *state = space->machine().driver_data<skykid_state>();
	state->m_scroll_y = offset;
}

WRITE8_HANDLER( skykid_flipscreen_priority_w )
{
	skykid_state *state = space->machine().driver_data<skykid_state>();
	state->m_priority = data;
	flip_screen_set(space->machine(), offset & 1);
}



/***************************************************************************

  Display Refresh

***************************************************************************/

/* the sprite generator IC is the same as Mappy */
static void draw_sprites(running_machine &machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	skykid_state *state = machine.driver_data<skykid_state>();
	UINT8 *spriteram = state->m_spriteram + 0x780;
	UINT8 *spriteram_2 = spriteram + 0x0800;
	UINT8 *spriteram_3 = spriteram_2 + 0x0800;
	int offs;

	for (offs = 0;offs < 0x80;offs += 2)
	{
		static const int gfx_offs[2][2] =
		{
			{ 0, 1 },
			{ 2, 3 }
		};
		int sprite = spriteram[offs] + ((spriteram_3[offs] & 0x80) << 1);
		int color = (spriteram[offs+1] & 0x3f);
		int sx = (spriteram_2[offs+1]) + 0x100*(spriteram_3[offs+1] & 1) - 71;
		int sy = 256 - spriteram_2[offs] - 7;
		int flipx = (spriteram_3[offs] & 0x01);
		int flipy = (spriteram_3[offs] & 0x02) >> 1;
		int sizex = (spriteram_3[offs] & 0x04) >> 2;
		int sizey = (spriteram_3[offs] & 0x08) >> 3;
		int x,y;

		sprite &= ~sizex;
		sprite &= ~(sizey << 1);

		if (flip_screen_get(machine))
		{
			flipx ^= 1;
			flipy ^= 1;
		}

		sy -= 16 * sizey;
		sy = (sy & 0xff) - 32;	// fix wraparound

		for (y = 0;y <= sizey;y++)
		{
			for (x = 0;x <= sizex;x++)
			{
				drawgfx_transmask(bitmap,cliprect,machine.gfx[2],
					sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
					color,
					flipx,flipy,
					sx + 16*x,sy + 16*y,
					colortable_get_transpen_mask(machine.colortable, machine.gfx[2], color, 0xff));
			}
		}
	}
}


SCREEN_UPDATE( skykid )
{
	skykid_state *state = screen->machine().driver_data<skykid_state>();
	if (flip_screen_get(screen->machine()))
	{
		tilemap_set_scrollx(state->m_bg_tilemap, 0, 189 - (state->m_scroll_x ^ 1));
		tilemap_set_scrolly(state->m_bg_tilemap, 0, 7 - state->m_scroll_y);
	}
	else
	{
		tilemap_set_scrollx(state->m_bg_tilemap, 0, state->m_scroll_x + 35);
		tilemap_set_scrolly(state->m_bg_tilemap, 0, state->m_scroll_y + 25);
	}

	tilemap_draw(bitmap,cliprect,state->m_bg_tilemap,TILEMAP_DRAW_OPAQUE,0);

	if (state->m_priority & 0x04)
	{
		// textlayer priority enabled?
		int cat, pri = state->m_priority >> 4;

		// draw low priority tiles
		tilemap_draw(bitmap, cliprect, state->m_tx_tilemap, pri, 0);

		draw_sprites(screen->machine(), bitmap, cliprect);

		// draw the other tiles
		for (cat = 0; cat < 0xf; cat++)
			if (cat != pri) tilemap_draw(bitmap, cliprect, state->m_tx_tilemap, cat, 0);
	}
	else
	{
		draw_sprites(screen->machine(), bitmap, cliprect);
		tilemap_draw(bitmap, cliprect, state->m_tx_tilemap, TILEMAP_DRAW_ALL_CATEGORIES, 0);
	}

	return 0;
}
