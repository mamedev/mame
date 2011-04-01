/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

  (c) 12/2/1998 Lee Taylor

  2006 - major rewrite by couriersud

***************************************************************************/

#include "emu.h"
#include "includes/m10.h"

static const UINT32 extyoffs[] =
{
	STEP256(0, 8)
};


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


static void get_tile_info( running_machine &machine, tile_data *tileinfo, tilemap_memory_index tile_index, void *param )
{
	m10_state *state = machine.driver_data<m10_state>();

	SET_TILE_INFO(0, state->m_videoram[tile_index], state->m_colorram[tile_index] & 0x07, 0);
}


WRITE8_HANDLER( m10_colorram_w )
{
	m10_state *state = space->machine().driver_data<m10_state>();

	if (state->m_colorram[offset] != data)
	{
		tilemap_mark_tile_dirty(state->m_tx_tilemap, offset);
		state->m_colorram[offset] = data;
	}
}


WRITE8_HANDLER( m10_chargen_w )
{
	m10_state *state = space->machine().driver_data<m10_state>();

	if (state->m_chargen[offset] != data)
	{
		state->m_chargen[offset] = data;
		gfx_element_mark_dirty(state->m_back_gfx, offset >> (3 + 5));
	}
}


WRITE8_HANDLER( m15_chargen_w )
{
	m10_state *state = space->machine().driver_data<m10_state>();

	if (state->m_chargen[offset] != data)
	{
		state->m_chargen[offset] = data;
		gfx_element_mark_dirty(space->machine().gfx[0], offset >> 3);
	}
}


INLINE void plot_pixel_m10( running_machine &machine, bitmap_t *bm, int x, int y, int col )
{
	m10_state *state = machine.driver_data<m10_state>();

	if (!state->m_flip)
		*BITMAP_ADDR16(bm, y, x) = col;
	else
		*BITMAP_ADDR16(bm, (IREMM10_VBSTART - 1) - (y - IREMM10_VBEND) + 6,
				(IREMM10_HBSTART - 1) - (x - IREMM10_HBEND)) = col; // only when flip_screen(?)
}

VIDEO_START( m10 )
{
	m10_state *state = machine.driver_data<m10_state>();

	state->m_tx_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan, 8, 8, 32, 32);
	tilemap_set_transparent_pen(state->m_tx_tilemap, 0);
	tilemap_set_scrolldx(state->m_tx_tilemap, 0, 62);
	tilemap_set_scrolldy(state->m_tx_tilemap, 0, 0);

	state->m_back_gfx = gfx_element_alloc(machine, &backlayout, state->m_chargen, 8, 0);

	machine.gfx[1] = state->m_back_gfx;
	return ;
}

VIDEO_START( m15 )
{
	m10_state *state = machine.driver_data<m10_state>();

	machine.gfx[0] = gfx_element_alloc(machine, &charlayout, state->m_chargen, 8, 0);

	state->m_tx_tilemap = tilemap_create(machine, get_tile_info,tilemap_scan, 8, 8, 32, 32);
	tilemap_set_scrolldx(state->m_tx_tilemap, 0, 116);
	tilemap_set_scrolldy(state->m_tx_tilemap, 0, 0);

	return ;
}

/***************************************************************************

  Draw the game screen in the given bitmap_t.

***************************************************************************/

SCREEN_UPDATE( m10 )
{
	m10_state *state = screen->machine().driver_data<m10_state>();
	int offs;
	static const int color[4]= { 3, 3, 5, 5 };
	static const int xpos[4] = { 4*8, 26*8, 7*8, 6*8};
	int i;

	bitmap_fill(bitmap, cliprect, 0);

	for (i = 0; i < 4; i++)
		if (state->m_flip)
			drawgfx_opaque(bitmap, cliprect, state->m_back_gfx, i, color[i], 1, 1, 31 * 8 - xpos[i], 6);
		else
			drawgfx_opaque(bitmap, cliprect, state->m_back_gfx, i, color[i], 0, 0, xpos[i], 0);

	if (state->m_bottomline)
	{
		int y;

		for (y = IREMM10_VBEND; y < IREMM10_VBSTART; y++)
			plot_pixel_m10(screen->machine(), bitmap, 16, y, 1);
	}

	for (offs = state->m_videoram_size - 1; offs >= 0; offs--)
		tilemap_mark_tile_dirty(state->m_tx_tilemap, offs);

	tilemap_set_flip(state->m_tx_tilemap, state->m_flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	tilemap_draw(bitmap, cliprect, state->m_tx_tilemap, 0, 0);

	return 0;
}


/***************************************************************************

  Draw the game screen in the given bitmap_t.

***************************************************************************/

SCREEN_UPDATE( m15 )
{
	m10_state *state = screen->machine().driver_data<m10_state>();
	int offs;

	for (offs = state->m_videoram_size - 1; offs >= 0; offs--)
		tilemap_mark_tile_dirty(state->m_tx_tilemap, offs);

	//tilemap_mark_all_tiles_dirty(state->m_tx_tilemap);
	tilemap_set_flip(state->m_tx_tilemap, state->m_flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	tilemap_draw(bitmap, cliprect, state->m_tx_tilemap, 0, 0);

	return 0;
}
