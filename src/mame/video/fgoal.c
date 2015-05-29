// license:???
// copyright-holders:Stefan Jokisch
/***************************************************************************

    Taito Field Goal video emulation

***************************************************************************/

#include "emu.h"
#include "includes/fgoal.h"


WRITE8_MEMBER(fgoal_state::color_w)
{
	m_current_color = data & 3;
}


WRITE8_MEMBER(fgoal_state::ypos_w)
{
	m_ypos = data;
}


WRITE8_MEMBER(fgoal_state::xpos_w)
{
	m_xpos = data;
}


void fgoal_state::video_start()
{
	m_screen->register_screen_bitmap(m_fgbitmap);
	m_screen->register_screen_bitmap(m_bgbitmap);

	save_item(NAME(m_fgbitmap));
	save_item(NAME(m_bgbitmap));
}


UINT32 fgoal_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const UINT8* VRAM = m_video_ram;

	int x;
	int y;
	int n;

	/* draw color overlay foreground and background */

	if (m_player == 1 && (ioport("IN1")->read() & 0x40))
	{
		m_gfxdecode->gfx(0)->zoom_opaque(m_fgbitmap,cliprect,
			0, (m_player << 2) | m_current_color,
			1, 1,
			0, 16,
			0x40000,
			0x40000);

		m_gfxdecode->gfx(1)->zoom_opaque(m_bgbitmap,cliprect,
			0, 0,
			1, 1,
			0, 16,
			0x40000,
			0x40000);
	}
	else
	{
		m_gfxdecode->gfx(0)->zoom_opaque(m_fgbitmap,cliprect,
			0, (m_player << 2) | m_current_color,
			0, 0,
			0, 0,
			0x40000,
			0x40000);

		m_gfxdecode->gfx(1)->zoom_opaque(m_bgbitmap,cliprect,
			0, 0,
			0, 0,
			0, 0,
			0x40000,
			0x40000);
	}

	/* the ball has a fixed color */

	for (y = m_ypos; y < m_ypos + 8; y++)
	{
		for (x = m_xpos; x < m_xpos + 8; x++)
		{
			if (y < 256 && x < 256)
			{
				m_fgbitmap.pix16(y, x) = 128 + 16;
			}
		}
	}

	/* draw bitmap layer */

	for (y = 0; y < 256; y++)
	{
		UINT16* p = &bitmap.pix16(y);

		const UINT16* FG = &m_fgbitmap.pix16(y);
		const UINT16* BG = &m_bgbitmap.pix16(y);

		for (x = 0; x < 256; x += 8)
		{
			UINT8 v = *VRAM++;

			for (n = 0; n < 8; n++)
			{
				if (v & (1 << n))
				{
					p[x + n] = FG[x + n];
				}
				else
				{
					p[x + n] = BG[x + n];
				}
			}
		}
	}
	return 0;
}
