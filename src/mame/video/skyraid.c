// license:???
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Sky Raider video emulation

***************************************************************************/

#include "emu.h"
#include "includes/skyraid.h"


void skyraid_state::video_start()
{
	m_helper.allocate(128, 240);
}


void skyraid_state::draw_text(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const UINT8* p = m_alpha_num_ram;

	int i;

	for (i = 0; i < 4; i++)
	{
		int x;
		int y;

		y = 136 + 16 * (i ^ 1);

		for (x = 0; x < bitmap.width(); x += 16)
			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, *p++, 0, 0, 0,   x, y, 0);
	}
}


void skyraid_state::draw_terrain(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const UINT8* p = memregion("user1")->base();

	int x;
	int y;

	for (y = 0; y < bitmap.height(); y++)
	{
		int offset = (16 * m_scroll + 16 * ((y + 1) / 2)) & 0x7FF;

		x = 0;

		while (x < bitmap.width())
		{
			UINT8 val = p[offset++];

			int color = val / 32;
			int count = val % 32;

			rectangle r(x, x + 31 - count, y, y+ 1);

			bitmap.fill(color, r);

			x += 32 - count;
		}
	}
}


void skyraid_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	for (i = 0; i < 4; i++)
	{
		int code = m_obj_ram[8 + 2 * i + 0] & 15;
		int flag = m_obj_ram[8 + 2 * i + 1] & 15;
		int vert = m_pos_ram[8 + 2 * i + 0];
		int horz = m_pos_ram[8 + 2 * i + 1];

		vert -= 31;

		if (flag & 1)
			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code ^ 15, code >> 3, 0, 0,
				horz / 2, vert, 2);
	}
}


void skyraid_state::draw_missiles(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	/* hardware is restricted to one sprite per scanline */

	for (i = 0; i < 4; i++)
	{
		int code = m_obj_ram[2 * i + 0] & 15;
		int vert = m_pos_ram[2 * i + 0];
		int horz = m_pos_ram[2 * i + 1];

		vert -= 15;
		horz -= 31;

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
			code ^ 15, 0, 0, 0,
			horz / 2, vert, 0);
	}
}


void skyraid_state::draw_trapezoid(bitmap_ind16& dst, bitmap_ind16& src)
{
	const UINT8* p = memregion("user2")->base();

	int x;
	int y;

	for (y = 0; y < dst.height(); y++)
	{
		UINT16* pSrc = &src.pix16(y);
		UINT16* pDst = &dst.pix16(y);

		int x1 = 0x000 + p[(y & ~1) + 0];
		int x2 = 0x100 + p[(y & ~1) + 1];

		for (x = x1; x < x2; x++)
			pDst[x] = pSrc[128 * (x - x1) / (x2 - x1)];
	}
}


UINT32 skyraid_state::screen_update_skyraid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	rectangle helper_clip = cliprect;
	helper_clip &= m_helper.cliprect();

	draw_terrain(m_helper, helper_clip);
	draw_sprites(m_helper, helper_clip);
	draw_missiles(m_helper, helper_clip);
	draw_trapezoid(bitmap, m_helper);
	draw_text(bitmap, cliprect);
	return 0;
}
