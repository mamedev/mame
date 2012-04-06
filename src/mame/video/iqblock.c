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

	state->m_bg_tilemap->set_transparent_pen(0);
	state->m_fg_tilemap->set_scroll_cols(64);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(iqblock_state::iqblock_fgvideoram_w)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(iqblock_state::iqblock_bgvideoram_w)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

READ8_MEMBER(iqblock_state::iqblock_bgvideoram_r)
{
	return m_bgvideoram[offset];
}

WRITE8_MEMBER(iqblock_state::iqblock_fgscroll_w)
{
	m_fg_tilemap->set_scrolly(offset,data);
}



/***************************************************************************

  Display refresh

***************************************************************************/

SCREEN_UPDATE_IND16( iqblock )
{
	iqblock_state *state = screen.machine().driver_data<iqblock_state>();
	if (!state->m_videoenable) return 0;
	state->m_fg_tilemap->draw(bitmap, cliprect, 0,0);
	state->m_bg_tilemap->draw(bitmap, cliprect, 0,0);

	return 0;
}

