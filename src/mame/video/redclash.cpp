// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

  video.c

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

PALETTE_INIT_MEMBER(redclash_state,redclash)
{
	const uint8_t *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 5) & 0x01;
		r = 0x47 * bit0 + 0x97 * bit1;

		/* green component */
		bit0 = (color_prom[i] >> 2) & 0x01;
		bit1 = (color_prom[i] >> 6) & 0x01;
		g = 0x47 * bit0 + 0x97 * bit1;

		/* blue component */
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = 0x47 * bit0 + 0x97 * bit1;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* star colors */
	for (i = 0x20; i < 0x40; i++)
	{
		int bit0, bit1;
		int r, g, b;

		/* red component */
		bit0 = ((i - 0x20) >> 3) & 0x01;
		bit1 = ((i - 0x20) >> 4) & 0x01;
		b = 0x47 * bit0 + 0x97 * bit1;

		/* green component */
		bit0 = ((i - 0x20) >> 1) & 0x01;
		bit1 = ((i - 0x20) >> 2) & 0x01;
		g = 0x47 * bit0 + 0x97 * bit1;

		/* blue component */
		bit0 = ((i - 0x20) >> 0) & 0x01;
		r = 0x47 * bit0;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* characters */
	for (i = 0; i < 0x20; i++)
	{
		uint8_t ctabentry = ((i << 3) & 0x18) | ((i >> 2) & 0x07);
		palette.set_pen_indirect(i, ctabentry);
	}

	/* sprites */
	for (i = 0x20; i < 0x40; i++)
	{
		uint8_t ctabentry = color_prom[(i - 0x20) >> 1];

		ctabentry = bitswap<8>((color_prom[i - 0x20] >> 0) & 0x0f, 7,6,5,4,0,1,2,3);
		palette.set_pen_indirect(i + 0x00, ctabentry);

		ctabentry = bitswap<8>((color_prom[i - 0x20] >> 4) & 0x0f, 7,6,5,4,0,1,2,3);
		palette.set_pen_indirect(i + 0x20, ctabentry);
	}

	/* stars */
	for (i = 0x60; i < 0x80; i++)
		palette.set_pen_indirect(i, (i - 0x60) + 0x20);
}


WRITE8_MEMBER( redclash_state::redclash_videoram_w )
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER( redclash_state::redclash_gfxbank_w )
{
	if (m_gfxbank != (data & 0x01))
	{
		m_gfxbank = data & 0x01;
		machine().tilemap().mark_all_dirty();
	}
}

WRITE8_MEMBER( redclash_state::redclash_flipscreen_w )
{
	flip_screen_set(data & 0x01);
}

WRITE8_MEMBER( redclash_state::redclash_star_reset_w )
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
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(redclash_state::get_fg_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);
}

void redclash_state::redclash_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *spriteram = m_spriteram;
	int i, offs;

	for (offs = m_spriteram.bytes() - 0x20; offs >= 0; offs -= 0x20)
	{
		i = 0;
		while (i < 0x20 && spriteram[offs + i] != 0)
			i += 4;

		while (i > 0)
		{
			i -= 4;

			if (spriteram[offs + i] & 0x80)
			{
				int color = spriteram[offs + i + 2] & 0x0f;
				int sx = spriteram[offs + i + 3];
				int sy = offs / 4 + (spriteram[offs + i] & 0x07);


				switch ((spriteram[offs + i] & 0x18) >> 3)
				{
					case 3: /* 24x24 */
					{
						int code = ((spriteram[offs + i + 1] & 0xf0) >> 4) + ((m_gfxbank & 1) << 4);

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
						if (spriteram[offs + i] & 0x20) /* zero hour spaceships */
						{
							int code = ((spriteram[offs + i + 1] & 0xf8) >> 3) + ((m_gfxbank & 1) << 5);
							int bank = (spriteram[offs + i + 1] & 0x02) >> 1;

							m_gfxdecode->gfx(4+bank)->transpen(bitmap,cliprect,
									code,
									color,
									0,0,
									sx,sy - 16,0);
						}
						else
						{
							int code = ((spriteram[offs + i + 1] & 0xf0) >> 4) + ((m_gfxbank & 1) << 4);

							m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
									code,
									color,
									0,0,
									sx,sy - 16,0);
						}
						break;

					case 1: /* 8x8 */
						m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
								spriteram[offs + i + 1],// + 4 * (spriteram[offs + i + 2] & 0x10),
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

void redclash_state::redclash_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = 0; offs < 0x20; offs++)
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

WRITE_LINE_MEMBER(redclash_state::screen_vblank_redclash)
{
	// falling edge
	if (!state)
		m_stars->update_state();
}

uint32_t redclash_state::screen_update_redclash(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	m_stars->draw(bitmap, cliprect, 0x60, true, 0x00, 0xff);
	redclash_draw_sprites(bitmap, cliprect);
	redclash_draw_bullets(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
