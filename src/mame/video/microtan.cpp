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
 *  for his site http://www.geoff.org.uk/microtan/index.htm
 *  and to Fabrice Frances <frances@ensica.fr>
 *  for his site http://oric.free.fr/microtan.html
 *
 ***************************************************************************/

#include "emu.h"
#include "includes/microtan.h"


WRITE8_MEMBER(microtan_state::videoram_w)
{
	if ((m_videoram[offset] != data) || (m_chunky_buffer[offset] != m_chunky_graphics))
	{
		m_videoram[offset] = data;
		m_bg_tilemap->mark_tile_dirty(offset);
		m_chunky_buffer[offset] = m_chunky_graphics;
	}
}

TILE_GET_INFO_MEMBER(microtan_state::get_bg_tile_info)
{
	int gfxn = m_chunky_buffer[tile_index];
	int code = m_videoram[tile_index];

	SET_TILE_INFO_MEMBER(gfxn, code, 0, 0);
}

void microtan_state::video_start()
{
	/* randomize video memory contents */
	for (uint16_t addr = 0; addr < m_videoram.bytes(); addr++)
		m_videoram[addr] = machine().rand() & 0xff;

	m_bg_tilemap = &machine().tilemap().create(*
			m_gfxdecode,
			tilemap_get_info_delegate(FUNC(microtan_state::get_bg_tile_info),this),
			TILEMAP_SCAN_ROWS, 8, 16, 32, 16);

	m_chunky_buffer = std::make_unique<uint8_t[]>(0x200);
	memset(m_chunky_buffer.get(), 0, 0x200);
	m_chunky_graphics = 0;

	save_pointer(NAME(m_chunky_buffer), 0x200);
	save_item(NAME(m_chunky_graphics));
}

uint32_t microtan_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void mt6809_state::video_start()
{
}
