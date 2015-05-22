// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/***************************************************************************

  timex.c

  Functions to emulate the video hardware of the Timex ZX Spectrum clones.

  Changes:

  DJR 08/02/00 - Added support for FLASH 1.
  DJR 16/05/00 - Support for TS2068/TC2048 hires and 64 column modes.
  DJR 19/05/00 - Speeded up Spectrum 128 screen refresh.
  DJR 23/05/00 - Preliminary support for border colour emulation.

***************************************************************************/

#include "emu.h"
#include "includes/spectrum.h"
#include "machine/ram.h"

inline void spectrum_state::spectrum_plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color)
{
	bitmap.pix16(y, x) = (UINT16)color;
}

/* Update FLASH status for ts2068. Assumes flash update every 1/2s. */
VIDEO_START_MEMBER(spectrum_state,ts2068)
{
	VIDEO_START_CALL_MEMBER( spectrum );
	m_frame_invert_count = 30;
}

void spectrum_state::screen_eof_timex(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		spectrum_UpdateBorderBitmap();

		m_frame_number++;

		if (m_frame_number >= m_frame_invert_count)
		{
			m_frame_number = 0;
			m_flash_invert = !m_flash_invert;
		}
	}
}


/*******************************************************************
 *
 *      Update the TS2068 display.
 *
 *      Port ff is used to set the display mode.
 *
 *      bits 2..0  Video Mode Select
 *      000 = Primary DFILE active   (at 0x4000-0x5aff)
 *      001 = Secondary DFILE active (at 0x6000-0x7aff)
 *      010 = Extended Colour Mode   (chars at 0x4000-0x57ff, colors 0x6000-0x7aff)
 *      110 = 64 column mode         (columns 0,2,4,...62 from DFILE 1
 *                                    columns 1,3,5,...63 from DFILE 2)
 *      other = unpredictable results
 *
 *      bits 5..3  64 column mode ink/paper selection (attribute value in brackets)
 *      000 = Black/White   (56)        100 = Green/Magenta (28)
 *      001 = Blue/Yellow   (49)        101 = Cyan/Red      (21)
 *      010 = Red/Cyan      (42)        110 = Yellow/Blue   (14)
 *      011 = Magenta/Green (35)        111 = White/Black   (7)
 *
 *******************************************************************/

/* Draw a scanline in TS2068/TC2048 hires mode (code modified from COUPE.C) */
void spectrum_state::ts2068_hires_scanline(bitmap_ind16 &bitmap, int y, int borderlines)
{
	int x,b,scrx,scry;
	unsigned short ink,pap;
	unsigned char *attr, *scr;

	scrx=TS2068_LEFT_BORDER;
	scry=((y&7) * 8) + ((y&0x38)>>3) + (y&0xC0);

	scr=m_ram->pointer() + y*32;
	attr=scr + 0x2000;

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
			{
				spectrum_plot_pixel(bitmap,scrx++,scry+borderlines,ink);
				spectrum_plot_pixel(bitmap,scrx++,scry+borderlines,ink);
			}
			else
			{
				spectrum_plot_pixel(bitmap,scrx++,scry+borderlines,pap);
				spectrum_plot_pixel(bitmap,scrx++,scry+borderlines,pap);
			}
		}
		scr++;
		attr++;
	}
}

/* Draw a scanline in TS2068/TC2048 64-column mode */
void spectrum_state::ts2068_64col_scanline(bitmap_ind16 &bitmap, int y, int borderlines, unsigned short inkcolor)
{
	int x,b,scrx,scry;
	unsigned char *scr1, *scr2;

	scrx=TS2068_LEFT_BORDER;
	scry=((y&7) * 8) + ((y&0x38)>>3) + (y&0xC0);

	scr1=m_ram->pointer() + y*32;
	scr2=scr1 + 0x2000;

	for (x=0;x<32;x++)
	{
		for (b=0x80;b!=0;b>>=1)
		{
			if (*scr1&b)
				spectrum_plot_pixel(bitmap,scrx++,scry+borderlines,inkcolor);
			else
				spectrum_plot_pixel(bitmap,scrx++,scry+borderlines,7-inkcolor);
		}
		scr1++;

		for (b=0x80;b!=0;b>>=1)
		{
			if (*scr2&b)
				spectrum_plot_pixel(bitmap,scrx++,scry+borderlines,inkcolor);
			else
				spectrum_plot_pixel(bitmap,scrx++,scry+borderlines,7-inkcolor);
		}
		scr2++;
	}
}

/* Draw a scanline in TS2068/TC2048 lores (normal Spectrum) mode */
void spectrum_state::ts2068_lores_scanline(bitmap_ind16 &bitmap, int y, int borderlines, int screen)
{
	int x,b,scrx,scry;
	unsigned short ink,pap;
	unsigned char *attr, *scr;

	scrx=TS2068_LEFT_BORDER;
	scry=((y&7) * 8) + ((y&0x38)>>3) + (y&0xC0);

	scr = m_ram->pointer() + y*32 + screen*0x2000;
	attr = m_ram->pointer() + ((scry>>3)*32) + screen*0x2000 + 0x1800;

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
			{
				spectrum_plot_pixel(bitmap,scrx++,scry+borderlines,ink);
				spectrum_plot_pixel(bitmap,scrx++,scry+borderlines,ink);
			}
			else
			{
				spectrum_plot_pixel(bitmap,scrx++,scry+borderlines,pap);
				spectrum_plot_pixel(bitmap,scrx++,scry+borderlines,pap);
			}
		}
		scr++;
		attr++;
	}
}

UINT32 spectrum_state::screen_update_ts2068(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* for now TS2068 will do a full-refresh */
	int count;

	if (m_border_bitmap.valid())
		copyscrollbitmap(bitmap, m_border_bitmap, 0, 0, 0, 0, cliprect);

	if ((m_port_ff_data & 7) == 6)
	{
		/* 64 Column mode */
		unsigned short inkcolor = (m_port_ff_data & 0x38) >> 3;
		for (count = 0; count < 192; count++)
			ts2068_64col_scanline(bitmap, count, TS2068_TOP_BORDER, inkcolor);
	}
	else if ((m_port_ff_data & 7) == 2)
	{
		/* Extended Color mode */
		for (count = 0; count < 192; count++)
			ts2068_hires_scanline(bitmap, count, TS2068_TOP_BORDER);
	}
	else if ((m_port_ff_data & 7) == 1)
	{
		/* Screen 6000-7aff */
		for (count = 0; count < 192; count++)
			ts2068_lores_scanline(bitmap, count, TS2068_TOP_BORDER, 1);
	}
	else
	{
		/* Screen 4000-5aff */
		for (count = 0; count < 192; count++)
			ts2068_lores_scanline(bitmap, count, TS2068_TOP_BORDER, 0);
	}

	return 0;
}

UINT32 spectrum_state::screen_update_tc2048(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* for now TS2068 will do a full-refresh */
	int count;

	if (m_border_bitmap.valid())
		copyscrollbitmap(bitmap, m_border_bitmap, 0, 0, 0, 0, cliprect);

	if ((m_port_ff_data & 7) == 6)
	{
		/* 64 Column mode */
		unsigned short inkcolor = (m_port_ff_data & 0x38) >> 3;
		for (count = 0; count < 192; count++)
			ts2068_64col_scanline(bitmap, count, SPEC_TOP_BORDER, inkcolor);
	}
	else if ((m_port_ff_data & 7) == 2)
	{
		/* Extended Color mode */
		for (count = 0; count < 192; count++)
			ts2068_hires_scanline(bitmap, count, SPEC_TOP_BORDER);
	}
	else if ((m_port_ff_data & 7) == 1)
	{
		/* Screen 6000-7aff */
		for (count = 0; count < 192; count++)
			ts2068_lores_scanline(bitmap, count, SPEC_TOP_BORDER, 1);
	}
	else
	{
		/* Screen 4000-5aff */
		for (count = 0; count < 192; count++)
			ts2068_lores_scanline(bitmap, count, SPEC_TOP_BORDER, 0);
	}

	return 0;
}
