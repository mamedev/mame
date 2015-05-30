// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/******************************************************************

Mr. F. Lea
(C) 1983 PACIFIC NOVELTY MFG. INC.

******************************************************************/

#include "emu.h"
#include "includes/mrflea.h"

WRITE8_MEMBER(mrflea_state::mrflea_gfx_bank_w)
{
	m_gfx_bank = data;

	if (data & ~0x14)
		logerror("unknown gfx bank: 0x%02x\n", data);
}

WRITE8_MEMBER(mrflea_state::mrflea_videoram_w)
{
	int bank = offset / 0x400;

	offset &= 0x3ff;
	m_videoram[offset] = data;
	m_videoram[offset + 0x400] = bank;
	/* the address range that tile data is written to sets one bit of
	  the bank select.  The remaining bits are from a video register. */
}

WRITE8_MEMBER(mrflea_state::mrflea_spriteram_w)
{
	if (offset & 2)
	{
		/* tile_number */
		m_spriteram[offset | 1] = offset & 1;
		offset &= ~1;
	}

	m_spriteram[offset] = data;
}

void mrflea_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	const UINT8 *source = m_spriteram;
	const UINT8 *finish = source + 0x100;
	rectangle clip = m_screen->visible_area();

	clip.max_x -= 24;
	clip.min_x += 16;

	while (source < finish)
	{
		int xpos = source[1] - 3;
		int ypos = source[0] - 16 + 3;
		int tile_number = source[2] + source[3] * 0x100;

		gfx->transpen(bitmap,clip,
			tile_number,
			0, /* color */
			0,0, /* no flip */
			xpos,ypos,0 );
		gfx->transpen(bitmap,clip,
			tile_number,
			0, /* color */
			0,0, /* no flip */
			xpos,256+ypos,0 );
		source += 4;
	}
}

void mrflea_state::draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	const UINT8 *source = m_videoram;
	gfx_element *gfx = m_gfxdecode->gfx(1);
	int sx, sy;
	int base = 0;

	if (BIT(m_gfx_bank, 2))
		base |= 0x400;

	if (BIT(m_gfx_bank, 4))
		base |= 0x200;

	for (sy = 0; sy < 256; sy += 8)
	{
		for (sx = 0; sx < 256; sx += 8)
		{
			int tile_number = base + source[0] + source[0x400] * 0x100;
			source++;

				gfx->opaque(bitmap,cliprect,
				tile_number,
				0, /* color */
				0,0, /* no flip */
				sx,sy );
		}
	}
}

UINT32 mrflea_state::screen_update_mrflea(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_background(bitmap, cliprect);
	draw_sprites(bitmap, cliprect);
	return 0;
}
