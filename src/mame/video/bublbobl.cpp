// license:BSD-3-Clause
// copyright-holders:Chris Moore, Nicola Salmoria
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/bublbobl.h"


UINT32 bublbobl_state::screen_update_bublbobl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;
	int sx, sy, xc, yc;
	int gfx_num, gfx_attr, gfx_offs;
	const UINT8 *prom;
	const UINT8 *prom_line;


	/* Bubble Bobble doesn't have a real video RAM. All graphics (characters */
	/* and sprites) are stored in the same memory region, and information on */
	/* the background character columns is stored in the area dd00-dd3f */

	/* This clears & redraws the entire screen each pass */
	bitmap.fill(255, cliprect);

	if (!m_video_enable)
		return 0;

	sx = 0;

	prom = memregion("proms")->base();
	for (offs = 0; offs < m_objectram.bytes(); offs += 4)
	{
		/* skip empty sprites */
		/* this is dword aligned so the UINT32 * cast shouldn't give problems */
		/* on any architecture */
		if (*(UINT32 *)(&m_objectram[offs]) == 0)
			continue;

		gfx_num = m_objectram[offs + 1];
		gfx_attr = m_objectram[offs + 3];
		prom_line = prom + 0x80 + ((gfx_num & 0xe0) >> 1);

		gfx_offs = ((gfx_num & 0x1f) * 0x80);
		if ((gfx_num & 0xa0) == 0xa0)
			gfx_offs |= 0x1000;

		sy = -m_objectram[offs + 0];

		for (yc = 0; yc < 32; yc++)
		{
			if (prom_line[yc / 2] & 0x08)   continue;   /* NEXT */

			if (!(prom_line[yc / 2] & 0x04))    /* next column */
			{
				sx = m_objectram[offs + 2];
				if (gfx_attr & 0x40) sx -= 256;
			}

			for (xc = 0; xc < 2; xc++)
			{
				int goffs, code, color, flipx, flipy, x, y;

				goffs = gfx_offs + xc * 0x40 + (yc & 7) * 0x02 + (prom_line[yc/2] & 0x03) * 0x10;
				code = m_videoram[goffs] + 256 * (m_videoram[goffs + 1] & 0x03) + 1024 * (gfx_attr & 0x0f);
				color = (m_videoram[goffs + 1] & 0x3c) >> 2;
				flipx = m_videoram[goffs + 1] & 0x40;
				flipy = m_videoram[goffs + 1] & 0x80;
				x = sx + xc * 8;
				y = (sy + yc * 8) & 0xff;

				if (flip_screen())
				{
					x = 248 - x;
					y = 248 - y;
					flipx = !flipx;
					flipy = !flipy;
				}

				m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
						code,
						color,
						flipx,flipy,
						x,y,15);
			}
		}

		sx += 16;
	}
	return 0;
}
