#include "emu.h"
#include "streams.h"
#include "cpu/cdp1802/cdp1802.h"
#include "sound/cdp1869.h"

/*

    TODO:

    - remove CDP1802 dependency
    - sound base frequencies are TPA/TPB
    - white noise
    - scanline based update
    - CMSEL output

*/

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define CDP1869_WEIGHT_RED		30 /* % of max luminance */
#define CDP1869_WEIGHT_GREEN	59
#define CDP1869_WEIGHT_BLUE		11

#define CDP1869_COLUMNS_HALF	20
#define CDP1869_COLUMNS_FULL	40
#define CDP1869_ROWS_HALF		12
#define CDP1869_ROWS_FULL_PAL	25
#define CDP1869_ROWS_FULL_NTSC	24

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

enum
{
	CDB0 = 0,
	CDB1,
	CDB2,
	CDB3,
	CDB4,
	CDB5,
	CCB0,
	CCB1
};

typedef struct _cdp1869_t cdp1869_t;
struct _cdp1869_t
{
	devcb_resolved_read8		in_page_ram_func;
	devcb_resolved_write8		out_page_ram_func;
	devcb_resolved_write_line	out_prd_func;
	devcb_resolved_read_line	in_pal_ntsc_func;

	running_device *device;
	const cdp1869_interface *intf;	/* interface */
	screen_device *screen;	/* screen */
	running_device *cpu;		/* CPU */
	sound_stream *stream;			/* sound output */
	int color_clock;

	/* video state */
	int prd;						/* predisplay */
	int dispoff;					/* display off */
	int fresvert;					/* full resolution vertical */
	int freshorz;					/* full resolution horizontal */
	int cmem;						/* character memory access mode */
	int dblpage;					/* double page mode */
	int line16;						/* 16-line hi-res mode */
	int line9;						/* 9 line mode */
	int cfc;						/* color format control */
	UINT8 col;						/* character color control */
	UINT8 bkg;						/* background color */
	UINT16 pma;						/* page memory address */
	UINT16 hma;						/* home memory address */

	/* video timer */
	emu_timer *prd_changed_timer;	/* predisplay changed timer */

	/* sound state */
	INT16 signal;					/* current signal */
	int incr;						/* initial wave state */
	int toneoff;					/* tone off */
	int wnoff;						/* white noise off */
	UINT8 tonediv;					/* tone divisor */
	UINT8 tonefreq;					/* tone range select */
	UINT8 toneamp;					/* tone output amplitude */
	UINT8 wnfreq;					/* white noise range select */
	UINT8 wnamp;					/* white noise output amplitude */
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE cdp1869_t *get_safe_token(running_device *device)
{
	assert(device != NULL);
	return (cdp1869_t *)downcast<legacy_device_base *>(device)->token();
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

#define CDP1869_IS_NTSC \
	(!devcb_call_read_line(&cdp1869->in_pal_ntsc_func))

/*-------------------------------------------------
    update_prd_changed_timer - update predisplay
    changed timer
-------------------------------------------------*/

static void update_prd_changed_timer(cdp1869_t *cdp1869)
{
	if (cdp1869->prd_changed_timer != NULL)
	{
		attotime duration;
		int start, end, level;
		int scanline = cdp1869->screen->vpos();
		int next_scanline;

		if (CDP1869_IS_NTSC)
		{
			start = CDP1869_SCANLINE_PREDISPLAY_START_NTSC;
			end = CDP1869_SCANLINE_PREDISPLAY_END_NTSC;
		}
		else
		{
			start = CDP1869_SCANLINE_PREDISPLAY_START_PAL;
			end = CDP1869_SCANLINE_PREDISPLAY_END_PAL;
		}

		if (scanline < start)
		{
			next_scanline = start;
			level = 0;
		}
		else if (scanline < end)
		{
			next_scanline = end;
			level = 1;
		}
		else
		{
			next_scanline = start;
			level = 0;
		}

		if (cdp1869->dispoff)
		{
			level = 1;
		}

		duration = cdp1869->screen->time_until_pos(next_scanline);
		timer_adjust_oneshot(cdp1869->prd_changed_timer, duration, level);
	}
}

/*-------------------------------------------------
    TIMER_CALLBACK( prd_changed_tick )
-------------------------------------------------*/

static TIMER_CALLBACK( prd_changed_tick )
{
	running_device *device = (running_device *)ptr;
	cdp1869_t *cdp1869 = get_safe_token(device);

	devcb_call_write_line(&cdp1869->out_prd_func, param);
	cdp1869->prd = param;

	update_prd_changed_timer(cdp1869);
}

/*-------------------------------------------------
    STATE_POSTLOAD( cdp1869_state_save_postload )
-------------------------------------------------*/

static STATE_POSTLOAD( cdp1869_state_save_postload )
{
	update_prd_changed_timer((cdp1869_t *)param);
}

/*-------------------------------------------------
    cdp1802_get_r_x - get CDP1802 R(X) value
-------------------------------------------------*/

static UINT16 cdp1802_get_r_x(cdp1869_t *cdp1869)
{
	return cpu_get_reg(cdp1869->cpu, CDP1802_R0 + cpu_get_reg(cdp1869->cpu, CDP1802_X));
}

/*-------------------------------------------------
    get_rgb - get RGB value
-------------------------------------------------*/

static rgb_t get_rgb(int i, int c, int l)
{
	int luma = 0, r, g, b;

	luma += (l & 4) ? CDP1869_WEIGHT_RED : 0;
	luma += (l & 1) ? CDP1869_WEIGHT_GREEN : 0;
	luma += (l & 2) ? CDP1869_WEIGHT_BLUE : 0;

	luma = (luma * 0xff) / 100;

	r = (c & 4) ? luma : 0;
	g = (c & 1) ? luma : 0;
	b = (c & 2) ? luma : 0;

	return MAKE_RGB(r, g, b);
}

/*-------------------------------------------------
    get_lines - get number of character lines
-------------------------------------------------*/

static int get_lines(running_device *device)
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	if (cdp1869->line16 && !cdp1869->dblpage)
	{
		return 16;
	}
	else if (!cdp1869->line9)
	{
		return 9;
	}
	else
	{
		return 8;
	}
}

/*-------------------------------------------------
    get_pmemsize - get page memory size
-------------------------------------------------*/

static UINT16 get_pmemsize(running_device *device, int cols, int rows)
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	int pmemsize = cols * rows;

	if (cdp1869->dblpage) pmemsize *= 2;
	if (cdp1869->line16) pmemsize *= 2;

	return pmemsize;
}

/*-------------------------------------------------
    get_pma - get page memory address
-------------------------------------------------*/

static UINT16 get_pma(running_device *device)
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	if (cdp1869->dblpage)
	{
		return cdp1869->pma;
	}
	else
	{
		return cdp1869->pma & 0x3ff;
	}
}

/*-------------------------------------------------
    get_pen - get pen for color bits
-------------------------------------------------*/

static int get_pen(running_device *device, int ccb0, int ccb1, int pcb)
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	int r = 0, g = 0, b = 0, color;

	switch (cdp1869->col)
	{
	case 0:
		r = ccb0;
		b = ccb1;
		g = pcb;
		break;

	case 1:
		r = ccb0;
		b = pcb;
		g = ccb1;
		break;

	case 2:
	case 3:
		r = pcb;
		b = ccb0;
		g = ccb1;
		break;
	}

	color = (r << 2) + (b << 1) + g;

	if (cdp1869->cfc)
	{
		return color + ((cdp1869->bkg + 1) * 8);
	}
	else
	{
		return color;
	}
}

/*-------------------------------------------------
    draw_line - draw character line
-------------------------------------------------*/

static void draw_line(running_device *device, bitmap_t *bitmap, int x, int y, int data, int color)
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	int i;

	data <<= 2;

	for (i = 0; i < CDP1869_CHAR_WIDTH; i++)
	{
		if (data & 0x80)
		{
			*BITMAP_ADDR16(bitmap, y, x) = color;

			if (!cdp1869->fresvert)
			{
				*BITMAP_ADDR16(bitmap, y + 1, x) = color;
			}

			if (!cdp1869->freshorz)
			{
				*BITMAP_ADDR16(bitmap, y, x + 1) = color;

				if (!cdp1869->fresvert)
				{
					*BITMAP_ADDR16(bitmap, y + 1, x + 1) = color;
				}
			}
		}

		if (!cdp1869->freshorz)
		{
			x++;
		}

		x++;

		data <<= 1;
	}
}

/*-------------------------------------------------
    draw_char - draw character
-------------------------------------------------*/

static void draw_char(running_device *device, bitmap_t *bitmap, int x, int y, UINT16 pma, const rectangle *screenrect)
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	UINT8 cma = 0;

	for (cma = 0; cma < get_lines(device); cma++)
	{
		UINT8 data = cdp1869->intf->char_ram_r(device, pma, cma);

		int ccb0 = BIT(data, CCB0);
		int ccb1 = BIT(data, CCB1);
		int pcb = BIT(cdp1869->intf->pcb_r(device, pma, cma), 0);

		int color = get_pen(device, ccb0, ccb1, pcb);

		draw_line(device, bitmap, screenrect->min_x + x, screenrect->min_y + y, data, color);

		y++;

		if (!cdp1869->fresvert)
		{
			y++;
		}
	}
}

/*-------------------------------------------------
    PALETTE_INIT( cdp1869 )
-------------------------------------------------*/

PALETTE_INIT( cdp1869 )
{
	int i, c, l;

	/* color-on-color display (CFC=0) */
	for (i = 0; i < 8; i++)
	{
		palette_set_color(machine, i, get_rgb(i, i, 15));
	}

	/* tone-on-tone display (CFC=1) */
	for (c = 0; c < 8; c++)
	{
		for (l = 0; l < 8; l++)
		{
			palette_set_color(machine, i, get_rgb(i, c, l));
			i++;
		}
	}
}

/*-------------------------------------------------
    cdp1869_out3_w - register 3 write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( cdp1869_out3_w )
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

	cdp1869_t *cdp1869 = get_safe_token(device);

	cdp1869->bkg = data & 0x07;
	cdp1869->cfc = BIT(data, 3);
	cdp1869->dispoff = BIT(data, 4);
	cdp1869->col = (data & 0x60) >> 5;
	cdp1869->freshorz = BIT(data, 7);
}

/*-------------------------------------------------
    cdp1869_out4_w - register 4 write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( cdp1869_out4_w )
{
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

	cdp1869_t *cdp1869 = get_safe_token(device);
	UINT16 word = cdp1802_get_r_x(cdp1869);

	cdp1869->toneamp = word & 0x0f;
	cdp1869->tonefreq = (word & 0x70) >> 4;
	cdp1869->toneoff = BIT(word, 7);
	cdp1869->tonediv = (word & 0x7f00) >> 8;
}

/*-------------------------------------------------
    cdp1869_out5_w - register 5 write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( cdp1869_out5_w )
{
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

	cdp1869_t *cdp1869 = get_safe_token(device);
	UINT16 word = cdp1802_get_r_x(cdp1869);

	cdp1869->cmem = BIT(word, 0);
	cdp1869->line9 = BIT(word, 3);

	if (CDP1869_IS_NTSC)
	{
		cdp1869->line16 = BIT(word, 5);
	}

	cdp1869->dblpage = BIT(word, 6);
	cdp1869->fresvert = BIT(word, 7);

	cdp1869->wnamp = (word & 0x0f00) >> 8;
	cdp1869->wnfreq = (word & 0x7000) >> 12;
	cdp1869->wnoff = BIT(word, 15);

	if (cdp1869->cmem)
	{
		cdp1869->pma = word;
	}
	else
	{
		cdp1869->pma = 0;
	}
}

/*-------------------------------------------------
    cdp1869_out6_w - register 6 write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( cdp1869_out6_w )
{
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

	cdp1869_t *cdp1869 = get_safe_token(device);
	UINT16 word = cdp1802_get_r_x(cdp1869);

	cdp1869->pma = word & 0x7ff;
}

/*-------------------------------------------------
    cdp1869_out7_w - register 7 write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( cdp1869_out7_w )
{
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

	cdp1869_t *cdp1869 = get_safe_token(device);
	UINT16 word = cdp1802_get_r_x(cdp1869);

	cdp1869->hma = word & 0x7fc;
}

/*-------------------------------------------------
    cdp1869_pageram_r - page memory read
-------------------------------------------------*/

READ8_DEVICE_HANDLER( cdp1869_pageram_r )
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	UINT16 pma;

	if (cdp1869->cmem)
	{
		pma = get_pma(device);
	}
	else
	{
		pma = offset;
	}

	return devcb_call_read8(&cdp1869->in_page_ram_func, pma);
}

/*-------------------------------------------------
    cdp1869_pageram_w - page memory write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( cdp1869_pageram_w )
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	UINT16 pma;

	if (cdp1869->cmem)
	{
		pma = get_pma(device);
	}
	else
	{
		pma = offset;
	}

	devcb_call_write8(&cdp1869->out_page_ram_func, pma, data);
}

/*-------------------------------------------------
    cdp1869_charram_r - character memory read
-------------------------------------------------*/

READ8_DEVICE_HANDLER( cdp1869_charram_r )
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	UINT8 cma = offset & 0x0f;
	UINT16 pma;

	if (cdp1869->cmem)
	{
		pma = get_pma(device);
	}
	else
	{
		pma = offset;
	}

	if (cdp1869->dblpage)
	{
		cma &= 0x07;
	}

	return cdp1869->intf->char_ram_r(device, pma, cma);
}

/*-------------------------------------------------
    cdp1869_charram_w - character memory write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( cdp1869_charram_w )
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	UINT8 cma = offset & 0x0f;
	UINT16 pma;

	if (cdp1869->cmem)
	{
		pma = get_pma(device);
	}
	else
	{
		pma = offset;
	}

	if (cdp1869->dblpage)
	{
		cma &= 0x07;
	}

	if (cdp1869->intf->char_ram_w)
	{
		cdp1869->intf->char_ram_w(device, pma, cma, data);
	}
}

/*-------------------------------------------------
    cdp1869_predisplay_r - predisplay read
-------------------------------------------------*/

READ_LINE_DEVICE_HANDLER( cdp1869_predisplay_r )
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	return cdp1869->prd;
}

/*-------------------------------------------------
    cdp1869_update - screen update
-------------------------------------------------*/

void cdp1869_update(running_device *device, bitmap_t *bitmap, const rectangle *cliprect)
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	rectangle screen_rect, outer;

	if (CDP1869_IS_NTSC)
	{
		outer.min_x = CDP1869_HBLANK_END;
		outer.max_x = CDP1869_HBLANK_START - 1;
		outer.min_y = CDP1869_SCANLINE_VBLANK_END_NTSC;
		outer.max_y = CDP1869_SCANLINE_VBLANK_START_NTSC - 1;
		screen_rect.min_x = CDP1869_SCREEN_START_NTSC;
		screen_rect.max_x = CDP1869_SCREEN_END - 1;
		screen_rect.min_y = CDP1869_SCANLINE_DISPLAY_START_NTSC;
		screen_rect.max_y = CDP1869_SCANLINE_DISPLAY_END_NTSC - 1;
	}
	else
	{
		outer.min_x = CDP1869_HBLANK_END;
		outer.max_x = CDP1869_HBLANK_START - 1;
		outer.min_y = CDP1869_SCANLINE_VBLANK_END_PAL;
		outer.max_y = CDP1869_SCANLINE_VBLANK_START_PAL - 1;
		screen_rect.min_x = CDP1869_SCREEN_START_PAL;
		screen_rect.max_x = CDP1869_SCREEN_END - 1;
		screen_rect.min_y = CDP1869_SCANLINE_DISPLAY_START_PAL;
		screen_rect.max_y = CDP1869_SCANLINE_DISPLAY_END_PAL - 1;
	}

	sect_rect(&outer, cliprect);
	bitmap_fill(bitmap, &outer, device->machine->pens[cdp1869->bkg]);

	if (!cdp1869->dispoff)
	{
		int sx, sy, rows, cols, width, height;
		UINT16 addr, pmemsize;

		width = CDP1869_CHAR_WIDTH;
		height = get_lines(device);

		if (!cdp1869->freshorz)
		{
			width *= 2;
		}

		if (!cdp1869->fresvert)
		{
			height *= 2;
		}

		cols = cdp1869->freshorz ? CDP1869_COLUMNS_FULL : CDP1869_COLUMNS_HALF;
		rows = (screen_rect.max_y - screen_rect.min_y + 1) / height;

		pmemsize = get_pmemsize(device, cols, rows);

		addr = cdp1869->hma;

		for (sy = 0; sy < rows; sy++)
		{
			for (sx = 0; sx < cols; sx++)
			{
				int x = sx * width;
				int y = sy * height;

				draw_char(device, bitmap, x, y, addr, &screen_rect);

				addr++;

				if (addr == pmemsize) addr = 0;
			}
		}
	}
}

/*-------------------------------------------------
    STREAM_UPDATE( cdp1869_stream_update )
-------------------------------------------------*/

static STREAM_UPDATE( cdp1869_stream_update )
{
	cdp1869_t *cdp1869 = (cdp1869_t *)param;
	INT16 signal = cdp1869->signal;
	stream_sample_t *buffer = outputs[0];

	memset( buffer, 0, samples * sizeof(*buffer) );

	if (!cdp1869->toneoff && cdp1869->toneamp)
	{
		double frequency = (cdp1869->device->clock() / 2) / (512 >> cdp1869->tonefreq) / (cdp1869->tonediv + 1);
//      double amplitude = cdp1869->toneamp * ((0.78*5) / 15);

		int rate = cdp1869->device->machine->sample_rate / 2;

		/* get progress through wave */
		int incr = cdp1869->incr;

		if (signal < 0)
		{
			signal = -(cdp1869->toneamp * (0x07fff / 15));
		}
		else
		{
			signal = cdp1869->toneamp * (0x07fff / 15);
		}

		while( samples-- > 0 )
		{
			*buffer++ = signal;
			incr -= frequency;
			while( incr < 0 )
			{
				incr += rate;
				signal = -signal;
			}
		}

		/* store progress through wave */
		cdp1869->incr = incr;
		cdp1869->signal = signal;
	}
/*
    if (!cdp1869->wnoff)
    {
        double amplitude = cdp1869->wnamp * ((0.78*5) / 15);

        for (int wndiv = 0; wndiv < 128; wndiv++)
        {
            double frequency = (cdp1869->clock / 2) / (4096 >> cdp1869->wnfreq) / (wndiv + 1):

            sum_square_wave(buffer, frequency, amplitude);
        }
    }
*/
}

/*-------------------------------------------------
    DEVICE_START( cdp1869 )
-------------------------------------------------*/

static DEVICE_START( cdp1869 )
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	/* validate arguments */
	cdp1869->intf = (const cdp1869_interface *)device->baseconfig().static_config();

	assert(cdp1869->intf->pcb_r != NULL);
	assert(cdp1869->intf->char_ram_r != NULL);

	/* resolve callbacks */
	devcb_resolve_read8(&cdp1869->in_page_ram_func, &cdp1869->intf->in_page_ram_func, device);
	devcb_resolve_write8(&cdp1869->out_page_ram_func, &cdp1869->intf->out_page_ram_func, device);
	devcb_resolve_write_line(&cdp1869->out_prd_func, &cdp1869->intf->out_prd_func, device);
	devcb_resolve_read_line(&cdp1869->in_pal_ntsc_func, &cdp1869->intf->in_pal_ntsc_func, device);

	/* set initial values */
	cdp1869->device = device;
	cdp1869->stream = stream_create(device, 0, 1, device->machine->sample_rate, cdp1869, cdp1869_stream_update);
	cdp1869->incr = 0;
	cdp1869->signal = 0x07fff;
	cdp1869->toneoff = 1;
	cdp1869->wnoff = 1;

	/* get the screen device */
	cdp1869->screen = downcast<screen_device *>(device->machine->device(cdp1869->intf->screen_tag));
	assert(cdp1869->screen != NULL);

	/* get the CPU device */
	cdp1869->cpu = device->machine->device(cdp1869->intf->cpu_tag);
	assert(cdp1869->cpu != NULL);

	/* allocate predisplay timer */
	cdp1869->prd_changed_timer = timer_alloc(device->machine, prd_changed_tick, (void *)device);
	update_prd_changed_timer(cdp1869);

	/* register for state saving */
	state_save_register_postload(device->machine, cdp1869_state_save_postload, cdp1869);

	state_save_register_device_item(device, 0, cdp1869->prd);
	state_save_register_device_item(device, 0, cdp1869->dispoff);
	state_save_register_device_item(device, 0, cdp1869->fresvert);
	state_save_register_device_item(device, 0, cdp1869->freshorz);
	state_save_register_device_item(device, 0, cdp1869->cmem);
	state_save_register_device_item(device, 0, cdp1869->dblpage);
	state_save_register_device_item(device, 0, cdp1869->line16);
	state_save_register_device_item(device, 0, cdp1869->line9);
	state_save_register_device_item(device, 0, cdp1869->cfc);
	state_save_register_device_item(device, 0, cdp1869->col);
	state_save_register_device_item(device, 0, cdp1869->bkg);
	state_save_register_device_item(device, 0, cdp1869->pma);
	state_save_register_device_item(device, 0, cdp1869->hma);

	state_save_register_device_item(device, 0, cdp1869->signal);
	state_save_register_device_item(device, 0, cdp1869->incr);
	state_save_register_device_item(device, 0, cdp1869->toneoff);
	state_save_register_device_item(device, 0, cdp1869->wnoff);
	state_save_register_device_item(device, 0, cdp1869->tonediv);
	state_save_register_device_item(device, 0, cdp1869->tonefreq);
	state_save_register_device_item(device, 0, cdp1869->toneamp);
	state_save_register_device_item(device, 0, cdp1869->wnfreq);
	state_save_register_device_item(device, 0, cdp1869->wnamp);
}

/*-------------------------------------------------
    DEVICE_GET_INFO( cdp1869 )
-------------------------------------------------*/

DEVICE_GET_INFO( cdp1869 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(cdp1869_t);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(cdp1869);	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							/* Nothing */								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "RCA CDP1869");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "RCA CDP1800");				break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(CDP1869, cdp1869);
