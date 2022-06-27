// license:BSD-3-Clause
// copyright-holders:David Haywood
// remaining gfx glitches

// layer priority register not fully understood

#include "emu.h"
#include "drgnmst.h"


rgb_t drgnmst_base_state::drgnmst_IIIIRRRRGGGGBBBB(uint32_t raw)
{
	int const bright = 0x5 + ((raw >> 12) & 0xf);    // TODO : verify brightness value from real pcb
	int r = (pal4bit((raw >> 8) & 0x0f) * bright) / 0x14;
	int g = (pal4bit((raw >> 4) & 0x0f) * bright) / 0x14;
	int b = (pal4bit((raw >> 0) & 0x0f) * bright) / 0x14;

	if (r < 0) r = 0;
	if (r > 0xff) r = 0xff;
	if (g < 0) g = 0;
	if (g > 0xff) g = 0xff;
	if (b < 0) b = 0;
	if (b > 0xff) b = 0xff;

	return rgb_t(r, g, b);
}

TILE_GET_INFO_MEMBER(drgnmst_base_state::get_fg_tile_info)
{
	// 8x8 tile layer
	int tileno, colour, flipyx;
	tileno = m_fg_videoram[tile_index * 2] & 0xfff;
	colour = m_fg_videoram[tile_index * 2 + 1] & 0x1f;
	flipyx = (m_fg_videoram[tile_index * 2 + 1] & 0x60)>>5;

	if ((m_fg_videoram[tile_index * 2] & 0x4000))
		tileno |= 0x10000;

	if ((m_fg_videoram[tile_index * 2] & 0x8000))
		tileno |= 0x8000;

	tileno ^= 0x18000;

//  tileno |= (BIT(tile_index, 5)) << 15; // 8x8 tile bank seems like cps1 (not on mastfury at least)
	tileinfo.set(1, tileno, colour, TILE_FLIPYX(flipyx));
}

void drgnmst_base_state::fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(drgnmst_base_state::get_bg_tile_info)
{
	// 32x32 tile layer
	int tileno, colour, flipyx;
	tileno = (m_bg_videoram[tile_index * 2] & 0x3ff) + 0xc00;
	colour = m_bg_videoram[tile_index * 2 + 1] & 0x1f;
	flipyx = (m_bg_videoram[tile_index * 2 + 1] & 0x60) >> 5;

	if ((m_bg_videoram[tile_index * 2] & 0x1000))
		tileno |= 0x1000;

	tileno ^= 0x1000;

	tileinfo.set(3, tileno, colour, TILE_FLIPYX(flipyx));
}

void drgnmst_base_state::bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(drgnmst_base_state::get_md_tile_info)
{
	// 16x16 tile layer
	int tileno, colour, flipyx;
	tileno = (m_md_videoram[tile_index * 2] & 0x3fff) - 0x2000;
	colour = m_md_videoram[tile_index * 2 + 1] & 0x1f;
	flipyx = (m_md_videoram[tile_index * 2 + 1] & 0x60) >> 5;

	if ((m_md_videoram[tile_index * 2] & 0x4000))
		tileno |= 0x4000;

	tileno ^= 0x4000;

	tileinfo.set(2, tileno, colour, TILE_FLIPYX(flipyx));
}

void drgnmst_base_state::md_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_md_videoram[offset]);
	m_md_tilemap->mark_tile_dirty(offset / 2);
}

void drgnmst_base_state::draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	uint16_t *source = m_spriteram->buffer();
	uint16_t *finish = source + 0x800 / 2;

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


TILEMAP_MAPPER_MEMBER(drgnmst_base_state::fg_tilemap_scan_cols)
{
	return ((col & 0x3f) << 5) | (row & 0x1f) | ((row & 0x20) << 6);
}

TILEMAP_MAPPER_MEMBER(drgnmst_base_state::md_tilemap_scan_cols)
{
	return ((col & 0x3f) << 4) | (row & 0x0f) | ((row & 0x30) << 6);
}

TILEMAP_MAPPER_MEMBER(drgnmst_base_state::bg_tilemap_scan_cols)
{
	return ((col & 0x3f) << 3) | (row & 0x07) | ((row & 0x38) << 6);
}

void drgnmst_base_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(drgnmst_base_state::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(drgnmst_base_state::fg_tilemap_scan_cols)), 8, 8, 64,64);
	m_fg_tilemap->set_transparent_pen(15);

	m_md_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(drgnmst_base_state::get_md_tile_info)), tilemap_mapper_delegate(*this, FUNC(drgnmst_base_state::md_tilemap_scan_cols)), 16, 16, 64,64);
	m_md_tilemap->set_transparent_pen(15);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(drgnmst_base_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(drgnmst_base_state::bg_tilemap_scan_cols)), 32, 32, 64,64);
	m_bg_tilemap->set_transparent_pen(15);

	// do the other tilemaps have rowscroll too? probably not ..
	m_md_tilemap->set_scroll_rows(1024);
}

void drgnmst_ym_state::video_start()
{
	drgnmst_base_state::video_start();
	m_alt_scrolling = true;
}

uint32_t drgnmst_base_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	int rowscroll_bank = (m_vidregs[4] & 0x30) >> 4;

	if (!m_alt_scrolling)
	{
		// drgnmst scrolling
		m_fg_tilemap->set_scrollx(0, m_vidregs[0x6] - 18); // verify (test mode colour test needs it)
		m_fg_tilemap->set_scrolly(0, m_vidregs[0x7]); // verify

		m_md_tilemap->set_scrolly(0, m_vidregs[0x9]); // verify
		for (int y = 0; y < 1024; y++)
			m_md_tilemap->set_scrollx(y, m_vidregs[0x8] - 16 + m_rowscrollram[y + 0x800 * rowscroll_bank]);

		m_bg_tilemap->set_scrollx(0, m_vidregs[0xa] - 18); // verify
		m_bg_tilemap->set_scrolly(0, m_vidregs[0xb]); // verify
	}
	else
	{
		// mastfury scrolling
		// does layer order change scroll offsets?

		int fgys = m_vidregs[0x7];
		m_fg_tilemap->set_scrollx(0, m_vidregs[0x6] - 14); // 14 = continue screen background
		m_fg_tilemap->set_scrolly(0, fgys); // verify

		int mgys = m_vidregs[0x8]; // skyscraper lift stage confirms this reg?
		m_md_tilemap->set_scrolly(0, mgys); // verify
		for (int y = 0; y < 1024; y++)
			m_md_tilemap->set_scrollx(y, m_vidregs[0x9] - 14 + m_rowscrollram[y + 0x800 * rowscroll_bank]); // 14 = char select backround, but vs screen is 16?

		m_bg_tilemap->set_scrollx(0, m_vidregs[0xa] - 18); // verify

		// this reg seems to be more closely related to md_tilemap again? is it some kind of split pos?
		//int bgys = m_vidregs[0xb] & 0x1ff;

		int bgys = m_vidregs[0x10]; // skyscraper lift stage confirms this reg?
		m_bg_tilemap->set_scrolly(0, bgys);
	}

	// TODO: figure out which bits relate to the order, like cps1?
	// 23da mastfury before attract portraits, ending
	// 12c8 mastfury power on
	switch (m_vidregs2[0])
	{

		case 0x23c0: // all ok
			m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			m_md_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			break;
		case 0x2cc0: // mastfury mr daeth stage all ok
			m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			m_md_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			break;
		case 0x38c0: // mastfury Sakamoto stage, Sya Ki stage same as above? but see note
			// should fg also go above sprites? (it partially obscures 'time over' and bonus stage items on Sakamoto stage
			// but explicitly changes from 2cc0 to display scores, which indicates there is maybe a difference)
			m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			m_md_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			break;
		case 0x2780: // mastfury skyscraper lift stage all ok
		case 0x279a: // mastfury continue screen all ok
			m_md_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			break;
		case 0x2d80: // all ok
		case 0x2cda: // mastfury win quotes all ok
			m_md_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			break;
		case 0x38da: // fg unsure
		case 0x215a: // fg unsure (mastfury title)
		case 0x2140: // all ok
			m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			m_md_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			break;
		case 0x2451: // fg unsure
		case 0x2d9a: // fg unsure
		case 0x2440: // all ok
		case 0x245a: // fg unsure, title screen
			m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			m_md_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			break;
		default:
			m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
			m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			m_md_tilemap->draw(screen, bitmap, cliprect, 0, 0);
			break;
	}

	draw_sprites(bitmap,cliprect);

	//popmessage("unknown video priority regs %04x\n", m_vidregs2[0]);

	return 0;
}

