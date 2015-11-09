// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/taxidriv.h"


WRITE8_MEMBER(taxidriv_state::spritectrl_w)
{
	m_spritectrl[offset] = data;
}



UINT32 taxidriv_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;
	int sx,sy;


	if (m_bghide)
	{
		bitmap.fill(0, cliprect);


		/* kludge to fix scroll after death */
		m_scroll[0] = m_scroll[1] = m_scroll[2] = m_scroll[3] = 0;
		m_spritectrl[2] = m_spritectrl[5] = m_spritectrl[8] = 0;
	}
	else
	{
		for (offs = 0;offs < 0x400;offs++)
		{
			sx = offs % 32;
			sy = offs / 32;

			m_gfxdecode->gfx(3)->opaque(bitmap,cliprect,
					m_vram3[offs],
					0,
					0,0,
					(sx*8-m_scroll[0])&0xff,(sy*8-m_scroll[1])&0xff);
		}

		for (offs = 0;offs < 0x400;offs++)
		{
			sx = offs % 32;
			sy = offs / 32;

			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
					m_vram2[offs]+256*m_vram2[offs+0x400],
					0,
					0,0,
					(sx*8-m_scroll[2])&0xff,(sy*8-m_scroll[3])&0xff,0);
		}

		if (m_spritectrl[2] & 4)
		{
			for (offs = 0;offs < 0x1000;offs++)
			{
				int color;

				sx = ((offs/2) % 64-m_spritectrl[0]-256*(m_spritectrl[2]&1))&0x1ff;
				sy = ((offs/2) / 64-m_spritectrl[1]-128*(m_spritectrl[2]&2))&0x1ff;

				color = (m_vram5[offs/4]>>(2*(offs&3)))&0x03;
				if (color)
				{
					if (sx > 0 && sx < 256 && sy > 0 && sy < 256)
						bitmap.pix16(sy, sx) = color;
				}
			}
		}

		if (m_spritectrl[5] & 4)
		{
			for (offs = 0;offs < 0x1000;offs++)
			{
				int color;

				sx = ((offs/2) % 64-m_spritectrl[3]-256*(m_spritectrl[5]&1))&0x1ff;
				sy = ((offs/2) / 64-m_spritectrl[4]-128*(m_spritectrl[5]&2))&0x1ff;

				color = (m_vram6[offs/4]>>(2*(offs&3)))&0x03;
				if (color)
				{
					if (sx > 0 && sx < 256 && sy > 0 && sy < 256)
						bitmap.pix16(sy, sx) = color;
				}
			}
		}

		if (m_spritectrl[8] & 4)
		{
			for (offs = 0;offs < 0x1000;offs++)
			{
				int color;

				sx = ((offs/2) % 64-m_spritectrl[6]-256*(m_spritectrl[8]&1))&0x1ff;
				sy = ((offs/2) / 64-m_spritectrl[7]-128*(m_spritectrl[8]&2))&0x1ff;

				color = (m_vram7[offs/4]>>(2*(offs&3)))&0x03;
				if (color)
				{
					if (sx > 0 && sx < 256 && sy > 0 && sy < 256)
						bitmap.pix16(sy, sx) = color;
				}
			}
		}

		for (offs = 0;offs < 0x400;offs++)
		{
			sx = offs % 32;
			sy = offs / 32;

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					m_vram1[offs],
					0,
					0,0,
					sx*8,sy*8,0);
		}

		for (offs = 0;offs < 0x2000;offs++)
		{
			int color;

			sx = (offs/2) % 64;
			sy = (offs/2) / 64;

			color = (m_vram4[offs/4]>>(2*(offs&3)))&0x03;
			if (color)
			{
				bitmap.pix16(sy, sx) = 2 * color;
			}
		}
	}

	for (offs = 0;offs < 0x400;offs++)
	{
		sx = offs % 32;
		sy = offs / 32;

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				m_vram0[offs],
				0,
				0,0,
				sx*8,sy*8,0);
	}
	return 0;
}
