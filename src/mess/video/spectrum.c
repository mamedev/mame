/***************************************************************************

  spectrum.c

  Functions to emulate the video hardware of the ZX Spectrum.

  Changes:

  DJR 08/02/00 - Added support for FLASH 1.
  DJR 16/05/00 - Support for TS2068/TC2048 hires and 64 column modes.
  DJR 19/05/00 - Speeded up Spectrum 128 screen refresh.
  DJR 23/05/00 - Preliminary support for border colour emulation.

***************************************************************************/

#include "emu.h"
#include "includes/spectrum.h"

/***************************************************************************
  Start the video hardware emulation.
***************************************************************************/
VIDEO_START_MEMBER(spectrum_state,spectrum)
{
	m_frame_invert_count = 25;
	m_frame_number = 0;
	m_flash_invert = 0;

	m_previous_border_x = 0; m_previous_border_y = 0;
	machine().primary_screen->register_screen_bitmap(m_border_bitmap);

	m_screen_location = m_video_ram;
}

VIDEO_START_MEMBER(spectrum_state,spectrum_128)
{
	m_frame_invert_count = 25;
	m_frame_number = 0;
	m_flash_invert = 0;

	m_previous_border_x = 0; m_previous_border_y = 0;
	machine().primary_screen->register_screen_bitmap(m_border_bitmap);
}


/* return the color to be used inverting FLASHing colors if necessary */
INLINE unsigned char get_display_color (unsigned char color, int invert)
{
	if (invert && (color & 0x80))
		return (color & 0xc0) + ((color & 0x38) >> 3) + ((color & 0x07) << 3);
	else
		return color;
}

/* Code to change the FLASH status every 25 frames. Note this must be
   independent of frame skip etc. */
void spectrum_state::screen_eof_spectrum(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		m_frame_number++;
		
		if (m_frame_number >= m_frame_invert_count)
		{
			m_frame_number = 0;
			m_flash_invert = !m_flash_invert;
		}


		spectrum_UpdateBorderBitmap(machine());
	}
}



/***************************************************************************
  Update the spectrum screen display.

  The screen consists of 312 scanlines as follows:
  64  border lines (the last 48 are actual border lines; the others may be
                    border lines or vertical retrace)
  192 screen lines
  56  border lines

  Each screen line has 48 left border pixels, 256 screen pixels and 48 right
  border pixels.

  Each scanline takes 224 T-states divided as follows:
  128 Screen (reads a screen and ATTR byte [8 pixels] every 4 T states)
  24  Right border
  48  Horizontal retrace
  24  Left border

  The 128K Spectrums have only 63 scanlines before the TV picture (311 total)
  and take 228 T-states per scanline.

***************************************************************************/

INLINE void spectrum_plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color)
{
	bitmap.pix16(y, x) = (UINT16)color;
}

UINT32 spectrum_state::screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// note, don't update borders in here, this can time travel w/regards to other timers and may end up giving you
	// screen positions earlier than the last write handler gave you

	/* for now do a full-refresh */
	int x, y, b, scrx, scry;
	unsigned short ink, pap;
	unsigned char *attr, *scr;
	//	int full_refresh = 1;

	if (m_border_bitmap.valid())
		copyscrollbitmap(bitmap, m_border_bitmap, 0, 0, 0, 0, cliprect);

	scr=m_screen_location;

	for (y=0; y<192; y++)
	{
		scrx=SPEC_LEFT_BORDER;
		scry=((y&7) * 8) + ((y&0x38)>>3) + (y&0xC0);
		attr=m_screen_location + ((scry>>3)*32) + 0x1800;

		for (x=0;x<32;x++)
		{
			/* Get ink and paper colour with bright */
			if (m_flash_invert && (*attr & 0x80))
			{
				ink=((*attr)>>3) & 0x0f;
				pap=((*attr) & 0x07) + (((*attr)>>3) & 0x08);
			}
			else
			{
				ink=((*attr) & 0x07) + (((*attr)>>3) & 0x08);
				pap=((*attr)>>3) & 0x0f;
			}

			for (b=0x80;b!=0;b>>=1)
			{
				if (*scr&b)
					spectrum_plot_pixel(bitmap,scrx++,SPEC_TOP_BORDER+scry,ink);
				else
					spectrum_plot_pixel(bitmap,scrx++,SPEC_TOP_BORDER+scry,pap);
			}

			scr++;
			attr++;
		}
	}

	return 0;
}


static const rgb_t spectrum_palette[16] = {
	MAKE_RGB(0x00, 0x00, 0x00),
	MAKE_RGB(0x00, 0x00, 0xbf),
	MAKE_RGB(0xbf, 0x00, 0x00),
	MAKE_RGB(0xbf, 0x00, 0xbf),
	MAKE_RGB(0x00, 0xbf, 0x00),
	MAKE_RGB(0x00, 0xbf, 0xbf),
	MAKE_RGB(0xbf, 0xbf, 0x00),
	MAKE_RGB(0xbf, 0xbf, 0xbf),
	MAKE_RGB(0x00, 0x00, 0x00),
	MAKE_RGB(0x00, 0x00, 0xff),
	MAKE_RGB(0xff, 0x00, 0x00),
	MAKE_RGB(0xff, 0x00, 0xff),
	MAKE_RGB(0x00, 0xff, 0x00),
	MAKE_RGB(0x00, 0xff, 0xff),
	MAKE_RGB(0xff, 0xff, 0x00),
	MAKE_RGB(0xff, 0xff, 0xff)
};
/* Initialise the palette */
PALETTE_INIT_MEMBER(spectrum_state,spectrum)
{
	palette_set_colors(machine(), 0, spectrum_palette, ARRAY_LENGTH(spectrum_palette));
}


/* The code below is just a per-pixel 'partial update' for the border */

INLINE void spectrum_GetNextPos(running_machine &machine, unsigned int &x, unsigned int &y)
{
	spectrum_state *state = machine.driver_data<spectrum_state>();
	int width = state->m_border_bitmap.width();
	int height = state->m_border_bitmap.height();

	x += 1;

	if (x>width)
	{
		x = 0;
		y += 1;

		if (y>height)
		{
			y = 0;
		}
	}
}


void spectrum_UpdateBorderBitmap(running_machine &machine)
{
	spectrum_state *state = machine.driver_data<spectrum_state>();
	unsigned int x = machine.primary_screen->hpos();
	unsigned int y = machine.primary_screen->vpos();
	int width = state->m_border_bitmap.width();
	int height = state->m_border_bitmap.height();


	if (state->m_border_bitmap.valid())
	{
		int colour = state->m_port_fe_data & 0x07;

		//printf("update border from %d,%d to %d,%d\n", state->m_previous_border_x, state->m_previous_border_y, x, y);

		do
		{
			if (state->m_previous_border_y < height)
			{
				UINT16* bm = &state->m_border_bitmap.pix16(state->m_previous_border_y);

				if (state->m_previous_border_x < width)
					bm[state->m_previous_border_x] = colour;	
			}

			spectrum_GetNextPos(machine, state->m_previous_border_x, state->m_previous_border_y);
		}
		while (!((state->m_previous_border_x == x) && (state->m_previous_border_y == y)));

	}
	else
	{
		// no border bitmap allocated? fatalerror?
	}


}