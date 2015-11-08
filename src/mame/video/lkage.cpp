// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

    video/lkage.c


    lkage_scroll[0x00]: text layer horizontal scroll
    lkage_scroll[0x01]: text layer vertical scroll
    lkage_scroll[0x02]: foreground layer horizontal scroll
    lkage_scroll[0x03]: foreground layer vertical scroll
    lkage_scroll[0x04]: background layer horizontal scroll
    lkage_scroll[0x05]: background layer vertical scroll

    lkage_vreg[0]: 0x00,0x04
        0x02: tx tile bank select (bygone only?)
        0x04: fg tile bank select
        0x08: ?

    lkage_vreg[1]: 0x7d
        0xf0: background/foreground palette select
        0x08: bg tile bank select
        0x07: priority config?

    lkage_vreg[2]: 0xf3
        0x03: flip screen x/y
        0xf0: normally 1111, but 1001 and 0001 inbetween stages (while the
        backgrounds are are being redrawn). These bits are probably used to enable
        individual layers, but we have no way of knowing the mapping.

    lkage_vreg:
        04 7d f3 : title screen 101
        0c 7d f3 : high score   101
        04 06 f3 : attract#1    110
        04 1e f3 : attract#2    110
        04 1e f3 : attract#3    110
        00 4e f3 : attract#4    110


***************************************************************************/

#include "emu.h"
#include "includes/lkage.h"


WRITE8_MEMBER(lkage_state::lkage_videoram_w)
{
	m_videoram[offset] = data;

	switch (offset / 0x400)
	{
	case 0:
		m_tx_tilemap->mark_tile_dirty(offset & 0x3ff);
		break;

	case 1:
		m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
		break;

	case 2:
		m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
		break;

	default:
		break;
	}
}

TILE_GET_INFO_MEMBER(lkage_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index + 0x800] + 256 * (m_bg_tile_bank ? 5 : 1);
	SET_TILE_INFO_MEMBER(0/*gfx*/, code, 0/*color*/, 0/*flags*/ );
}

TILE_GET_INFO_MEMBER(lkage_state::get_fg_tile_info)
{
	int code = m_videoram[tile_index + 0x400] + 256 * (m_fg_tile_bank ? 1 : 0);
	SET_TILE_INFO_MEMBER(0/*gfx*/, code, 0/*color*/, 0/*flags*/);
}

TILE_GET_INFO_MEMBER(lkage_state::get_tx_tile_info)
{
	int code = m_videoram[tile_index] + 256 * (m_tx_tile_bank ? 4 : 0);
	SET_TILE_INFO_MEMBER(0/*gfx*/, code, 0/*color*/, 0/*flags*/);
}

void lkage_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(lkage_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(lkage_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(lkage_state::get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scrolldx(-5, -5 + 24);
	m_fg_tilemap->set_scrolldx(-3, -3 + 24);
	m_tx_tilemap->set_scrolldx(-1, -1 + 24);
}


void lkage_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	const UINT8 *source = m_spriteram;
	const UINT8 *finish = source + 0x60;

	while (source < finish)
	{
		int attributes = source[2];
		/* 0x01: horizontal flip
		 * 0x02: vertical flip
		 * 0x04: bank select
		 * 0x08: sprite size
		 * 0x70: color
		 * 0x80: priority
		 */
		int priority_mask = 0;
		int color = (attributes >> 4) & 7;
		int flipx = attributes & 0x01;
		int flipy = attributes & 0x02;
		int height = (attributes & 0x08) ? 2 : 1;
		int sx = source[0] - 15 + m_sprite_dx;
		int sy = 256 - 16 * height - source[1];
		int sprite_number = source[3] + ((attributes & 0x04) << 6);
		int y;

		if (attributes & 0x80)
		{
			priority_mask = (0xf0 | 0xcc);
		}
		else
		{
			priority_mask = 0xf0;
		}

		if (flip_screen_x())
		{
			sx = 239 - sx - 24;
			flipx = !flipx;
		}
		if (flip_screen_y())
		{
			sy = 254 - 16 * height - sy;
			flipy = !flipy;
		}
		if (height == 2 && !flipy)
		{
			sprite_number ^= 1;
		}

		for (y = 0; y < height; y++)
		{
			m_gfxdecode->gfx(1)->prio_transpen(
				bitmap,
				cliprect,
				sprite_number ^ y,
				color,
				flipx,flipy,
				sx&0xff,
				sy + 16*y,
				screen.priority(),
				priority_mask,0 );
		}
		source += 4;
	}
}

UINT32 lkage_state::screen_update_lkage(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int bank;

	flip_screen_x_set(~m_vreg[2] & 0x01);
	flip_screen_y_set(~m_vreg[2] & 0x02);

	bank = m_vreg[1] & 0x08;

	if (m_bg_tile_bank != bank)
	{
		m_bg_tile_bank = bank;
		m_bg_tilemap->mark_all_dirty();
	}

	bank = m_vreg[0]&0x04;
	if (m_fg_tile_bank != bank)
	{
		m_fg_tile_bank = bank;
		m_fg_tilemap->mark_all_dirty();
	}

	bank = m_vreg[0]&0x02;
	if (m_tx_tile_bank != bank)
	{
		m_tx_tile_bank = bank;
		m_tx_tilemap->mark_all_dirty();
	}

	m_bg_tilemap->set_palette_offset(0x300 + (m_vreg[1] & 0xf0));
	m_fg_tilemap->set_palette_offset(0x200 + (m_vreg[1] & 0xf0));
	m_tx_tilemap->set_palette_offset(0x110);

	m_tx_tilemap->set_scrollx(0, m_scroll[0]);
	m_tx_tilemap->set_scrolly(0, m_scroll[1]);

	m_fg_tilemap->set_scrollx(0, m_scroll[2]);
	m_fg_tilemap->set_scrolly(0, m_scroll[3]);

	m_bg_tilemap->set_scrollx(0, m_scroll[4]);
	m_bg_tilemap->set_scrolly(0, m_scroll[5]);

	screen.priority().fill(0, cliprect);
	if ((m_vreg[2] & 0xf0) == 0xf0)
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 1);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, (m_vreg[1] & 2) ? 2 : 4);
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 4);
		draw_sprites(screen, bitmap, cliprect);
	}
	else
	{
		m_tx_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	}

	return 0;
}
