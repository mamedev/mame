// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

  redclash.cpp

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/redclash.h"
#include "video/resnet.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  I'm using the same palette conversion as Lady Bug, but the Zero Hour
  schematics show a different resistor network.

***************************************************************************/

void redclash_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 5);
		int const r = 0x47 * bit0 + 0x97 * bit1;

		// green component
		bit0 = BIT(color_prom[i], 2);
		bit1 = BIT(color_prom[i], 6);
		int const g = 0x47 * bit0 + 0x97 * bit1;

		// blue component
		bit0 = BIT(color_prom[i], 4);
		bit1 = BIT(color_prom[i], 7);
		int const b = 0x47 * bit0 + 0x97 * bit1;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// star colors
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1;

		// red component
		bit0 = BIT(i, 3);
		bit1 = BIT(i, 4);
		int const b = 0x47 * bit0 + 0x97 * bit1;

		// green component
		bit0 = BIT(i, 1);
		bit1 = BIT(i, 2);
		int const g = 0x47 * bit0 + 0x97 * bit1;

		// blue component
		bit0 = BIT(i, 0);
		int const r = 0x47 * bit0;

		palette.set_indirect_color(i + 0x20, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x20;

	// characters
	for (int i = 0; i < 0x20; i++)
	{
		uint8_t const ctabentry = ((i << 3) & 0x18) | ((i >> 2) & 0x07);
		palette.set_pen_indirect(i, ctabentry);
	}

	// sprites
	for (int i = 0; i < 0x20; i++)
	{
		uint8_t ctabentry;

		ctabentry = bitswap<4>((color_prom[i] >> 0) & 0x0f, 0,1,2,3);
		palette.set_pen_indirect(i + 0x20, ctabentry);

		ctabentry = bitswap<4>((color_prom[i] >> 4) & 0x0f, 0,1,2,3);
		palette.set_pen_indirect(i + 0x40, ctabentry);
	}

	// stars
	for (int i = 0; i < 0x20; i++)
		palette.set_pen_indirect(i + 0x60, i + 0x20);
}


WRITE8_MEMBER( redclash_state::videoram_w )
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER( redclash_state::gfxbank_w )
{
	if (m_gfxbank != (data & 0x01))
	{
		m_gfxbank = data & 0x01;
		machine().tilemap().mark_all_dirty();
	}
}

WRITE8_MEMBER( redclash_state::flipscreen_w )
{
	flip_screen_set(data & 0x01);
}

WRITE8_MEMBER( redclash_state::star_reset_w )
{
	m_stars->set_enable(true);
}

TILE_GET_INFO_MEMBER(redclash_state::get_fg_tile_info)
{
	int code = m_videoram[tile_index];
	int color = (m_videoram[tile_index] & 0x70) >> 4; // ??

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}


void redclash_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(redclash_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);
}

void redclash_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = m_spriteram.bytes() - 0x20; offs >= 0; offs -= 0x20)
	{
		int i = 0;
		while (i < 0x20 && m_spriteram[offs + i] != 0)
			i += 4;

		while (i > 0)
		{
			i -= 4;

			if (m_spriteram[offs + i] & 0x80)
			{
				int color = m_spriteram[offs + i + 2] & 0x0f;
				int sx = m_spriteram[offs + i + 3];
				int sy = offs / 4 + (m_spriteram[offs + i] & 0x07);


				switch ((m_spriteram[offs + i] & 0x18) >> 3)
				{
					case 3: /* 24x24 */
					{
						int code = ((m_spriteram[offs + i + 1] & 0xf0) >> 4) + ((m_gfxbank & 1) << 4);

						m_gfxdecode->gfx(3)->transpen(bitmap,cliprect,
								code,
								color,
								0,0,
								sx,sy - 16,0);
						/* wraparound */
						m_gfxdecode->gfx(3)->transpen(bitmap,cliprect,
								code,
								color,
								0,0,
								sx - 256,sy - 16,0);
						break;
					}

					case 2: /* 16x16 */
						if (m_spriteram[offs + i] & 0x20) /* zero hour spaceships */
						{
							int code = ((m_spriteram[offs + i + 1] & 0xf8) >> 3) + ((m_gfxbank & 1) << 5);
							int bank = (m_spriteram[offs + i + 1] & 0x02) >> 1;

							m_gfxdecode->gfx(4+bank)->transpen(bitmap,cliprect,
									code,
									color,
									0,0,
									sx,sy - 16,0);
						}
						else
						{
							int code = ((m_spriteram[offs + i + 1] & 0xf0) >> 4) + ((m_gfxbank & 1) << 4);

							m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
									code,
									color,
									0,0,
									sx,sy - 16,0);
						}
						break;

					case 1: /* 8x8 */
						m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
								m_spriteram[offs + i + 1],// + 4 * (m_spriteram[offs + i + 2] & 0x10),
								color,
								0,0,
								sx,sy - 16,0);
						break;

					case 0:
						popmessage("unknown sprite size 0");
						break;
				}
			}
		}
	}
}

void redclash_state::draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	for (int offs = 0; offs < 0x20; offs++)
	{
//      sx = m_videoramoffs];
		int sx = 8 * offs + (m_videoram[offs] & 0x07);   /* ?? */
		int sy = 0xff - m_videoram[offs + 0x20];

		if (flip_screen())
		{
			sx = 240 - sx;
		}

		if (cliprect.contains(sx, sy))
			bitmap.pix16(sy, sx) = 0x19;
	}
}

WRITE_LINE_MEMBER(redclash_state::screen_vblank)
{
	// falling edge
	if (!state)
		m_stars->update_state();
}

uint32_t redclash_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	m_stars->draw(bitmap, cliprect, 0x60, true, 0x00, 0xff);
	draw_sprites(bitmap, cliprect);
	draw_bullets(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
