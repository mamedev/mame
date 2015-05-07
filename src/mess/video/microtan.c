// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
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

TILE_GET_INFO_MEMBER(microtan_state::get_bg_tile_info)
{
	UINT8 *videoram = m_videoram;
	int gfxn = m_chunky_buffer[tile_index];
	int code = videoram[tile_index];

	SET_TILE_INFO_MEMBER(gfxn, code, 0, 0);
}

void microtan_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(microtan_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,
		8, 16, 32, 16);

	m_chunky_buffer = auto_alloc_array(machine(), UINT8, 0x200);
	memset(m_chunky_buffer, 0, 0x200);
	m_chunky_graphics = 0;
}

UINT32 microtan_state::screen_update_microtan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
