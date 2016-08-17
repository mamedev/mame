// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/shisen.h"

WRITE8_MEMBER(shisen_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(shisen_state::bankswitch_w)
{
	if (data & 0xc0) logerror("bank switch %02x\n",data);

	/* bits 0-2 select ROM bank */
	membank("bank1")->set_entry(data & 0x07);

	/* bits 3-5 select gfx bank */
	int bank = (data & 0x38) >> 3;

	if (m_gfxbank != bank)
	{
		m_gfxbank = bank;
		machine().tilemap().mark_all_dirty();
	}

	/* bits 6-7 unknown */
}

WRITE8_MEMBER(shisen_state::paletteram_w)
{
	m_paletteram[offset] = data;

	offset &= 0xff;

	m_palette->set_pen_color(offset, pal5bit(m_paletteram[offset + 0x000]), pal5bit(m_paletteram[offset + 0x100]), pal5bit(m_paletteram[offset + 0x200]));
}

TILE_GET_INFO_MEMBER(shisen_state::get_bg_tile_info)
{
	int offs = tile_index * 2;
	int code = m_videoram[offs] + ((m_videoram[offs + 1] & 0x0f) << 8) + (m_gfxbank << 12);
	int color = (m_videoram[offs + 1] & 0xf0) >> 4;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void shisen_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(shisen_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 64, 32);

	membank("bank1")->configure_entries(0, 8, memregion("maincpu")->base() + 0x10000, 0x4000);

	save_item(NAME(m_gfxbank));
}

UINT32 shisen_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// on Irem boards, screen flip is handled in both hardware and software.
	// this game doesn't have cocktail mode so if there's software control we don't
	// know where it is mapped.
	flip_screen_set(~ioport("DSW2")->read() & 1);


	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
