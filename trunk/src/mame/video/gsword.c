/***************************************************************************
  Great Swordsman

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/gsword.h"


static PALETTE_INIT( common )
{
	int i;

	/* characters */
	for (i = 0; i < 0x100; i++)
		colortable_entry_set_value(machine.colortable, i, i);

	/* sprites */
	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = (BITSWAP8(color_prom[i - 0x100],7,6,5,4,0,1,2,3) & 0x0f) | 0x80;
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}
}


PALETTE_INIT( josvolly )
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

	PALETTE_INIT_CALL(common);
}


PALETTE_INIT( gsword )
{
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0x100] >> 0) & 1;
		bit1 = (color_prom[i + 0x100] >> 1) & 1;
		bit2 = (color_prom[i + 0x100] >> 2) & 1;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i + 0x100] >> 3) & 1;
		bit1 = (color_prom[i + 0x000] >> 0) & 1;
		bit2 = (color_prom[i + 0x000] >> 1) & 1;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i + 0x000] >> 2) & 1;
		bit2 = (color_prom[i + 0x000] >> 3) & 1;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x200;

	PALETTE_INIT_CALL(common);
}

WRITE8_HANDLER( gsword_videoram_w )
{
	gsword_state *state = space->machine().driver_data<gsword_state>();
	UINT8 *videoram = state->m_videoram;
	videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset);
}

WRITE8_HANDLER( gsword_charbank_w )
{
	gsword_state *state = space->machine().driver_data<gsword_state>();
	if (state->m_charbank != data)
	{
		state->m_charbank = data;
		tilemap_mark_all_tiles_dirty_all(space->machine());
	}
}

WRITE8_HANDLER( gsword_videoctrl_w )
{
	gsword_state *state = space->machine().driver_data<gsword_state>();
	if (data & 0x8f)
	{
		popmessage("videoctrl %02x",data);
	}

	/* bits 5-6 are char palette bank */

	if (state->m_charpalbank != ((data & 0x60) >> 5))
	{
		state->m_charpalbank = (data & 0x60) >> 5;
		tilemap_mark_all_tiles_dirty_all(space->machine());
	}

	/* bit 4 is flip screen */

	if (state->m_flipscreen != (data & 0x10))
	{
		state->m_flipscreen = data & 0x10;
	    tilemap_mark_all_tiles_dirty_all(space->machine());
	}

	/* bit 0 could be used but unknown */

	/* other bits unused */
}

WRITE8_HANDLER( gsword_scroll_w )
{
	gsword_state *state = space->machine().driver_data<gsword_state>();
	tilemap_set_scrolly(state->m_bg_tilemap, 0, data);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	gsword_state *state = machine.driver_data<gsword_state>();
	UINT8 *videoram = state->m_videoram;
	int code = videoram[tile_index] + ((state->m_charbank & 0x03) << 8);
	int color = ((code & 0x3c0) >> 6) + 16 * state->m_charpalbank;
	int flags = state->m_flipscreen ? (TILE_FLIPX | TILE_FLIPY) : 0;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( gsword )
{
	gsword_state *state = machine.driver_data<gsword_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 64);
}

static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	gsword_state *state = machine.driver_data<gsword_state>();
	int offs;

	for (offs = 0; offs < state->m_spritexy_size - 1; offs+=2)
	{
		int sx,sy,flipx,flipy,spritebank,tile,color;

		if (state->m_spritexy_ram[offs]!=0xf1)
		{
			spritebank = 0;
			tile = state->m_spritetile_ram[offs];
			color = state->m_spritetile_ram[offs+1] & 0x3f;
			sy = 241-state->m_spritexy_ram[offs];
			sx = state->m_spritexy_ram[offs+1]-56;
			flipx = state->m_spriteattrib_ram[offs] & 0x02;
			flipy = state->m_spriteattrib_ram[offs] & 0x01;

			// Adjust sprites that should be far far right!
			if (sx<0) sx+=256;

			// Adjuste for 32x32 tiles(#128-256)
			if (tile > 127)
			{
				spritebank = 1;
				tile -= 128;
				sy-=16;
			}
			if (state->m_flipscreen)
			{
				flipx = !flipx;
				flipy = !flipy;
			}
			drawgfx_transmask(bitmap,cliprect,machine.gfx[1+spritebank],
					tile,
					color,
					flipx,flipy,
					sx,sy,
					colortable_get_transpen_mask(machine.colortable, machine.gfx[1+spritebank], color, 0x8f));
		}
	}
}

SCREEN_UPDATE( gsword )
{
	gsword_state *state = screen->machine().driver_data<gsword_state>();
	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}
