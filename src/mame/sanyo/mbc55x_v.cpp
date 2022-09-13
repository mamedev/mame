// license:BSD-3-Clause
// copyright-holders:Phill Harvey-Smith
/*
    video/mbc55x.c

    Machine driver for the Sanyo MBC-550 and MBC-555.

    Phill Harvey-Smith
    2011-01-29.

Taken from : http://www.seasip.info/VintagePC/sanyo.html

Video Controller

To a programmer, the MBC's video hardware appears as a 6845 chip and three bitmapped graphics planes.
The 6845 appears at I/O ports 30h (register select) and 32h (data). At system boot, it is programmed
for 25 lines x 72 columns. The RAM BIOS then reprograms it for 25 x 80. The ROM also provides timings
for what appears to be a different 80-column mode; it decides which to use by reading port 1Ch.
If bit 7 of the result is 1, the 72-column mode is used; otherwise, the 80-column mode is.

Here are the values written to the 6845 in each case, plus (for comparison) the values used by a real CGA:

    6845 Register            | IO.SYS | ROM 80 | ROM 72 | Real CGA
=============================+========+========+========+==========
    Horizontal total         |  112   |  101   |  112   |    83
    Horizontal display end   |   80   |   80   |   72   |    80
    Horizontal sync pos      |   89   |   83   |   85   |    81
    Horizontal sync width    |   72   |   72   |   74   |     1
    Vertical total           |   65   |  105   |   65   |    26
    Vertical total adjust    |    0   |    2   |    0   |     0
    Vertical displayed       |   50   |  100   |   50   |    26
    Vertical sync position   |   56   |  100   |   56   |    25
    Interlace                |    0   |    0   |    0   |     2
    Max scan address         |    3   |    3   |    3   |     7
    Cursor start             |    0   |    0   |    0   |     6
    Cursor end               |    0   |    0   |    0   |     7
=============================+========+========+========+==========

The important thing to note here is that from the 6845's point of view, a character is 4 lines high.
This explains why the framebuffer memory is mapped as it is.

The framebuffers

The MBC video RAM is composed of three planes - green, red and blue. The green plane occupies main memory,
and its position varies; writes to port 10h set its address:

Value | Address
======+========
    4 | 0C000h
    5 | 1C000h     (other values have not been tested)
    6 | 2C000h
    7 | 3C000h
======+========

The red and blue planes appear to have fixed locations of F0000h and F4000h respectively.

When output goes to a composite monitor, the green plane is usually used by itself.
The red plane becomes "blink", causing pixels in it to blink; and the blue plane becomes "bright".

Within each plane, memory is organised as 50 rows of 320 bytes (288 bytes in 72-column mode).
This corresponds to a rectangle, 640 (576) pixels wide and four pixels high. The first four bytes
give the leftmost column of the rectangle, the next four give the next column, and so on:

[--byte 0--] [--byte 4--] [--byte  8--] [--byte 12--] ...
[--byte 1--] [--byte 5--] [--byte  9--] [--byte 13--] ...
[--byte 2--] [--byte 6--] [--byte 10--] [--byte 14--] ...
[--byte 3--] [--byte 7--] [--byte 11--] [--byte 15--] ...

*************************************************************************************************************/

#include "emu.h"

#include "mbc55x.h"

MC6845_UPDATE_ROW( mbc55x_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

	uint8_t const *const ram = &m_ram->pointer()[0];
	uint8_t const *const red = &m_video_mem[RED_PLANE_OFFSET];
	uint8_t const *const blue = &m_video_mem[BLUE_PLANE_OFFSET];
	uint8_t const *green;

	switch(m_vram_page)
	{
		case 4  : green=&ram[0x08000]; break;
		case 5  : green=&ram[0x1C000]; break;
		case 6  : green=&ram[0x2C000]; break;
		case 7  : green=&ram[0x3C000]; break;
		default :
			green=&ram[0x0C000];
	}

	int offset=((ma*4) + ra) % COLOUR_PLANE_SIZE;

	for(int x_pos=0; x_pos<x_count; x_pos++)
	{
		uint16_t mem = (offset+(x_pos*4)) % COLOUR_PLANE_SIZE;
		uint8_t rpx=red[mem];
		uint8_t gpx=green[mem];
		uint8_t bpx=blue[mem];

		uint8_t bitno=0x80;
		uint8_t shifts=7;

		for(int pixelno=0; pixelno<8; pixelno++)
		{
			uint8_t rb=(rpx & bitno) >> shifts;
			uint8_t gb=(gpx & bitno) >> shifts;
			uint8_t bb=(bpx & bitno) >> shifts;

			uint8_t colour=(rb<<2) | (gb<<1) | (bb<<0);

			bitmap.pix(y, (x_pos*8)+pixelno)=palette[colour];
			//logerror("set pixel (%d,%d)=%d\n",y, ((x_pos*8)+pixelno),colour);
			bitno=bitno>>1;
			shifts--;
		}
	}
}

WRITE_LINE_MEMBER( mbc55x_state::vid_hsync_changed )
{
}

WRITE_LINE_MEMBER( mbc55x_state::vid_vsync_changed )
{
}

static constexpr rgb_t mbc55x_pens[SCREEN_NO_COLOURS]
{
	// normal brightness
	{ 0x00, 0x00, 0x00 }, // black
	{ 0x00, 0x00, 0x80 }, // blue
	{ 0x00, 0x80, 0x00 }, // green
	{ 0x00, 0x80, 0x80 }, // cyan
	{ 0x80, 0x00, 0x00 }, // red
	{ 0x80, 0x00, 0x80 }, // magenta
	{ 0x80, 0x80, 0x00 }, // yellow
	{ 0x80, 0x80, 0x80 }  // light grey
};

void mbc55x_state::mbc55x_palette(palette_device &palette) const
{
	logerror("initializing palette\n");

	palette.set_pen_colors(0, mbc55x_pens);
}

/* Video ram page register */

uint8_t mbc55x_state::vram_page_r()
{
	return m_vram_page;
}

void mbc55x_state::vram_page_w(uint8_t data)
{
	logerror("%s : set vram page to %02X\n", machine().describe_context(),data);

	m_vram_page=data;
}

void mbc55x_state::video_start()
{
}

void mbc55x_state::video_reset()
{
	// When we reset clear the video registers and video memory.
	memset(&m_video_mem,0,sizeof(m_video_mem));

	logerror("Video reset\n");
}
