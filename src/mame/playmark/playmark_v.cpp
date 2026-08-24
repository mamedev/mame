// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Pierpaolo Prazzoli, Quench
#include "emu.h"
#include "playmark.h"
#include "screen.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(bigtwin_state::bigtwin_get_tx_tile_info)
{
	const u16 code = m_txtvideoram[2 * tile_index];
	const u16 color = m_txtvideoram[2 * tile_index + 1];

	tileinfo.set(2, code, color, 0);
}

TILE_GET_INFO_MEMBER(bigtwin_state::bigtwin_get_fg_tile_info)
{
	const u16 code = m_fgvideoram[2 * tile_index];
	const u16 color = m_fgvideoram[2 * tile_index + 1];

	tileinfo.set(1, code, color, 0);
}


TILE_GET_INFO_MEMBER(wbeachvl_state::get_tx_tile_info)
{
	const u16 code = m_txtvideoram[2 * tile_index];
	const u16 color = m_txtvideoram[2 * tile_index + 1];

	tileinfo.set(2, code, (color >> 2), 0);
}

TILE_GET_INFO_MEMBER(wbeachvl_state::get_fg_tile_info)
{
	const u16 code = m_fgvideoram[2 * tile_index];
	const u16 color = m_fgvideoram[2 * tile_index + 1];

	tileinfo.set(1, (code & 0x7fff), (color >> 2) + 8, BIT(code, 15) ? TILE_FLIPX : 0);
}

TILE_GET_INFO_MEMBER(wbeachvl_state::get_bg_tile_info)
{
	const u16 code = m_bgvideoram[2 * tile_index];
	const u16 color = m_bgvideoram[2 * tile_index + 1];

	tileinfo.set(1, (code & 0x7fff), (color >> 2), BIT(code, 15) ? TILE_FLIPX : 0);
}


TILE_GET_INFO_MEMBER(playmark_state::hrdtimes_get_tx_tile_info)
{
	const u16 code = m_txtvideoram[tile_index] & 0x0fff;
	const u16 colr = m_txtvideoram[tile_index] & 0xe000;

	tileinfo.set(3, code, (colr >> 13), 0);
}

TILE_GET_INFO_MEMBER(playmark_state::hrdtimes_get_fg_tile_info)
{
	const u16 code = m_fgvideoram[tile_index] & 0x1fff;
	const u16 colr = m_fgvideoram[tile_index] & 0xe000;

	tileinfo.set(2, code, (colr >> 13) + 8, 0);
}

TILE_GET_INFO_MEMBER(playmark_state::hrdtimes_get_bg_tile_info)
{
	const u16 code = m_bgvideoram[tile_index] & 0x1fff;
	const u16 colr = m_bgvideoram[tile_index] & 0xe000;

	tileinfo.set(1, code, (colr >> 13), 0);
}


TILE_GET_INFO_MEMBER(playmark_state::bigtwinb_get_tx_tile_info)
{
	const u16 code = m_txtvideoram[tile_index] & 0x0fff;
	const u16 colr = m_txtvideoram[tile_index] & 0xf000;

	tileinfo.set(3, code, (colr >> 12), 0);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void bigtwin_state::init_bitmap()
{
	m_bg_bitmap.fill(0x100);

	save_item(NAME(m_bg_bitmap));
	save_item(NAME(m_bgscrollx));
	save_item(NAME(m_bgscrolly));
	save_item(NAME(m_bg_enable));
	save_item(NAME(m_bg_full_size));
}

VIDEO_START_MEMBER(bigtwin_state,bigtwin)
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bigtwin_state::bigtwin_get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bigtwin_state::bigtwin_get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tx_tilemap->set_transparent_pen(0);

	m_xoffset = 0;
	m_yoffset = 0;

	m_pri_masks[0] = 0;
	m_pri_masks[1] = 0;
	m_pri_masks[2] = 0;

	init_bitmap();
}


VIDEO_START_MEMBER(playmark_state,bigtwinb)
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(playmark_state::bigtwinb_get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(playmark_state::hrdtimes_get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(playmark_state::hrdtimes_get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tx_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scrolldx(-4, -4);

	m_xoffset = 1;
	m_yoffset = 0;

	m_pri_masks[0] = 0;
	m_pri_masks[1] = 0;
	m_pri_masks[2] = 0;
}


void wbeachvl_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wbeachvl_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wbeachvl_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wbeachvl_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);

	m_tx_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

	m_xoffset = 0;
	m_yoffset = 0;

	m_pri_masks[0] = 0xfff0;
	m_pri_masks[1] = 0xfffc;
	m_pri_masks[2] = 0;

	save_item(NAME(m_fgscrollx));
	save_item(NAME(m_fg_rowscroll_enable));
}

VIDEO_START_MEMBER(bigtwin_state,excelsr)
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bigtwin_state::bigtwin_get_tx_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bigtwin_state::bigtwin_get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tx_tilemap->set_transparent_pen(0);

	m_xoffset = 0;
	m_yoffset = 0;

	m_pri_masks[0] = 0;
	m_pri_masks[1] = 0xfffc;
	m_pri_masks[2] = 0xfff0;

	init_bitmap();
}

VIDEO_START_MEMBER(playmark_state,hotmind)
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(playmark_state::hrdtimes_get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(playmark_state::hrdtimes_get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(playmark_state::hrdtimes_get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tx_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

	m_tx_tilemap->set_scrolldx(-14, -14);
	m_fg_tilemap->set_scrolldx(-14, -14);
	m_bg_tilemap->set_scrolldx(-14, -14);

	m_xoffset = -9;
	m_yoffset = -8;

	m_pri_masks[0] = 0xfff0;
	m_pri_masks[1] = 0xfffc;
	m_pri_masks[2] = 0;
}

VIDEO_START_MEMBER(playmark_state,luckboomh)
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(playmark_state::hrdtimes_get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(playmark_state::hrdtimes_get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(playmark_state::hrdtimes_get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tx_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

	m_tx_tilemap->set_scrolldx(-9,  -9);
	m_fg_tilemap->set_scrolldx(-11, -11);
	m_bg_tilemap->set_scrolldx(-13, -13);

	m_xoffset = -9;
	m_yoffset = -8;

	m_pri_masks[0] = 0xfff0;
	m_pri_masks[1] = 0xfffc;
	m_pri_masks[2] = 0;
}



// Hard Times level 2-4 boss(truck) needs this.. or something similar.
static constexpr int TILES_PER_PAGE_Y = 0x20;
static constexpr int TILES_PER_PAGE_X = 0x20;
static constexpr int PAGES_PER_TMAP_Y = 0x1;
static constexpr int PAGES_PER_TMAP_X = 0x4;

TILEMAP_MAPPER_MEMBER(playmark_state::hrdtimes_tilemap_scan_pages)
{
	return  (col / TILES_PER_PAGE_X) * TILES_PER_PAGE_Y * TILES_PER_PAGE_X * PAGES_PER_TMAP_Y +
			(col % TILES_PER_PAGE_X) +

			(row / TILES_PER_PAGE_Y) * TILES_PER_PAGE_Y * TILES_PER_PAGE_X +
			(row % TILES_PER_PAGE_Y) * TILES_PER_PAGE_X;
}

// there's enough ram for 64*128 on each tilemap..

VIDEO_START_MEMBER(playmark_state,hrdtimes)
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(playmark_state::hrdtimes_get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(playmark_state::hrdtimes_get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(playmark_state::hrdtimes_tilemap_scan_pages)), 16, 16, 128, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(playmark_state::hrdtimes_get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tx_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

	m_tx_tilemap->set_scrolldx(-14, -14);
	m_fg_tilemap->set_scrolldx(-10, -10);
	m_bg_tilemap->set_scrolldx(-12, -12);

	m_xoffset = -8;
	m_yoffset = -8;

	m_pri_masks[0] = 0xfff0;
	m_pri_masks[1] = 0xfffc;
	m_pri_masks[2] = 0;
}

/***************************************************************************

  Memory handlers

***************************************************************************/

void bigtwin_state::bitmap_w(offs_t offset, u16 data, u16 mem_mask)
{
	data = COMBINE_DATA(&m_bgvideoram[offset]);
	const u32 x = offset & 0x1ff;
	const u32 y = (offset >> 9) & 0x1ff;
	m_bg_bitmap.pix(y, x) = 0x100 | (data & 0xff);
}

void bigtwin_state::bigtwin_scroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	data = COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0: m_tx_tilemap->set_scrollx(0, data + 2); break;
		case 1: m_tx_tilemap->set_scrolly(0, data); break;
		case 2: m_bgscrollx = -(data + 4); break;
		case 3: m_bgscrolly = (-data) & 0x1ff;
				m_bg_enable = BIT(data, 9);
				m_bg_full_size = BIT(data, 10);
				break;
		case 4: m_fg_tilemap->set_scrollx(0, data + 6); break;
		case 5: m_fg_tilemap->set_scrolly(0, data); break;
	}
}

void wbeachvl_state::scroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	data = COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0: m_tx_tilemap->set_scrollx(0, data + 2); break;
		case 1: m_tx_tilemap->set_scrolly(0, data); break;
		case 2: m_fgscrollx = data + 4; break;
		case 3: m_fg_tilemap->set_scrolly(0, data & 0x3ff);
				m_fg_rowscroll_enable = BIT(data, 11);
				break;
		case 4: m_bg_tilemap->set_scrollx(0, data + 6); break;
		case 5: m_bg_tilemap->set_scrolly(0, data); break;
	}
}

void bigtwin_state::excelsr_scroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	data = COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0: m_tx_tilemap->set_scrollx(0, data + 2); break;
		case 1: m_tx_tilemap->set_scrolly(0, data); break;
		case 2: m_bgscrollx = -data; break;
		case 3: m_bgscrolly = (-data + 2) & 0x1ff;
				m_bg_enable = BIT(data, 9);
				m_bg_full_size = BIT(data, 10);
				break;
		case 4: m_fg_tilemap->set_scrollx(0, data + 6); break;
		case 5: m_fg_tilemap->set_scrolly(0, data);   break;
	}
}

void playmark_state::hrdtimes_scroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	data = COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0: m_tx_tilemap->set_scrollx(0, data); break;
		case 1: m_tx_tilemap->set_scrolly(0, data); break;
		case 2: m_fg_tilemap->set_scrollx(0, data); break;
		case 3: m_fg_tilemap->set_scrolly(0, data); break;
		case 4: m_bg_tilemap->set_scrollx(0, data); break;
		case 5: m_bg_tilemap->set_scrolly(0, data); break;
	}
}

/***************************************************************************

  Display refresh

***************************************************************************/

void playmark_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int codeshift)
{
	int start_offset = m_spriteram.bytes() / 2 - 4;
	const int height = m_gfxdecode->gfx(0)->height();
	const int colordiv = m_gfxdecode->gfx(0)->granularity() / 16;

	// find the "end of list" to draw the sprites in reverse order
	for (int offs = 4; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		if (m_spriteram[offs + 3 - 4] == 0x2000) // end of list marker
		{
			start_offset = offs - 4;
			break;
		}
	}

	for (int offs = start_offset; offs >= 4; offs -= 4)
	{
		int sy = m_spriteram[offs + 3 - 4];   // -4? what the... ???

		const bool flipx = BIT(sy, 14);
		const int sx = (m_spriteram[offs + 1] & 0x01ff) - 16 - 7;
		sy = (256 - 8 - height - sy) & 0xff;
		const int code = m_spriteram[offs + 2] >> codeshift;
		const int color = ((m_spriteram[offs + 1] & 0x3e00) >> 9) / colordiv;
		int pri = (m_spriteram[offs + 1] & 0x8000) >> 15;

		if (!pri && (color & 0x0c) == 0x0c)
			pri = 2;

		m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect,
					code,
					color,
					flipx, 0,
					sx + m_xoffset, sy + m_yoffset,
					screen.priority(), m_pri_masks[pri], m_sprtranspen);
	}
}


void playmark_state::bigtwinb_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int start_offset = m_spriteram.bytes() / 2 - 4;
	const int height = m_gfxdecode->gfx(0)->height();

	// find the "end of list" to draw the sprites in reverse order
	for (int offs = 4; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		if (m_spriteram[offs + 3 - 4] == 0x2000) // end of list marker
		{
			start_offset = offs - 4;
			break;
		}
	}

	for (int offs = start_offset; offs >= 4; offs -= 4)
	{
		int sy = m_spriteram[offs + 3 - 4];   // -4? what the... ???

		const bool flipx = BIT(sy, 14);
		const int sx = (m_spriteram[offs + 1] & 0x01ff) - 16 - 7;
		sy = (256 - 8 - height - sy) & 0xff;
		const int code = m_spriteram[offs + 2] >> 4;
		const int color = ((m_spriteram[offs + 1] & 0xf000) >> 12);

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					flipx, 0,
					sx + m_xoffset, sy + m_yoffset, m_sprtranspen);
	}
}

void bigtwin_state::draw_bitmap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u8 shift = m_bg_full_size ? 0 : 1;
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		const u32 srcy = ((y - m_bgscrolly) & 0x1ff) << shift;
		if (srcy >= m_bg_bitmap.height())
			continue;

		u16 const *const srcline = &m_bg_bitmap.pix(srcy);
		u16 *const dstline = &bitmap.pix(y);
		u8 *const priline = &screen.priority().pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			const u32 srcx = ((x - m_bgscrollx) & 0x1ff) << shift;
			if (srcx >= m_bg_bitmap.width())
				continue;

			const u16 pix = srcline[srcx];
			if ((pix & 0xff) != 0)
			{
				dstline[x] = pix;
				priline[x] |= 2;
			}
		}
	}
}

u32 bigtwin_state::screen_update_bigtwin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	if (m_bg_enable)
		draw_bitmap(screen, bitmap, cliprect);
	draw_sprites(screen, bitmap, cliprect, 4);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


u32 playmark_state::screen_update_bigtwinb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// video enabled
	if (BIT(m_scroll[6], 0))
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		bigtwinb_draw_sprites(screen, bitmap, cliprect);
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
		bitmap.fill(m_palette->black_pen(), cliprect);
	return 0;
}

u32 bigtwin_state::screen_update_excelsr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 1);
	if (m_bg_enable)
		draw_bitmap(screen, bitmap, cliprect);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 4);
	draw_sprites(screen, bitmap, cliprect, 2);
	return 0;
}

u32 wbeachvl_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_fg_rowscroll_enable)
	{
		m_fg_tilemap->set_scroll_rows(512);
		for (int i = 0; i < 256; i++)
			m_fg_tilemap->set_scrollx(i + 1, m_rowscroll[8 * i]);
	}
	else
	{
		m_fg_tilemap->set_scroll_rows(1);
		m_fg_tilemap->set_scrollx(0, m_fgscrollx);
	}

	screen.priority().fill(0, cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 1);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 2);
	draw_sprites(screen, bitmap, cliprect, 0);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

u32 playmark_state::screen_update_hrdtimes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	// video enabled
	if (BIT(m_scroll[6], 0))
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 1);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 2);
		draw_sprites(screen, bitmap, cliprect, 2);
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
		bitmap.fill(m_palette->black_pen(), cliprect);
	return 0;
}
