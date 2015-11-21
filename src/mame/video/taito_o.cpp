// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/***************************************************************************

Based on taito_h.c

***************************************************************************/

#include "emu.h"
#include "includes/taito_o.h"


/* These are hand-tuned values */
static const int zoomy_conv_table[] =
{
/*    +0   +1   +2   +3   +4   +5   +6   +7    +8   +9   +a   +b   +c   +d   +e   +f */
	0x00,0x01,0x01,0x02,0x02,0x03,0x04,0x05, 0x06,0x06,0x07,0x08,0x09,0x0a,0x0a,0x0b,   /* 0x00 */
	0x0b,0x0c,0x0c,0x0d,0x0e,0x0e,0x0f,0x10, 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x16,
	0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e, 0x1f,0x20,0x21,0x22,0x24,0x25,0x26,0x27,
	0x28,0x2a,0x2b,0x2c,0x2e,0x30,0x31,0x32, 0x34,0x36,0x37,0x38,0x3a,0x3c,0x3e,0x3f,

	0x40,0x41,0x42,0x42,0x43,0x43,0x44,0x44, 0x45,0x45,0x46,0x46,0x47,0x47,0x48,0x49,   /* 0x40 */
	0x4a,0x4a,0x4b,0x4b,0x4c,0x4d,0x4e,0x4f, 0x4f,0x50,0x51,0x51,0x52,0x53,0x54,0x55,
	0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d, 0x5e,0x5f,0x60,0x61,0x62,0x63,0x64,0x66,
	0x67,0x68,0x6a,0x6b,0x6c,0x6e,0x6f,0x71, 0x72,0x74,0x76,0x78,0x80,0x7b,0x7d,0x7f
};


void taitoo_state::parentj_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority )
{
	/* Y chain size is 16/32?/64/64? pixels. X chain size
	   is always 64 pixels. */

	address_space &space = machine().driver_data()->generic_space();
	static const int size[] = { 1, 2, 4, 4 };
	int x0, y0, x, y, dx, dy, ex, ey, zx, zy;
	int ysize;
	int j, k;
	int offs;                   /* sprite RAM offset */
	int tile_offs;              /* sprite chain offset */
	int zoomx, zoomy;           /* zoom value */

	for (offs = 0x03f8 / 2; offs >= 0; offs -= 0x008 / 2)
	{
		if (offs <  0x01b0 && priority == 0)    continue;
		if (offs >= 0x01b0 && priority == 1)    continue;

		x0        =  m_tc0080vco->sprram_r(space, offs + 1, 0xffff) & 0x3ff;
		y0        =  m_tc0080vco->sprram_r(space, offs + 0, 0xffff) & 0x3ff;
		zoomx     = (m_tc0080vco->sprram_r(space, offs + 2, 0xffff) & 0x7f00) >> 8;
		zoomy     = (m_tc0080vco->sprram_r(space, offs + 2, 0xffff) & 0x007f);
		tile_offs = (m_tc0080vco->sprram_r(space, offs + 3, 0xffff) & 0x1fff) << 2;
		ysize     = size[(m_tc0080vco->sprram_r(space, offs, 0xffff) & 0x0c00) >> 10];

		if (tile_offs)
		{
			/* Convert zoomy value to real value as zoomx */
			zoomy = zoomy_conv_table[zoomy];

			if (zoomx < 63)
			{
				dx = 8 + (zoomx + 2) / 8;
				ex = (zoomx + 2) % 8;
				zx = ((dx << 1) + ex) << 11;
			}
			else
			{
				dx = 16 + (zoomx - 63) / 4;
				ex = (zoomx - 63) % 4;
				zx = (dx + ex) << 12;
			}

			if (zoomy < 63)
			{
				dy = 8 + (zoomy + 2) / 8;
				ey = (zoomy + 2) % 8;
				zy = ((dy << 1) + ey) << 11;
			}
			else
			{
				dy = 16 + (zoomy - 63) / 4;
				ey = (zoomy - 63) % 4;
				zy = (dy + ey) << 12;
			}

			if (x0 >= 0x200) x0 -= 0x400;
			if (y0 >= 0x200) y0 -= 0x400;

			if (m_tc0080vco->flipscreen_r())
			{
				x0 = 497 - x0;
				y0 = 498 - y0;
				dx = -dx;
				dy = -dy;
			}
			else
			{
				x0 += 1;
				y0 += 2;
			}

			y = y0;
			for (j = 0; j < ysize; j ++)
			{
				x = x0;
				for (k = 0; k < 4; k ++)
				{
					if (tile_offs >= 0x1000)
					{
						int tile, color, flipx, flipy;

						tile  = m_tc0080vco->cram_0_r(space, tile_offs, 0xffff) & 0x7fff;
						color = m_tc0080vco->cram_1_r(space, tile_offs, 0xffff) & 0x001f;
						flipx = m_tc0080vco->cram_1_r(space, tile_offs, 0xffff) & 0x0040;
						flipy = m_tc0080vco->cram_1_r(space, tile_offs, 0xffff) & 0x0080;

						if (m_tc0080vco->flipscreen_r())
						{
							flipx ^= 0x0040;
							flipy ^= 0x0080;
						}


									m_gfxdecode->gfx(0)->zoom_transpen(bitmap,cliprect,
									tile,
									color,
									flipx, flipy,
									x, y,
									zx, zy, 0
						);
					}
					tile_offs ++;
					x += dx;
				}
				y += dy;
			}
		}
	}
}


UINT32 taitoo_state::screen_update_parentj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tc0080vco->tilemap_update();

	bitmap.fill(0, cliprect);

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 0);

	parentj_draw_sprites(bitmap, cliprect, 0);
	parentj_draw_sprites(bitmap, cliprect, 1);

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);
	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 2, 0, 0);

	return 0;
}
