// license:BSD-3-Clause
// copyright-holders:Acho A. Tang, Alex W. Jackson
/*****************************************************************************

B-Wings  (c) 1984 Data East Corporation
Zaviga   (c) 1984 Data East Corporation

drivers by Acho A. Tang
revised by Alex W. Jackson

*****************************************************************************/
// Directives

#include "emu.h"
#include "includes/bwing.h"


//****************************************************************************
// Exports


WRITE8_MEMBER(bwing_state::videoram_w)
{
	m_videoram[offset] = data;
	m_charmap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(bwing_state::fgscrollram_w)
{
	m_fgscrollram[offset] = data;
	m_fgmap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(bwing_state::bgscrollram_w)
{
	m_bgscrollram[offset] = data;
	m_bgmap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(bwing_state::gfxram_w)
{
	m_gfxram[offset] = data;
	int whichgfx = (offset & 0x1000) ? 3 : 2;
	m_gfxdecode->gfx(whichgfx)->mark_dirty((offset & 0xfff) / 32);
}


WRITE8_MEMBER(bwing_state::scrollreg_w)
{
	m_sreg[offset] = data;

	switch (offset)
	{
		case 6: m_palatch = data; break; // one of the palette components is latched through I/O(yike)

		case 7:
			m_mapmask = data;
			m_vrambank->set_bank(data >> 6);
		break;
	}
}


WRITE8_MEMBER(bwing_state::paletteram_w)
{
	static const float rgb[4][3] = {
		{0.85f, 0.95f, 1.00f},
		{0.90f, 1.00f, 1.00f},
		{0.80f, 1.00f, 1.00f},
		{0.75f, 0.90f, 1.10f}
	};
	int r, g, b, i;

	m_paletteram[offset] = data;

	r = ~data & 7;
	g = ~(data >> 4) & 7;
	b = ~m_palatch & 7;

	r = ((r << 5) + (r << 2) + (r >> 1));
	g = ((g << 5) + (g << 2) + (g >> 1));
	b = ((b << 5) + (b << 2) + (b >> 1));

	if ((i = ioport("EXTRA")->read()) < 4)
	{
		r = (float)r * rgb[i][0];
		g = (float)g * rgb[i][1];
		b = (float)b * rgb[i][2];
		if (r > 0xff) r = 0xff;
		if (g > 0xff) g = 0xff;
		if (b > 0xff) b = 0xff;
	}

	m_palette->set_pen_color(offset, rgb_t(r, g, b));
}

//****************************************************************************
// Initializations

TILE_GET_INFO_MEMBER(bwing_state::get_fgtileinfo)
{
	SET_TILE_INFO_MEMBER(2, m_fgscrollram[tile_index] & 0x7f, m_fgscrollram[tile_index] >> 7, 0);
}

TILE_GET_INFO_MEMBER(bwing_state::get_bgtileinfo)
{
	SET_TILE_INFO_MEMBER(3, m_bgscrollram[tile_index] & 0x7f, m_bgscrollram[tile_index] >> 7, 0);
}

TILE_GET_INFO_MEMBER(bwing_state::get_charinfo)
{
	SET_TILE_INFO_MEMBER(0, m_videoram[tile_index], 0, 0);
}

TILEMAP_MAPPER_MEMBER(bwing_state::scan_cols)
{
	return (row & 0xf) | ((col & 0xf) << 4) | ((row & 0x30) << 4) | ((col & 0x30) << 6);
}


void bwing_state::video_start()
{
	int i;

	m_charmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bwing_state::get_charinfo),this), TILEMAP_SCAN_COLS, 8, 8, 32, 32);
	m_fgmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bwing_state::get_fgtileinfo),this), tilemap_mapper_delegate(FUNC(bwing_state::scan_cols),this), 16, 16, 64, 64);
	m_bgmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bwing_state::get_bgtileinfo),this), tilemap_mapper_delegate(FUNC(bwing_state::scan_cols),this), 16, 16, 64, 64);

	m_charmap->set_transparent_pen(0);
	m_fgmap->set_transparent_pen(0);

	for (i = 0; i < 8; i++)
		m_sreg[i] = 0;
}

//****************************************************************************
// Realtime

void bwing_state::draw_sprites( bitmap_ind16 &bmp, const rectangle &clip, UINT8 *ram, int pri )
{
	int attrib, fx, fy, code, x, y, color, i;
	gfx_element *gfx = m_gfxdecode->gfx(1);

	for (i = 0; i < 0x200; i += 4)
	{
		attrib = ram[i];
		code   = ram[i + 1];
		y      = ram[i + 2];
		x      = ram[i + 3];
		color  = (attrib >> 3) & 1;

		if (!(attrib & 1) || color != pri)
			continue;

		code += (attrib << 3) & 0x100;
		y -= (attrib << 1) & 0x100;
		x -= (attrib << 2) & 0x100;

		fx = attrib & 0x04;
		fy = ~attrib & 0x02;

		// normal/cocktail
		if (m_mapmask & 0x20)
		{
			fx = !fx;
			fy = !fy;
			x = 240 - x;
			y = 240 - y;
		}

		// single/double
		if (!(attrib & 0x10))
				gfx->transpen(bmp,clip, code, color, fx, fy, x, y, 0);
		else
				gfx->zoom_transpen(bmp,clip, code, color, fx, fy, x, y, 1<<16, 2<<16, 0);
	}
}


UINT32 bwing_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	unsigned flip, x, y, shiftx;

	if (m_mapmask & 0x20)
	{
		flip = TILEMAP_FLIPX;
		shiftx = -8;
	}
	else
	{
		flip = TILEMAP_FLIPY;
		shiftx = 8;
	}

	// draw background
	if (!(m_mapmask & 1))
	{
		m_bgmap->set_flip(flip);
		x = ((m_sreg[1]<<2 & 0x300) + m_sreg[2] + shiftx) & 0x3ff;
		m_bgmap->set_scrollx(0, x);
		y = (m_sreg[1]<<4 & 0x300) + m_sreg[3];
		m_bgmap->set_scrolly(0, y);
		m_bgmap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
		bitmap.fill(m_palette->black_pen(), cliprect);

	// draw low priority sprites
	draw_sprites(bitmap, cliprect, m_spriteram, 0);

	// draw foreground
	if (!(m_mapmask & 2))
	{
		m_fgmap->set_flip(flip);
		x = ((m_sreg[1] << 6 & 0x300) + m_sreg[4] + shiftx) & 0x3ff;
		m_fgmap->set_scrollx(0, x);
		y = (m_sreg[1] << 8 & 0x300) + m_sreg[5];
		m_fgmap->set_scrolly(0, y);
		m_fgmap->draw(screen, bitmap, cliprect, 0, 0);
	}

	// draw high priority sprites
	draw_sprites(bitmap, cliprect, m_spriteram, 1);

	// draw text layer
//  if (m_mapmask & 4)
	{
		m_charmap->set_flip(flip);
		m_charmap->draw(screen, bitmap, cliprect, 0, 0);
	}
	return 0;
}
