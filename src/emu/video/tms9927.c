/**********************************************************************

    TI TMS9927 and compatible CRT controller emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "tms9927.h"


static const UINT8 chars_per_row_value[8] = { 20, 32, 40, 64, 72, 80, 96, 132 };
static const UINT8 skew_bits_value[4] = { 0, 1, 2, 2 };


#define HCOUNT(t)				((t)->reg[0] + 1)
#define INTERLACED(t)			(((t)->reg[1] >> 7) & 0x01)
#define HSYNC_WIDTH(t)			(((t)->reg[1] >> 4) & 0x0f)
#define HSYNC_DELAY(t)			(((t)->reg[1] >> 0) & 0x07)
#define SCANS_PER_DATA_ROW(t)	((((t)->reg[2] >> 3) & 0x0f) + 1)
#define CHARS_PER_DATA_ROW(t)	(chars_per_row_value[((t)->reg[2] >> 0) & 0x07])
#define SKEW_BITS(t)			(skew_bits_value[((t)->reg[3] >> 6) & 0x03])
#define DATA_ROWS_PER_FRAME(t)	((((t)->reg[3] >> 0) & 0x3f) + 1)
#define SCAN_LINES_PER_FRAME(t)	(((t)->reg[4] * 2) + 256)
#define VERTICAL_DATA_START(t)	((t)->reg[5])
#define LAST_DISP_DATA_ROW(t)	((t)->reg[6] & 0x3f)
#define CURSOR_CHAR_ADDRESS(t)	((t)->reg[7])
#define CURSOR_ROW_ADDRESS(t)	((t)->reg[8] & 0x3f)


typedef struct _tms9927_state tms9927_state;
struct _tms9927_state
{
	/* driver-controlled state */
	const tms9927_interface *intf;
	screen_device *screen;
	const UINT8 *selfload;

	/* live state */
	UINT32	clock;
	UINT8	reg[9];
	UINT8	start_datarow;
	UINT8	reset;
	UINT8	hpixels_per_column;

	/* derived state; no need to save */
	UINT8	valid_config;
	UINT16	total_hpix, total_vpix;
	UINT16	visible_hpix, visible_vpix;
};


static void tms9927_state_save_postload(tms9927_state *state);
static void recompute_parameters(tms9927_state *tms, int postload);


const tms9927_interface tms9927_null_interface = { 0 };


/* makes sure that the passed in device is the right type */
INLINE tms9927_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == TMS9927);
	return (tms9927_state *)downcast<legacy_device_base *>(device)->token();
}


static void tms9927_state_save_postload(tms9927_state *state)
{
	recompute_parameters(state, TRUE);
}


static void generic_access(device_t *device, offs_t offset)
{
	tms9927_state *tms = get_safe_token(device);

	switch (offset)
	{
		case 0x07:	/* Processor Self Load */
		case 0x0f:	/* Non-processor self-load */
			if (tms->selfload != NULL)
			{
				int cur;

				for (cur = 0; cur < 7; cur++)
					tms9927_w(device, cur, tms->selfload[cur]);
				for (cur = 0; cur < 1; cur++)
					tms9927_w(device, cur + 0xc, tms->selfload[cur + 7]);
			}
			else
				popmessage("tms9927: self-load initiated with no PROM!");

			/* processor self-load waits with reset enabled;
               non-processor just goes ahead */
			tms->reset = (offset == 0x07);
			break;

		case 0x0a:	/* Reset */
			if (!tms->reset)
			{
				tms->screen->update_now();
				tms->reset = TRUE;
			}
			break;

		case 0x0b:	/* Up scroll */
mame_printf_debug("Up scroll\n");
			tms->screen->update_now();
			tms->start_datarow = (tms->start_datarow + 1) % DATA_ROWS_PER_FRAME(tms);
			break;

		case 0x0e:	/* Start timing chain */
			if (tms->reset)
			{
				tms->screen->update_now();
				tms->reset = FALSE;
				recompute_parameters(tms, FALSE);
			}
			break;
	}
}


WRITE8_DEVICE_HANDLER( tms9927_w )
{
	tms9927_state *tms = get_safe_token(device);

	switch (offset)
	{
		case 0x00:	/* HORIZONTAL CHARACTER COUNT */
		case 0x01:	/* INTERLACED / HSYNC WIDTH / HSYNC DELAY */
		case 0x02:	/* SCANS PER DATA ROW / CHARACTERS PER DATA ROW */
		case 0x03:	/* SKEW BITS / DATA ROWS PER FRAME */
		case 0x04:	/* SCAN LINES / FRAME */
		case 0x05:	/* VERTICAL DATA START */
		case 0x06:	/* LAST DISPLAYED DATA ROW */
			tms->reg[offset] = data;
			recompute_parameters(tms, FALSE);
			break;

		case 0x0c:	/* LOAD CURSOR CHARACTER ADDRESS */
		case 0x0d:	/* LOAD CURSOR ROW ADDRESS */
mame_printf_debug("Cursor address changed\n");
			tms->reg[offset - 0x0c + 7] = data;
			recompute_parameters(tms, FALSE);
			break;

		default:
			generic_access(device, offset);
			break;
	}
}


READ8_DEVICE_HANDLER( tms9927_r )
{
	tms9927_state *tms = get_safe_token(device);

	switch (offset)
	{
		case 0x08:	/* READ CURSOR CHARACTER ADDRESS */
		case 0x09:	/* READ CURSOR ROW ADDRESS */
			return tms->reg[offset - 0x08 + 7];

		default:
			generic_access(device, offset);
			break;
	}
	return 0xff;
}


int tms9927_screen_reset(device_t *device)
{
	tms9927_state *tms = get_safe_token(device);
	return tms->reset;
}


int tms9927_upscroll_offset(device_t *device)
{
	tms9927_state *tms = get_safe_token(device);
	return tms->start_datarow;
}


int tms9927_cursor_bounds(device_t *device, rectangle &bounds)
{
	tms9927_state *tms = get_safe_token(device);
	int cursorx = CURSOR_CHAR_ADDRESS(tms);
	int cursory = CURSOR_ROW_ADDRESS(tms);

	bounds.min_x = cursorx * tms->hpixels_per_column;
	bounds.max_x = bounds.min_x + tms->hpixels_per_column - 1;
	bounds.min_y = cursory * SCANS_PER_DATA_ROW(tms);
	bounds.max_y = bounds.min_y + SCANS_PER_DATA_ROW(tms) - 1;

	return (cursorx < HCOUNT(tms) && cursory <= LAST_DISP_DATA_ROW(tms));
}


static void recompute_parameters(tms9927_state *tms, int postload)
{
	UINT16 offset_hpix, offset_vpix;
	attoseconds_t refresh;
	rectangle visarea;

	if (tms->intf == NULL || tms->reset)
		return;

	/* compute the screen sizes */
	tms->total_hpix = HCOUNT(tms) * tms->hpixels_per_column;
	tms->total_vpix = SCAN_LINES_PER_FRAME(tms);

	/* determine the visible area, avoid division by 0 */
	tms->visible_hpix = CHARS_PER_DATA_ROW(tms) * tms->hpixels_per_column;
	tms->visible_vpix = (LAST_DISP_DATA_ROW(tms) + 1) * SCANS_PER_DATA_ROW(tms);

	/* determine the horizontal/vertical offsets */
	offset_hpix = HSYNC_DELAY(tms) * tms->hpixels_per_column;
	offset_vpix = VERTICAL_DATA_START(tms);

	mame_printf_debug("TMS9937: Total = %dx%d, Visible = %dx%d, Offset=%dx%d, Skew=%d\n", tms->total_hpix, tms->total_vpix, tms->visible_hpix, tms->visible_vpix, offset_hpix, offset_vpix, SKEW_BITS(tms));

	/* see if it all makes sense */
	tms->valid_config = TRUE;
	if (tms->visible_hpix > tms->total_hpix || tms->visible_vpix > tms->total_vpix)
	{
		tms->valid_config = FALSE;
		logerror("tms9927: invalid visible size (%dx%d) versus total size (%dx%d)\n", tms->visible_hpix, tms->visible_vpix, tms->total_hpix, tms->total_vpix);
	}

	/* update */
	if (!tms->valid_config)
		return;

	/* create a visible area */
	/* fix me: how do the offsets fit in here? */
	visarea.set(0, tms->visible_hpix - 1, 0, tms->visible_vpix - 1);

	refresh = HZ_TO_ATTOSECONDS(tms->clock) * tms->total_hpix * tms->total_vpix;

	tms->screen->configure(tms->total_hpix, tms->total_vpix, visarea, refresh);
}


/* device interface */
static DEVICE_START( tms9927 )
{
	tms9927_state *tms = get_safe_token(device);

	/* validate arguments */
	assert(device != NULL);

	tms->intf = (const tms9927_interface *)device->static_config();

	if (tms->intf != NULL)
	{
		assert(device->clock() > 0);
		assert(tms->intf->hpixels_per_column > 0);

		/* copy the initial parameters */
		tms->clock = device->clock();
		tms->hpixels_per_column = tms->intf->hpixels_per_column;

		/* get the screen device */
		tms->screen = downcast<screen_device *>(device->machine().device(tms->intf->screen_tag));
		assert(tms->screen != NULL);

		/* get the self-load PROM */
		if (tms->intf->selfload_region != NULL)
		{
			tms->selfload = device->machine().root_device().memregion(tms->intf->selfload_region)->base();
			assert(tms->selfload != NULL);
		}
	}

	/* register for state saving */
	device->machine().save().register_postload(save_prepost_delegate(FUNC(tms9927_state_save_postload), tms));

	device->save_item(NAME(tms->clock));
	device->save_item(NAME(tms->reg));
	device->save_item(NAME(tms->start_datarow));
	device->save_item(NAME(tms->reset));
	device->save_item(NAME(tms->hpixels_per_column));
}


static DEVICE_STOP( tms9927 )
{
	tms9927_state *tms = get_safe_token(device);

	mame_printf_debug("TMS9937: Final params: (%d, %d, %d, %d, %d, %d, %d)\n",
						tms->clock,
						tms->total_hpix,
						0, tms->visible_hpix,
						tms->total_vpix,
						0, tms->visible_vpix);
}


static DEVICE_RESET( tms9927 )
{
}


DEVICE_GET_INFO( tms9927 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(tms9927_state);			break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(tms9927);	break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME(tms9927);		break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(tms9927);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "TMS9927");					break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "TMS9927 CRTC");			break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

DEVICE_GET_INFO( crt5027 )
{
	switch (state)
	{
		case DEVINFO_STR_NAME:							strcpy(info->s, "CRT5027");					break;
		default:										DEVICE_GET_INFO_CALL(tms9927);				break;
	}
}

DEVICE_GET_INFO( crt5037 )
{
	switch (state)
	{
		case DEVINFO_STR_NAME:							strcpy(info->s, "CRT5037");					break;
		default:										DEVICE_GET_INFO_CALL(tms9927);				break;
	}
}

DEVICE_GET_INFO( crt5057 )
{
	switch (state)
	{
		case DEVINFO_STR_NAME:							strcpy(info->s, "CRT5057");					break;
		default:										DEVICE_GET_INFO_CALL(tms9927);				break;
	}
}


DEFINE_LEGACY_DEVICE(TMS9927, tms9927);
DEFINE_LEGACY_DEVICE(CRT5027, crt5027);
DEFINE_LEGACY_DEVICE(CRT5037, crt5037);
DEFINE_LEGACY_DEVICE(CRT5057, crt5057);
