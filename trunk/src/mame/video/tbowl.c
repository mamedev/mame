/* video/tbowl.c */

/* see drivers/tbowl.c for more info */

#include "emu.h"
#include "includes/tbowl.h"


/* Foreground Layer (tx) Tilemap */

static TILE_GET_INFO( get_tx_tile_info )
{
	tbowl_state *state = machine.driver_data<tbowl_state>();
	int tileno;
	int col;

	tileno = state->m_txvideoram[tile_index] | ((state->m_txvideoram[tile_index+0x800] & 0x07) << 8);
	col = (state->m_txvideoram[tile_index+0x800] & 0xf0) >> 4;

	SET_TILE_INFO(0,tileno,col,0);
}

WRITE8_HANDLER( tbowl_txvideoram_w )
{
	tbowl_state *state = space->machine().driver_data<tbowl_state>();
	state->m_txvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_tx_tilemap,offset & 0x7ff);
}

/* Bottom BG Layer (bg) Tilemap */

static TILE_GET_INFO( get_bg_tile_info )
{
	tbowl_state *state = machine.driver_data<tbowl_state>();
	int tileno;
	int col;

	tileno = state->m_bgvideoram[tile_index] | ((state->m_bgvideoram[tile_index+0x1000] & 0x0f) << 8);
	col = (state->m_bgvideoram[tile_index+0x1000] & 0xf0) >> 4;

	SET_TILE_INFO(1,tileno,col,0);
}

WRITE8_HANDLER( tbowl_bg2videoram_w )
{
	tbowl_state *state = space->machine().driver_data<tbowl_state>();
	state->m_bg2videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg2_tilemap,offset & 0xfff);
}

WRITE8_HANDLER (tbowl_bgxscroll_lo)
{
	tbowl_state *state = space->machine().driver_data<tbowl_state>();
	state->m_xscroll = (state->m_xscroll & 0xff00) | data;
}

WRITE8_HANDLER (tbowl_bgxscroll_hi)
{
	tbowl_state *state = space->machine().driver_data<tbowl_state>();
	state->m_xscroll = (state->m_xscroll & 0x00ff) | (data << 8);
}

WRITE8_HANDLER (tbowl_bgyscroll_lo)
{
	tbowl_state *state = space->machine().driver_data<tbowl_state>();
	state->m_yscroll = (state->m_yscroll & 0xff00) | data;
}

WRITE8_HANDLER (tbowl_bgyscroll_hi)
{
	tbowl_state *state = space->machine().driver_data<tbowl_state>();
	state->m_yscroll = (state->m_yscroll & 0x00ff) | (data << 8);
}

/* Middle BG Layer (bg2) Tilemaps */

static TILE_GET_INFO( get_bg2_tile_info )
{
	tbowl_state *state = machine.driver_data<tbowl_state>();
	int tileno;
	int col;

	tileno = state->m_bg2videoram[tile_index] | ((state->m_bg2videoram[tile_index+0x1000] & 0x0f) << 8);
	tileno ^= 0x400;
	col = (state->m_bg2videoram[tile_index+0x1000] & 0xf0) >> 4;

	SET_TILE_INFO(2,tileno,col,0);
}

WRITE8_HANDLER( tbowl_bgvideoram_w )
{
	tbowl_state *state = space->machine().driver_data<tbowl_state>();
	state->m_bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap,offset & 0xfff);
}

WRITE8_HANDLER (tbowl_bg2xscroll_lo)
{
	tbowl_state *state = space->machine().driver_data<tbowl_state>();
	state->m_bg2xscroll = (state->m_bg2xscroll & 0xff00) | data;
}

WRITE8_HANDLER (tbowl_bg2xscroll_hi)
{
	tbowl_state *state = space->machine().driver_data<tbowl_state>();
	state->m_bg2xscroll = (state->m_bg2xscroll & 0x00ff) | (data << 8);
}

WRITE8_HANDLER (tbowl_bg2yscroll_lo)
{
	tbowl_state *state = space->machine().driver_data<tbowl_state>();
	state->m_bg2yscroll = (state->m_bg2yscroll & 0xff00) | data;
}

WRITE8_HANDLER (tbowl_bg2yscroll_hi)
{
	tbowl_state *state = space->machine().driver_data<tbowl_state>();
	state->m_bg2yscroll = (state->m_bg2yscroll & 0x00ff) | (data << 8);
}

static void draw_sprites(running_machine &machine, bitmap_t *bitmap,const rectangle *cliprect, int xscroll)
{
	tbowl_state *state = machine.driver_data<tbowl_state>();
	int offs;
	static const UINT8 layout[8][8] =
	{
		{0,1,4,5,16,17,20,21},
		{2,3,6,7,18,19,22,23},
		{8,9,12,13,24,25,28,29},
		{10,11,14,15,26,27,30,31},
		{32,33,36,37,48,49,52,53},
		{34,35,38,39,50,51,54,55},
		{40,41,44,45,56,57,60,61},
		{42,43,46,47,58,59,62,63}
	};

	for (offs = 0;offs < 0x800;offs += 8)
	{
		if (state->m_spriteram[offs+0] & 0x80)	/* enable */
		{
			int code,color,sizex,sizey,flipx,flipy,xpos,ypos;
			int x,y;//,priority,priority_mask;

			code = (state->m_spriteram[offs+2])+(state->m_spriteram[offs+1]<<8);
			color = (state->m_spriteram[offs+3])&0x1f;
			sizex = 1 << ((state->m_spriteram[offs+0] & 0x03) >> 0);
			sizey = 1 << ((state->m_spriteram[offs+0] & 0x0c) >> 2);

			flipx = (state->m_spriteram[offs+0])&0x20;
			flipy = 0;
			xpos = (state->m_spriteram[offs+6])+((state->m_spriteram[offs+4]&0x03)<<8);
			ypos = (state->m_spriteram[offs+5])+((state->m_spriteram[offs+4]&0x10)<<4);

			/* bg: 1; fg:2; text: 4 */

			for (y = 0;y < sizey;y++)
			{
				for (x = 0;x < sizex;x++)
				{
					int sx = xpos + 8*(flipx?(sizex-1-x):x);
					int sy = ypos + 8*(flipy?(sizey-1-y):y);

					sx -= xscroll;

					drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
							code + layout[y][x],
							color,
							flipx,flipy,
							sx,sy,0 );

					/* wraparound */
					drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
							code + layout[y][x],
							color,
							flipx,flipy,
							sx,sy-0x200,0 );

					/* wraparound */
					drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
							code + layout[y][x],
							color,
							flipx,flipy,
							sx-0x400,sy,0 );

					/* wraparound */
					drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
							code + layout[y][x],
							color,
							flipx,flipy,
							sx-0x400,sy-0x200,0 );



				}
			}
		}
	}

}


/*** Video Start / Update ***/

VIDEO_START( tbowl )
{
	tbowl_state *state = machine.driver_data<tbowl_state>();
	state->m_tx_tilemap = tilemap_create(machine, get_tx_tile_info,tilemap_scan_rows, 8, 8,64,32);
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows, 16, 16,128,32);
	state->m_bg2_tilemap = tilemap_create(machine, get_bg2_tile_info,tilemap_scan_rows, 16, 16,128,32);

	tilemap_set_transparent_pen(state->m_tx_tilemap,0);
	tilemap_set_transparent_pen(state->m_bg_tilemap,0);
	tilemap_set_transparent_pen(state->m_bg2_tilemap,0);
}


SCREEN_UPDATE( tbowl )
{
	tbowl_state *state = screen->machine().driver_data<tbowl_state>();
	device_t *left_screen  = screen->machine().device("lscreen");
	device_t *right_screen = screen->machine().device("rscreen");

	if (screen == left_screen)
	{
		tilemap_set_scrollx(state->m_bg_tilemap,  0, state->m_xscroll );
		tilemap_set_scrolly(state->m_bg_tilemap,  0, state->m_yscroll );
		tilemap_set_scrollx(state->m_bg2_tilemap, 0, state->m_bg2xscroll );
		tilemap_set_scrolly(state->m_bg2_tilemap, 0, state->m_bg2yscroll );
		tilemap_set_scrollx(state->m_tx_tilemap,  0, 0 );
		tilemap_set_scrolly(state->m_tx_tilemap,  0, 0 );

		bitmap_fill(bitmap,cliprect,0x100); /* is there a register controling the colour? looks odd when screen is blank */
		tilemap_draw(bitmap,cliprect,state->m_bg_tilemap,0,0);
		draw_sprites(screen->machine(), bitmap,cliprect, 0);
		tilemap_draw(bitmap,cliprect,state->m_bg2_tilemap,0,0);
		tilemap_draw(bitmap,cliprect,state->m_tx_tilemap,0,0);
	}
	else if (screen == right_screen)
	{
		tilemap_set_scrollx(state->m_bg_tilemap,  0, state->m_xscroll+32*8 );
		tilemap_set_scrolly(state->m_bg_tilemap,  0, state->m_yscroll );
		tilemap_set_scrollx(state->m_bg2_tilemap, 0, state->m_bg2xscroll+32*8 );
		tilemap_set_scrolly(state->m_bg2_tilemap, 0, state->m_bg2yscroll );
		tilemap_set_scrollx(state->m_tx_tilemap,  0, 32*8 );
		tilemap_set_scrolly(state->m_tx_tilemap,  0, 0 );

		bitmap_fill(bitmap,cliprect,0x100); /* is there a register controling the colour? looks odd when screen is blank */
		tilemap_draw(bitmap,cliprect,state->m_bg_tilemap,0,0);
		draw_sprites(screen->machine(), bitmap,cliprect, 32*8);
		tilemap_draw(bitmap,cliprect,state->m_bg2_tilemap,0,0);
		tilemap_draw(bitmap,cliprect,state->m_tx_tilemap,0,0);
	}
	return 0;
}
