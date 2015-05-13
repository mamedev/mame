// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

  aquarius.c

  Functions to emulate the video hardware of the aquarius.

***************************************************************************/

#include "emu.h"
#include "includes/aquarius.h"


static const unsigned short aquarius_palette[] =
{
	0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0,10, 0,11, 0,12, 0,13, 0,14, 0,15, 0,
	0, 1, 1, 1, 2, 1, 3, 1, 4, 1, 5, 1, 6, 1, 7, 1, 8, 1, 9, 1,10, 1,11, 1,12, 1,13, 1,14, 1,15, 1,
	0, 2, 1, 2, 2, 2, 3, 2, 4, 2, 5, 2, 6, 2, 7, 2, 8, 2, 9, 2,10, 2,11, 2,12, 2,13, 2,14, 2,15, 2,
	0, 3, 1, 3, 2, 3, 3, 3, 4, 3, 5, 3, 6, 3, 7, 3, 8, 3, 9, 3,10, 3,11, 3,12, 3,13, 3,14, 3,15, 3,
	0, 4, 1, 4, 2, 4, 3, 4, 4, 4, 5, 4, 6, 4, 7, 4, 8, 4, 9, 4,10, 4,11, 4,12, 4,13, 4,14, 4,15, 4,
	0, 5, 1, 5, 2, 5, 3, 5, 4, 5, 5, 5, 6, 5, 7, 5, 8, 5, 9, 5,10, 5,11, 5,12, 5,13, 5,14, 5,15, 5,
	0, 6, 1, 6, 2, 6, 3, 6, 4, 6, 5, 6, 6, 6, 7, 6, 8, 6, 9, 6,10, 6,11, 6,12, 6,13, 6,14, 6,15, 6,
	0, 7, 1, 7, 2, 7, 3, 7, 4, 7, 5, 7, 6, 7, 7, 7, 8, 7, 9, 7,10, 7,11, 7,12, 7,13, 7,14, 7,15, 7,
	0, 8, 1, 8, 2, 8, 3, 8, 4, 8, 5, 8, 6, 8, 7, 8, 8, 8, 9, 8,10, 8,11, 8,12, 8,13, 8,14, 8,15, 8,
	0, 9, 1, 9, 2, 9, 3, 9, 4, 9, 5, 9, 6, 9, 7, 9, 8, 9, 9, 9,10, 9,11, 9,12, 9,13, 9,14, 9,15, 9,
	0,10, 1,10, 2,10, 3,10, 4,10, 5,10, 6,10, 7,10, 8,10, 9,10,10,10,11,10,12,10,13,10,14,10,15,10,
	0,11, 1,11, 2,11, 3,11, 4,11, 5,11, 6,11, 7,11, 8,11, 9,11,10,11,11,11,12,11,13,11,14,11,15,11,
	0,12, 1,12, 2,12, 3,12, 4,12, 5,12, 6,12, 7,12, 8,12, 9,12,10,12,11,12,12,12,13,12,14,12,15,12,
	0,13, 1,13, 2,13, 3,13, 4,13, 5,13, 6,13, 7,13, 8,13, 9,13,10,13,11,13,12,13,13,13,14,13,15,13,
	0,14, 1,14, 2,14, 3,14, 4,14, 5,14, 6,14, 7,14, 8,14, 9,14,10,14,11,14,12,14,13,14,14,14,15,14,
	0,15, 1,15, 2,15, 3,15, 4,15, 5,15, 6,15, 7,15, 8,15, 9,15,10,15,11,15,12,15,13,15,14,15,15,15,
};

PALETTE_INIT_MEMBER(aquarius_state, aquarius)
{
	int i;

	for (i = 0; i < 16; i++)
		m_palette->set_indirect_color(i, m_tea1002->color(i));

	for (i = 0; i < 512; i++)
		m_palette->set_pen_indirect(i, aquarius_palette[i]);
}

WRITE8_MEMBER(aquarius_state::aquarius_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(aquarius_state::aquarius_colorram_w)
{
	m_colorram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(aquarius_state::aquarius_gettileinfo)
{
	UINT8 *videoram = m_videoram;
	int bank = 0;
	int code = videoram[tile_index];
	int color = m_colorram[tile_index];
	int flags = 0;

	SET_TILE_INFO_MEMBER(bank, code, color, flags);
}

void aquarius_state::video_start()
{
	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(aquarius_state::aquarius_gettileinfo),this), TILEMAP_SCAN_ROWS, 8, 8, 40, 25);
}

UINT32 aquarius_state::screen_update_aquarius(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
