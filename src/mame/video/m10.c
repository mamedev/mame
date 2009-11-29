/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

  (c) 12/2/1998 Lee Taylor

  2006 - major rewrite by couriersud

***************************************************************************/

#include "driver.h"
#include "m10.h"

static UINT32 extyoffs[32 * 8];	// FIXME: this should be moved to m10_state, but backlayout would have problems


static const gfx_layout backlayout =
{
	8,8*32,	/* 8*(8*32) characters */
	4,		/* 4 characters */
	1,		/* 1 bit per pixel */
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	EXTENDED_YOFFS,
	32*8*8,	/* every char takes 8 consecutive bytes */
	NULL, extyoffs
};

static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	1,		/* 1 bit per pixel */
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};

static UINT32 tilemap_scan( UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows )
{
	return (31 - col) * 32 + row;
}


static void get_tile_info( running_machine *machine, tile_data *tileinfo, tilemap_memory_index tile_index, void *param )
{
	m10_state *state = (m10_state *)machine->driver_data;

	SET_TILE_INFO(0, state->videoram[tile_index], state->colorram[tile_index] & 0x07, 0);
}


WRITE8_HANDLER( m10_colorram_w )
{
	m10_state *state = (m10_state *)space->machine->driver_data;

	if (state->colorram[offset] != data)
	{
		tilemap_mark_tile_dirty(state->tx_tilemap, offset);
		state->colorram[offset] = data;
	}
}


WRITE8_HANDLER( m10_chargen_w )
{
	m10_state *state = (m10_state *)space->machine->driver_data;

	if (state->chargen[offset] != data)
	{
		state->chargen[offset] = data;
		gfx_element_mark_dirty(state->back_gfx, offset >> (3 + 5));
	}
}


WRITE8_HANDLER( m15_chargen_w )
{
	m10_state *state = (m10_state *)space->machine->driver_data;

	if (state->chargen[offset] != data)
	{
		state->chargen[offset] = data;
		gfx_element_mark_dirty(space->machine->gfx[0], offset >> 3);
	}
}


INLINE void plot_pixel_m10( running_machine *machine, bitmap_t *bm, int x, int y, int col )
{
	m10_state *state = (m10_state *)machine->driver_data;

	if (!state->flip)
		*BITMAP_ADDR16(bm, y, x) = col;
	else
		*BITMAP_ADDR16(bm, (IREMM10_VBSTART - 1) - (y - IREMM10_VBEND) + 6,
				(IREMM10_HBSTART - 1) - (x - IREMM10_HBEND)) = col; // only when flip_screen(?)
}

VIDEO_START( m10 )
{
	m10_state *state = (m10_state *)machine->driver_data;
	int i;

	for (i = 0; i < 32 * 8; i++)
		extyoffs[i] = i * 8;

	state->tx_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan, 8, 8, 32, 32);
	tilemap_set_transparent_pen(state->tx_tilemap, 0);
	tilemap_set_scrolldx(state->tx_tilemap, 0, 62);
	tilemap_set_scrolldy(state->tx_tilemap, 0, 0);

	state->back_gfx = gfx_element_alloc(machine, &backlayout, state->chargen, 8, 0);

	machine->gfx[1] = state->back_gfx;
	return ;
}

VIDEO_START( m15 )
{
	m10_state *state = (m10_state *)machine->driver_data;

	machine->gfx[0] = gfx_element_alloc(machine, &charlayout, state->chargen, 8, 0);

	state->tx_tilemap = tilemap_create(machine, get_tile_info,tilemap_scan, 8, 8, 32, 32);
	tilemap_set_scrolldx(state->tx_tilemap, 0, 116);
	tilemap_set_scrolldy(state->tx_tilemap, 0, 0);

	return ;
}

/***************************************************************************

  Draw the game screen in the given bitmap_t.

***************************************************************************/

VIDEO_UPDATE( m10 )
{
	m10_state *state = (m10_state *)screen->machine->driver_data;
	int offs;
	static const int color[4]= { 3, 3, 5, 5 };
	static const int xpos[4] = { 4*8, 26*8, 7*8, 6*8};
	int i;

	bitmap_fill(bitmap, cliprect, 0);

	for (i = 0; i < 4; i++)
		if (state->flip)
			drawgfx_opaque(bitmap, cliprect, state->back_gfx, i, color[i], 1, 1, 31 * 8 - xpos[i], 6);
		else
			drawgfx_opaque(bitmap, cliprect, state->back_gfx, i, color[i], 0, 0, xpos[i], 0);

	if (state->bottomline)
	{
		int y;

		for (y = IREMM10_VBEND; y < IREMM10_VBSTART; y++)
			plot_pixel_m10(screen->machine, bitmap, 16, y, 1);
	}

	for (offs = state->videoram_size - 1; offs >= 0; offs--)
		tilemap_mark_tile_dirty(state->tx_tilemap, offs);

	tilemap_set_flip(state->tx_tilemap, state->flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);

	return 0;
}


/***************************************************************************

  Draw the game screen in the given bitmap_t.

***************************************************************************/

VIDEO_UPDATE( m15 )
{
	m10_state *state = (m10_state *)screen->machine->driver_data;
	int offs;

	for (offs = state->videoram_size - 1; offs >= 0; offs--)
		tilemap_mark_tile_dirty(state->tx_tilemap, offs);

	//tilemap_mark_all_tiles_dirty(state->tx_tilemap);
	tilemap_set_flip(state->tx_tilemap, state->flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);

	return 0;
}
