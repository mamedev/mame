/***************************************************************************

  nascom1.c

  Functions to emulate the video hardware of the nascom1.

***************************************************************************/

#include "emu.h"
#include "includes/nascom1.h"

UINT32 nascom1_state::screen_update_nascom1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram;
	int sy, sx;

	for (sx = 0; sx < 48; sx++)
	{
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, videoram[0x03ca + sx],
			1, 0, 0, sx * 8, 0);
	}

	for (sy = 0; sy < 15; sy++)
	{
		for (sx = 0; sx < 48; sx++)
		{
				m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, videoram[0x000a + (sy * 64) + sx],
				1, 0, 0, sx * 8, (sy + 1) * 16);
		}
	}
	return 0;
}

UINT32 nascom1_state::screen_update_nascom2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram;
	int sy, sx;

	for (sx = 0; sx < 48; sx++)
	{
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, videoram[0x03ca + sx],
			1, 0, 0, sx * 8, 0);
	}

	for (sy = 0; sy < 15; sy++)
	{
		for (sx = 0; sx < 48; sx++)
		{
				m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, videoram[0x000a + (sy * 64) + sx],
				1, 0, 0, sx * 8, (sy + 1) * 14);
		}
	}
	return 0;
}
