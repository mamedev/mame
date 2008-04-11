#include "driver.h"
#include "sndintrf.h"
#include "streams.h"
#include "cpu/cdp1802/cdp1802.h"
#include "video/cdp1869.h"
#include "sound/cdp1869.h"

/*

    TODO:

    - connect to sound system when possible
    - white noise
    - scanline based update
    - fix flashing spaceship in destryer/altair
    - fix flashing player in draco

*/

#define CDP1869_WEIGHT_RED		30 /* % of max luminance */
#define CDP1869_WEIGHT_GREEN	59
#define CDP1869_WEIGHT_BLUE		11

#define CDP1869_COLUMNS_HALF	20
#define CDP1869_COLUMNS_FULL	40
#define CDP1869_ROWS_HALF		12
#define CDP1869_ROWS_FULL_PAL	25
#define CDP1869_ROWS_FULL_NTSC	24

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
	const cdp1869_interface *intf;	/* interface */
	const device_config *screen;	/* screen */
	sound_stream *stream;			/* sound output */

	/* video state */
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

INLINE cdp1869_t *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);

	return (cdp1869_t *)device->token;
}

/* Predisplay Timer */

static void update_prd_changed_timer(cdp1869_t *cdp1869)
{
	if (cdp1869->prd_changed_timer != NULL)
	{
		attotime duration;
		int start, end, level;
		int scanline = video_screen_get_vpos(cdp1869->screen);
		int next_scanline;

		if (cdp1869->intf->pal_ntsc == CDP1869_NTSC)
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

		duration = video_screen_get_time_until_pos(cdp1869->screen, next_scanline, 0);
		timer_adjust_oneshot(cdp1869->prd_changed_timer, duration, level);
	}
}

static TIMER_CALLBACK( prd_changed_tick )
{
	const device_config *device = ptr;
	cdp1869_t *cdp1869 = get_safe_token(device);

	/* call the callback function -- we know it exists */
	cdp1869->intf->on_prd_changed(device, param);

	update_prd_changed_timer(cdp1869);
}

/* State Save Postload */

static STATE_POSTLOAD( cdp1869_state_save_postload )
{
	update_prd_changed_timer(param);
}

/* CDP1802 X Register */

static UINT16 cdp1802_get_r_x(void)
{
	return activecpu_get_reg(CDP1802_R0 + activecpu_get_reg(CDP1802_X));
}

/* Palette Initialization */

static rgb_t cdp1869_get_rgb(int i, int c, int l)
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

/* Screen Update */

static int cdp1869_get_lines(const device_config *device)
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

static UINT16 cdp1869_get_pmemsize(const device_config *device, int cols, int rows)
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	int pmemsize = cols * rows;

	if (cdp1869->dblpage) pmemsize *= 2;
	if (cdp1869->line16) pmemsize *= 2;

	return pmemsize;
}

static UINT16 cdp1869_get_pma(const device_config *device)
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

static int cdp1869_get_pen(const device_config *device, int ccb0, int ccb1, int pcb)
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

static void cdp1869_draw_line(const device_config *device, bitmap_t *bitmap, int x, int y, int data, int color)
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

static void cdp1869_draw_char(const device_config *device, bitmap_t *bitmap, int x, int y, UINT16 pma, const rectangle *screenrect)
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	UINT8 cma = 0;

	for (cma = 0; cma < cdp1869_get_lines(device); cma++)
	{
		UINT8 data = cdp1869->intf->char_ram_r(device, pma, cma);

		int ccb0 = BIT(data, CCB0);
		int ccb1 = BIT(data, CCB1);
		int pcb = cdp1869->intf->pcb_r(device, pma, cma);

		int color = cdp1869_get_pen(device, ccb0, ccb1, pcb);

		cdp1869_draw_line(device, bitmap, screenrect->min_x + x, screenrect->min_y + y, data, color);

		y++;

		if (!cdp1869->fresvert)
		{
			y++;
		}
	}
}

/* Palette Initialization */

PALETTE_INIT( cdp1869 )
{
	int i, c, l;

	// color-on-color display (CFC=0)

	for (i = 0; i < 8; i++)
	{
		palette_set_color(machine, i, cdp1869_get_rgb(i, i, 15));
	}

	// tone-on-tone display (CFC=1)

	for (c = 0; c < 8; c++)
	{
		for (l = 0; l < 8; l++)
		{
			palette_set_color(machine, i, cdp1869_get_rgb(i, c, l));
			i++;
		}
	}
}

/* Register Access */

WRITE8_DEVICE_HANDLER( cdp1869_out3_w )
{
	cdp1869_t *cdp1869 = get_safe_token(device);

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

	cdp1869->bkg = data & 0x07;
	cdp1869->cfc = BIT(data, 3);
	cdp1869->dispoff = BIT(data, 4);
	cdp1869->col = (data & 0x60) >> 5;
	cdp1869->freshorz = BIT(data, 7);
}

WRITE8_DEVICE_HANDLER( cdp1869_out4_w )
{
	cdp1869_t *cdp1869 = get_safe_token(device);

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

	cdp1869->toneamp = word & 0x0f;
	cdp1869->tonefreq = (word & 0x70) >> 4;
	cdp1869->toneoff = BIT(word, 7);
	cdp1869->tonediv = (word & 0x7f00) >> 8;

	// fork out to CDP1869 sound core
	cdp1869_set_toneamp(0, cdp1869->toneamp);
	cdp1869_set_tonefreq(0, cdp1869->tonefreq);
	cdp1869_set_toneoff(0, cdp1869->toneoff);
	cdp1869_set_tonediv(0, cdp1869->tonediv);
}

WRITE8_DEVICE_HANDLER( cdp1869_out5_w )
{
	cdp1869_t *cdp1869 = get_safe_token(device);

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

	cdp1869->cmem = BIT(word, 0);
	cdp1869->line9 = BIT(word, 3);

	if (cdp1869->intf->pal_ntsc == CDP1869_NTSC)
	{
		cdp1869->line16 = BIT(word, 5);
	}

	cdp1869->dblpage = BIT(word, 6);
	cdp1869->fresvert = BIT(word, 7);

	cdp1869->wnamp = (word & 0x0f00) >> 8;
	cdp1869->wnfreq = (word & 0x7000) >> 12;
	cdp1869->wnoff = BIT(word, 15);

	// fork out to CDP1869 sound core
	cdp1869_set_wnamp(0, cdp1869->wnamp);
	cdp1869_set_wnfreq(0, cdp1869->wnfreq);
	cdp1869_set_wnoff(0, cdp1869->wnoff);

	if (cdp1869->cmem)
	{
		cdp1869->pma = word;
	}
	else
	{
		cdp1869->pma = 0;
	}
}

WRITE8_DEVICE_HANDLER( cdp1869_out6_w )
{
	cdp1869_t *cdp1869 = get_safe_token(device);

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

	cdp1869->pma = word & 0x7ff;
}

WRITE8_DEVICE_HANDLER( cdp1869_out7_w )
{
	cdp1869_t *cdp1869 = get_safe_token(device);

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

	cdp1869->hma = word & 0x7fc;
}

/* Page RAM Access */

READ8_DEVICE_HANDLER( cdp1869_pageram_r )
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	UINT16 pma;

	if (cdp1869->cmem)
	{
		pma = cdp1869_get_pma(device);
	}
	else
	{
		pma = offset;
	}

	return cdp1869->intf->page_ram_r(device, pma);
}

WRITE8_DEVICE_HANDLER( cdp1869_pageram_w )
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	UINT16 pma;

	if (cdp1869->cmem)
	{
		pma = cdp1869_get_pma(device);
	}
	else
	{
		pma = offset;
	}

	cdp1869->intf->page_ram_w(device, pma, data);
}

/* Character RAM Access */

READ8_DEVICE_HANDLER( cdp1869_charram_r )
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	if (cdp1869->cmem)
	{
		UINT16 pma = cdp1869_get_pma(device);
		UINT8 cma = offset & 0x0f;

		if (cdp1869_get_lines(device) == 8)
		{
			cma &= 0x07;
		}

		return cdp1869->intf->char_ram_r(device, pma, cma);
	}
	else
	{
		return 0xff;
	}
}

WRITE8_DEVICE_HANDLER( cdp1869_charram_w )
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	if (cdp1869->cmem)
	{
		UINT16 pma = cdp1869_get_pma(device);
		UINT8 cma = offset & 0x0f;

		if (cdp1869_get_lines(device) == 8)
		{
			cma &= 0x07;
		}

		cdp1869->intf->char_ram_w(device, pma, cma, data);
	}
}

/* Screen Update */

void cdp1869_update(const device_config *device, bitmap_t *bitmap, const rectangle *cliprect)
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	rectangle screen_rect, outer;

	switch (cdp1869->intf->pal_ntsc)
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
	fillbitmap(bitmap, device->machine->pens[cdp1869->bkg], &outer);

	if (!cdp1869->dispoff)
	{
		int sx, sy, rows, cols, width, height;
		UINT16 addr, pmemsize;

		width = CDP1869_CHAR_WIDTH;
		height = cdp1869_get_lines(device);

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

		pmemsize = cdp1869_get_pmemsize(device, cols, rows);

		addr = cdp1869->hma;

		for (sy = 0; sy < rows; sy++)
		{
			for (sx = 0; sx < cols; sx++)
			{
				int x = sx * width;
				int y = sy * height;

				cdp1869_draw_char(device, bitmap, x, y, addr, &screen_rect);

				addr++;

				if (addr == pmemsize) addr = 0;
			}
		}
	}
}

/* Sound Update */

#ifdef UNUSED_FUNCTION
static void cdp1869_sum_square_wave(stream_sample_t *buffer, double frequency, double amplitude)
{
	// do a fast fourier transform on all the waves in the white noise range
}

static void cdp1869_sound_update(const device_config *device, stream_sample_t **inputs, stream_sample_t **_buffer, int length)
{
	cdp1869_t *cdp1869 = get_safe_token(device);

	INT16 signal = cdp1869->signal;
	stream_sample_t *buffer = _buffer[0];

	memset(buffer, 0, length * sizeof(*buffer));

	if (!cdp1869->toneoff && cdp1869->toneamp)
	{
		double frequency = (cdp1869->intf->pixel_clock / 2) / (512 >> cdp1869->tonefreq) / (cdp1869->tonediv + 1);

		int rate = device->machine->sample_rate / 2;

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

		while( length-- > 0 )
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

    if (!cdp1869->wnoff)
    {
		int wndiv;
        double amplitude = cdp1869->wnamp * ((0.78*5) / 15);

        for (wndiv = 0; wndiv < 128; wndiv++)
        {
            double frequency = (cdp1869->intf->pixel_clock / 2) / (4096 >> cdp1869->wnfreq) / (wndiv + 1);

            cdp1869_sum_square_wave(buffer, frequency, amplitude);
        }
    }
}
#endif

/* Device Interface */

static DEVICE_START( cdp1869 )
{
	cdp1869_t *cdp1869 = get_safe_token(device);
	char unique_tag[30];

	// validate arguments

	assert(device != NULL);
	assert(device->tag != NULL);
	assert(strlen(device->tag) < 20);
	assert(device->static_config != NULL);

	cdp1869->intf = device->static_config;

	assert(cdp1869->intf->page_ram_r != NULL);
	assert(cdp1869->intf->page_ram_w != NULL);
	assert(cdp1869->intf->pcb_r != NULL);
	assert(cdp1869->intf->char_ram_r != NULL);
	assert(cdp1869->intf->char_ram_w != NULL);

	// set initial values

	cdp1869->toneoff = 1;
	cdp1869->wnoff = 1;

	// get the screen device

	cdp1869->screen = device_list_find_by_tag(device->machine->config->devicelist, VIDEO_SCREEN, cdp1869->intf->screen_tag);
	assert(cdp1869->screen != NULL);

	// allocate timers

	if (cdp1869->intf->on_prd_changed != NULL)
	{
		cdp1869->prd_changed_timer = timer_alloc(prd_changed_tick, (void *)device);
		update_prd_changed_timer(cdp1869);
	}

	// register for state saving

	state_save_combine_module_and_tag(unique_tag, "CDP1869", device->tag);
	state_save_register_postload(device->machine, cdp1869_state_save_postload, cdp1869);

	state_save_register_item(unique_tag, 0, cdp1869->dispoff);
	state_save_register_item(unique_tag, 0, cdp1869->fresvert);
	state_save_register_item(unique_tag, 0, cdp1869->freshorz);
	state_save_register_item(unique_tag, 0, cdp1869->cmem);
	state_save_register_item(unique_tag, 0, cdp1869->dblpage);
	state_save_register_item(unique_tag, 0, cdp1869->line16);
	state_save_register_item(unique_tag, 0, cdp1869->line9);
	state_save_register_item(unique_tag, 0, cdp1869->cfc);
	state_save_register_item(unique_tag, 0, cdp1869->col);
	state_save_register_item(unique_tag, 0, cdp1869->bkg);
	state_save_register_item(unique_tag, 0, cdp1869->pma);
	state_save_register_item(unique_tag, 0, cdp1869->hma);

	state_save_register_item(unique_tag, 0, cdp1869->signal);
	state_save_register_item(unique_tag, 0, cdp1869->incr);
	state_save_register_item(unique_tag, 0, cdp1869->toneoff);
	state_save_register_item(unique_tag, 0, cdp1869->wnoff);
	state_save_register_item(unique_tag, 0, cdp1869->tonediv);
	state_save_register_item(unique_tag, 0, cdp1869->tonefreq);
	state_save_register_item(unique_tag, 0, cdp1869->toneamp);
	state_save_register_item(unique_tag, 0, cdp1869->wnfreq);
	state_save_register_item(unique_tag, 0, cdp1869->wnamp);
}

static DEVICE_SET_INFO( cdp1869 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}

DEVICE_GET_INFO( cdp1869_video )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(cdp1869_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;								break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_SET_INFO:						info->set_info = DEVICE_SET_INFO_NAME(cdp1869); break;
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(cdp1869);	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							/* Nothing */								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							info->s = "RCA CDP1869";					break;
		case DEVINFO_STR_FAMILY:						info->s = "RCA CDP1869";					break;
		case DEVINFO_STR_VERSION:						info->s = "1.0";							break;
		case DEVINFO_STR_SOURCE_FILE:					info->s = __FILE__;							break;
		case DEVINFO_STR_CREDITS:						info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}
