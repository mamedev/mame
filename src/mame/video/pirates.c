#include "emu.h"
#include "includes/pirates.h"


/* Video Hardware */

/* tilemaps */

static TILE_GET_INFO( get_tx_tile_info )
{
	pirates_state *state = machine.driver_data<pirates_state>();
	int code = state->m_tx_tileram[tile_index*2];
	int colr = state->m_tx_tileram[tile_index*2+1];

	SET_TILE_INFO(0,code,colr,0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	pirates_state *state = machine.driver_data<pirates_state>();
	int code = state->m_fg_tileram[tile_index*2];
	int colr = state->m_fg_tileram[tile_index*2+1]+0x80;

	SET_TILE_INFO(0,code,colr,0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	pirates_state *state = machine.driver_data<pirates_state>();
	int code = state->m_bg_tileram[tile_index*2];
	int colr = state->m_bg_tileram[tile_index*2+1]+ 0x100;

	SET_TILE_INFO(0,code,colr,0);
}


/* video start / update */

VIDEO_START(pirates)
{
	pirates_state *state = machine.driver_data<pirates_state>();
	state->m_tx_tilemap = tilemap_create(machine, get_tx_tile_info,tilemap_scan_cols,8,8,36,32);

	/* Not sure how big they can be, Pirates uses only 32 columns, Genix 44 */
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_cols,8,8,64,32);
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_cols,     8,8,64,32);

	tilemap_set_transparent_pen(state->m_tx_tilemap,0);
	tilemap_set_transparent_pen(state->m_fg_tilemap,0);
}



WRITE16_HANDLER( pirates_tx_tileram_w )
{
	pirates_state *state = space->machine().driver_data<pirates_state>();
	COMBINE_DATA(state->m_tx_tileram+offset);
	tilemap_mark_tile_dirty(state->m_tx_tilemap,offset/2);
}

WRITE16_HANDLER( pirates_fg_tileram_w )
{
	pirates_state *state = space->machine().driver_data<pirates_state>();
	COMBINE_DATA(state->m_fg_tileram+offset);
	tilemap_mark_tile_dirty(state->m_fg_tilemap,offset/2);
}

WRITE16_HANDLER( pirates_bg_tileram_w )
{
	pirates_state *state = space->machine().driver_data<pirates_state>();
	COMBINE_DATA(state->m_bg_tileram+offset);
	tilemap_mark_tile_dirty(state->m_bg_tilemap,offset/2);
}



static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	pirates_state *state = machine.driver_data<pirates_state>();
	const gfx_element *gfx = machine.gfx[1];
	UINT16 *source = state->m_spriteram + 4;
	UINT16 *finish = source + 0x800/2-4;

	while( source<finish )
	{
		int xpos, ypos, flipx, flipy, code, color;

		xpos = source[1] - 32;
		ypos = source[-1];	// indeed...

		if (ypos & 0x8000) break;	/* end-of-list marker */

		code = source[2] >> 2;
		color = source[0] & 0xff;
		flipx = source[2] & 2;
		flipy = source[2] & 1;

		ypos = 0xf2 - ypos;

		drawgfx_transpen(bitmap,cliprect,gfx,
				code,
				color,
				flipx,flipy,
				xpos,ypos,0);

		source+=4;
	}
}

SCREEN_UPDATE(pirates)
{
	pirates_state *state = screen->machine().driver_data<pirates_state>();
	tilemap_set_scrollx(state->m_bg_tilemap,0,state->m_scroll[0]);
	tilemap_set_scrollx(state->m_fg_tilemap,0,state->m_scroll[0]);
	tilemap_draw(bitmap,cliprect,state->m_bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,state->m_fg_tilemap,0,0);
	draw_sprites(screen->machine(),bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,state->m_tx_tilemap,0,0);
	return 0;
}
