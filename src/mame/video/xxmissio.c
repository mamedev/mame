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

WRITE8_MEMBER(xxmissio_state::xxmissio_flipscreen_w)
{
	m_flipscreen = data & 0x01;
}

WRITE8_MEMBER(xxmissio_state::xxmissio_bgram_w)
{
	int x = (offset + (m_xscroll >> 3)) & 0x1f;
	offset = (offset & 0x7e0) | x;

	m_bgram[offset] = data;
}
READ8_MEMBER(xxmissio_state::xxmissio_bgram_r)
{
	int x = (offset + (m_xscroll >> 3)) & 0x1f;
	offset = (offset & 0x7e0) | x;

	return m_bgram[offset];
}

WRITE8_MEMBER(xxmissio_state::xxmissio_paletteram_w)
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

	state->m_bg_tilemap->set_scroll_cols(1);
	state->m_bg_tilemap->set_scroll_rows(1);
	state->m_bg_tilemap->set_scrolldx(2, 12);

	state->m_fg_tilemap->set_transparent_pen(0);
}


static void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, const gfx_element *gfx)
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


SCREEN_UPDATE_IND16( xxmissio )
{
	xxmissio_state *state = screen.machine().driver_data<xxmissio_state>();
	screen.machine().tilemap().mark_all_dirty();
	screen.machine().tilemap().set_flip_all(state->m_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	state->m_bg_tilemap->set_scrollx(0, state->m_xscroll * 2);
	state->m_bg_tilemap->set_scrolly(0, state->m_yscroll);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, screen.machine().gfx[1]);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);

	return 0;
}
