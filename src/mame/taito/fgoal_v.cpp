// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

    Taito Field Goal video emulation

***************************************************************************/

#include "emu.h"
#include "fgoal.h"


void fgoal_state::color_w(uint8_t data)
{
	m_current_color = data & 3;
}


void fgoal_state::ypos_w(uint8_t data)
{
	m_ypos = data;
}


void fgoal_state::xpos_w(uint8_t data)
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


uint32_t fgoal_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *VRAM = m_video_ram;

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

	for (int y = m_ypos; y < m_ypos + 8; y++)
	{
		for (int x = m_xpos; x < m_xpos + 8; x++)
		{
			if (y < 256 && x < 256)
			{
				m_fgbitmap.pix(y, x) = 128 + 16;
			}
		}
	}

	/* draw bitmap layer */

	for (int y = 0; y < 256; y++)
	{
		uint16_t *const p = &bitmap.pix(y);

		uint16_t const *const FG = &m_fgbitmap.pix(y);
		uint16_t const *const BG = &m_bgbitmap.pix(y);

		for (int x = 0; x < 256; x += 8)
		{
			uint8_t v = *VRAM++;

			for (int n = 0; n < 8; n++)
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
