/*******************************************************************************

XX Mission (c) 1986 UPL

Video hardware driver by Uki

    31/Mar/2001 -

*******************************************************************************/

#include "emu.h"
#include "includes/xxmissio.h"


WRITE8_DEVICE_HANDLER( xxmissio_scroll_x_w )
{
	xxmissio_state *state = device->machine().driver_data<xxmissio_state>();
	state->m_xscroll = data;
}
WRITE8_DEVICE_HANDLER( xxmissio_scroll_y_w )
{
	xxmissio_state *state = device->machine().driver_data<xxmissio_state>();
	state->m_yscroll = data;
}

WRITE8_HANDLER( xxmissio_flipscreen_w )
{
	xxmissio_state *state = space->machine().driver_data<xxmissio_state>();
	state->m_flipscreen = data & 0x01;
}

WRITE8_HANDLER( xxmissio_bgram_w )
{
	xxmissio_state *state = space->machine().driver_data<xxmissio_state>();
	int x = (offset + (state->m_xscroll >> 3)) & 0x1f;
	offset = (offset & 0x7e0) | x;

	state->m_bgram[offset] = data;
}
READ8_HANDLER( xxmissio_bgram_r )
{
	xxmissio_state *state = space->machine().driver_data<xxmissio_state>();
	int x = (offset + (state->m_xscroll >> 3)) & 0x1f;
	offset = (offset & 0x7e0) | x;

	return state->m_bgram[offset];
}

WRITE8_HANDLER( xxmissio_paletteram_w )
{
	paletteram_BBGGRRII_w(space,offset,data);
}

/****************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	xxmissio_state *state = machine.driver_data<xxmissio_state>();
	int code = ((state->m_bgram[0x400 | tile_index] & 0xc0) << 2) | state->m_bgram[0x000 | tile_index];
	int color =  state->m_bgram[0x400 | tile_index] & 0x0f;

	SET_TILE_INFO(2, code, color, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	xxmissio_state *state = machine.driver_data<xxmissio_state>();
	int code = state->m_fgram[0x000 | tile_index];
	int color = state->m_fgram[0x400 | tile_index] & 0x07;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( xxmissio )
{
	xxmissio_state *state = machine.driver_data<xxmissio_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 8, 32, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 16, 8, 32, 32);

	tilemap_set_scroll_cols(state->m_bg_tilemap, 1);
	tilemap_set_scroll_rows(state->m_bg_tilemap, 1);
	tilemap_set_scrolldx(state->m_bg_tilemap, 2, 12);

	tilemap_set_transparent_pen(state->m_fg_tilemap, 0);
}


static void draw_sprites(bitmap_t *bitmap, const rectangle *cliprect, const gfx_element *gfx)
{
	xxmissio_state *state = gfx->machine().driver_data<xxmissio_state>();
	int offs;
	int chr,col;
	int x,y,px,py,fx,fy;

	for (offs=0; offs<0x800; offs +=0x20)
	{
		chr = state->m_spriteram[offs];
		col = state->m_spriteram[offs+3];

		fx = ((col & 0x10) >> 4) ^ state->m_flipscreen;
		fy = ((col & 0x20) >> 5) ^ state->m_flipscreen;

		x = state->m_spriteram[offs+1]*2;
		y = state->m_spriteram[offs+2];

		chr = chr + ((col & 0x40) << 2);
		col = col & 0x07;

		if (state->m_flipscreen==0)
		{
			px = x-8;
			py = y;
		}
		else
		{
			px = 480-x-6;
			py = 240-y;
		}

		px &= 0x1ff;

		drawgfx_transpen(bitmap,cliprect,gfx,
			chr,
			col,
			fx,fy,
			px,py,0);

		if (px>0x1e0)
			drawgfx_transpen(bitmap,cliprect,gfx,
				chr,
				col,
				fx,fy,
				px-0x200,py,0);

	}
}


SCREEN_UPDATE( xxmissio )
{
	xxmissio_state *state = screen->machine().driver_data<xxmissio_state>();
	tilemap_mark_all_tiles_dirty_all(screen->machine());
	tilemap_set_flip_all(screen->machine(), state->m_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	tilemap_set_scrollx(state->m_bg_tilemap, 0, state->m_xscroll * 2);
	tilemap_set_scrolly(state->m_bg_tilemap, 0, state->m_yscroll);

	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	draw_sprites(bitmap, cliprect, screen->machine().gfx[1]);
	tilemap_draw(bitmap, cliprect, state->m_fg_tilemap, 0, 0);

	return 0;
}
