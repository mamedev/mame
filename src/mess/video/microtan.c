/***************************************************************************
 *  Microtan 65
 *
 *  video hardware
 *
 *  Juergen Buchmueller <pullmoll@t-online.de>, Jul 2000
 *
 *  Thanks go to Geoff Macdonald <mail@geoff.org.uk>
 *  for his site http:://www.geo255.redhotant.com
 *  and to Fabrice Frances <frances@ensica.fr>
 *  for his site http://www.ifrance.com/oric/microtan.html
 *
 ***************************************************************************/

#include "emu.h"
#include "includes/microtan.h"




WRITE8_MEMBER(microtan_state::microtan_videoram_w)
{
	UINT8 *videoram = m_videoram;
	if ((videoram[offset] != data) || (m_chunky_buffer[offset] != m_chunky_graphics))
	{
		videoram[offset] = data;
		m_bg_tilemap->mark_tile_dirty(offset);
		m_chunky_buffer[offset] = m_chunky_graphics;
	}
}

static TILE_GET_INFO(get_bg_tile_info)
{
	microtan_state *state = machine.driver_data<microtan_state>();
	UINT8 *videoram = state->m_videoram;
	int gfxn = state->m_chunky_buffer[tile_index];
	int code = videoram[tile_index];

	SET_TILE_INFO(gfxn, code, 0, 0);
}

VIDEO_START( microtan )
{
	microtan_state *state = machine.driver_data<microtan_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		8, 16, 32, 16);

	state->m_chunky_buffer = auto_alloc_array(machine, UINT8, 0x200);
	memset(state->m_chunky_buffer, 0, 0x200);
	state->m_chunky_graphics = 0;
}

SCREEN_UPDATE_IND16( microtan )
{
	microtan_state *state = screen.machine().driver_data<microtan_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
