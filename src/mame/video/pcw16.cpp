// license:GPL-2.0+
// copyright-holders:Kevin Thacker
#include "emu.h"
#include "includes/pcw16.h"
#include "machine/ram.h"


#if 0
/* 16 colours, + 1 for border */
static const unsigned short pcw16_colour_table[PCW16_NUM_COLOURS] =
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
	29, 30, 31
};
#endif

static const rgb_t pcw16_palette[PCW16_NUM_COLOURS] =
{
	rgb_t(0x080, 0x080, 0x080),  /* light grey */
	rgb_t(0x080, 0x080, 0x080),  /* light grey */
	rgb_t(0x000, 0x080, 0x080),  /* magenta */
	rgb_t(0x000, 0x080, 0x080),  /* magenta */
	rgb_t(0x080, 0x080, 0x080),  /* light grey */
	rgb_t(0x080, 0x080, 0x080),  /* light grey */
	rgb_t(0x0ff, 0x080, 0x080),  /* pastel green */
	rgb_t(0x0ff, 0x080, 0x080),  /* pastel green */
	rgb_t(0x000, 0x000, 0x080),  /* blue */
	rgb_t(0x000, 0x000, 0x000),  /* black */
	rgb_t(0x000, 0x080, 0x0ff),  /* mauve */
	rgb_t(0x000, 0x000, 0x0ff),  /* bright blue */
	rgb_t(0x000, 0x080, 0x000),  /* red */
	rgb_t(0x000, 0x0ff, 0x000),  /* bright red */
	rgb_t(0x000, 0x0ff, 0x080),  /* purple */
	rgb_t(0x000, 0x0ff, 0x0ff),  /* bright magenta */
	rgb_t(0x0ff, 0x000, 0x080),  /* sea green */
	rgb_t(0x0ff, 0x000, 0x0ff),  /* bright green */
	rgb_t(0x0ff, 0x080, 0x0ff),  /* pastel cyan */
	rgb_t(0x0ff, 0x000, 0x0ff),  /* bright cyan */
	rgb_t(0x0ff, 0x080, 0x000),  /* lime green */
	rgb_t(0x0ff, 0x0ff, 0x000),  /* bright yellow */
	rgb_t(0x0ff, 0x0ff, 0x080),  /* pastel yellow */
	rgb_t(0x0ff, 0x0ff, 0x0ff),  /* bright white */
	rgb_t(0x080, 0x000, 0x080),  /* cyan */
	rgb_t(0x080, 0x000, 0x000),  /* green */
	rgb_t(0x080, 0x080, 0x0ff),  /* pastel blue */
	rgb_t(0x080, 0x000, 0x0ff),  /* sky blue */
	rgb_t(0x080, 0x080, 0x000),  /* yellow */
	rgb_t(0x080, 0x0ff, 0x000),  /* orange */
	rgb_t(0x080, 0x0ff, 0x080),  /* pink */
	rgb_t(0x080, 0x0ff, 0x0ff),  /* pastel magenta */
};


inline void pcw16_state::pcw16_plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color)
{
	bitmap.pix16(y, x) = (UINT16)color;
}

/* Initialise the palette */
PALETTE_INIT_MEMBER(pcw16_state, pcw16)
{
	palette.set_pen_colors(0, pcw16_palette, ARRAY_LENGTH(pcw16_palette));
}

void pcw16_state::video_start()
{
}

/* 640, 1 bit per pixel */
void pcw16_state::pcw16_vh_decode_mode0(bitmap_ind16 &bitmap, int x, int y, unsigned char byte)
{
	int b;
	int local_byte;
	int cols[2];
	int px;

	local_byte = byte;

	cols[0] = m_colour_palette[0];
	cols[1] = m_colour_palette[1];

	px = x;
	for (b=0; b<8; b++)
	{
		pcw16_plot_pixel(bitmap, px, y, cols[(local_byte>>7) & 0x01]);
		px++;

		local_byte = local_byte<<1;
	}
}

/* 320, 2 bits per pixel */
void pcw16_state::pcw16_vh_decode_mode1(bitmap_ind16 &bitmap, int x, int y, unsigned char byte)
{
	int b;
	int px;
	int local_byte;
	int cols[4];

	for (b=0; b<3; b++)
	{
		cols[b] = m_colour_palette[b];
	}

	local_byte = byte;

	px = x;
	for (b=0; b<4; b++)
	{
		int col;

		col = cols[((local_byte>>6) & 0x03)];

		pcw16_plot_pixel(bitmap, px, y, col);
		px++;
		pcw16_plot_pixel(bitmap, px, y, col);
		px++;

		local_byte = local_byte<<2;
	}
}

/* 160, 4 bits per pixel */
void pcw16_state::pcw16_vh_decode_mode2(bitmap_ind16 &bitmap, int x, int y, unsigned char byte)
{
	int px;
	int b;
	int local_byte;
	int cols[2];

	cols[0] = m_colour_palette[0];
	cols[1] = m_colour_palette[1];
	local_byte = byte;

	px = x;
	for (b=0; b<2; b++)
	{
		int col;

		col = cols[((local_byte>>4)&0x0f)];

		pcw16_plot_pixel(bitmap, px, y, col);
		px++;
		pcw16_plot_pixel(bitmap, px, y, col);
		px++;
		pcw16_plot_pixel(bitmap, px, y, col);
		px++;
		pcw16_plot_pixel(bitmap, px, y, col);
		px++;

		local_byte = local_byte<<4;
	}
}

/***************************************************************************
  Draw the game screen in the given bitmap_ind16.
  Do NOT call osd_update_display() from this function,
  it will be called by the main emulation engine.
***************************************************************************/
UINT32 pcw16_state::screen_update_pcw16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *ram = m_ram->pointer();
	unsigned char *pScanLine = (unsigned char *)ram + 0x0fc00;  //0x03c00;  //0x020FC00;

	int y;
	int x;

	int border_colour;

	border_colour = m_video_control & 31;

	/* reverse video? */
	if (m_video_control & (1<<7))
	{
		/* colour 0 and colour 1 need to be inverted? - what happens in mode 1 and 2 - ignored? or is bit 1 toggled,
		or is whole lot toggled? */

		/* force border to be colour 1 */
		border_colour = m_colour_palette[1];
	}

	if ((m_video_control & (1<<6))==0)
	{
		/* blank */
		rectangle rect(0, PCW16_SCREEN_WIDTH, 0, PCW16_SCREEN_HEIGHT);
		bitmap.fill(border_colour, rect);
	}
	else
	{
		/* no blank */


		/* render top border */
		rectangle rect(0, PCW16_SCREEN_WIDTH, 0, PCW16_BORDER_HEIGHT);
		bitmap.fill(border_colour, rect);

		/* render bottom border */
		rect.set(0, PCW16_SCREEN_WIDTH, PCW16_BORDER_HEIGHT + PCW16_DISPLAY_HEIGHT, PCW16_BORDER_HEIGHT + PCW16_DISPLAY_HEIGHT + PCW16_BORDER_HEIGHT);
		bitmap.fill(border_colour, rect);

		/* render border on either side of display */
		bitmap.plot_box(0,                                          PCW16_BORDER_HEIGHT, 8, PCW16_DISPLAY_HEIGHT, border_colour);
		bitmap.plot_box(PCW16_DISPLAY_WIDTH + PCW16_BORDER_WIDTH,   PCW16_BORDER_HEIGHT, 8, PCW16_DISPLAY_HEIGHT, border_colour);

		/* render display */
		for (y=0; y<PCW16_DISPLAY_HEIGHT; y++)
		{
			int b;
			int ScanLineAddr;
			int Addr;
			int AddrUpper;
			int mode;

			/* get line address */
			ScanLineAddr = (pScanLine[0] & 0x0ff) | ((pScanLine[1] & 0x0ff)<<8);

			/* generate address */
			Addr = (ScanLineAddr & 0x03fff)<<4;

			/* get upper bits of addr */
			AddrUpper = Addr & (~0x0ffff);

			/* get mode */
			mode = ((ScanLineAddr>>14) & 0x03);

			/* set initial x position */
			x = PCW16_BORDER_WIDTH;

			for (b=0; b<80; b++)
			{
				int byte;

				byte = ram[Addr];

				switch (mode)
				{
					case 0:
					{
						pcw16_vh_decode_mode0(bitmap, x, y+PCW16_BORDER_HEIGHT, byte);
					}
					break;

					case 1:
					{
						pcw16_vh_decode_mode1(bitmap, x, y+PCW16_BORDER_HEIGHT, byte);
					}
					break;

					case 3:
					case 2:
					{
						pcw16_vh_decode_mode2(bitmap, x, y+PCW16_BORDER_HEIGHT, byte);
					}
					break;
				}

				/* only lowest 16 bits are incremented between fetches */
				Addr = ((Addr+1) & 0x0ffff) | AddrUpper;

				x=x+8;
			}

			pScanLine+=2;
		}
	}
	return 0;
}
