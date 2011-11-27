/*************************************************************************

    Namco Pac Man

**************************************************************************

    This file is used by the Pac Man, Pengo & Jr Pac Man drivers.

    Pengo & Pac Man are almost identical, the only differences being the
    extra gfx bank in Pengo, and the need to compensate for an hardware
    sprite positioning "bug" in Pac Man.

    Jr Pac Man has the same sprite hardware as Pac Man, the extra bank
    from Pengo and a scrolling playfield at the expense of one color per row
    for the playfield so it can fit in the same amount of ram.

**************************************************************************/

#include "emu.h"
#include "includes/pacman.h"
#include "video/resnet.h"


static const rectangle spritevisiblearea =
{
	2*8, 34*8-1,
	0*8, 28*8-1
};



/***************************************************************************

  Convert the color PROMs into a more useable format.


  Pac Man has a 32x8 palette PROM and a 256x4 color lookup table PROM.

  Pengo has a 32x8 palette PROM and a 1024x4 color lookup table PROM.

  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED


  Jr. Pac Man has two 256x4 palette PROMs (the three msb of the address are
  grounded, so the effective colors are only 32) and one 256x4 color lookup
  table PROM.

  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
  bit 0 -- 470 ohm resistor  -- GREEN

  bit 3 -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT( pacman )
{
	static const int resistances[3] = { 1000, 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3, &resistances[0], rweights, 0, 0,
			3, &resistances[0], gweights, 0, 0,
			2, &resistances[1], bweights, 0, 0);

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 32);

	/* create a lookup table for the palette */
	for (i = 0; i < 32; i++)
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

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 32;

	/* allocate the colortable */
	for (i = 0; i < 64*4; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;

		/* first palette bank */
		colortable_entry_set_value(machine.colortable, i, ctabentry);

		/* second palette bank */
		colortable_entry_set_value(machine.colortable, i + 64*4, 0x10 + ctabentry);
	}
}

static TILEMAP_MAPPER( pacman_scan_rows )
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

static TILE_GET_INFO( pacman_get_tile_info )
{
	pacman_state *state = machine.driver_data<pacman_state>();
	int code = state->m_videoram[tile_index] | (state->m_charbank << 8);
	int attr = (state->m_colorram[tile_index] & 0x1f) | (state->m_colortablebank << 5) | (state->m_palettebank << 6 );

	SET_TILE_INFO(0,code,attr,0);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static void init_save_state(running_machine &machine)
{
	pacman_state *state = machine.driver_data<pacman_state>();
	state_save_register_global(machine, state->m_charbank);
	state_save_register_global(machine, state->m_spritebank);
	state_save_register_global(machine, state->m_palettebank);
	state_save_register_global(machine, state->m_colortablebank);
	state_save_register_global(machine, state->m_flipscreen);
	state_save_register_global(machine, state->m_bgpriority);
}


VIDEO_START( pacman )
{
	pacman_state *state = machine.driver_data<pacman_state>();
	init_save_state(machine);

	state->m_charbank = 0;
	state->m_spritebank = 0;
	state->m_palettebank = 0;
	state->m_colortablebank = 0;
	state->m_flipscreen = 0;
	state->m_bgpriority = 0;
	state->m_inv_spr = 0;

	/* In the Pac Man based games (NOT Pengo) the first two sprites must be offset */
	/* one pixel to the left to get a more correct placement */
	state->m_xoffsethack = 1;

	state->m_bg_tilemap = tilemap_create( machine, pacman_get_tile_info, pacman_scan_rows,  8, 8, 36, 28 );

	tilemap_set_scrolldx( state->m_bg_tilemap, 0, 384 - 288 );
	tilemap_set_scrolldy( state->m_bg_tilemap, 0, 264 - 224 );
}

VIDEO_START( birdiy )
{
	pacman_state *state = machine.driver_data<pacman_state>();

	VIDEO_START_CALL( pacman );
	state->m_xoffsethack = 0;
	state->m_inv_spr = 1; // sprites are mirrored in X-axis compared to normal behaviour
}

WRITE8_HANDLER( pacman_videoram_w )
{
	pacman_state *state = space->machine().driver_data<pacman_state>();
	state->m_videoram[offset] = data;
	tilemap_mark_tile_dirty( state->m_bg_tilemap, offset );
}

WRITE8_HANDLER( pacman_colorram_w )
{
	pacman_state *state = space->machine().driver_data<pacman_state>();
	state->m_colorram[offset] = data;
	tilemap_mark_tile_dirty( state->m_bg_tilemap, offset );
}

WRITE8_HANDLER( pacman_flipscreen_w )
{
	pacman_state *state = space->machine().driver_data<pacman_state>();
	state->m_flipscreen = data & 1;
	tilemap_set_flip( state->m_bg_tilemap, state->m_flipscreen * ( TILEMAP_FLIPX + TILEMAP_FLIPY ) );
}


SCREEN_UPDATE( pacman )
{
	pacman_state *state = screen->machine().driver_data<pacman_state>();
	if (state->m_bgpriority != 0)
		bitmap_fill(bitmap,cliprect,0);
	else
		tilemap_draw(bitmap,cliprect,state->m_bg_tilemap,TILEMAP_DRAW_OPAQUE,0);

	if( screen->machine().generic.spriteram_size )
	{
		UINT8 *spriteram = screen->machine().generic.spriteram.u8;
		UINT8 *spriteram_2 = screen->machine().generic.spriteram2.u8;
		int offs;

		rectangle spriteclip = spritevisiblearea;
		sect_rect(&spriteclip, cliprect);

		/* Draw the sprites. Note that it is important to draw them exactly in this */
		/* order, to have the correct priorities. */
		for (offs = screen->machine().generic.spriteram_size - 2;offs > 2*2;offs -= 2)
		{
			int color;
			int sx,sy;
			UINT8 fx,fy;

			if(state->m_inv_spr)
			{
				sx = spriteram_2[offs + 1];
				sy = 240 - (spriteram_2[offs]);
			}
			else
			{
				sx = 272 - spriteram_2[offs + 1];
				sy = spriteram_2[offs] - 31;
			}

			fx = (spriteram[offs] & 1) ^ state->m_inv_spr;
			fy = (spriteram[offs] & 2) ^ ((state->m_inv_spr) << 1);

			color = ( spriteram[offs + 1] & 0x1f ) | (state->m_colortablebank << 5) | (state->m_palettebank << 6 );

			drawgfx_transmask(bitmap,&spriteclip,screen->machine().gfx[1],
					( spriteram[offs] >> 2 ) | (state->m_spritebank << 6),
					color,
					fx,fy,
					sx,sy,
					colortable_get_transpen_mask(screen->machine().colortable, screen->machine().gfx[1], color & 0x3f, 0));

			/* also plot the sprite with wraparound (tunnel in Crush Roller) */
			drawgfx_transmask(bitmap,&spriteclip,screen->machine().gfx[1],
					( spriteram[offs] >> 2 ) | (state->m_spritebank << 6),
					color,
					fx,fy,
					sx - 256,sy,
					colortable_get_transpen_mask(screen->machine().colortable, screen->machine().gfx[1], color & 0x3f, 0));
		}
		/* In the Pac Man based games (NOT Pengo) the first two sprites must be offset */
		/* one pixel to the left to get a more correct placement */
		for (offs = 2*2;offs >= 0;offs -= 2)
		{
			int color;
			int sx,sy;
			UINT8 fx,fy;

			if(state->m_inv_spr)
			{
				sx = spriteram_2[offs + 1];
				sy = 240 - (spriteram_2[offs]);
			}
			else
			{
				sx = 272 - spriteram_2[offs + 1];
				sy = spriteram_2[offs] - 31;
			}
			color = ( spriteram[offs + 1] & 0x1f ) | (state->m_colortablebank << 5) | (state->m_palettebank << 6 );

			fx = (spriteram[offs] & 1) ^ state->m_inv_spr;
			fy = (spriteram[offs] & 2) ^ ((state->m_inv_spr) << 1);

			drawgfx_transmask(bitmap,&spriteclip,screen->machine().gfx[1],
					( spriteram[offs] >> 2 ) | (state->m_spritebank << 6),
					color,
					fx,fy,
					sx,sy + state->m_xoffsethack,
					colortable_get_transpen_mask(screen->machine().colortable, screen->machine().gfx[1], color & 0x3f, 0));

			/* also plot the sprite with wraparound (tunnel in Crush Roller) */
			drawgfx_transmask(bitmap,&spriteclip,screen->machine().gfx[1],
					( spriteram[offs] >> 2 ) | (state->m_spritebank << 6),
					color,
					fy,fx,			//FIXME: flipping bits are really supposed to be inverted here?
					sx - 256,sy + state->m_xoffsethack,
					colortable_get_transpen_mask(screen->machine().colortable, screen->machine().gfx[1], color & 0x3f, 0));
		}
	}

	if (state->m_bgpriority != 0)
		tilemap_draw(bitmap,cliprect,state->m_bg_tilemap,0,0);
	return 0;
}


/*************************************************************************

    Sega Pengo

**************************************************************************/

VIDEO_START( pengo )
{
	pacman_state *state = machine.driver_data<pacman_state>();
	init_save_state(machine);

	state->m_charbank = 0;
	state->m_spritebank = 0;
	state->m_palettebank = 0;
	state->m_colortablebank = 0;
	state->m_flipscreen = 0;
	state->m_bgpriority = 0;
	state->m_inv_spr = 0;
	state->m_xoffsethack = 0;

	state->m_bg_tilemap = tilemap_create( machine, pacman_get_tile_info, pacman_scan_rows,  8, 8, 36, 28 );

	tilemap_set_scrolldx( state->m_bg_tilemap, 0, 384 - 288 );
	tilemap_set_scrolldy( state->m_bg_tilemap, 0, 264 - 224 );
}

WRITE8_HANDLER( pengo_palettebank_w )
{
	pacman_state *state = space->machine().driver_data<pacman_state>();
	if (state->m_palettebank != data)
	{
		state->m_palettebank = data;
		tilemap_mark_all_tiles_dirty( state->m_bg_tilemap );
	}
}

WRITE8_HANDLER( pengo_colortablebank_w )
{
	pacman_state *state = space->machine().driver_data<pacman_state>();
	if (state->m_colortablebank != data)
	{
		state->m_colortablebank = data;
		tilemap_mark_all_tiles_dirty( state->m_bg_tilemap );
	}
}

WRITE8_HANDLER( pengo_gfxbank_w )
{
	pacman_state *state = space->machine().driver_data<pacman_state>();
	if (state->m_charbank != (data & 1))
	{
		state->m_spritebank = data & 1;
		state->m_charbank = data & 1;
		tilemap_mark_all_tiles_dirty( state->m_bg_tilemap );
	}
}


/*************************************************************************

S2650 Games

**************************************************************************/

static TILE_GET_INFO( s2650_get_tile_info )
{
	pacman_state *state = machine.driver_data<pacman_state>();
	int colbank, code, attr;

	colbank = state->m_s2650games_tileram[tile_index & 0x1f] & 0x3;

	code = state->m_videoram[tile_index] + (colbank << 8);
	attr = state->m_colorram[tile_index & 0x1f];

	SET_TILE_INFO(0,code,attr & 0x1f,0);
}

VIDEO_START( s2650games )
{
	pacman_state *state = machine.driver_data<pacman_state>();
	init_save_state(machine);

	state->m_charbank = 0;
	state->m_spritebank = 0;
	state->m_palettebank = 0;
	state->m_colortablebank = 0;
	state->m_flipscreen = 0;
	state->m_bgpriority = 0;
	state->m_inv_spr = 0;
	state->m_xoffsethack = 1;

	state->m_bg_tilemap = tilemap_create( machine, s2650_get_tile_info,tilemap_scan_rows,8,8,32,32 );

	tilemap_set_scroll_cols(state->m_bg_tilemap, 32);
}

SCREEN_UPDATE( s2650games )
{
	pacman_state *state = screen->machine().driver_data<pacman_state>();
	UINT8 *spriteram = screen->machine().generic.spriteram.u8;
	UINT8 *spriteram_2 = screen->machine().generic.spriteram2.u8;
	int offs;

	tilemap_draw(bitmap,cliprect,state->m_bg_tilemap,0,0);

	for (offs = screen->machine().generic.spriteram_size - 2;offs > 2*2;offs -= 2)
	{
		int color;
		int sx,sy;


		sx = 255 - spriteram_2[offs + 1];
		sy = spriteram_2[offs] - 15;
		color = spriteram[offs + 1] & 0x1f;

		/* TODO: ?? */
		drawgfx_transmask(bitmap,cliprect,screen->machine().gfx[1],
				(spriteram[offs] >> 2) | ((state->m_s2650games_spriteram[offs] & 3) << 6),
				color,
				spriteram[offs] & 1,spriteram[offs] & 2,
				sx,sy,
				colortable_get_transpen_mask(screen->machine().colortable, screen->machine().gfx[1], color & 0x3f, 0));
	}
	/* In the Pac Man based games (NOT Pengo) the first two sprites must be offset */
	/* one pixel to the left to get a more correct placement */
	for (offs = 2*2;offs >= 0;offs -= 2)
	{
		int color;
		int sx,sy;


		sx = 255 - spriteram_2[offs + 1];
		sy = spriteram_2[offs] - 15;
		color = spriteram[offs + 1] & 0x1f;

		/* TODO: ?? */
		drawgfx_transmask(bitmap,cliprect,screen->machine().gfx[1],
				(spriteram[offs] >> 2) | ((state->m_s2650games_spriteram[offs] & 3)<<6),
				color,
				spriteram[offs] & 1,spriteram[offs] & 2,
				sx,sy + state->m_xoffsethack,
				colortable_get_transpen_mask(screen->machine().colortable, screen->machine().gfx[1], color & 0x3f, 0));
	}
	return 0;
}

WRITE8_HANDLER( s2650games_videoram_w )
{
	pacman_state *state = space->machine().driver_data<pacman_state>();
	state->m_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap,offset);
}

WRITE8_HANDLER( s2650games_colorram_w )
{
	pacman_state *state = space->machine().driver_data<pacman_state>();
	int i;
	state->m_colorram[offset & 0x1f] = data;
	for (i = offset; i < 0x0400; i += 32)
		tilemap_mark_tile_dirty(state->m_bg_tilemap, i);
}

WRITE8_HANDLER( s2650games_scroll_w )
{
	pacman_state *state = space->machine().driver_data<pacman_state>();
	tilemap_set_scrolly(state->m_bg_tilemap, offset, data);
}

WRITE8_HANDLER( s2650games_tilesbank_w )
{
	pacman_state *state = space->machine().driver_data<pacman_state>();
	state->m_s2650games_tileram[offset] = data;
	tilemap_mark_all_tiles_dirty(state->m_bg_tilemap);
}


/*************************************************************************

Jr. Pac-Man

**************************************************************************/

/*
   0 -   31 = column 2 - 33 attr (used for all 54 rows)
  64 - 1791 = column 2 - 33 code (54 rows)
1794 - 1821 = column 34 code (28 rows)
1826 - 1853 = column 35 code (28 rows)
1858 - 1885 = column 0 code (28 rows)
1890 - 1917 = column 1 code (28 rows)
1922 - 1949 = column 34 attr (28 rows)
1954 - 1981 = column 35 attr (28 rows)
1986 - 2013 = column 0 attr (28 rows)
2018 - 2045 = column 1 attr (28 rows)
*/

static TILEMAP_MAPPER( jrpacman_scan_rows )
{
	int offs;

	row += 2;
	col -= 2;
	if ((col & 0x20) && (row & 0x20))
		offs = 0;
	else if (col & 0x20)
		offs = row + (((col&0x3) | 0x38)<< 5);
	else
		offs = col + (row << 5);
	return offs;
}

static TILE_GET_INFO( jrpacman_get_tile_info )
{
	pacman_state *state = machine.driver_data<pacman_state>();
	int color_index, code, attr;
	if( tile_index < 1792 )
	{
		color_index = tile_index & 0x1f;
	}
	else
	{
		color_index = tile_index + 0x80;
	}

	code = state->m_videoram[tile_index] | (state->m_charbank << 8);
	attr = (state->m_videoram[color_index] & 0x1f) | (state->m_colortablebank << 5) | (state->m_palettebank << 6 );

	SET_TILE_INFO(0,code,attr,0);
}

static void jrpacman_mark_tile_dirty( running_machine &machine, int offset )
{
	pacman_state *state = machine.driver_data<pacman_state>();
	if( offset < 0x20 )
	{
		/* line color - mark whole line as dirty */
		int i;
		for( i = 2 * 0x20; i < 56 * 0x20; i += 0x20 )
		{
			tilemap_mark_tile_dirty( state->m_bg_tilemap, offset + i );
		}
	}
	else if (offset < 1792)
	{
		/* tiles for playfield */
		tilemap_mark_tile_dirty( state->m_bg_tilemap, offset );
	}
	else
	{
		/* tiles & colors for top and bottom two rows */
		tilemap_mark_tile_dirty( state->m_bg_tilemap, offset & ~0x80 );
	}
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
VIDEO_START( jrpacman )
{
	pacman_state *state = machine.driver_data<pacman_state>();
	init_save_state(machine);

	state->m_charbank = 0;
	state->m_spritebank = 0;
	state->m_palettebank = 0;
	state->m_colortablebank = 0;
	state->m_flipscreen = 0;
	state->m_bgpriority = 0;
	state->m_inv_spr = 0;
	state->m_xoffsethack = 1;

	state->m_bg_tilemap = tilemap_create( machine, jrpacman_get_tile_info,jrpacman_scan_rows,8,8,36,54 );

	tilemap_set_transparent_pen( state->m_bg_tilemap, 0 );
	tilemap_set_scroll_cols( state->m_bg_tilemap, 36 );
}

WRITE8_HANDLER( jrpacman_videoram_w )
{
	pacman_state *state = space->machine().driver_data<pacman_state>();
	state->m_videoram[offset] = data;
	jrpacman_mark_tile_dirty(space->machine(), offset);
}

WRITE8_HANDLER( jrpacman_charbank_w )
{
	pacman_state *state = space->machine().driver_data<pacman_state>();
	if (state->m_charbank != (data & 1))
	{
		state->m_charbank = data & 1;
		tilemap_mark_all_tiles_dirty(state->m_bg_tilemap);
	}
}

WRITE8_HANDLER( jrpacman_spritebank_w )
{
	pacman_state *state = space->machine().driver_data<pacman_state>();
	state->m_spritebank = (data & 1);
}

WRITE8_HANDLER( jrpacman_scroll_w )
{
	pacman_state *state = space->machine().driver_data<pacman_state>();
	int i;
	for( i = 2; i < 34; i++ )
	{
		tilemap_set_scrolly( state->m_bg_tilemap, i, data );
	}
}

WRITE8_HANDLER( jrpacman_bgpriority_w )
{
	pacman_state *state = space->machine().driver_data<pacman_state>();
	state->m_bgpriority = (data & 1);
}
