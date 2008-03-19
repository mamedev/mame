#include "driver.h"
#include "deprecat.h"
#include "cpu/cdp1802/cdp1802.h"
#include "video/cdp1869.h"
#include "sound/cdp1869.h"

/*

	TODO:

	- convert CDP1869 into a device with video/sound
	- add predisplay/display timers

*/

typedef struct
{
	int ntsc_pal;
	int dispoff;
	int fresvert, freshorz;
	int dblpage, line16, line9, cmem, cfc;
	UINT8 col, bkg;
	UINT16 pma, hma;
	UINT8 cram[CDP1869_CHARRAM_SIZE];
	UINT8 pram[CDP1869_PAGERAM_SIZE];
	int cramsize, pramsize;
	UINT8 (*color_callback)(UINT8 cramdata, UINT16 cramaddr, UINT16 pramaddr);
} CDP1869_VIDEO_CONFIG;

static CDP1869_VIDEO_CONFIG cdp1869;

static UINT16 cdp1802_get_r_x(void)
{
	return activecpu_get_reg(CDP1802_R0 + activecpu_get_reg(CDP1802_X));
}

WRITE8_HANDLER ( cdp1869_out3_w )
{
	/*
      bit   description

        0   bkg green
        1   bkg blue
        2   bkg red
        3   cfc
        4   disp off
        5   colb0
        6   colb1
        7   fres horz
    */

	cdp1869.bkg = (data & 0x07);
	cdp1869.cfc = (data & 0x08) >> 3;
	cdp1869.dispoff = (data & 0x10) >> 4;
	cdp1869.col = (data & 0x60) >> 5;
	cdp1869.freshorz = (data & 0x80) >> 7;
}

WRITE8_HANDLER ( cdp1869_out4_w )
{
	UINT16 word = cdp1802_get_r_x();

	/*
      bit   description

        0   tone amp 2^0
        1   tone amp 2^1
        2   tone amp 2^2
        3   tone amp 2^3
        4   tone freq sel0
        5   tone freq sel1
        6   tone freq sel2
        7   tone off
        8   tone / 2^0
        9   tone / 2^1
       10   tone / 2^2
       11   tone / 2^3
       12   tone / 2^4
       13   tone / 2^5
       14   tone / 2^6
       15   always 0
    */

	cdp1869_set_toneamp(0, word & 0x0f);
	cdp1869_set_tonefreq(0, (word & 0x70) >> 4);
	cdp1869_set_toneoff(0, (word & 0x80) >> 7);
	cdp1869_set_tonediv(0, (word & 0x7f00) >> 8);
}

WRITE8_HANDLER ( cdp1869_out5_w )
{
	UINT16 word = cdp1802_get_r_x();

	/*
      bit   description

        0   cmem access mode
        1   x
        2   x
        3   9-line
        4   x
        5   16 line hi-res
        6   double page
        7   fres vert
        8   wn amp 2^0
        9   wn amp 2^1
       10   wn amp 2^2
       11   wn amp 2^3
       12   wn freq sel0
       13   wn freq sel1
       14   wn freq sel2
       15   wn off
    */

	cdp1869.cmem = (word & 0x01);
	cdp1869.line9 = (word & 0x08) >> 3;
	cdp1869.line16 = (word & 0x20) >> 5;
	cdp1869.dblpage = (word & 0x40) >> 6;
	cdp1869.fresvert = (word & 0x80) >> 7;

	cdp1869_set_wnamp(0, (word & 0x0f00) >> 8);
	cdp1869_set_wnfreq(0, (word & 0x7000) >> 12);
	cdp1869_set_wnoff(0, (word & 0x8000) >> 15);
}

WRITE8_HANDLER ( cdp1869_out6_w )
{
	UINT16 word = cdp1802_get_r_x();

	/*
      bit   description

        0   pma0 reg
        1   pma1 reg
        2   pma2 reg
        3   pma3 reg
        4   pma4 reg
        5   pma5 reg
        6   pma6 reg
        7   pma7 reg
        8   pma8 reg
        9   pma9 reg
       10   pma10 reg
       11   x
       12   x
       13   x
       14   x
       15   x
    */

	cdp1869.pma = word & 0x7ff;
}

WRITE8_HANDLER ( cdp1869_out7_w )
{
	UINT16 word = cdp1802_get_r_x();

	/*
      bit   description

        0   x
        1   x
        2   hma2 reg
        3   hma3 reg
        4   hma4 reg
        5   hma5 reg
        6   hma6 reg
        7   hma7 reg
        8   hma8 reg
        9   hma9 reg
       10   hma10 reg
       11   x
       12   x
       13   x
       14   x
       15   x
    */

	cdp1869.hma = word & 0x7fc;
}

static void cdp1869_set_color(int i, int c, int l)
{
	int luma = 0, r, g, b;

	luma += (l & 4) ? CDP1869_WEIGHT_RED : 0;
	luma += (l & 1) ? CDP1869_WEIGHT_GREEN : 0;
	luma += (l & 2) ? CDP1869_WEIGHT_BLUE : 0;

	luma = (luma * 0xff) / 100;

	r = (c & 4) ? luma : 0;
	g = (c & 1) ? luma : 0;
	b = (c & 2) ? luma : 0;

	palette_set_color( Machine, i, MAKE_RGB(r, g, b) );
}

PALETTE_INIT( cdp1869 )
{
	int i, c, l;

	// color-on-color display (CFC=0)

	for (i = 0; i < 8; i++)
	{
		cdp1869_set_color(i, i, 15);
	}

	// tone-on-tone display (CFC=1)

	for (c = 0; c < 8; c++)
	{
		for (l = 0; l < 8; l++)
		{
			cdp1869_set_color(i, c, l);
			i++;
		}
	}
}

static int cdp1869_get_lines(void)
{
	if (cdp1869.line16 && !cdp1869.dblpage)
	{
		return 16;
	}
	else if (!cdp1869.line9)
	{
		return 9;
	}
	else
	{
		return 8;
	}
}

static UINT16 cdp1869_get_pmemsize(int cols, int rows)
{
	int pmemsize = cols * rows;

	if (cdp1869.dblpage) pmemsize *= 2;
	if (cdp1869.line16) pmemsize *= 2;

	return pmemsize;
}

static UINT16 cdp1869_get_pma(void)
{
	if (cdp1869.dblpage)
	{
		return cdp1869.pma;
	}
	else
	{
		return cdp1869.pma & 0x3ff;
	}
}

UINT16 cdp1869_get_cma(UINT16 offset)
{
	int column = cdp1869.pram[cdp1869_get_pma()];
	int row = offset & 0x07;

	int addr = (column << 3) + row;

	if ((offset & 0x08) && (cdp1869_get_lines() > 8))
	{
		addr += (CDP1869_CHARRAM_SIZE / 2);
	}

	return addr;
}

static UINT8 cdp1869_read_pageram(UINT16 addr)
{
	if (addr >= cdp1869.pramsize)
	{
		return 0xff;
	}
	else
	{
		return cdp1869.pram[addr];
	}
}

#ifdef UNUSED_FUNCTION
UINT8 cdp1869_read_charram(UINT16 addr)
{
	if (addr >= cdp1869.cramsize)
	{
		return 0xff;
	}
	else
	{
		return cdp1869.cram[addr];
	}
}
#endif

WRITE8_HANDLER ( cdp1869_charram_w )
{
	if (cdp1869.cmem)
	{
		UINT16 addr = cdp1869_get_cma(offset);
		cdp1869.cram[addr] = data;
	}
}

READ8_HANDLER ( cdp1869_charram_r )
{
	if (cdp1869.cmem)
	{
		UINT16 addr = cdp1869_get_cma(offset);
		return cdp1869.cram[addr];
	}
	else
	{
		return 0xff;
	}
}

WRITE8_HANDLER ( cdp1869_pageram_w )
{
	UINT16 addr;

	if (cdp1869.cmem)
	{
		addr = cdp1869_get_pma();
	}
	else
	{
		addr = offset;
	}

	cdp1869.pram[addr] = data;
}

READ8_HANDLER ( cdp1869_pageram_r )
{
	UINT16 addr;

	if (cdp1869.cmem)
	{
		addr = cdp1869_get_pma();
	}
	else
	{
		addr = offset;
	}

	return cdp1869.pram[addr];
}

static int cdp1869_get_color(int ccb0, int ccb1, int pcb)
{
	int r = 0, g = 0, b = 0, color;

	switch (cdp1869.col)
	{
	case 0:
		r = ccb0;
		g = pcb;
		b = ccb1;
		break;

	case 1:
		r = ccb0;
		g = ccb1;
		b = pcb;
		break;

	case 2:
	case 3:
		r = pcb;
		g = ccb1;
		b = ccb0;
		break;
	}

	color = (r << 2) + (b << 1) + g;

	if (cdp1869.cfc)
	{
		return color + ((cdp1869.bkg + 1) * 8);
	}
	else
	{
		return color;
	}
}

static void cdp1869_draw_line(running_machine *machine, bitmap_t *bitmap, int x, int y, int data, int color)
{
	int i;

	data <<= 2;

	for (i = 0; i < CDP1869_CHAR_WIDTH; i++)
	{
		if (data & 0x80)
		{
			*BITMAP_ADDR16(bitmap, y, x) = machine->pens[color];

			if (!cdp1869.fresvert)
			{
				*BITMAP_ADDR16(bitmap, y + 1, x) = machine->pens[color];
			}

			if (!cdp1869.freshorz)
			{
				*BITMAP_ADDR16(bitmap, y, x + 1) = machine->pens[color];

				if (!cdp1869.fresvert)
				{
					*BITMAP_ADDR16(bitmap, y + 1, x + 1) = machine->pens[color];
				}
			}
		}

		if (!cdp1869.freshorz)
		{
			x++;
		}

		x++;

		data <<= 1;
	}
}

static void cdp1869_draw_char(running_machine *machine, bitmap_t *bitmap, int x, int y, UINT16 pramaddr, const rectangle *screenrect)
{
	int i;
	UINT8 code = cdp1869_read_pageram(pramaddr);
	UINT16 addr = code * 8;
	UINT16 addr2 = addr + (CDP1869_CHARRAM_SIZE / 2);

	for (i = 0; i < cdp1869_get_lines(); i++)
	{
		UINT8 data = cdp1869.cram[addr];

		UINT8 colorbits = cdp1869.color_callback(data, addr, pramaddr);

		int pcb = colorbits & 0x01;
		int ccb1 = (colorbits & 0x02) >> 1;
		int ccb0 = (colorbits & 0x04) >> 2;

		int color = cdp1869_get_color(ccb0, ccb1, pcb);

		cdp1869_draw_line(machine, bitmap, screenrect->min_x + x, screenrect->min_y + y, data, color);

		addr++;
		y++;

		if (!cdp1869.fresvert)
		{
			y++;
		}

		if (i == 7)
		{
			addr = addr2;
		}
	}
}

VIDEO_START( cdp1869 )
{
	state_save_register_item("cdp1869", 0, cdp1869.ntsc_pal);
	state_save_register_item("cdp1869", 0, cdp1869.dispoff);
	state_save_register_item("cdp1869", 0, cdp1869.fresvert);
	state_save_register_item("cdp1869", 0, cdp1869.freshorz);
	state_save_register_item("cdp1869", 0, cdp1869.dblpage);
	state_save_register_item("cdp1869", 0, cdp1869.line16);
	state_save_register_item("cdp1869", 0, cdp1869.line9);
	state_save_register_item("cdp1869", 0, cdp1869.cmem);
	state_save_register_item("cdp1869", 0, cdp1869.cfc);
	state_save_register_item("cdp1869", 0, cdp1869.col);
	state_save_register_item("cdp1869", 0, cdp1869.bkg);
	state_save_register_item("cdp1869", 0, cdp1869.pma);
	state_save_register_item("cdp1869", 0, cdp1869.hma);
	state_save_register_item_array("cdp1869", 0, cdp1869.cram);
	state_save_register_item_array("cdp1869", 0, cdp1869.pram);
	state_save_register_item("cdp1869", 0, cdp1869.cramsize);
	state_save_register_item("cdp1869", 0, cdp1869.pramsize);
}

VIDEO_UPDATE( cdp1869 )
{
	fillbitmap(bitmap, get_black_pen(screen->machine), cliprect);

	if (!cdp1869.dispoff)
	{
		int sx, sy, rows, cols, width, height;
		UINT16 addr, pmemsize;
		rectangle screen_rect, outer;

		switch (cdp1869.ntsc_pal)
		{
		case CDP1869_NTSC:
			outer.min_x = CDP1869_HBLANK_END;
			outer.max_x = CDP1869_HBLANK_START - 1;
			outer.min_y = CDP1869_SCANLINE_VBLANK_END_NTSC;
			outer.max_y = CDP1869_SCANLINE_VBLANK_START_NTSC - 1;
			screen_rect.min_x = CDP1869_SCREEN_START_NTSC;
			screen_rect.max_x = CDP1869_SCREEN_END - 1;
			screen_rect.min_y = CDP1869_SCANLINE_DISPLAY_START_NTSC;
			screen_rect.max_y = CDP1869_SCANLINE_DISPLAY_END_NTSC - 1;
			break;

		default:
		case CDP1869_PAL:
			outer.min_x = CDP1869_HBLANK_END;
			outer.max_x = CDP1869_HBLANK_START - 1;
			outer.min_y = CDP1869_SCANLINE_VBLANK_END_PAL;
			outer.max_y = CDP1869_SCANLINE_VBLANK_START_PAL - 1;
			screen_rect.min_x = CDP1869_SCREEN_START_PAL;
			screen_rect.max_x = CDP1869_SCREEN_END - 1;
			screen_rect.min_y = CDP1869_SCANLINE_DISPLAY_START_PAL;
			screen_rect.max_y = CDP1869_SCANLINE_DISPLAY_END_PAL - 1;
			break;
		}

		sect_rect(&outer, cliprect);
		fillbitmap(bitmap, screen->machine->pens[cdp1869.bkg], &outer);

		width = CDP1869_CHAR_WIDTH;
		height = cdp1869_get_lines();

		if (!cdp1869.freshorz)
		{
			width *= 2;
		}

		if (!cdp1869.fresvert)
		{
			height *= 2;
		}

		cols = cdp1869.freshorz ? CDP1869_COLUMNS_FULL : CDP1869_COLUMNS_HALF;
		rows = (screen_rect.max_y - screen_rect.min_y + 1) / height;

		pmemsize = cdp1869_get_pmemsize(cols, rows);

		addr = cdp1869.hma;

		for (sy = 0; sy < rows; sy++)
		{
			for (sx = 0; sx < cols; sx++)
			{
				int x = sx * width;
				int y = sy * height;

				cdp1869_draw_char(screen->machine, bitmap, x, y, addr, &screen_rect);

				addr++;

				if (addr == pmemsize) addr = 0;
			}
		}
	}

	return 0;
}

void cdp1869_configure(const CDP1869_interface *intf)
{
	if (intf->charrom_region != REGION_INVALID)
	{
		UINT8 *memory = memory_region(intf->charrom_region);
		memcpy(cdp1869.cram, memory, intf->charram_size);
	}

	cdp1869.ntsc_pal = intf->video_format;
	cdp1869.cramsize = intf->charram_size;
	cdp1869.pramsize = intf->pageram_size;
	cdp1869.color_callback = intf->get_color_bits;
}
