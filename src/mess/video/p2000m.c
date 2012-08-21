/**********************************************************************

    p2000m.c

    Functions to emulate video hardware of the p2000m

**********************************************************************/

#include "includes/p2000t.h"




VIDEO_START( p2000m )
{
	p2000t_state *state = machine.driver_data<p2000t_state>();
	state->m_frame_count = 0;
}


SCREEN_UPDATE_IND16( p2000m )
{
	p2000t_state *state = screen.machine().driver_data<p2000t_state>();
	UINT8 *videoram = state->m_p_videoram;
	int offs, sx, sy, code, loop;

	for (offs = 0; offs < 80 * 24; offs++)
	{
		sy = (offs / 80) * 20;
		sx = (offs % 80) * 12;

		if ((state->m_frame_count > 25) && (videoram[offs + 2048] & 0x40))
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

		drawgfxzoom_opaque (bitmap, cliprect, screen.machine().gfx[0], code,
			videoram[offs + 2048] & 0x08 ? 0 : 1, 0, 0, sx, sy, 0x20000, 0x20000);

		if (videoram[offs] & 0x80)
		{
			for (loop = 0; loop < 12; loop++)
			{
				bitmap.pix16(sy + 18, sx + loop) = 0;	/* cursor */
				bitmap.pix16(sy + 19, sx + loop) = 0;	/* cursor */
			}
		}
	}

	return 0;
}
