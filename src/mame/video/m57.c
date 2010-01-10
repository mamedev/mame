/****************************************************************************

    Irem M57 hardware

****************************************************************************/

#include "emu.h"
#include "includes/iremz80.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Tropical Angel has two 256x4 character palette PROMs, one 32x8 sprite
  palette PROM, and one 256x4 sprite color lookup table PROM.

  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably something like this; note that RED and BLUE
  are swapped wrt the usual configuration.

  bit 7 -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
  bit 0 -- 1  kohm resistor  -- BLUE

***************************************************************************/

PALETTE_INIT( m57 )
{
	int i;

	machine->colortable = colortable_alloc(machine, 32 * 8 + 16);

	/* character palette */
	for (i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = 0;
		bit1 = (color_prom[256] >> 2) & 0x01;
		bit2 = (color_prom[256] >> 3) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[256] >> 0) & 0x01;
		bit2 = (color_prom[256] >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r,g,b));
		colortable_entry_set_value(machine->colortable, i, i);
		color_prom++;
	}

	color_prom += 256;
	/* color_prom now points to the beginning of the sprite palette */

	/* sprite palette */
	for (i = 0; i < 16; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		colortable_palette_set_color(machine->colortable, i + 256, MAKE_RGB(r,g,b));
		color_prom++;
	}

	color_prom += 16;
	/* color_prom now points to the beginning of the sprite lookup table */


	/* sprite lookup table */
	for (i = 0; i < 32 * 8; i++)
	{
		colortable_entry_set_value(machine->colortable, i + 32 * 8, 256 + (~*color_prom & 0x0f));
		color_prom++;
	}
}


/*************************************
 *
 *  Tilemap info callback
 *
 *************************************/

static TILE_GET_INFO( get_tile_info )
{
	irem_z80_state *state = (irem_z80_state *)machine->driver_data;

	UINT8 attr = state->videoram[tile_index * 2 + 0];
	UINT16 code = state->videoram[tile_index * 2 + 1] | ((attr & 0xc0) << 2);

	SET_TILE_INFO(0, code, attr & 0x0f, TILE_FLIPXY(attr >> 4));
}


/*************************************
 *
 *  Video RAM access
 *
 *************************************/

WRITE8_HANDLER( m57_videoram_w )
{
	irem_z80_state *state = (irem_z80_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}


/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( m57 )
{
	irem_z80_state *state = (irem_z80_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows,  8, 8, 32, 32);
	tilemap_set_scroll_rows(state->bg_tilemap, 256);

	state_save_register_global(machine, state->flipscreen);
}


/*************************************
 *
 *  Outputs
 *
 *************************************/

WRITE8_HANDLER( m57_flipscreen_w )
{
	irem_z80_state *state = (irem_z80_state *)space->machine->driver_data;

	/* screen flip is handled both by software and hardware */
	state->flipscreen = (data & 0x01) ^ (~input_port_read(space->machine, "DSW2") & 0x01);
	tilemap_set_flip(state->bg_tilemap, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	coin_counter_w(space->machine, 0,data & 0x02);
	coin_counter_w(space->machine, 1,data & 0x20);
}


/*************************************
 *
 *  Background rendering
 *
 *************************************/

static void draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	irem_z80_state *state = (irem_z80_state *)machine->driver_data;
	int y,x;
	INT16 scrolly;

	// from 64 to 127: not wrapped
	for (y = 64; y < 128; y++)
		tilemap_set_scrollx(state->bg_tilemap, y, state->scrollram[0x40]);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	// from 128 to 255: wrapped
	for (y = 128; y <= cliprect->max_y; y++)
	{
		scrolly = state->scrollram[y] + (state->scrollram[y + 0x100] << 8);

		if (scrolly >= 0)
		{
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			{
				if ((x + scrolly) <= cliprect->max_x)
					*BITMAP_ADDR16(bitmap, y, x) = *BITMAP_ADDR16(bitmap, y, x + scrolly);
				else
					*BITMAP_ADDR16(bitmap, y, x) = *BITMAP_ADDR16(bitmap, y, cliprect->max_x);
			}
		} else {
			for (x = cliprect->max_x; x >= cliprect->min_x; x--)
			{
				if ((x + scrolly) >= cliprect->min_x)
					*BITMAP_ADDR16(bitmap, y, x) = *BITMAP_ADDR16(bitmap, y, x + scrolly);
				else
					*BITMAP_ADDR16(bitmap, y, x) = *BITMAP_ADDR16(bitmap, y, cliprect->min_x);
			}
		}
	}
}

/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	irem_z80_state *state = (irem_z80_state *)machine->driver_data;
	int offs;

	for (offs = state->spriteram_size - 4; offs >= 0; offs -= 4)
	{
		UINT8 attributes = state->spriteram[offs + 1];
		int sx = state->spriteram[offs + 3];
		int sy = ((224 - state->spriteram[offs + 0] - 32) & 0xff) + 32;
		int code = state->spriteram[offs + 2];
		int color = attributes & 0x1f;
		int flipy = attributes & 0x80;
		int flipx = attributes & 0x40;

		int tile_number = code & 0x3f;

		int bank = 0;
		if (code & 0x80) bank += 1;
		if (attributes & 0x20) bank += 2;

		if (state->flipscreen)
		{
			sx = 240 - sx;
			sy = 224 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transmask(bitmap, cliprect, machine->gfx[1 + bank],
			tile_number,
			color,
			flipx, flipy,
			sx, sy,
			colortable_get_transpen_mask(machine->colortable, machine->gfx[1], color, 256 + 15));
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

VIDEO_UPDATE( m57 )
{
	draw_background(screen->machine, bitmap, cliprect);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
