// license:BSD-3-Clause
// copyright-holders:Lee Taylor, Couriersud
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

  (c) 12/2/1998 Lee Taylor

  2006 - major rewrite by couriersud

***************************************************************************/

#include "emu.h"
#include "includes/m10.h"

static const UINT32 extyoffs[] =
{
	STEP256(0, 8)
};


static const gfx_layout backlayout =
{
	8,8*32, /* 8*(8*32) characters */
	4,      /* 4 characters */
	1,      /* 1 bit per pixel */
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	EXTENDED_YOFFS,
	32*8*8, /* every char takes 8 consecutive bytes */
	nullptr, extyoffs
};

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	256,    /* 256 characters */
	1,      /* 1 bit per pixel */
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

TILEMAP_MAPPER_MEMBER(m10_state::tilemap_scan)
{
	return (31 - col) * 32 + row;
}


TILE_GET_INFO_MEMBER(m10_state::get_tile_info)
{
	SET_TILE_INFO_MEMBER(0, m_videoram[tile_index], m_colorram[tile_index] & 0x07, 0);
}


WRITE8_MEMBER(m10_state::m10_colorram_w)
{
	if (m_colorram[offset] != data)
	{
		m_tx_tilemap->mark_tile_dirty(offset);
		m_colorram[offset] = data;
	}
}


WRITE8_MEMBER(m10_state::m10_chargen_w)
{
	if (m_chargen[offset] != data)
	{
		m_chargen[offset] = data;
		m_back_gfx->mark_dirty(offset >> (3 + 5));
	}
}


WRITE8_MEMBER(m10_state::m15_chargen_w)
{
	if (m_chargen[offset] != data)
	{
		m_chargen[offset] = data;
		m_gfxdecode->gfx(0)->mark_dirty(offset >> 3);
	}
}


inline void m10_state::plot_pixel_m10( bitmap_ind16 &bm, int x, int y, int col )
{
	if (!m_flip)
		bm.pix16(y, x) = col;
	else
		bm.pix16((IREMM10_VBSTART - 1) - (y - IREMM10_VBEND),
				(IREMM10_HBSTART - 1) - (x - IREMM10_HBEND)) = col; // only when flip_screen(?)
}

VIDEO_START_MEMBER(m10_state,m10)
{
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m10_state::get_tile_info),this), tilemap_mapper_delegate(FUNC(m10_state::tilemap_scan),this), 8, 8, 32, 32);
	m_tx_tilemap->set_transparent_pen(0);

	m_gfxdecode->set_gfx(1, std::make_unique<gfx_element>(m_palette, backlayout, m_chargen, 0, 8, 0));
	m_back_gfx = m_gfxdecode->gfx(1);
	return ;
}

VIDEO_START_MEMBER(m10_state,m15)
{
	m_gfxdecode->set_gfx(0,std::make_unique<gfx_element>(m_palette, charlayout, m_chargen, 0, 8, 0));

	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m10_state::get_tile_info),this),tilemap_mapper_delegate(FUNC(m10_state::tilemap_scan),this), 8, 8, 32, 32);

	return ;
}

/***************************************************************************

  Draw the game screen in the given bitmap_ind16.

***************************************************************************/

UINT32 m10_state::screen_update_m10(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;
	static const int color[4]= { 3, 3, 5, 5 };
	static const int xpos[4] = { 4*8, 26*8, 7*8, 6*8};
	int i;

	bitmap.fill(0, cliprect);

	for (i = 0; i < 4; i++)
		if (m_flip)
				m_back_gfx->opaque(bitmap,cliprect, i, color[i], 1, 1, 31 * 8 - xpos[i], 0);
		else
				m_back_gfx->opaque(bitmap,cliprect, i, color[i], 0, 0, xpos[i], 0);

	if (m_bottomline)
	{
		int y;

		for (y = IREMM10_VBEND; y < IREMM10_VBSTART; y++)
			plot_pixel_m10(bitmap, 16, y, 1);
	}

	for (offs = m_videoram.bytes() - 1; offs >= 0; offs--)
		m_tx_tilemap->mark_tile_dirty(offs);

	m_tx_tilemap->set_flip(m_flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


/***************************************************************************

  Draw the game screen in the given bitmap_ind16.

***************************************************************************/

UINT32 m10_state::screen_update_m15(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	for (offs = m_videoram.bytes() - 1; offs >= 0; offs--)
		m_tx_tilemap->mark_tile_dirty(offs);

	//m_tx_tilemap->mark_all_dirty();
	m_tx_tilemap->set_flip(m_flip ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
