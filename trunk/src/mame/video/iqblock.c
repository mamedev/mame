#include "emu.h"
#include "includes/iqblock.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	iqblock_state *state = machine.driver_data<iqblock_state>();
	int code = state->m_bgvideoram[tile_index] + (state->m_bgvideoram[tile_index + 0x800] << 8);
	SET_TILE_INFO(
			0,
			code &(state->m_video_type ? 0x1fff : 0x3fff),
			state->m_video_type? (2*(code >> 13)+1) : (4*(code >> 14)+3),
			0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	iqblock_state *state = machine.driver_data<iqblock_state>();
	int code = state->m_fgvideoram[tile_index];
	SET_TILE_INFO(
			1,
			code & 0x7f,
			(code & 0x80) ? 3 : 0,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( iqblock )
{
	iqblock_state *state = machine.driver_data<iqblock_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows,     8, 8,64,32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,8,32,64, 8);

	tilemap_set_transparent_pen(state->m_bg_tilemap,0);
	tilemap_set_scroll_cols(state->m_fg_tilemap,64);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( iqblock_fgvideoram_w )
{
	iqblock_state *state = space->machine().driver_data<iqblock_state>();
	state->m_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_fg_tilemap,offset);
}

WRITE8_HANDLER( iqblock_bgvideoram_w )
{
	iqblock_state *state = space->machine().driver_data<iqblock_state>();
	state->m_bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap,offset & 0x7ff);
}

READ8_HANDLER( iqblock_bgvideoram_r )
{
	iqblock_state *state = space->machine().driver_data<iqblock_state>();
	return state->m_bgvideoram[offset];
}

WRITE8_HANDLER( iqblock_fgscroll_w )
{
	iqblock_state *state = space->machine().driver_data<iqblock_state>();
	tilemap_set_scrolly(state->m_fg_tilemap,offset,data);
}



/***************************************************************************

  Display refresh

***************************************************************************/

SCREEN_UPDATE( iqblock )
{
	iqblock_state *state = screen->machine().driver_data<iqblock_state>();
	if (!state->m_videoenable) return 0;
	tilemap_draw(bitmap,cliprect,state->m_fg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,state->m_bg_tilemap,0,0);

	return 0;
}

