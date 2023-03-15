// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/***************************************************************************

  Nintendo 8080 video emulation

***************************************************************************/

#include "emu.h"
#include "n8080.h"


void n8080_state::n8080_video_control_w(uint8_t data)
{
	m_sheriff_color_mode = (data >> 3) & 3;
	m_sheriff_color_data = (data >> 0) & 7;
	flip_screen_set(data & 0x20);
}


void n8080_state::n8080_palette(palette_device &palette) const
{
	for (int i = 0; i < 8; i++)
		palette.set_pen_color(i, pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
}


void helifire_state::helifire_palette(palette_device &palette) const
{
	n8080_palette(palette);

	for (int i = 0; i < 0x100; i++)
	{
		int const level = 0xff * exp(-3 * i / 255.); // capacitor discharge

		palette.set_pen_color(0x000 + 8 + i, rgb_t(0x00, 0x00, level)); // shades of blue
		palette.set_pen_color(0x100 + 8 + i, rgb_t(0x00, 0xc0, level)); // shades of blue w/ green star

		palette.set_pen_color(0x200 + 8 + i, rgb_t(level, 0x00, 0x00)); // shades of red
		palette.set_pen_color(0x300 + 8 + i, rgb_t(level, 0xc0, 0x00)); // shades of red w/ green star
	}
}


void spacefev_state::start_red_cannon()
{
	m_red_cannon = 1;
	m_cannon_timer->adjust(attotime::from_usec(550 * 68 * 10));
}


TIMER_CALLBACK_MEMBER(spacefev_state::stop_red_cannon)
{
	m_red_cannon = 0;
	m_cannon_timer->adjust(attotime::never);
}


void helifire_state::next_line()
{
	m_mv++;

	if (m_sc % 4 == 2)
	{
		m_mv %= 256;
	}
	else
	{
		if (flip_screen())
			m_mv %= 255;
		else
			m_mv %= 257;
	}

	if (m_mv == 128)
	{
		m_sc++;
	}
}


void spacefev_state::video_start()
{
	m_cannon_timer = timer_alloc(FUNC(spacefev_state::stop_red_cannon), this);

	flip_screen_set(0);

	save_item(NAME(m_red_screen));
	save_item(NAME(m_red_cannon));
}


void sheriff_state::video_start()
{
	flip_screen_set(0);

	save_item(NAME(m_sheriff_color_mode));
	save_item(NAME(m_sheriff_color_data));
}


void helifire_state::video_start()
{
	uint8_t data = 0;
	int i;

	save_item(NAME(m_mv));
	save_item(NAME(m_sc));
	save_item(NAME(m_flash));
	save_item(NAME(m_LSFR));

	for (i = 0; i < 63; i++)
	{
		int bit =
			(data >> 6) ^
			(data >> 7) ^ 1;

		data = (data << 1) | (bit & 1);

		m_LSFR[i] = data;
	}

	flip_screen_set(0);
}


uint32_t spacefev_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const bool mono = bool(m_video_conf->read());
	uint8_t mask = flip_screen() ? 0xff : 0x00;

	uint8_t const *pRAM = m_videoram;
	uint8_t const *const pPROM = m_prom->base();

	for (int y = 0; y < 256; y++)
	{
		uint16_t *const pLine = &bitmap.pix(y ^ mask);

		for (int x = 0; x < 256; x += 8)
		{
			uint8_t color = 0;

			if (m_red_screen)
				color = 1;
			else
			{
				uint8_t val = pPROM[x >> 3];

				if ((x >> 3) == 0x06)
				{
					color = m_red_cannon ? 1 : 7;
				}

				if ((x >> 3) == 0x1b)
				{
					static const uint8_t ufo_color[] =
					{
						1, // red
						2, // green
						7, // white
						3, // yellow
						5, // magenta
						6, // cyan
					};

					int cycle = screen.frame_number() / 32;

					color = ufo_color[cycle % 6];
				}

				for (int n = color + 1; n < 8; n++)
				{
					if (~val & (1 << n))
					{
						color = n;
					}
				}
			}

			if (mono)
				color = 7; // force B&W here

			for (int n = 0; n < 8; n++)
			{
				pLine[(x + n) ^ mask] = (pRAM[x >> 3] & (1 << n)) ? color : 0;
			}
		}

		pRAM += 32;
	}
	return 0;
}


uint32_t sheriff_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t mask = flip_screen() ? 0xff : 0x00;

	uint8_t const *pRAM = m_videoram;
	uint8_t const *const pPROM = m_prom->base();

	for (int y = 0; y < 256; y++)
	{
		uint16_t *const pLine = &bitmap.pix(y ^ mask);

		for (int x = 0; x < 256; x += 8)
		{
			uint8_t color = pPROM[32 * (y >> 3) + (x >> 3)];

			if (m_sheriff_color_mode == 1 && !(color & 8))
				color = m_sheriff_color_data ^ 7;

			if (m_sheriff_color_mode == 2)
				color = m_sheriff_color_data ^ 7;

			if (m_sheriff_color_mode == 3)
				color = 7;

			for (int n = 0; n < 8; n++)
			{
				pLine[(x + n) ^ mask] = ((pRAM[x >> 3] >> n) & 1) ? (color & 7) : 0;
			}
		}

		pRAM += 32;
	}
	return 0;
}


uint32_t helifire_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const int SUN_BRIGHTNESS = m_pot[0]->read();
	const int SEA_BRIGHTNESS = m_pot[1]->read();

	static const int wave[8] = { 0, 1, 2, 2, 2, 1, 0, 0 };

	unsigned saved_mv = m_mv;
	unsigned saved_sc = m_sc;

	for (int y = 0; y < 256; y++)
	{
		uint16_t *const pLine = &bitmap.pix(y);

		int level = 120 + wave[m_mv & 7];

		// draw sky
		for (int x = level; x < 256; x++)
		{
			pLine[x] = 0x200 + 8 + SUN_BRIGHTNESS + x - level;
		}

		// draw stars
		if (m_mv % 8 == 4) // upper half
		{
			int step = (320 * (m_mv - 0)) % sizeof m_LSFR;

			int data =
				((m_LSFR[step] & 1) << 6) |
				((m_LSFR[step] & 2) << 4) |
				((m_LSFR[step] & 4) << 2) |
				((m_LSFR[step] & 8) << 0);

			pLine[0x80 + data] |= 0x100;
		}

		if (m_mv % 8 == 5) // lower half
		{
			int step = (320 * (m_mv - 1)) % sizeof m_LSFR;

			int data =
				((m_LSFR[step] & 1) << 6) |
				((m_LSFR[step] & 2) << 4) |
				((m_LSFR[step] & 4) << 2) |
				((m_LSFR[step] & 8) << 0);

			pLine[0x00 + data] |= 0x100;
		}

		// draw sea
		for (int x = 0; x < level; x++)
		{
			pLine[x] = 8 + SEA_BRIGHTNESS + x;
		}

		// draw foreground
		for (int x = 0; x < 256; x += 8)
		{
			int offset = 32 * y + (x >> 3);

			for (int n = 0; n < 8; n++)
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

		// next line
		next_line();
	}

	m_mv = saved_mv;
	m_sc = saved_sc;
	return 0;
}


WRITE_LINE_MEMBER(helifire_state::screen_vblank)
{
	// falling edge
	if (!state)
	{
		int n = (m_screen->frame_number() >> 1) % sizeof m_LSFR;

		int i;

		for (i = 0; i < 8; i++)
		{
			int R = (i & 1);
			int G = (i & 2);
			int B = (i & 4);

			if (m_flash)
			{
				if (m_LSFR[n] & 0x20)
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
			next_line();
		}
	}
}
