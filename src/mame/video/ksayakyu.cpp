// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
#include "emu.h"
#include "includes/ksayakyu.h"


WRITE8_MEMBER(ksayakyu_state::ksayakyu_videoram_w)
{
	m_videoram[offset]=data;
	m_textmap->mark_tile_dirty(offset >> 1);
}

WRITE8_MEMBER(ksayakyu_state::ksayakyu_videoctrl_w)
{
	/*
	    bits:
	    76543210
	          xx - ?? layers enable ?
	         x   - screen flip
	       xx    - ??
	    xxx      - scroll offset

	 */
	m_video_ctrl = data;

	m_flipscreen = data & 4;
	flip_screen_set(m_flipscreen);
	m_tilemap->set_scrolly(0, (data & 0xe0) << 3);
	if(m_flipscreen)
		m_tilemap->set_flip((data & 2) ? TILEMAP_FLIPY : TILEMAP_FLIPX | TILEMAP_FLIPY);
	else
		m_tilemap->set_flip((data & 2) ? TILEMAP_FLIPX : 0);
}

PALETTE_INIT_MEMBER(ksayakyu_state, ksayakyu)
{
	const UINT8 *prom = memregion("proms")->base();
	int r, g, b, i;

	for (i = 0; i < 0x100; i++)
	{
		r = (prom[i] & 0x07) >> 0;
		g = (prom[i] & 0x38) >> 3;
		b = (prom[i] & 0xc0) >> 6;

		palette.set_pen_color(i, pal3bit(r), pal3bit(g), pal2bit(b));
	}
}

TILE_GET_INFO_MEMBER(ksayakyu_state::get_ksayakyu_tile_info)
{
	int code = memregion("user1")->base()[tile_index];
	int attr = memregion("user1")->base()[tile_index + 0x2000];
	code += (attr & 3) << 8;
	SET_TILE_INFO_MEMBER(1, code, ((attr >> 2) & 0x0f) * 2, (attr & 0x80) ? TILE_FLIPX : 0);
}

/*
xy-- ---- flip bits
--cc cc-- color
---- --bb bank select
*/
TILE_GET_INFO_MEMBER(ksayakyu_state::get_text_tile_info)
{
	int code = m_videoram[tile_index * 2 + 1];
	int attr = m_videoram[tile_index * 2];
	int flags = ((attr & 0x80) ? TILE_FLIPX : 0) | ((attr & 0x40) ? TILE_FLIPY : 0);
	int color = (attr & 0x3c) >> 2;

	code |= (attr & 3) << 8;

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

/*
[0] X--- ---- flip x
    -ttt tttt tile number
[1] yyyy yyyy Y offset
[2] xxxx xxxx X offset
[3]
*/

void ksayakyu_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	const UINT8 *source = m_spriteram + m_spriteram.bytes() - 4;
	const UINT8 *finish = m_spriteram;

	while (source>=finish) /* is order correct ? */
	{
		int sx = source[2];
		int sy = 240 - source[1];
		int attributes = source[3];
		int tile = source[0];
		int flipx = (tile & 0x80) ? 1 : 0;
		int flipy = 0;

		gfx_element *gfx = m_gfxdecode->gfx(2);

		if (m_flipscreen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx ^= 1;
			flipy ^= 1;
		}

			gfx->transpen(bitmap,cliprect,
				tile & 0x7f,
				(attributes & 0x78) >> 3,
				flipx,flipy,
				sx,sy,0 );

		source -= 4;
	}
}

void ksayakyu_state::video_start()
{
	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ksayakyu_state::get_ksayakyu_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32 * 8);
	m_textmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ksayakyu_state::get_text_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_textmap->set_transparent_pen(0);
}

UINT32 ksayakyu_state::screen_update_ksayakyu(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	if (m_video_ctrl & 1)
		m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	m_textmap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
