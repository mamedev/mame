// license:BSD-3-Clause
// copyright-holders:David Haywood
// remaining gfx glitches

// layer priority register not fully understood

#include "emu.h"
#include "includes/drgnmst.h"


TILE_GET_INFO_MEMBER(drgnmst_state::get_drgnmst_fg_tile_info)
{
	int tileno, colour, flipyx;
	tileno = m_fg_videoram[tile_index * 2] & 0xfff;
	colour = m_fg_videoram[tile_index * 2 + 1] & 0x1f;
	flipyx = (m_fg_videoram[tile_index * 2 + 1] & 0x60)>>5;

	SET_TILE_INFO_MEMBER(1, tileno, colour, TILE_FLIPYX(flipyx));
}

WRITE16_MEMBER(drgnmst_state::drgnmst_fg_videoram_w)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(drgnmst_state::get_drgnmst_bg_tile_info)
{
	int tileno, colour, flipyx;
	tileno = (m_bg_videoram[tile_index * 2]& 0x1fff) + 0x800;
	colour = m_bg_videoram[tile_index * 2 + 1] & 0x1f;
	flipyx = (m_bg_videoram[tile_index * 2 + 1] & 0x60) >> 5;

	SET_TILE_INFO_MEMBER(3, tileno, colour, TILE_FLIPYX(flipyx));
}

WRITE16_MEMBER(drgnmst_state::drgnmst_bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(drgnmst_state::get_drgnmst_md_tile_info)
{
	int tileno, colour, flipyx;
	tileno = (m_md_videoram[tile_index * 2] & 0x7fff) - 0x2000;
	colour = m_md_videoram[tile_index * 2 + 1] & 0x1f;
	flipyx = (m_md_videoram[tile_index * 2 + 1] & 0x60) >> 5;

	SET_TILE_INFO_MEMBER(2, tileno, colour, TILE_FLIPYX(flipyx));
}

WRITE16_MEMBER(drgnmst_state::drgnmst_md_videoram_w)
{
	COMBINE_DATA(&m_md_videoram[offset]);
	m_md_tilemap->mark_tile_dirty(offset / 2);
}

void drgnmst_state::draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	UINT16 *source = m_spriteram;
	UINT16 *finish = source + 0x800 / 2;

	while (source < finish)
	{
		int xpos, ypos, number, flipx, flipy, wide, high;
		int x, y;
		int incx, incy;
		int colr;

		number = source[2];
		xpos = source[0];
		ypos = source[1];
		flipx = source[3] & 0x0020;
		flipy = source[3] & 0x0040;
		wide = (source[3] & 0x0f00) >> 8;
		high = (source[3] & 0xf000) >> 12;
		colr = (source[3] & 0x001f);

		if ((source[3] & 0xff00) == 0xff00) break;


		if (!flipx) { incx = 16;} else { incx = -16; xpos += 16 * wide; }
		if (!flipy) { incy = 16;} else { incy = -16; ypos += 16 * high; }


		for (y = 0; y <= high; y++)
		{
			for (x = 0; x <= wide; x++)
			{
				int realx, realy, realnumber;

				realx = xpos + incx * x;
				realy = ypos + incy * y;
				realnumber = number + x + y * 16;

					gfx->transpen(bitmap,cliprect, realnumber, colr, flipx, flipy, realx, realy, 15);
			}
		}
		source += 4;
	}
}


TILEMAP_MAPPER_MEMBER(drgnmst_state::drgnmst_fg_tilemap_scan_cols)
{
	return (col * 32) + (row & 0x1f) + ((row & 0xe0) >> 5) * 2048;
}

TILEMAP_MAPPER_MEMBER(drgnmst_state::drgnmst_md_tilemap_scan_cols)
{
	return (col * 16) + (row & 0x0f) + ((row & 0xf0) >> 4) * 1024;
}

TILEMAP_MAPPER_MEMBER(drgnmst_state::drgnmst_bg_tilemap_scan_cols)
{
	return (col * 8) + (row & 0x07) + ((row & 0xf8) >> 3) * 512;
}

void drgnmst_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(drgnmst_state::get_drgnmst_fg_tile_info),this), tilemap_mapper_delegate(FUNC(drgnmst_state::drgnmst_fg_tilemap_scan_cols),this), 8, 8, 64,64);
	m_fg_tilemap->set_transparent_pen(15);

	m_md_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(drgnmst_state::get_drgnmst_md_tile_info),this), tilemap_mapper_delegate(FUNC(drgnmst_state::drgnmst_md_tilemap_scan_cols),this), 16, 16, 64,64);
	m_md_tilemap->set_transparent_pen(15);

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(drgnmst_state::get_drgnmst_bg_tile_info),this), tilemap_mapper_delegate(FUNC(drgnmst_state::drgnmst_bg_tilemap_scan_cols),this), 32, 32, 64,64);
	m_bg_tilemap->set_transparent_pen(15);

	// do the other tilemaps have rowscroll too? probably not ..
	m_md_tilemap->set_scroll_rows(1024);
}

UINT32 drgnmst_state::screen_update_drgnmst(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y, rowscroll_bank;

	m_bg_tilemap->set_scrollx(0, m_vidregs[10] - 18); // verify
	m_bg_tilemap->set_scrolly(0, m_vidregs[11]); // verify

//  m_md_tilemap->set_scrollx(0, m_vidregs[8] - 16); // rowscrolled
	m_md_tilemap->set_scrolly(0, m_vidregs[9]); // verify

	m_fg_tilemap->set_scrollx(0, m_vidregs[6] - 18); // verify (test mode colour test needs it)
	m_fg_tilemap->set_scrolly(0, m_vidregs[7]); // verify

	rowscroll_bank = (m_vidregs[4] & 0x30) >> 4;

	for (y = 0; y < 1024; y++)
		m_md_tilemap->set_scrollx(y, m_vidregs[8] - 16 + m_rowscrollram[y + 0x800 * rowscroll_bank]);

	// todo: figure out which bits relate to the order
	switch (m_vidregs2[0])
	{
		case 0x2451: // fg unsure
		case 0x2d9a: // fg unsure
		case 0x2440: // all ok
		case 0x245a: // fg unsure, title screen
			m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			m_md_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			break;
		case 0x23c0: // all ok
			m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			m_md_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			break;
		case 0x38da: // fg unsure
		case 0x215a: // fg unsure
		case 0x2140: // all ok
			m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			m_md_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			break;
		case 0x2d80: // all ok
			m_md_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			break;
		default:
			m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			m_md_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			logerror ("unknown video priority regs %04x\n", m_vidregs2[0]);

	}

	draw_sprites(bitmap,cliprect);

//  popmessage ("x %04x x %04x x %04x x %04x x %04x", m_vidregs2[0], m_vidregs[12], m_vidregs[13], m_vidregs[14], m_vidregs[15]);
//  popmessage ("x %04x x %04x y %04x y %04x z %04x z %04x",m_vidregs[0],m_vidregs[1],m_vidregs[2],m_vidregs[3],m_vidregs[4],m_vidregs[5]);

	return 0;
}
