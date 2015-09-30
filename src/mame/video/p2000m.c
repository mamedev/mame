// license:???
// copyright-holders:Paul Daniels
/**********************************************************************

    p2000m.c

    Functions to emulate video hardware of the p2000m

**********************************************************************/

#include "includes/p2000t.h"




VIDEO_START_MEMBER(p2000t_state,p2000m)
{
	m_frame_count = 0;
}


UINT32 p2000t_state::screen_update_p2000m(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram;
	int offs, sx, sy, code, loop;

	for (offs = 0; offs < 80 * 24; offs++)
	{
		sy = (offs / 80) * 20;
		sx = (offs % 80) * 12;

		if ((m_frame_count > 25) && (videoram[offs + 2048] & 0x40))
			code = 32;
		else
		{
			code = videoram[offs];
			if ((videoram[offs + 2048] & 0x01) && (code & 0x20))
			{
				code += (code & 0x40) ? 64 : 96;
			} else {
				code &= 0x7f;
			}
			if (code < 32) code = 32;
		}

		m_gfxdecode->gfx(0)->zoom_opaque(bitmap,cliprect, code,
			videoram[offs + 2048] & 0x08 ? 0 : 1, 0, 0, sx, sy, 0x20000, 0x20000);

		if (videoram[offs] & 0x80)
		{
			for (loop = 0; loop < 12; loop++)
			{
				bitmap.pix16(sy + 18, sx + loop) = 0;   /* cursor */
				bitmap.pix16(sy + 19, sx + loop) = 0;   /* cursor */
			}
		}
	}

	return 0;
}
