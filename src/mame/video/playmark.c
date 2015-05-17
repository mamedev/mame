// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Pierpaolo Prazzoli, Quench
#include "emu.h"
#include "includes/playmark.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(playmark_state::bigtwin_get_tx_tile_info)
{
	UINT16 code = m_videoram1[2 * tile_index];
	UINT16 color = m_videoram1[2 * tile_index + 1];
	SET_TILE_INFO_MEMBER(2, code, color, 0);
}

TILE_GET_INFO_MEMBER(playmark_state::bigtwin_get_fg_tile_info)
{
	UINT16 code = m_videoram2[2 * tile_index];
	UINT16 color = m_videoram2[2 * tile_index + 1];
	SET_TILE_INFO_MEMBER(1, code, color, 0);
}


TILE_GET_INFO_MEMBER(playmark_state::wbeachvl_get_tx_tile_info)
{
	UINT16 code = m_videoram1[2 * tile_index];
	UINT16 color = m_videoram1[2 * tile_index + 1];

	SET_TILE_INFO_MEMBER(2, code, (color >> 2), 0);
}

TILE_GET_INFO_MEMBER(playmark_state::wbeachvl_get_fg_tile_info)
{
	UINT16 code = m_videoram2[2 * tile_index];
	UINT16 color = m_videoram2[2 * tile_index + 1];

	SET_TILE_INFO_MEMBER(1, (code & 0x7fff), (color >> 2) + 8, (code & 0x8000) ? TILE_FLIPX : 0);
}

TILE_GET_INFO_MEMBER(playmark_state::wbeachvl_get_bg_tile_info)
{
	UINT16 code = m_videoram3[2 * tile_index];
	UINT16 color = m_videoram3[2 * tile_index + 1];

	SET_TILE_INFO_MEMBER(1, (code & 0x7fff), (color >> 2), (code & 0x8000) ? TILE_FLIPX : 0);
}


TILE_GET_INFO_MEMBER(playmark_state::hrdtimes_get_tx_tile_info)
{
	int code = m_videoram1[tile_index] & 0x0fff;
	int colr = m_videoram1[tile_index] & 0xe000;

	SET_TILE_INFO_MEMBER(3, code, (colr >> 13), 0);
}

TILE_GET_INFO_MEMBER(playmark_state::hrdtimes_get_fg_tile_info)
{
	int code = m_videoram2[tile_index] & 0x1fff;
	int colr = m_videoram2[tile_index] & 0xe000;

	SET_TILE_INFO_MEMBER(2, code, (colr >> 13) + 8, 0);
}

TILE_GET_INFO_MEMBER(playmark_state::hrdtimes_get_bg_tile_info)
{
	int code = m_videoram3[tile_index] & 0x1fff;
	int colr = m_videoram3[tile_index] & 0xe000;

	SET_TILE_INFO_MEMBER(1, code, (colr >> 13), 0);
}


TILE_GET_INFO_MEMBER(playmark_state::bigtwinb_get_tx_tile_info)
{
	int code = m_videoram1[tile_index] & 0x0fff;
	int colr = m_videoram1[tile_index] & 0xf000;

	SET_TILE_INFO_MEMBER(3, code, (colr >> 12), 0);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(playmark_state,bigtwin)
{
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::bigtwin_get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::bigtwin_get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tx_tilemap->set_transparent_pen(0);

	m_xoffset = 0;
	m_yoffset = 0;

	m_pri_masks[0] = 0;
	m_pri_masks[1] = 0;
	m_pri_masks[2] = 0;
}


VIDEO_START_MEMBER(playmark_state,bigtwinb)
{
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::bigtwinb_get_tx_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::hrdtimes_get_fg_tile_info),this),TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::hrdtimes_get_bg_tile_info),this),TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tx_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scrolldx(-4, -4);

	m_xoffset = 1;
	m_yoffset = 0;

	m_pri_masks[0] = 0;
	m_pri_masks[1] = 0;
	m_pri_masks[2] = 0;
}


VIDEO_START_MEMBER(playmark_state,wbeachvl)
{
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::wbeachvl_get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::wbeachvl_get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::wbeachvl_get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);

	m_tx_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

	m_xoffset = 0;
	m_yoffset = 0;

	m_pri_masks[0] = 0xfff0;
	m_pri_masks[1] = 0xfffc;
	m_pri_masks[2] = 0;
}

VIDEO_START_MEMBER(playmark_state,excelsr)
{
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::bigtwin_get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::bigtwin_get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tx_tilemap->set_transparent_pen(0);

	m_xoffset = 0;
	m_yoffset = 0;

	m_pri_masks[0] = 0;
	m_pri_masks[1] = 0xfffc;
	m_pri_masks[2] = 0xfff0;
}

VIDEO_START_MEMBER(playmark_state,hotmind)
{
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::hrdtimes_get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::hrdtimes_get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::hrdtimes_get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

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
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::hrdtimes_get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::hrdtimes_get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::hrdtimes_get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tx_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

	m_tx_tilemap->set_scrolldx( -9,  -9);
	m_fg_tilemap->set_scrolldx(-11, -11);
	m_bg_tilemap->set_scrolldx(-13, -13);

	m_xoffset = -9;
	m_yoffset = -8;

	m_pri_masks[0] = 0xfff0;
	m_pri_masks[1] = 0xfffc;
	m_pri_masks[2] = 0;
}



// hard times level 2-4 boss(truck) needs this.. or something similar.
#define TILES_PER_PAGE_Y    (0x20)
#define TILES_PER_PAGE_X    (0x20)
#define PAGES_PER_TMAP_Y    (0x1)
#define PAGES_PER_TMAP_X    (0x4)

TILEMAP_MAPPER_MEMBER(playmark_state::playmark_tilemap_scan_pages)
{
	return  (col / TILES_PER_PAGE_X) * TILES_PER_PAGE_Y * TILES_PER_PAGE_X * PAGES_PER_TMAP_Y +
			(col % TILES_PER_PAGE_X) +

			(row / TILES_PER_PAGE_Y) * TILES_PER_PAGE_Y * TILES_PER_PAGE_X +
			(row % TILES_PER_PAGE_Y) * TILES_PER_PAGE_X;
}

// theres enough ram for 64*128 on each tilemap..

VIDEO_START_MEMBER(playmark_state,hrdtimes)
{
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::hrdtimes_get_tx_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::hrdtimes_get_fg_tile_info),this),tilemap_mapper_delegate(FUNC(playmark_state::playmark_tilemap_scan_pages),this), 16, 16, 128, 32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(playmark_state::hrdtimes_get_bg_tile_info),this),TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

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

WRITE16_MEMBER(playmark_state::wbeachvl_txvideoram_w)
{
	COMBINE_DATA(&m_videoram1[offset]);
	m_tx_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(playmark_state::wbeachvl_fgvideoram_w)
{
	COMBINE_DATA(&m_videoram2[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(playmark_state::wbeachvl_bgvideoram_w)
{
	COMBINE_DATA(&m_videoram3[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(playmark_state::hrdtimes_txvideoram_w)
{
	COMBINE_DATA(&m_videoram1[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(playmark_state::hrdtimes_fgvideoram_w)
{
	COMBINE_DATA(&m_videoram2[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(playmark_state::hrdtimes_bgvideoram_w)
{
	COMBINE_DATA(&m_videoram3[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(playmark_state::bigtwin_scroll_w)
{
	data = COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0:     m_tx_tilemap->set_scrollx(0, data + 2); break;
		case 1:     m_tx_tilemap->set_scrolly(0, data);   break;
		case 2:     m_bgscrollx = -(data + 4);                    break;
		case 3:     m_bgscrolly = (-data) & 0x1ff;
				m_bg_enable = data & 0x0200;
				m_bg_full_size = data & 0x0400;
				break;
		case 4:     m_fg_tilemap->set_scrollx(0, data + 6); break;
		case 5:     m_fg_tilemap->set_scrolly(0, data);   break;
	}
}

WRITE16_MEMBER(playmark_state::wbeachvl_scroll_w)
{
	data = COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0:     m_tx_tilemap->set_scrollx(0, data + 2); break;
		case 1:     m_tx_tilemap->set_scrolly(0, data);   break;
		case 2:     m_fgscrollx = data + 4;break;
		case 3:     m_fg_tilemap->set_scrolly(0, data & 0x3ff);
				m_fg_rowscroll_enable = data & 0x0800;
				break;
		case 4:     m_bg_tilemap->set_scrollx(0, data + 6); break;
		case 5:     m_bg_tilemap->set_scrolly(0, data);   break;
	}
}

WRITE16_MEMBER(playmark_state::excelsr_scroll_w)
{
	data = COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0: m_tx_tilemap->set_scrollx(0, data + 2); break;
		case 1:     m_tx_tilemap->set_scrolly(0, data);   break;
		case 2:     m_bgscrollx = -data;    break;
		case 3:     m_bgscrolly = (-data + 2)& 0x1ff;
				m_bg_enable = data & 0x0200;
				m_bg_full_size = data & 0x0400;
				break;
		case 4: m_fg_tilemap->set_scrollx(0, data + 6); break;
		case 5: m_fg_tilemap->set_scrolly(0, data);   break;
	}
}

WRITE16_MEMBER(playmark_state::hrdtimes_scroll_w)
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

void playmark_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int codeshift )
{
	int offs, start_offset = m_spriteram.bytes() / 2 - 4;
	int height = m_gfxdecode->gfx(0)->height();
	int colordiv = m_gfxdecode->gfx(0)->granularity() / 16;
	UINT16 *spriteram = m_spriteram;

	// find the "end of list" to draw the sprites in reverse order
	for (offs = 4; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		if (spriteram[offs + 3 - 4] == 0x2000) /* end of list marker */
		{
			start_offset = offs - 4;
			break;
		}
	}

	for (offs = start_offset; offs >= 4; offs -= 4)
	{
		int sx, sy, code, color, flipx, pri;

		sy = spriteram[offs + 3 - 4];   /* -4? what the... ??? */

		flipx = sy & 0x4000;
		sx = (spriteram[offs + 1] & 0x01ff) - 16 - 7;
		sy = (256 - 8 - height - sy) & 0xff;
		code = spriteram[offs + 2] >> codeshift;
		color = ((spriteram[offs + 1] & 0x3e00) >> 9) / colordiv;
		pri = (spriteram[offs + 1] & 0x8000) >> 15;

		if(!pri && (color & 0x0c) == 0x0c)
			pri = 2;

		m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect,
					code,
					color,
					flipx,0,
					sx + m_xoffset,sy + m_yoffset,
					screen.priority(),m_pri_masks[pri],0);
	}
}


void playmark_state::bigtwinb_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int codeshift )
{
	int offs, start_offset = m_spriteram.bytes() / 2 - 4;
	int height = m_gfxdecode->gfx(0)->height();
	UINT16 *spriteram = m_spriteram;

	// find the "end of list" to draw the sprites in reverse order
	for (offs = 4; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		if (spriteram[offs + 3 - 4] == 0x2000) /* end of list marker */
		{
			start_offset = offs - 4;
			break;
		}
	}

	for (offs = start_offset; offs >= 4; offs -= 4)
	{
		int sx, sy, code, color, flipx;

		sy = spriteram[offs + 3 - 4];   /* -4? what the... ??? */

		flipx = sy & 0x4000;
		sx = (spriteram[offs + 1] & 0x01ff) - 16 - 7;
		sy = (256 - 8 - height - sy) & 0xff;
		code = spriteram[offs + 2] >> codeshift;
		color = ((spriteram[offs + 1] & 0xf000) >> 12);

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					flipx,0,
					sx + m_xoffset,sy + m_yoffset, 0);
	}
}

void playmark_state::draw_bitmap( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int x, y, count;
	int color;
	UINT8 *pri;

	count = 0;
	for (y = 0; y < 512; y++)
	{
		for (x = 0; x < 512; x++)
		{
			color = m_bgvideoram[count] & 0xff;

			if (color)
			{
				if (m_bg_full_size)
				{
					bitmap.pix16((y + m_bgscrolly) & 0x1ff, (x + m_bgscrollx) & 0x1ff) = 0x100 + color;

					pri = &screen.priority().pix8((y + m_bgscrolly) & 0x1ff);
					pri[(x + m_bgscrollx) & 0x1ff] |= 2;
				}
				else
				{
					/* 50% size */
					if(!(x % 2) && !(y % 2))
					{
						bitmap.pix16((y / 2 + m_bgscrolly) & 0x1ff, (x / 2 + m_bgscrollx) & 0x1ff) = 0x100 + color;

						pri = &screen.priority().pix8((y / 2 + m_bgscrolly) & 0x1ff);
						pri[(x / 2 + m_bgscrollx) & 0x1ff] |= 2;
					}
				}
			}

			count++;
		}
	}
}

UINT32 playmark_state::screen_update_bigtwin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	if (m_bg_enable)
		draw_bitmap(screen, bitmap, cliprect);
	draw_sprites(screen, bitmap, cliprect, 4);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


UINT32 playmark_state::screen_update_bigtwinb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// video enabled
	if (m_scroll[6] & 1)
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		bigtwinb_draw_sprites(screen, bitmap, cliprect, 4);
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
		bitmap.fill(m_palette->black_pen(), cliprect);
	return 0;
}

UINT32 playmark_state::screen_update_excelsr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 1);
	if (m_bg_enable)
		draw_bitmap(screen, bitmap, cliprect);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 4);
	draw_sprites(screen, bitmap, cliprect, 2);
	return 0;
}

UINT32 playmark_state::screen_update_wbeachvl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_fg_rowscroll_enable)
	{
		int i;

		m_fg_tilemap->set_scroll_rows(512);
		for (i = 0; i < 256; i++)
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

UINT32 playmark_state::screen_update_hrdtimes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	// video enabled
	if (m_scroll[6] & 1)
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
