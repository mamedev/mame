// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/***************************************************************************

  pcw.cpp

  Functions to emulate the video hardware of the Amstrad PCW.

***************************************************************************/

#include "emu.h"
#include "pcw.h"
#include "machine/ram.h"

#define LOG_PALETTE  (1U <<  1) // LOGs what palette is choosen
#define LOG_VOFF     (1U <<  2) // LOGs when video is OFF

//#define VERBOSE (LOG_PALETTE)
//#define LOG_OUTPUT_FUNC printf

#include "logmacro.h"

#define LOGPALETTE(...) LOGMASKED(LOG_PALETTE, __VA_ARGS__)
#define LOGVOFF(...)    LOGMASKED(LOG_VOFF,    __VA_ARGS__)

inline void pcw_state::pcw_plot_pixel(bitmap_ind16 &bitmap, int x, int y, uint32_t color)
{
	bitmap.pix(y, x) = (uint16_t)color;
}

/***************************************************************************
  Start the video hardware emulation.
***************************************************************************/

void pcw_state::video_start()
{
	rectangle rect(0, PCW_PRINTER_WIDTH - 1, 0, PCW_PRINTER_HEIGHT - 1);

	m_prn_output = std::make_unique<bitmap_ind16>(PCW_PRINTER_WIDTH,PCW_PRINTER_HEIGHT);
	m_prn_output->fill(1, rect);

	m_roller_ram_addr = 0;
}

/* black/white printer */
static const rgb_t pcw_printer_palette[PCW_NUM_COLOURS] =
{
	rgb_t(0x000, 0x000, 0x000),
	rgb_t(0x0ff, 0x0ff, 0x0ff)
};

/* black/white */
static const rgb_t pcw_9xxx_palette[PCW_NUM_COLOURS] =
{
	rgb_t(0x000, 0x000, 0x000),
	rgb_t(0x0ff, 0x0ff, 0x0ff)
};

/* black/green */
static const rgb_t pcw_8xxx_palette[PCW_NUM_COLOURS] =
{
	rgb_t(0x000, 0x000, 0x000),
	rgb_t(0x04a, 0x0ff, 0x000)
};

/* Initialise the palette */
void pcw_state::set_8xxx_palette(palette_device &palette) const
{
	LOGPALETTE("Choosing green on white palette for CRT\n");
	palette.set_pen_colors(0, pcw_8xxx_palette);
}

void pcw_state::set_9xxx_palette(palette_device &palette) const
{
	LOGPALETTE("Choosing black on white palette for CRT\n");
	palette.set_pen_colors(0, pcw_9xxx_palette);
}

void pcw_state::set_printer_palette(palette_device &palette) const
{
	LOGPALETTE("Choosing black on white palette for printer\n");
	palette.set_pen_colors(0, pcw_printer_palette);
}

/***************************************************************************
  Draw the game screen in the given bitmap_ind16.
  Do NOT call osd_update_display() from this function,
  it will be called by the main emulation engine.
***************************************************************************/
uint32_t pcw_state::screen_update_pcw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,b;
	unsigned short roller_ram_offs;
	unsigned char *roller_ram_ptr;
	int pen0,pen1;

	pen0 = 0;
	pen1 = 1;

	/* invert? */
	if (m_vdu_video_control_register & (1<<7))
	{
		/* yes */
		pen1^=1;
		pen0^=1;
	}

	/* video enable? */
	if ((m_vdu_video_control_register & (1<<6))!=0)
	{
		/* render top border */
		rectangle rect(0, PCW_SCREEN_WIDTH, 0, PCW_BORDER_HEIGHT);
		bitmap.fill(pen0, rect);

		/* render bottom border */
		rect.set(0, PCW_SCREEN_WIDTH, PCW_BORDER_HEIGHT + PCW_DISPLAY_HEIGHT, PCW_BORDER_HEIGHT + PCW_DISPLAY_HEIGHT + PCW_BORDER_HEIGHT);
		bitmap.fill(pen0, rect);

		/* offset to start in table */
		roller_ram_offs = (m_roller_ram_offset<<1);

		for (y=0; y<256; y++)
		{
			int by;
			unsigned short line_data;
			unsigned char *line_ptr;

			// The PCWs are reportedly slowed 15% by the video circuits inserting WAIT states while accessing the vram
			// steal clock cycles from the CPU: (4.000.000 * 0.15) / 50Hz / 256 scanlines == ~47 (46.88)
			m_maincpu->adjust_icount(-47);

			x = PCW_BORDER_WIDTH;

			roller_ram_ptr = m_ram->pointer() + m_roller_ram_addr + roller_ram_offs;

			/* get line address */
			/* b16-14 control which bank the line is to be found in, b13-3 the address in the bank (in 16-byte units),
			   and b2-0 the offset. Thus a roller RAM address bbbxxxxxxxxxxxyyy indicates bank bbb, address 00xxxxxxxxxxx0yyy. */
			line_data = ((unsigned char *)roller_ram_ptr)[0] | (((unsigned char *)roller_ram_ptr)[1]<<8);

			/* calculate address of pixel data */
			line_ptr = m_ram->pointer() + ((line_data & 0x0e000)<<1) + ((line_data & 0x01ff8)<<1) + (line_data & 0x07);

			for (by=0; by<90; by++)
			{
				unsigned char byte;

				byte = line_ptr[0];

				for (b=0; b<8; b++)
				{
					if (byte & 0x080)
					{
						pcw_plot_pixel(bitmap,x+b, y+PCW_BORDER_HEIGHT, pen1);
					}
					else
					{
						pcw_plot_pixel(bitmap,x+b, y+PCW_BORDER_HEIGHT, pen0);

					}
					byte = byte<<1;
				}

				x = x + 8;


				line_ptr = line_ptr+8;
			}

			/* update offset, wrap within 512 byte range */
			roller_ram_offs+=2;
			roller_ram_offs&=511;

		}

		/* render border */
		/* 8 pixels either side of display */
		for (y=0; y<256; y++)
		{
			pcw_plot_pixel(bitmap, 0, y+PCW_BORDER_HEIGHT, pen0);
			pcw_plot_pixel(bitmap, 1, y+PCW_BORDER_HEIGHT, pen0);
			pcw_plot_pixel(bitmap, 2, y+PCW_BORDER_HEIGHT, pen0);
			pcw_plot_pixel(bitmap, 3, y+PCW_BORDER_HEIGHT, pen0);
			pcw_plot_pixel(bitmap, 4, y+PCW_BORDER_HEIGHT, pen0);
			pcw_plot_pixel(bitmap, 5, y+PCW_BORDER_HEIGHT, pen0);
			pcw_plot_pixel(bitmap, 6, y+PCW_BORDER_HEIGHT, pen0);
			pcw_plot_pixel(bitmap, 7, y+PCW_BORDER_HEIGHT, pen0);

			pcw_plot_pixel(bitmap, PCW_BORDER_WIDTH+PCW_DISPLAY_WIDTH+0, y+PCW_BORDER_HEIGHT, pen0);
			pcw_plot_pixel(bitmap, PCW_BORDER_WIDTH+PCW_DISPLAY_WIDTH+1, y+PCW_BORDER_HEIGHT, pen0);
			pcw_plot_pixel(bitmap, PCW_BORDER_WIDTH+PCW_DISPLAY_WIDTH+2, y+PCW_BORDER_HEIGHT, pen0);
			pcw_plot_pixel(bitmap, PCW_BORDER_WIDTH+PCW_DISPLAY_WIDTH+3, y+PCW_BORDER_HEIGHT, pen0);
			pcw_plot_pixel(bitmap, PCW_BORDER_WIDTH+PCW_DISPLAY_WIDTH+4, y+PCW_BORDER_HEIGHT, pen0);
			pcw_plot_pixel(bitmap, PCW_BORDER_WIDTH+PCW_DISPLAY_WIDTH+5, y+PCW_BORDER_HEIGHT, pen0);
			pcw_plot_pixel(bitmap, PCW_BORDER_WIDTH+PCW_DISPLAY_WIDTH+6, y+PCW_BORDER_HEIGHT, pen0);
			pcw_plot_pixel(bitmap, PCW_BORDER_WIDTH+PCW_DISPLAY_WIDTH+7, y+PCW_BORDER_HEIGHT, pen0);
		}
	}
	else
	{
		LOGVOFF("Video not enabled\n");
		/* not video - render whole lot in pen 0 */
		rectangle rect(0, PCW_SCREEN_WIDTH, 0, PCW_SCREEN_HEIGHT);
		bitmap.fill(pen1, rect);
	}
	return 0;
}

uint32_t pcw_state::screen_update_pcw_printer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// printer output
	int32_t feed;
	rectangle rect(0, PCW_PRINTER_WIDTH - 1, 0, PCW_PRINTER_HEIGHT - 1);
	feed = -(m_paper_feed / 2);
	copyscrollbitmap(bitmap,*m_prn_output,0,nullptr,1,&feed,rect);
	bitmap.pix(PCW_PRINTER_HEIGHT-1, m_printer_headpos) = 0;
	bitmap.pix(PCW_PRINTER_HEIGHT-2, m_printer_headpos) = 0;
	bitmap.pix(PCW_PRINTER_HEIGHT-3, m_printer_headpos) = 0;
	bitmap.pix(PCW_PRINTER_HEIGHT-1, m_printer_headpos-1) = 0;
	bitmap.pix(PCW_PRINTER_HEIGHT-2, m_printer_headpos-1) = 0;
	bitmap.pix(PCW_PRINTER_HEIGHT-1, m_printer_headpos+1) = 0;
	bitmap.pix(PCW_PRINTER_HEIGHT-2, m_printer_headpos+1) = 0;
	return 0;
}
