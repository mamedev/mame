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

	state->m_tx_tilemap->set_transparent_pen(0);
	state->m_fg_tilemap->set_transparent_pen(0);
}



WRITE16_MEMBER(pirates_state::pirates_tx_tileram_w)
{
	COMBINE_DATA(m_tx_tileram+offset);
	m_tx_tilemap->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(pirates_state::pirates_fg_tileram_w)
{
	COMBINE_DATA(m_fg_tileram+offset);
	m_fg_tilemap->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(pirates_state::pirates_bg_tileram_w)
{
	COMBINE_DATA(m_bg_tileram+offset);
	m_bg_tilemap->mark_tile_dirty(offset/2);
}



static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

SCREEN_UPDATE_IND16(pirates)
{
	pirates_state *state = screen.machine().driver_data<pirates_state>();
	state->m_bg_tilemap->set_scrollx(0,state->m_scroll[0]);
	state->m_fg_tilemap->set_scrollx(0,state->m_scroll[0]);
	state->m_bg_tilemap->draw(bitmap, cliprect, 0,0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(),bitmap,cliprect);
	state->m_tx_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}
