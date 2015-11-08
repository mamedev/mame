// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/***************************************************************************

  Nintendo 8080 video emulation

***************************************************************************/

#include "emu.h"
#include "includes/n8080.h"


WRITE8_MEMBER(n8080_state::n8080_video_control_w)
{
	m_sheriff_color_mode = (data >> 3) & 3;
	m_sheriff_color_data = (data >> 0) & 7;
	flip_screen_set(data & 0x20);
}


PALETTE_INIT_MEMBER(n8080_state,n8080)
{
	int i;

	for (i = 0; i < 8; i++)
		palette.set_pen_color(i, pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
}


PALETTE_INIT_MEMBER(n8080_state,helifire)
{
	int i;

	PALETTE_INIT_NAME(n8080)(palette);

	for (i = 0; i < 0x100; i++)
	{
		int level = 0xff * exp(-3 * i / 255.); /* capacitor discharge */

		palette.set_pen_color(0x000 + 8 + i, rgb_t(0x00, 0x00, level));   /* shades of blue */
		palette.set_pen_color(0x100 + 8 + i, rgb_t(0x00, 0xC0, level));   /* shades of blue w/ green star */

		palette.set_pen_color(0x200 + 8 + i, rgb_t(level, 0x00, 0x00));   /* shades of red */
		palette.set_pen_color(0x300 + 8 + i, rgb_t(level, 0xC0, 0x00));   /* shades of red w/ green star */
	}
}


void n8080_state::spacefev_start_red_cannon(  )
{
	m_spacefev_red_cannon = 1;
	m_cannon_timer->adjust(attotime::from_usec(550 * 68 * 10));
}


TIMER_CALLBACK_MEMBER(n8080_state::spacefev_stop_red_cannon)
{
	m_spacefev_red_cannon = 0;
	m_cannon_timer->adjust(attotime::never);
}


void n8080_state::helifire_next_line(  )
{
	m_helifire_mv++;

	if (m_helifire_sc % 4 == 2)
	{
		m_helifire_mv %= 256;
	}
	else
	{
		if (flip_screen())
			m_helifire_mv %= 255;
		else
			m_helifire_mv %= 257;
	}

	if (m_helifire_mv == 128)
	{
		m_helifire_sc++;
	}
}


VIDEO_START_MEMBER(n8080_state,spacefev)
{
	m_cannon_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(n8080_state::spacefev_stop_red_cannon),this));

	flip_screen_set(0);

	save_item(NAME(m_spacefev_red_screen));
	save_item(NAME(m_spacefev_red_cannon));
}


VIDEO_START_MEMBER(n8080_state,sheriff)
{
	flip_screen_set(0);

	save_item(NAME(m_sheriff_color_mode));
	save_item(NAME(m_sheriff_color_data));
}


VIDEO_START_MEMBER(n8080_state,helifire)
{
	UINT8 data = 0;
	int i;

	save_item(NAME(m_helifire_mv));
	save_item(NAME(m_helifire_sc));
	save_item(NAME(m_helifire_flash));
	save_item(NAME(m_helifire_LSFR));

	for (i = 0; i < 63; i++)
	{
		int bit =
			(data >> 6) ^
			(data >> 7) ^ 1;

		data = (data << 1) | (bit & 1);

		m_helifire_LSFR[i] = data;
	}

	flip_screen_set(0);
}


UINT32 n8080_state::screen_update_spacefev(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 mask = flip_screen() ? 0xff : 0x00;

	int x;
	int y;

	const UINT8* pRAM = m_videoram;
	const UINT8* pPROM = memregion("proms")->base();

	for (y = 0; y < 256; y++)
	{
		UINT16* pLine = &bitmap.pix16(y ^ mask);

		for (x = 0; x < 256; x += 8)
		{
			int n;

			UINT8 color = 0;

			if (m_spacefev_red_screen)
				color = 1;
			else
			{
				UINT8 val = pPROM[x >> 3];

				if ((x >> 3) == 0x06)
				{
					color = m_spacefev_red_cannon ? 1 : 7;
				}

				if ((x >> 3) == 0x1b)
				{
					static const UINT8 ufo_color[] =
					{
						1, /* red     */
						2, /* green   */
						7, /* white   */
						3, /* yellow  */
						5, /* magenta */
						6, /* cyan    */
					};

					int cycle = screen.frame_number() / 32;

					color = ufo_color[cycle % 6];
				}

				for (n = color + 1; n < 8; n++)
				{
					if (~val & (1 << n))
					{
						color = n;
					}
				}
			}

			for (n = 0; n < 8; n++)
			{
				pLine[(x + n) ^ mask] = (pRAM[x >> 3] & (1 << n)) ? color : 0;
			}
		}

		pRAM += 32;
	}
	return 0;
}


UINT32 n8080_state::screen_update_sheriff(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 mask = flip_screen() ? 0xff : 0x00;

	const UINT8* pPROM = memregion("proms")->base();

	int x;
	int y;

	const UINT8* pRAM = m_videoram;

	for (y = 0; y < 256; y++)
	{
		UINT16* pLine = &bitmap.pix16(y ^ mask);

		for (x = 0; x < 256; x += 8)
		{
			int n;

			UINT8 color = pPROM[32 * (y >> 3) + (x >> 3)];

			if (m_sheriff_color_mode == 1 && !(color & 8))
				color = m_sheriff_color_data ^ 7;

			if (m_sheriff_color_mode == 2)
				color = m_sheriff_color_data ^ 7;

			if (m_sheriff_color_mode == 3)
				color = 7;

			for (n = 0; n < 8; n++)
			{
				pLine[(x + n) ^ mask] = ((pRAM[x >> 3] >> n) & 1) ? (color & 7) : 0;
			}
		}

		pRAM += 32;
	}
	return 0;
}


UINT32 n8080_state::screen_update_helifire(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int SUN_BRIGHTNESS = ioport("POT0")->read();
	int SEA_BRIGHTNESS = ioport("POT1")->read();

	static const int wave[8] = { 0, 1, 2, 2, 2, 1, 0, 0 };

	unsigned saved_mv = m_helifire_mv;
	unsigned saved_sc = m_helifire_sc;

	int x;
	int y;

	for (y = 0; y < 256; y++)
	{
		UINT16* pLine = &bitmap.pix16(y);

		int level = 120 + wave[m_helifire_mv & 7];

		/* draw sky */

		for (x = level; x < 256; x++)
		{
			pLine[x] = 0x200 + 8 + SUN_BRIGHTNESS + x - level;
		}

		/* draw stars */

		if (m_helifire_mv % 8 == 4) /* upper half */
		{
			int step = (320 * (m_helifire_mv - 0)) % sizeof m_helifire_LSFR;

			int data =
				((m_helifire_LSFR[step] & 1) << 6) |
				((m_helifire_LSFR[step] & 2) << 4) |
				((m_helifire_LSFR[step] & 4) << 2) |
				((m_helifire_LSFR[step] & 8) << 0);

			pLine[0x80 + data] |= 0x100;
		}

		if (m_helifire_mv % 8 == 5) /* lower half */
		{
			int step = (320 * (m_helifire_mv - 1)) % sizeof m_helifire_LSFR;

			int data =
				((m_helifire_LSFR[step] & 1) << 6) |
				((m_helifire_LSFR[step] & 2) << 4) |
				((m_helifire_LSFR[step] & 4) << 2) |
				((m_helifire_LSFR[step] & 8) << 0);

			pLine[0x00 + data] |= 0x100;
		}

		/* draw sea */

		for (x = 0; x < level; x++)
		{
			pLine[x] = 8 + SEA_BRIGHTNESS + x;
		}

		/* draw foreground */

		for (x = 0; x < 256; x += 8)
		{
			int offset = 32 * y + (x >> 3);

			int n;

			for (n = 0; n < 8; n++)
			{
				if (flip_screen())
				{
					if ((m_videoram[offset ^ 0x1fff] << n) & 0x80)
					{
						pLine[x + n] = m_colorram[offset ^ 0x1fff] & 7;
					}
				}
				else
				{
					if ((m_videoram[offset] >> n) & 1)
					{
						pLine[x + n] = m_colorram[offset] & 7;
					}
				}
			}
		}

		/* next line */

		helifire_next_line();
	}

	m_helifire_mv = saved_mv;
	m_helifire_sc = saved_sc;
	return 0;
}


void n8080_state::screen_eof_helifire(screen_device &screen, bool state)
{
	// falling edge
	if (!state)
	{
		int n = (m_screen->frame_number() >> 1) % sizeof m_helifire_LSFR;

		int i;

		for (i = 0; i < 8; i++)
		{
			int R = (i & 1);
			int G = (i & 2);
			int B = (i & 4);

			if (m_helifire_flash)
			{
				if (m_helifire_LSFR[n] & 0x20)
				{
					G |= B;
				}

				if (m_screen->frame_number() & 0x04)
				{
					R |= G;
				}
			}

			m_palette->set_pen_color(i,
				R ? 255 : 0,
				G ? 255 : 0,
				B ? 255 : 0);
		}

		for (i = 0; i < 256; i++)
		{
			helifire_next_line();
		}
	}
}
