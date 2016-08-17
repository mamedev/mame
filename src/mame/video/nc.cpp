// license:GPL-2.0+
// copyright-holders:Wilbert Pol, Kevin Thacker
/***************************************************************************

  nc.c

  Functions to emulate the video hardware of the Amstrad PCW.

***************************************************************************/

#include "emu.h"
#include "includes/nc.h"
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
PALETTE_INIT_MEMBER(nc_state, nc)
{
	palette.set_pen_colors(0, nc_palette, ARRAY_LENGTH(nc_palette));
}


void nc_state::nc200_video_set_backlight(int state)
{
	m_nc200_backlight = state;
}


/***************************************************************************
  Draw the game screen in the given bitmap_ind16.
  Do NOT call osd_update_display() from this function,
  it will be called by the main emulation engine.
***************************************************************************/
UINT32 nc_state::screen_update_nc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y;
	int b;
	int x;
	int height, width;
	int pens[2];

	if (m_type==NC_TYPE_200)
	{
		height = NC200_SCREEN_HEIGHT;
		width = NC200_SCREEN_WIDTH;

		if (m_nc200_backlight)
		{
			pens[0] = 2;
			pens[1] = 3;
		}
		else
		{
			pens[0] = 0;
			pens[1] = 1;
		}
	}
	else
	{
		height = NC_SCREEN_HEIGHT;
		width = NC_SCREEN_WIDTH;
		pens[0] = 2;
		pens[1] = 3;
	}


	for (y=0; y<height; y++)
	{
		int by;
		/* 64 bytes per line */
		char *line_ptr = ((char*)m_ram->pointer()) + m_display_memory_start + (y<<6);

		x = 0;
		for (by=0; by<width>>3; by++)
		{
			int px;
			unsigned char byte;

			byte = line_ptr[0];

			px = x;
			for (b=0; b<8; b++)
			{
				bitmap.pix16(y, px) = pens[(byte>>7) & 0x01];
				byte = byte<<1;
				px++;
			}

			x = px;

			line_ptr = line_ptr+1;
		}
	}
	return 0;
}
