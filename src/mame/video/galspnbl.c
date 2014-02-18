#include "emu.h"
#include "includes/galspnbl.h"


PALETTE_INIT_MEMBER(galspnbl_state, galspnbl)
{
	int i;

	/* initialize 555 RGB lookup */
	for (i = 0; i < 32768; i++)
		palette.set_pen_color(i + 1024, pal5bit(i >> 5), pal5bit(i >> 10), pal5bit(i >> 0));
}



void galspnbl_state::draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	offs_t offs;

//  int screenscroll = 4 - (m_scroll[0] & 0xff);

	for (offs = 0; offs < 0x20000; offs++)
	{
		int y = offs >> 9;
		int x = offs & 0x1ff;

		bitmap.pix16(y, x) = 1024 + (m_bgvideoram[offs] >> 1);
	}
}


UINT32 galspnbl_state::screen_update_galspnbl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	draw_background(bitmap, cliprect);

	galspnbl_draw_sprites(screen, m_gfxdecode, bitmap, cliprect, 0,  m_spriteram, m_spriteram.bytes());

	for (offs = 0; offs < 0x1000 / 2; offs++)
	{
		int sx, sy, code, attr, color;

		code = m_videoram[offs];
		attr = m_colorram[offs];
		color = (attr & 0x00f0) >> 4;
		sx = offs % 64;
		sy = offs / 64;

		/* What is this? A priority/half transparency marker? */
		if (!(attr & 0x0008))
		{
			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					0,0,
//                  16*sx + screenscroll,8*sy,
					16*sx,8*sy,0);
		}
	}

	galspnbl_draw_sprites(screen, m_gfxdecode, bitmap, cliprect, 1, m_spriteram, m_spriteram.bytes());
	return 0;
}
