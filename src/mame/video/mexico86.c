// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"
#include "includes/mexico86.h"


WRITE8_MEMBER(mexico86_state::mexico86_bankswitch_w)
{
	if ((data & 7) > 5)
		popmessage("Switching to invalid bank!");

	membank("bank1")->set_entry(data & 0x07);

	m_charbank = BIT(data, 5);
}



UINT32 mexico86_state::screen_update_mexico86(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;
	int sx, sy, xc, yc;
	int gfx_num, gfx_attr, gfx_offs;


	/* Bubble Bobble doesn't have a real video RAM. All graphics (characters */
	/* and sprites) are stored in the same memory region, and information on */
	/* the background character columns is stored inthe area dd00-dd3f */
	bitmap.fill(255, cliprect);

	sx = 0;

	/* the score display seems to be outside of the main objectram. */
	for (offs = 0; offs < m_objectram.bytes() + 0x200; offs += 4)
	{
		int height;

		if (offs >= m_objectram.bytes() && offs < m_objectram.bytes() + 0x180)
			continue;

		if (offs >= m_objectram.bytes() + 0x1c0)
			continue;

		/* skip empty sprites */
		/* this is dword aligned so the UINT32 * cast shouldn't give problems */
		/* on any architecture */
		if (*(UINT32 *)(&m_objectram[offs]) == 0)
			continue;

		gfx_num = m_objectram[offs + 1];
		gfx_attr = m_objectram[offs + 3];

		if (!BIT(gfx_num, 7))  /* 16x16 sprites */
		{
			gfx_offs = ((gfx_num & 0x1f) * 0x80) + ((gfx_num & 0x60) >> 1) + 12;
			height = 2;
		}
		else    /* tilemaps (each sprite is a 16x256 column) */
		{
			gfx_offs = ((gfx_num & 0x3f) * 0x80);
			height = 32;
		}

		if ((gfx_num & 0xc0) == 0xc0)   /* next column */
			sx += 16;
		else
		{
			sx = m_objectram[offs + 2];
			//if (gfx_attr & 0x40) sx -= 256;
		}
		sy = 256 - height * 8 - (m_objectram[offs + 0]);

		for (xc = 0; xc < 2; xc++)
		{
			for (yc = 0; yc < height; yc++)
			{
				int goffs, code, color, flipx, flipy, x, y;

				goffs = gfx_offs + xc * 0x40 + yc * 0x02;
				code = m_videoram[goffs] + ((m_videoram[goffs + 1] & 0x07) << 8)
						+ ((m_videoram[goffs + 1] & 0x80) << 4) + (m_charbank << 12);
				color = ((m_videoram[goffs + 1] & 0x38) >> 3) + ((gfx_attr & 0x02) << 2);
				flipx = m_videoram[goffs + 1] & 0x40;
				flipy = 0;

				//x = sx + xc * 8;
				x = (sx + xc * 8) & 0xff;
				y = (sy + yc * 8) & 0xff;

				m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
						code,
						color,
						flipx,flipy,
						x,y,15);
			}
		}
	}
	return 0;
}

UINT32 mexico86_state::screen_update_kikikai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;
	int sx, sy, yc;
	int gfx_num, /*gfx_attr,*/ gfx_offs;
	int height;
	int goffs, code, color, y;
	int tx, ty;

	bitmap.fill(m_palette->black_pen(), cliprect);
	sx = 0;
	for (offs = 0; offs < m_objectram.bytes(); offs += 4)
	{
		if (*(UINT32*)(m_objectram + offs) == 0)
			continue;

		ty = m_objectram[offs];
		gfx_num = m_objectram[offs + 1];
		tx = m_objectram[offs + 2];
		//gfx_attr = m_objectram[offs + 3];

		if (gfx_num & 0x80)
		{
			gfx_offs = ((gfx_num & 0x3f) << 7);
			height = 32;
			if (gfx_num & 0x40) sx += 16;
			else sx = tx;
		}
		else
		{
			if (!(ty && tx)) continue;
			gfx_offs = ((gfx_num & 0x1f) << 7) + ((gfx_num & 0x60) >> 1) + 12;
			height = 2;
			sx = tx;
		}

		sy = 256 - (height << 3) - ty;

		height <<= 1;
		for (yc = 0; yc < height; yc += 2)
		{
			y = (sy + (yc << 2)) & 0xff;
			goffs = gfx_offs + yc;
			code = m_videoram[goffs] + ((m_videoram[goffs + 1] & 0x1f) << 8);
			color = (m_videoram[goffs + 1] & 0xe0) >> 5;
			goffs += 0x40;

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					0,0,
					sx&0xff,y,15);

			code = m_videoram[goffs] + ((m_videoram[goffs + 1] & 0x1f) << 8);
			color = (m_videoram[goffs + 1] & 0xe0) >> 5;

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					0,0,
					(sx+8)&0xff,y,15);
		}
	}
	return 0;
}
