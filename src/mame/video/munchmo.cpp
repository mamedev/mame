// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
#include "emu.h"
#include "includes/munchmo.h"


void munchmo_state::munchmo_palette(palette_device &palette) const
{
	u8 const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = 0x4f * bit0 + 0xa8 * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

WRITE_LINE_MEMBER(munchmo_state::palette_bank_0_w)
{
	m_palette_bank = (state ? 1 : 0) | (m_palette_bank & 2);
}

WRITE_LINE_MEMBER(munchmo_state::palette_bank_1_w)
{
	m_palette_bank = (state ? 2 : 0) | (m_palette_bank & 1);
}

WRITE_LINE_MEMBER(munchmo_state::flipscreen_w)
{
	m_flipscreen = state;
}


void munchmo_state::video_start()
{
	m_tmpbitmap = std::make_unique<bitmap_ind16>(512, 512);
}

void munchmo_state::draw_status( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int row;

	for (row = 0; row < 4; row++)
	{
		int sy, sx = (row & 1) * 8;
		const u8 *source = m_status_vram + (~row & 1) * 32;
		if (row <= 1)
		{
			source += 2 * 32;
			sx += 256 + 32 + 16;
		}

		for (sy = 0; sy < 256; sy += 8)
		{
				gfx->opaque(bitmap,cliprect,
				*source++,
				0, /* color */
				0,0, /* no flip */
				sx,sy );
		}
	}
}

void munchmo_state::draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
/*
    ROM B1.2C contains 256 tilemaps defining 4x4 configurations of
    the tiles in ROM B2.2B
*/
	u8 *rom = memregion("gfx2")->base();
	gfx_element *gfx = m_gfxdecode->gfx(1);
	int offs;

	for (offs = 0; offs < 0x100; offs++)
	{
		int sy = (offs % 16) * 32;
		int sx = (offs / 16) * 32;
		int tile_number = m_videoram[offs];
		int row, col;

		for (row = 0; row < 4; row++)
		{
			for (col = 0; col < 4; col++)
			{
					gfx->opaque(*m_tmpbitmap,m_tmpbitmap->cliprect(),
					rom[col + tile_number * 4 + row * 0x400],
					m_palette_bank,
					0,0, /* flip */
					sx + col * 8, sy + row * 8 );
			}
		}
	}

	{
		int scrollx = -(m_vreg[2] *2 + (m_vreg[3] >> 7)) - 64 - 128 - 16;
		int scrolly = 0;

		copyscrollbitmap(bitmap, *m_tmpbitmap, 1, &scrollx, 1, &scrolly, cliprect);
	}
}

void munchmo_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int scroll = m_vreg[2];
	int flags = m_vreg[3];                           /*   XB?????? */
	int xadjust = - 128 - 16 - ((flags & 0x80) ? 1 : 0);
	int bank = (flags & 0x40) ? 1 : 0;
	gfx_element *gfx = m_gfxdecode->gfx(2 + bank);
	int color_base = m_palette_bank * 4 + 3;
	int i, j;
	int firstsprite = m_vreg[0] & 0x3f;
	for (i = firstsprite; i < firstsprite + 0x40; i++)
	{
		for (j = 0; j < 8; j++)
		{
			int offs = (j << 6) | (i & 0x3f);
			int tile_number = m_sprite_tile[offs];       /*   ETTTTTTT */
			int attributes = m_sprite_attr[offs];        /*   XYYYYYCC */
			int sx = m_sprite_xpos[offs];                /*   XXXXXXX? */
			int sy = (offs >> 6) << 5;                  /* Y YY------ */
			sy += (attributes >> 2) & 0x1f;
			if( attributes & 0x80 )
			{
				sx = (sx >> 1) | (tile_number & 0x80);
				sx = 2 * ((- 32 - scroll - sx) & 0xff) + xadjust;
					gfx->transpen(bitmap,cliprect,
					0x7f - (tile_number & 0x7f),
					color_base - (attributes & 0x03),
					0,0,                            /* no flip */
					sx,sy, 7 );
			}
		}
	}
}

u32 munchmo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_background(bitmap, cliprect);
	draw_sprites(bitmap, cliprect);
	draw_status(bitmap, cliprect);
	return 0;
}
