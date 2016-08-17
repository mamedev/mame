// license:BSD-3-Clause
// copyright-holders:Uki
/*******************************************************************************

Dr. Micro (c) 1983 Sanritsu

Video hardware
        driver by Uki

*******************************************************************************/

#include "emu.h"
#include "includes/drmicro.h"


/****************************************************************************/

WRITE8_MEMBER(drmicro_state::drmicro_videoram_w)
{
	m_videoram[offset] = data;

	if (offset < 0x800)
		m_bg2->mark_tile_dirty((offset & 0x3ff));
	else
		m_bg1->mark_tile_dirty((offset & 0x3ff));
}


/****************************************************************************/

TILE_GET_INFO_MEMBER(drmicro_state::get_bg1_tile_info)
{
	int code, col, flags;

	code = m_videoram[tile_index + 0x0800];
	col = m_videoram[tile_index + 0x0c00];

	code += (col & 0xc0) << 2;
	flags = ((col & 0x20) ? TILEMAP_FLIPY : 0) | ((col & 0x10) ? TILEMAP_FLIPX : 0);
	col &= 0x0f;

	SET_TILE_INFO_MEMBER(0, code, col, flags);
}

TILE_GET_INFO_MEMBER(drmicro_state::get_bg2_tile_info)
{
	int code, col, flags;

	code = m_videoram[tile_index + 0x0000];
	col = m_videoram[tile_index + 0x0400];

	code += (col & 0xc0) << 2;
	flags = ((col & 0x20) ? TILEMAP_FLIPY : 0) | ((col & 0x10) ? TILEMAP_FLIPX : 0);
	col &= 0x0f;

	SET_TILE_INFO_MEMBER(1, code, col, flags);
}

/****************************************************************************/

PALETTE_INIT_MEMBER(drmicro_state, drmicro)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	for (i = 0; i < 0x200; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}

void drmicro_state::video_start()
{
	m_videoram = std::make_unique<UINT8[]>(0x1000);
	save_pointer(NAME(m_videoram.get()), 0x1000);

	m_bg1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(drmicro_state::get_bg1_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(drmicro_state::get_bg2_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg2->set_transparent_pen(0);
}

UINT32 drmicro_state::screen_update_drmicro(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs, adr, g;
	int chr, col, attr;
	int x, y, fx, fy;

	m_bg1->draw(screen, bitmap, cliprect, 0, 0);
	m_bg2->draw(screen, bitmap, cliprect, 0, 0);

	/* draw sprites */
	for (g = 0; g < 2; g++)
	{
		adr = 0x800 * g;

		for (offs = 0x00; offs < 0x20; offs += 4)
		{
			x = m_videoram[offs + adr + 3];
			y = m_videoram[offs + adr + 0];
			attr = m_videoram[offs + adr + 2];
			chr = m_videoram[offs + adr + 1];

			fx = (chr & 0x01) ^ m_flipscreen;
			fy = ((chr & 0x02) >> 1) ^ m_flipscreen;

			chr = (chr >> 2) | (attr & 0xc0);

			col = (attr & 0x0f) + 0x00;

			if (!m_flipscreen)
				y = (240 - y) & 0xff;
			else
				x = (240 - x) & 0xff;

			m_gfxdecode->gfx(3-g)->transpen(bitmap,cliprect,
					chr,
					col,
					fx,fy,
					x,y,0);

			if (x > 240)
			{
				m_gfxdecode->gfx(3-g)->transpen(bitmap,cliprect,
						chr,
						col,
						fx,fy,
						x-256,y,0);
			}
		}
	}
	return 0;
}
