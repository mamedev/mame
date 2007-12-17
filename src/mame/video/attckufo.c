/***************************************************************************

  Attack ufo video emulation
  based on MOS 6560 emulator by
  PeT mess@utanet.at

  Differences between 6560 and Attack Ufo chip:
  - no invert mode
  - no multicolor
  - 16 col chars

***************************************************************************/

#include "driver.h"
#include "sound/custom.h"
#include "includes/attckufo.h"

#define MAX_LINES 261

const rgb_t attckufo_palette[] =
{
/* ripped from vice, a very excellent emulator */
	MAKE_RGB(0x00, 0x00, 0x00),
	MAKE_RGB(0xff, 0xff, 0xff),
	MAKE_RGB(0xf0, 0x00, 0x00),
	MAKE_RGB(0x00, 0xf0, 0xf0),

	MAKE_RGB(0x60, 0x00, 0x60),
	MAKE_RGB(0x00, 0xa0, 0x00),
	MAKE_RGB(0x00, 0x00, 0xf0),
	MAKE_RGB(0xd0, 0xd0, 0x00),

	MAKE_RGB(0xc0, 0xa0, 0x00),
	MAKE_RGB(0xff, 0xa0, 0x00),
	MAKE_RGB(0xf0, 0x80, 0x80),
	MAKE_RGB(0x00, 0xff, 0xff),

	MAKE_RGB(0xff, 0x00, 0xff),
	MAKE_RGB(0x00, 0xff, 0x00),
	MAKE_RGB(0x00, 0xa0, 0xff),
	MAKE_RGB(0xff, 0xff, 0x00)
};



UINT8 attckufo_regs[16];

#define CHARS_X ((int)attckufo_regs[2]&0x7f)
#define CHARS_Y (((int)attckufo_regs[3]&0x7e)>>1)


#define XSIZE (CHARS_X*8)
#define YSIZE (CHARS_Y*8)

#define CHARGENADDR (((int)attckufo_regs[5]&0xf)<<10)
#define VIDEOADDR ( ( ((int)attckufo_regs[5]&0xf0)<<(10-4))\
		    | ( ((int)attckufo_regs[2]&0x80)<<(9-7)) )

static int rasterline = 0;

static int chars_x, chars_y;
static int xsize, ysize ;
static int chargenaddr, videoaddr;

WRITE8_HANDLER ( attckufo_port_w )
{
	switch (offset)
	{
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
			attckufo_soundport_w (offset, data);
			break;
	}
	if (attckufo_regs[offset] != data)
	{
		switch (offset)
		{
			case 0:
			case 1:
			case 2:
			case 3:
			case 5:
			case 0xe:
			case 0xf:
				video_screen_update_partial(0, rasterline);
				break;
		}
		attckufo_regs[offset] = data;
		switch (offset)
		{
			case 2:
				/* ntsc values >= 31 behave like 31 */
				chars_x = CHARS_X;
				videoaddr = VIDEOADDR;
				xsize = XSIZE;
				break;
			case 3:
				chars_y = CHARS_Y;
				ysize = YSIZE;
				break;
			case 5:
				chargenaddr = CHARGENADDR;
				videoaddr = VIDEOADDR;
				break;
		}
	}
}

 READ8_HANDLER ( attckufo_port_r )
{
	int val;

	switch (offset)
	{
		case 3:
			val = ((rasterline & 1) << 7) | (attckufo_regs[offset] & 0x7f);
			break;
		case 4:						   /*rasterline */
			val = (rasterline / 2) & 0xff;
			break;
		default:
			val = attckufo_regs[offset];
			break;
	}
	return val;
}

INTERRUPT_GEN( attckufo_raster_interrupt )
{
	rasterline++;
	if (rasterline >= MAX_LINES)
		rasterline = 0;
}

VIDEO_UPDATE( attckufo )
{
	int x, y, yy;

	for (y = cliprect->min_y & ~7; y <= cliprect->max_y; y += 8)
	{
		UINT16 *destrow = BITMAP_ADDR16(bitmap, y, 0);
		int offs = videoaddr + (y / 8) * chars_x;
		int ymin = MAX(y, cliprect->min_y) & 7;
		int ymax = MIN(y + 7, cliprect->max_y) & 7;

		for (x = cliprect->min_x & ~7; x <= cliprect->max_x; x += 8)
		{
			UINT8 ch = program_read_byte(offs + x/8);
			UINT8 attr = program_read_byte(offs + x/8 + 0x400) & 0xf;
			UINT16 *dest = destrow;

			for (yy = ymin; yy <= ymax; yy++)
			{
				UINT8 code = program_read_byte(chargenaddr + ch * 8 + yy);
				dest[x + 0] = (code & 0x80) ? attr : 0;
				dest[x + 1] = (code & 0x40) ? attr : 0;
				dest[x + 2] = (code & 0x20) ? attr : 0;
				dest[x + 3] = (code & 0x10) ? attr : 0;
				dest[x + 4] = (code & 0x08) ? attr : 0;
				dest[x + 5] = (code & 0x04) ? attr : 0;
				dest[x + 6] = (code & 0x02) ? attr : 0;
				dest[x + 7] = (code & 0x01) ? attr : 0;
				dest += bitmap->rowpixels;
			}
		}
	}
	return 0;
}
