/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

  (c) 12/2/1998 Lee Taylor

  2006 - major rewrite by couriersud

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "m10.h"

static tilemap *		tx_tilemap;
static gfx_element *	back_gfx;
static UINT32			extyoffs[32*8];

static const gfx_layout backlayout =
{
	8,8*32,	/* 8*8 characters */
	4,		/* 256 characters */
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

static UINT32 tilemap_scan(UINT32 col,UINT32 row,UINT32 num_cols,UINT32 num_rows)
{
	//m10_state *state = Machine->driver_data;

	return (31-col)*32 + row;
}


static void get_tile_info(running_machine *machine, tile_data *tileinfo, tilemap_memory_index tile_index, void *param)
{
	SET_TILE_INFO(0, videoram[tile_index], colorram[tile_index] & 0x07, 0);
}


WRITE8_HANDLER( m10_colorram_w )
{
	if (colorram[offset] != data)
	{
		tilemap_mark_tile_dirty(tx_tilemap, offset);
		colorram[offset] = data;
	}
}


WRITE8_HANDLER( m15_chargen_w )
{
	m10_state *state = Machine->driver_data;

	if (state->chargen[offset] != data)
	{
		state->chargen[offset] = data;
		/* not very effective ... dirty would be better */
		decodechar(Machine->gfx[0],offset >> 3,state->chargen);
	}
}


INLINE void plot_pixel_m10(mame_bitmap *bm, int x, int y, int col)
{
	m10_state *state = Machine->driver_data;

	if (!state->flip)
		*BITMAP_ADDR16(bm, y, x) = col;
	else
		*BITMAP_ADDR16(bm, (IREMM10_VBSTART - 1)- (y - IREMM10_VBEND) + 6
				, (IREMM10_HBSTART - 1)- (x- IREMM10_HBEND)) = col; // only when flip_screen(?)
}

VIDEO_START( m10 )
{
	//m10_state *state = machine->driver_data;
	int i;

	for (i=0;i<32*8;i++)
		extyoffs[i] = i*8;

	tx_tilemap = tilemap_create(get_tile_info,tilemap_scan,8,8,32,32);
	tilemap_set_transparent_pen(tx_tilemap, 0);
	tilemap_set_scrolldx(tx_tilemap, 0, 62);
	tilemap_set_scrolldy(tx_tilemap, 0, 0);

	back_gfx = allocgfx(&backlayout);
	back_gfx->total_colors = 8;

	machine->gfx[1] = back_gfx;
	return ;
}

VIDEO_START( m15 )
{
	m10_state *state = machine->driver_data;

	machine->gfx[0] = allocgfx(&charlayout);
	machine->gfx[0]->total_colors = 8;

	decodegfx(machine->gfx[0], state->chargen,0,256);

	tx_tilemap = tilemap_create(get_tile_info,tilemap_scan,8,8,32,32);
	tilemap_set_scrolldx(tx_tilemap, 0, 116);
	tilemap_set_scrolldy(tx_tilemap, 0, 0);

	return ;
}

/***************************************************************************

  Draw the game screen in the given mame_bitmap.

***************************************************************************/
VIDEO_UPDATE( m10 )
{
	m10_state *state = machine->driver_data;
	int offs;
	static const int color[4]= { 3, 3, 5, 5 };
	static const int xpos[4] = { 4*8, 26*8, 7*8, 6*8};
	int i;

	fillbitmap(bitmap,machine->pens[0],cliprect);

	decodegfx(back_gfx, state->chargen,0,4);
	for (i=0;i<4;i++)
		if (state->flip)
			drawgfx(bitmap, back_gfx, i, color[i], 1, 1, 31*8 - xpos[i], 6, cliprect, 0, 0);
		else
			drawgfx(bitmap, back_gfx, i, color[i], 0, 0, xpos[i], 0, cliprect, 0, 0);

	if (state->bottomline)
	{
		int y;

		for (y = IREMM10_VBEND;y < IREMM10_VBSTART;y++)
			plot_pixel_m10(bitmap,16,y,1);
	}

	for (offs = videoram_size - 1;offs >= 0;offs--)
		tilemap_mark_tile_dirty(tx_tilemap, offs);

	tilemap_set_flip(tx_tilemap, state->flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);

	return 0;
}


/***************************************************************************

  Draw the game screen in the given mame_bitmap.

***************************************************************************/
VIDEO_UPDATE( m15 )
{
	m10_state *state = machine->driver_data;
	int offs;

	for (offs = videoram_size - 1;offs >= 0;offs--)
		tilemap_mark_tile_dirty(tx_tilemap, offs);

	//tilemap_mark_all_tiles_dirty(tx_tilemap);
	tilemap_set_flip(tx_tilemap, state->flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);

	return 0;
}

