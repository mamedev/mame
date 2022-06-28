// license:GPL-2.0+
// copyright-holders:Wilbert Pol, Kevin Thacker
/***************************************************************************

  nc.c

  Functions to emulate the video hardware of the Amstrad PCW.

***************************************************************************/

#include "emu.h"
#include "nc.h"
#include "machine/ram.h"

/***************************************************************************
  Start the video hardware emulation.
***************************************************************************/

void nc_state::video_start()
{
}

#if 0
/* two colours */
static const unsigned short nc_colour_table[NC_NUM_COLOURS] =
{
	0, 1,2,3
};
#endif

/* black/white */
static const rgb_t nc_palette[NC_NUM_COLOURS] =
{
	rgb_t(0x060, 0x060, 0x060),
	rgb_t(0x000, 0x000, 0x000),
	rgb_t(0x080, 0x0a0, 0x060),
	rgb_t(0x000, 0x000, 0x000)
};


/* Initialise the palette */
void nc_state::nc_colours(palette_device &palette) const
{
	palette.set_pen_colors(0, nc_palette);
}


void nc200_state::nc200_video_set_backlight(int state)
{
	m_nc200_backlight = state;
}


/***************************************************************************
  Draw the game screen in the given bitmap_ind16.
  Do NOT call osd_update_display() from this function,
  it will be called by the main emulation engine.
***************************************************************************/
uint32_t nc_state::screen_update_nc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int height, int width, int const (&pens)[2])
{
	for (int y = 0; y < height; y++)
	{
		/* 64 bytes per line */
		char const *line_ptr = ((char const *)m_ram->pointer()) + m_display_memory_start + (y<<6);

		for (int x = 0, by = 0; by < (width >> 3); by++)
		{
			unsigned char byte = line_ptr[0];

			for (int b = 0; b < 8; b++)
			{
				bitmap.pix(y, x) = pens[(byte>>7) & 0x01];
				byte = byte<<1;
				x++;
			}

			line_ptr++;
		}
	}
	return 0;
}


uint32_t nc100_state::screen_update_nc100(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const pens[]{ 2, 3 };
	return screen_update_nc(screen, bitmap, cliprect, NC_SCREEN_HEIGHT, NC_SCREEN_WIDTH, pens);
}


uint32_t nc200_state::screen_update_nc200(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const pens[]{ m_nc200_backlight ? 2 : 0, m_nc200_backlight ? 3 : 1 };
	return screen_update_nc(screen, bitmap, cliprect, NC200_SCREEN_HEIGHT, NC200_SCREEN_WIDTH, pens);
}
