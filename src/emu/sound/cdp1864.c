/**********************************************************************

    RCA CDP1864C COS/MOS PAL Compatible Color TV Interface

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - interlace mode
    - PAL output, currently using RGB
    - cpu synchronization

        SC1 and SC0 are used to provide CDP1864C-to-CPU synchronization for a jitter-free display.
        During every horizontal sync the CDP1864C samples SC0 and SC1 for SC0 = 1 and SC1 = 0
        (CDP1800 execute state). Detection of a fetch cycle causes the CDP1864C to skip cycles to
        attain synchronization. (i.e. picture moves 8 pixels to the right)

*/

#include "emu.h"
#include "streams.h"
#include "cdp1864.h"
#include "cpu/cdp1802/cdp1802.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define CDP1864_DEFAULT_LATCH		0x35

#define CDP1864_CYCLES_DMA_START	2*8
#define CDP1864_CYCLES_DMA_ACTIVE	8*8
#define CDP1864_CYCLES_DMA_WAIT		6*8

static const int CDP1864_BACKGROUND_COLOR_SEQUENCE[] = { 2, 0, 1, 4 };

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _cdp1864_t cdp1864_t;
struct _cdp1864_t
{
	devcb_resolved_read_line		in_rdata_func;
	devcb_resolved_read_line		in_bdata_func;
	devcb_resolved_read_line		in_gdata_func;
	devcb_resolved_write_line		out_int_func;
	devcb_resolved_write_line		out_dmao_func;
	devcb_resolved_write_line		out_efx_func;

	const device_config *screen;	/* screen */
	bitmap_t *bitmap;				/* bitmap */
	sound_stream *stream;			/* sound output */

	/* video state */
	int disp;						/* display on */
	int dmaout;						/* DMA request active */
	int bgcolor;					/* background color */
	int con;						/* color on */

	/* sound state */
	int aoe;						/* audio on */
	int latch;						/* sound latch */
	INT16 signal;					/* current signal */
	int incr;						/* initial wave state */

	/* timers */
	emu_timer *int_timer;			/* interrupt timer */
	emu_timer *efx_timer;			/* EFx timer */
	emu_timer *dma_timer;			/* DMA timer */

	const device_config *cpu;
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE cdp1864_t *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	return (cdp1864_t *)device->token;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    TIMER_CALLBACK( cdp1864_int_tick )
-------------------------------------------------*/

static TIMER_CALLBACK( cdp1864_int_tick )
{
	const device_config *device = (const device_config *) ptr;
	cdp1864_t *cdp1864 = get_safe_token(device);

	int scanline = video_screen_get_vpos(cdp1864->screen);

	if (scanline == CDP1864_SCANLINE_INT_START)
	{
		if (cdp1864->disp)
		{
			devcb_call_write_line(&cdp1864->out_int_func, ASSERT_LINE);
		}

		timer_adjust_oneshot(cdp1864->int_timer, video_screen_get_time_until_pos(cdp1864->screen, CDP1864_SCANLINE_INT_END, 0), 0);
	}
	else
	{
		if (cdp1864->disp)
		{
			devcb_call_write_line(&cdp1864->out_int_func, CLEAR_LINE);
		}

		timer_adjust_oneshot(cdp1864->int_timer, video_screen_get_time_until_pos(cdp1864->screen, CDP1864_SCANLINE_INT_START, 0), 0);
	}
}

/*-------------------------------------------------
    TIMER_CALLBACK( cdp1864_efx_tick )
-------------------------------------------------*/

static TIMER_CALLBACK( cdp1864_efx_tick )
{
	const device_config *device = (const device_config *) ptr;
	cdp1864_t *cdp1864 = get_safe_token(device);

	int scanline = video_screen_get_vpos(cdp1864->screen);

	switch (scanline)
	{
	case CDP1864_SCANLINE_EFX_TOP_START:
		devcb_call_write_line(&cdp1864->out_efx_func, ASSERT_LINE);
		timer_adjust_oneshot(cdp1864->efx_timer, video_screen_get_time_until_pos(cdp1864->screen, CDP1864_SCANLINE_EFX_TOP_END, 0), 0);
		break;

	case CDP1864_SCANLINE_EFX_TOP_END:
		devcb_call_write_line(&cdp1864->out_efx_func, CLEAR_LINE);
		timer_adjust_oneshot(cdp1864->efx_timer, video_screen_get_time_until_pos(cdp1864->screen, CDP1864_SCANLINE_EFX_BOTTOM_START, 0), 0);
		break;

	case CDP1864_SCANLINE_EFX_BOTTOM_START:
		devcb_call_write_line(&cdp1864->out_efx_func, ASSERT_LINE);
		timer_adjust_oneshot(cdp1864->efx_timer, video_screen_get_time_until_pos(cdp1864->screen, CDP1864_SCANLINE_EFX_BOTTOM_END, 0), 0);
		break;

	case CDP1864_SCANLINE_EFX_BOTTOM_END:
		devcb_call_write_line(&cdp1864->out_efx_func, CLEAR_LINE);
		timer_adjust_oneshot(cdp1864->efx_timer, video_screen_get_time_until_pos(cdp1864->screen, CDP1864_SCANLINE_EFX_TOP_START, 0), 0);
		break;
	}
}

/*-------------------------------------------------
    TIMER_CALLBACK( cdp1864_dma_tick )
-------------------------------------------------*/

static TIMER_CALLBACK( cdp1864_dma_tick )
{
	const device_config *device = (const device_config *) ptr;
	cdp1864_t *cdp1864 = get_safe_token(device);

	int scanline = video_screen_get_vpos(cdp1864->screen);

	if (cdp1864->dmaout)
	{
		if (cdp1864->disp)
		{
			if (scanline >= CDP1864_SCANLINE_DISPLAY_START && scanline < CDP1864_SCANLINE_DISPLAY_END)
			{
				devcb_call_write_line(&cdp1864->out_dmao_func, CLEAR_LINE);
			}
		}

		timer_adjust_oneshot(cdp1864->dma_timer, cpu_clocks_to_attotime(machine->firstcpu, CDP1864_CYCLES_DMA_WAIT), 0);

		cdp1864->dmaout = 0;
	}
	else
	{
		if (cdp1864->disp)
		{
			if (scanline >= CDP1864_SCANLINE_DISPLAY_START && scanline < CDP1864_SCANLINE_DISPLAY_END)
			{
				devcb_call_write_line(&cdp1864->out_dmao_func, ASSERT_LINE);
			}
		}

		timer_adjust_oneshot(cdp1864->dma_timer, cpu_clocks_to_attotime(machine->firstcpu, CDP1864_CYCLES_DMA_ACTIVE), 0);

		cdp1864->dmaout = 1;
	}
}

/*-------------------------------------------------
    cdp1864_init_palette - initialize palette
-------------------------------------------------*/

static void cdp1864_init_palette(const device_config *device, const cdp1864_interface *intf)
{
	int i;

	double res_total = intf->res_r + intf->res_g + intf->res_b + intf->res_bkg;

	int weight_r = (intf->res_r / res_total) * 100;
	int weight_g = (intf->res_g / res_total) * 100;
	int weight_b = (intf->res_b / res_total) * 100;
	int weight_bkg = (intf->res_bkg / res_total) * 100;

	for (i = 0; i < 16; i++)
	{
		int r, g, b, luma = 0;

		luma += (i & 4) ? weight_r : 0;
		luma += (i & 1) ? weight_g : 0;
		luma += (i & 2) ? weight_b : 0;
		luma += (i & 8) ? 0 : weight_bkg;

		luma = (luma * 0xff) / 100;

		r = (i & 4) ? luma : 0;
		g = (i & 1) ? luma : 0;
		b = (i & 2) ? luma : 0;

		palette_set_color_rgb( device->machine, i, r, g, b );
	}
}

/*-------------------------------------------------
    cdp1864_aoe_w - audio output enable
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( cdp1864_aoe_w )
{
	cdp1864_t *cdp1864 = get_safe_token(device);

	if (!state)
	{
		cdp1864->latch = CDP1864_DEFAULT_LATCH;
	}

	cdp1864->aoe = state;
}

/*-------------------------------------------------
    cdp1864_dispon_r - turn display on
-------------------------------------------------*/

READ8_DEVICE_HANDLER( cdp1864_dispon_r )
{
	cdp1864_t *cdp1864 = get_safe_token(device);

	cdp1864->disp = 1;

	return 0xff;
}

/*-------------------------------------------------
    cdp1864_dispoff_r - turn display off
-------------------------------------------------*/

READ8_DEVICE_HANDLER( cdp1864_dispoff_r )
{
	cdp1864_t *cdp1864 = get_safe_token(device);

	cdp1864->disp = 0;

	devcb_call_write_line(&cdp1864->out_int_func, CLEAR_LINE);
	devcb_call_write_line(&cdp1864->out_dmao_func, CLEAR_LINE);

	return 0xff;
}

/*-------------------------------------------------
    cdp1864_step_bgcolor_w - step background color
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( cdp1864_step_bgcolor_w )
{
	cdp1864_t *cdp1864 = get_safe_token(device);

	cdp1864->disp = 1;

	if (++cdp1864->bgcolor > 3) cdp1864->bgcolor = 0;
}

/*-------------------------------------------------
    cdp1864_con_w - color on write
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( cdp1864_con_w )
{
	cdp1864_t *cdp1864 = get_safe_token(device);

	if (!state)
	{
		cdp1864->con = 0;
	}
}

/*-------------------------------------------------
    cdp1864_tone_latch_w - load tone latch
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( cdp1864_tone_latch_w )
{
	cdp1864_t *cdp1864 = get_safe_token(device);

	cdp1864->latch = data;
}

/*-------------------------------------------------
    cdp1864_dma_w - write DMA byte
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( cdp1864_dma_w )
{
	cdp1864_t *cdp1864 = get_safe_token(device);

	int rdata = 1, bdata = 1, gdata = 1;
	int sx = video_screen_get_hpos(cdp1864->screen) + 4;
	int y = video_screen_get_vpos(cdp1864->screen);
	int x;

	if (!cdp1864->con)
	{
		rdata = devcb_call_read_line(&cdp1864->in_rdata_func);
		bdata = devcb_call_read_line(&cdp1864->in_bdata_func);
		gdata = devcb_call_read_line(&cdp1864->in_gdata_func);
	}

	for (x = 0; x < 8; x++)
	{
		int color = CDP1864_BACKGROUND_COLOR_SEQUENCE[cdp1864->bgcolor] + 8;

		if (BIT(data, 7))
		{
			color = (gdata << 2) | (bdata << 1) | rdata;
		}

		*BITMAP_ADDR16(cdp1864->bitmap, y, sx + x) = color;

		data <<= 1;
	}
}

/*-------------------------------------------------
    STREAM_UPDATE( cdp1864_stream_update )
-------------------------------------------------*/

static STREAM_UPDATE( cdp1864_stream_update )
{
	cdp1864_t *cdp1864 = get_safe_token(device);

	INT16 signal = cdp1864->signal;
	stream_sample_t *buffer = outputs[0];

	memset( buffer, 0, samples * sizeof(*buffer) );

	if (cdp1864->aoe)
	{
		double frequency = cpu_get_clock(cdp1864->cpu) / 8 / 4 / (cdp1864->latch + 1) / 2;
		int rate = device->machine->sample_rate / 2;

		/* get progress through wave */
		int incr = cdp1864->incr;

		if (signal < 0)
		{
			signal = -0x7fff;
		}
		else
		{
			signal = 0x7fff;
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
		cdp1864->incr = incr;
		cdp1864->signal = signal;
	}
}

/*-------------------------------------------------
    cdp1864_update - update screen
-------------------------------------------------*/

void cdp1864_update(const device_config *device, bitmap_t *bitmap, const rectangle *cliprect)
{
	cdp1864_t *cdp1864 = get_safe_token(device);

	if (cdp1864->disp)
	{
		copybitmap(bitmap, cdp1864->bitmap, 0, 0, 0, 0, cliprect);
		bitmap_fill(cdp1864->bitmap, cliprect, CDP1864_BACKGROUND_COLOR_SEQUENCE[cdp1864->bgcolor] + 8);
	}
	else
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(device->machine));
	}
}

/*-------------------------------------------------
    DEVICE_START( cdp1864 )
-------------------------------------------------*/

static DEVICE_START( cdp1864 )
{
	cdp1864_t *cdp1864 = get_safe_token(device);
	const cdp1864_interface *intf = (const cdp1864_interface *) device->static_config;

	/* resolve callbacks */
	devcb_resolve_read_line(&cdp1864->in_rdata_func, &intf->in_rdata_func, device);
	devcb_resolve_read_line(&cdp1864->in_bdata_func, &intf->in_bdata_func, device);
	devcb_resolve_read_line(&cdp1864->in_gdata_func, &intf->in_gdata_func, device);
	devcb_resolve_write_line(&cdp1864->out_int_func, &intf->out_int_func, device);
	devcb_resolve_write_line(&cdp1864->out_dmao_func, &intf->out_dmao_func, device);
	devcb_resolve_write_line(&cdp1864->out_efx_func, &intf->out_efx_func, device);

	/* get the cpu */
	cdp1864->cpu = device->machine->device(intf->cpu_tag);

	/* get the screen device */
	cdp1864->screen = device->machine->device(intf->screen_tag);
	assert(cdp1864->screen != NULL);

	/* allocate the temporary bitmap */
	cdp1864->bitmap = auto_bitmap_alloc(device->machine, video_screen_get_width(cdp1864->screen), video_screen_get_height(cdp1864->screen), video_screen_get_format(cdp1864->screen));
	bitmap_fill(cdp1864->bitmap, 0, CDP1864_BACKGROUND_COLOR_SEQUENCE[cdp1864->bgcolor] + 8);

	/* initialize the palette */
	cdp1864_init_palette(device, intf);

	/* create sound stream */
	cdp1864->stream = stream_create(device, 0, 1, device->machine->sample_rate, cdp1864, cdp1864_stream_update);

	/* create the timers */
	cdp1864->int_timer = timer_alloc(device->machine, cdp1864_int_tick, (void *)device);
	cdp1864->efx_timer = timer_alloc(device->machine, cdp1864_efx_tick, (void *)device);
	cdp1864->dma_timer = timer_alloc(device->machine, cdp1864_dma_tick, (void *)device);

	/* register for state saving */
	state_save_register_device_item(device, 0, cdp1864->disp);
	state_save_register_device_item(device, 0, cdp1864->dmaout);
	state_save_register_device_item(device, 0, cdp1864->bgcolor);
	state_save_register_device_item(device, 0, cdp1864->con);

	state_save_register_device_item(device, 0, cdp1864->aoe);
	state_save_register_device_item(device, 0, cdp1864->latch);
	state_save_register_device_item(device, 0, cdp1864->signal);
	state_save_register_device_item(device, 0, cdp1864->incr);

	state_save_register_device_item_bitmap(device, 0, cdp1864->bitmap);
}

/*-------------------------------------------------
    DEVICE_RESET( cdp1864 )
-------------------------------------------------*/

static DEVICE_RESET( cdp1864 )
{
	cdp1864_t *cdp1864 = get_safe_token(device);

	timer_adjust_oneshot(cdp1864->int_timer, video_screen_get_time_until_pos(cdp1864->screen, CDP1864_SCANLINE_INT_START, 0), 0);
	timer_adjust_oneshot(cdp1864->efx_timer, video_screen_get_time_until_pos(cdp1864->screen, CDP1864_SCANLINE_EFX_TOP_START, 0), 0);
	timer_adjust_oneshot(cdp1864->dma_timer, cpu_clocks_to_attotime(device->machine->firstcpu, CDP1864_CYCLES_DMA_START), 0);

	cdp1864->disp = 0;
	cdp1864->dmaout = 0;
	cdp1864->bgcolor = 0;
	cdp1864->con = 1;

	devcb_call_write_line(&cdp1864->out_int_func, CLEAR_LINE);
	devcb_call_write_line(&cdp1864->out_dmao_func, CLEAR_LINE);
	devcb_call_write_line(&cdp1864->out_efx_func, CLEAR_LINE);

	cdp1864_aoe_w(device, 0);
}

/*-------------------------------------------------
    DEVICE_GET_INFO( cdp1864 )
-------------------------------------------------*/

DEVICE_GET_INFO( cdp1864 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(cdp1864_t);						break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;										break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(cdp1864);			break;
		case DEVINFO_FCT_STOP:							/* Nothing */										break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(cdp1864);			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "RCA CDP1864");						break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "RCA CDP1800");						break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");								break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);							break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright MESS Team");				break;
	}
}
