// license:BSD-3-Clause
// copyright-holders:Yochizo
/***************************************************************************

Functions to emulate the video hardware of the machine.

 Video hardware of this hardware is almost similar with "mexico86". So,
 most routines are derived from mexico86 driver.

***************************************************************************/


#include "emu.h"
#include "includes/exzisus.h"


/***************************************************************************
  Screen refresh
***************************************************************************/

UINT32 exzisus_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;
	int sx, sy, xc, yc;
	int gfx_num, gfx_attr, gfx_offs;

	/* Is this correct ? */
	bitmap.fill(1023, cliprect);

	/* ---------- 1st TC0010VCU ---------- */
	sx = 0;
	for (offs = 0 ; offs < m_objectram0.bytes() ; offs += 4)
	{
		int height;

		/* Skip empty sprites. */
		if ( !(*(UINT32 *)(&m_objectram0[offs])) )
		{
			continue;
		}

		gfx_num = m_objectram0[offs + 1];
		gfx_attr = m_objectram0[offs + 3];

		if ((gfx_num & 0x80) == 0)  /* 16x16 sprites */
		{
			gfx_offs = ((gfx_num & 0x7f) << 3);
			height = 2;

			sx = m_objectram0[offs + 2];
			sx |= (gfx_attr & 0x40) << 2;
		}
		else    /* tilemaps (each sprite is a 16x256 column) */
		{
			gfx_offs = ((gfx_num & 0x3f) << 7) + 0x0400;
			height = 32;

			if (gfx_num & 0x40)         /* Next column */
			{
				sx += 16;
			}
			else
			{
				sx = m_objectram0[offs + 2];
				sx |= (gfx_attr & 0x40) << 2;
			}
		}

		sy = 256 - (height << 3) - (m_objectram0[offs]);

		for (xc = 0 ; xc < 2 ; xc++)
		{
			int goffs = gfx_offs;
			for (yc = 0 ; yc < height ; yc++)
			{
				int code, color, x, y;

				code  = (m_videoram0[goffs + 1] << 8) | m_videoram0[goffs];
				color = (m_videoram0[goffs + 1] >> 6) | (gfx_attr & 0x0f);
				x = (sx + (xc << 3)) & 0xff;
				y = (sy + (yc << 3)) & 0xff;

				if (flip_screen())
				{
					x = 248 - x;
					y = 248 - y;
				}

				m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
						code & 0x3fff,
						color,
						flip_screen(), flip_screen(),
						x, y, 15);
				goffs += 2;
			}
			gfx_offs += height << 1;
		}
	}

	/* ---------- 2nd TC0010VCU ---------- */
	sx = 0;
	for (offs = 0 ; offs < m_objectram1.bytes() ; offs += 4)
	{
		int height;

		/* Skip empty sprites. */
		if ( !(*(UINT32 *)(&m_objectram1[offs])) )
		{
			continue;
		}

		gfx_num = m_objectram1[offs + 1];
		gfx_attr = m_objectram1[offs + 3];

		if ((gfx_num & 0x80) == 0)  /* 16x16 sprites */
		{
			gfx_offs = ((gfx_num & 0x7f) << 3);
			height = 2;

			sx = m_objectram1[offs + 2];
			sx |= (gfx_attr & 0x40) << 2;
		}
		else    /* tilemaps (each sprite is a 16x256 column) */
		{
			gfx_offs = ((gfx_num & 0x3f) << 7) + 0x0400;    ///
			height = 32;

			if (gfx_num & 0x40)         /* Next column */
			{
				sx += 16;
			}
			else
			{
				sx = m_objectram1[offs + 2];
				sx |= (gfx_attr & 0x40) << 2;
			}
		}
		sy = 256 - (height << 3) - (m_objectram1[offs]);

		for (xc = 0 ; xc < 2 ; xc++)
		{
			int goffs = gfx_offs;
			for (yc = 0 ; yc < height ; yc++)
			{
				int code, color, x, y;

				code  = (m_videoram1[goffs + 1] << 8) | m_videoram1[goffs];
				color = (m_videoram1[goffs + 1] >> 6) | (gfx_attr & 0x0f);
				x = (sx + (xc << 3)) & 0xff;
				y = (sy + (yc << 3)) & 0xff;

				if (flip_screen())
				{
					x = 248 - x;
					y = 248 - y;
				}

				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
						code & 0x3fff,
						color,
						flip_screen(), flip_screen(),
						x, y, 15);
				goffs += 2;
			}
			gfx_offs += height << 1;
		}
	}
	return 0;
}
