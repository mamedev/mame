// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

  video/advision.c

  Routines to control the Adventurevision video hardware

  Video hardware is composed of a vertical array of 40 LEDs which is
  reflected off a spinning mirror, to give a resolution of 150 x 40 at 15 FPS.

***************************************************************************/

#include "emu.h"
#include "includes/advision.h"

#include <algorithm>


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void advision_state::video_start()
{
	m_video_hpos = 0;
	m_display = std::make_unique<uint8_t []>(8 * 8 * 256);
	std::fill_n(m_display.get(), 8 * 8 * 256, 0);
	save_pointer(NAME(m_display), 8 * 8 * 256);
	save_item(NAME(m_video_hpos));
}

/***************************************************************************

  Initialise the palette.

***************************************************************************/

void advision_state::advision_palette(palette_device &palette) const
{
	// 8 shades of RED
	for (int i = 0; i < 8; i++)
		m_palette->set_pen_color(i, pal3bit(i), 0x00, 0x00);
}

/***************************************************************************

  Update the display data.

***************************************************************************/

void advision_state::vh_write(int data)
{
	if (m_video_bank >= 1 && m_video_bank <=5)
		m_led_latch[m_video_bank] = data;
}

void advision_state::vh_update(int x)
{
	uint8_t *dst = &m_display[x];

	for (int y = 0; y < 8; y++)
	{
		uint8_t data = m_led_latch[7 - y];

		for (int i = 0; i < 8; i++)
		{
			if (!BIT(data, 7 - i))
				dst[i * 256] = 8;
		}

		m_led_latch[7 - y] = 0xff;
		dst += 8 * 256;
	}
}


/***************************************************************************

  Refresh the video screen

***************************************************************************/

uint32_t advision_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if ((m_frame_count++ % 4) == 0)
	{
		m_frame_start = 1;
		m_video_hpos = 0;
	}

	for (int x = 0; x < 150; x++)
	{
		uint8_t *led = &m_display[x];

		for (int y = 0; y < 128; y+=2)
		{
			if (*led > 0)
				bitmap.pix16(30 + y, 85 + x) = --(*led);
			else
				bitmap.pix16(30 + y, 85 + x) = 0;

			led += 256;
		}
	}
	return 0;
}
