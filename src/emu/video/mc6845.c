/**********************************************************************

    Motorola MC6845 and compatible CRT controller emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

    The following variations exist that are different in
    functionality and not just in speed rating(1):
        * Motorola 6845, 6845-1
        * Hitachi 46505
        * Rockwell 6545, 6545-1 (= Synertek SY6545-1)
        * Commodore 6545-1

    (1) as per the document at
    http://www.6502.org/users/andre/hwinfo/crtc/diffs.html

    The various speed rated devices are identified by a letter,
    for example M68A45, M68B45, etc.

    The chip is originally designed by Hitachi, not by Motorola.

**********************************************************************/

#include "driver.h"
#include "mc6845.h"


#define LOG		(0)


struct _mc6845_t
{
	running_machine *machine;
	const mc6845_interface *intf;

	/* internal registers */
	UINT8	address_latch;
	UINT8	horiz_total;
	UINT8	horiz_disp;
	UINT8	horiz_sync_pos;
	UINT8	sync_width;
	UINT8	vert_total;
	UINT8	vert_total_adj;
	UINT8	vert_disp;
	UINT8	vert_sync_pos;
	UINT8	intl_skew;
	UINT8	max_ras_addr;
	UINT8	cursor_start_ras;
	UINT8	cursor_end_ras;
	UINT16	start_addr;
	UINT16	cursor;
	UINT16	light_pen;

	emu_timer *display_enable_changed_timer;
	UINT8	cursor_state;	/* 0 = off, 1 = on */
	UINT8	cursor_blink_count;

	/* saved screen parameters so we don't call
       video_screen_configure() unneccessarily.
       Do NOT state save these! */
	UINT16	last_horiz_total;
	UINT16	last_vert_total;
	UINT16	last_max_x;
	UINT16	last_max_y;
	UINT16  current_ma;		/* the MA address currently drawn */
	int		has_valid_parameters;
};


static void mc6845_state_save_postload(void *param);
static void configure_screen(mc6845_t *mc6845, int postload);
static void update_timer(mc6845_t *mc6845);
static TIMER_CALLBACK( display_enable_changed_timer_cb );


static void mc6845_state_save_postload(void *param)
{
	mc6845_t *mc6845 = (mc6845_t *)param;

	configure_screen(mc6845, TRUE);
}


void mc6845_address_w(mc6845_t *mc6845, UINT8 data)
{
	mc6845->address_latch = data & 0x1f;
}


UINT8 mc6845_register_r(mc6845_t *mc6845)
{
	UINT8 ret = 0;

	switch (mc6845->address_latch)
	{
		case 12:
			/* Motorola version only */
			ret = mc6845->start_addr >> 8;
			break;
		case 13:
			/* Motorola version only */
			ret = mc6845->start_addr;
			break;
		case 14:
			ret = mc6845->cursor >> 8;
			break;
		case 15:
			ret = mc6845->cursor;
			break;
		case 16:
			ret = mc6845->light_pen >> 8;
			break;
		case 17:
			ret = mc6845->light_pen;
			break;
		default:
			/* all other registers are write only */
			break;
	}

	return ret;
}


void mc6845_register_w(mc6845_t *mc6845, UINT8 data)
{
	int call_configure_screen = FALSE;
	if (LOG)  logerror("M6845 PC %04x: reg 0x%02x = 0x%02x\n", activecpu_get_pc(), mc6845->address_latch, data);

	switch (mc6845->address_latch)
	{
		case 0:
			mc6845->horiz_total = data;
			call_configure_screen = TRUE;
			break;
		case 1:
			mc6845->horiz_disp = data;
			call_configure_screen = TRUE;
			break;
		case 2:
			mc6845->horiz_sync_pos = data;
			break;
		case 3:
			mc6845->sync_width = data & 0x0f;
			break;
		case 4:
			mc6845->vert_total = data & 0x7f;
			call_configure_screen = TRUE;
			break;
		case 5:
			mc6845->vert_total_adj = data & 0x1f;
			call_configure_screen = TRUE;
			break;
		case 6:
			mc6845->vert_disp = data & 0x7f;
			call_configure_screen = TRUE;
			break;
		case 7:
			mc6845->vert_sync_pos = data & 0x7f;
			break;
		case 8:
			mc6845->intl_skew = data & 0x03;
			break;
		case 9:
			mc6845->max_ras_addr = data & 0x1f;
			call_configure_screen = TRUE;
			break;
		case 10:
			mc6845->cursor_start_ras = data & 0x7f;
			break;
		case 11:
			mc6845->cursor_end_ras = data & 0x1f;
			break;
		case 12:
			mc6845->start_addr &= 0x00ff;
			mc6845->start_addr |= (data & 0x3f) << 8;
			call_configure_screen = TRUE;
			break;
		case 13:
			mc6845->start_addr &= 0xff00;
			mc6845->start_addr |= data;
			call_configure_screen = TRUE;
			break;
		case 14:
			mc6845->cursor &= 0x00ff;
			mc6845->cursor |= (data & 0x3f) << 8;
			break;
		case 15:
			mc6845->cursor &= 0xff00;
			mc6845->cursor |= data;
			break;
		case 16:	/* read-only */
			break;
		case 17:	/* read-only */
			break;
		default:
			break;
	}

	if (call_configure_screen)
		configure_screen(mc6845, FALSE);
}


static void configure_screen(mc6845_t *mc6845, int postload)
{
	if (mc6845->intf)
	{
		/* compute the screen sizes */
		UINT16 horiz_total = (mc6845->horiz_total + 1) * mc6845->intf->hpixels_per_column;
		UINT16 vert_total = (mc6845->vert_total + 1) * (mc6845->max_ras_addr + 1) + mc6845->vert_total_adj;

		/* determine the visible area, avoid division by 0 */
		UINT16 max_x = mc6845->horiz_disp * mc6845->intf->hpixels_per_column - 1;
		UINT16 max_y = mc6845->vert_disp * (mc6845->max_ras_addr + 1) - 1;

		/* update only if screen parameters changed, unless we are coming here after loading the saved state */
		if (postload ||
		    (horiz_total != mc6845->last_horiz_total) || (vert_total != mc6845->last_vert_total) ||
			(max_x != mc6845->last_max_x) || (max_y != mc6845->last_max_y))
		{
			/* update the screen only if we have valid data */
			if ((mc6845->horiz_total > 0) && (max_x < horiz_total) && (mc6845->vert_total > 0) && (max_y < vert_total))
			{
				rectangle visarea;

				attoseconds_t refresh = HZ_TO_ATTOSECONDS(mc6845->intf->clock) * (mc6845->horiz_total + 1) * vert_total;

				visarea.min_x = 0;
				visarea.min_y = 0;
				visarea.max_x = max_x;
				visarea.max_y = max_y;

				if (LOG) logerror("M6845 config screen: HTOTAL: %x  VTOTAL: %x  MAX_X: %x  MAX_Y: %x  FPS: %f\n",
								  horiz_total, vert_total, max_x, max_y, 1 / ATTOSECONDS_TO_DOUBLE(refresh));

				video_screen_configure(mc6845->intf->scrnum, horiz_total, vert_total, &visarea, refresh);

				mc6845->has_valid_parameters = TRUE;
			}
			else
				mc6845->has_valid_parameters = FALSE;

			mc6845->last_horiz_total = horiz_total;
			mc6845->last_vert_total = vert_total;
			mc6845->last_max_x = max_x;
			mc6845->last_max_y = max_y;

			update_timer(mc6845);
		}
	}
}


static int is_display_enabled(mc6845_t *mc6845)
{
	UINT16 y = video_screen_get_vpos(mc6845->intf->scrnum);
	UINT16 x = video_screen_get_hpos(mc6845->intf->scrnum);

	return (y <= mc6845->last_max_y) && (x <= mc6845->last_max_x);
}


static void update_timer(mc6845_t *mc6845)
{
	INT16 next_y;
	UINT16 next_x;
	attotime duration;

	if (mc6845->has_valid_parameters && (mc6845->display_enable_changed_timer != 0))
	{
		if (is_display_enabled(mc6845))
		{
			/* we are in a display region,
               get the location of the next blanking start */

			/* normally, it's at end the current raster line */
			next_y = video_screen_get_vpos(mc6845->intf->scrnum);
			next_x = mc6845->last_max_x + 1;

			/* but if visible width = horiz_total, then we need
               to go to the beginning of VBLANK */
			if (next_x == mc6845->last_horiz_total)
			{
				next_y = mc6845->last_max_y + 1;
				next_x = 0;

				/* abnormal case, no vertical blanking, either */
				if (next_y == mc6845->last_vert_total)
					next_y = -1;
			}
		}
		else
		{
			/* we are in a blanking region,
               get the location of the next display start */
			next_x = 0;
			next_y = (video_screen_get_vpos(mc6845->intf->scrnum) + 1) % mc6845->last_vert_total;

			/* if we would now fall in the vertical blanking, we need
               to go to the top of the screen */
			if (next_y > mc6845->last_max_y)
				next_y = 0;
		}

		if (next_y != -1)
			duration = video_screen_get_time_until_pos(mc6845->intf->scrnum, next_y, next_x);
		else
			duration = attotime_never;

		timer_adjust_oneshot(mc6845->display_enable_changed_timer, duration, 0);
	}
}


static TIMER_CALLBACK( display_enable_changed_timer_cb )
{
	mc6845_t *mc6845 = ptr;

	/* call the callback function -- we know it exists */
	mc6845->intf->display_enable_changed(mc6845->machine, mc6845, is_display_enabled(mc6845));

	update_timer(mc6845);
}


UINT16 mc6845_get_ma(mc6845_t *mc6845)
{
	UINT16 ret;

	if (mc6845->has_valid_parameters)
	{
		/* get the current raster positions and clamp them to the visible region */
		int y = video_screen_get_vpos(0);
		int x = video_screen_get_hpos(0);

		/* since the MA counter stops in the blanking regions, if we are in a
           VBLANK, both X and Y are at their max */
		if ((y > mc6845->last_max_y) || (x > mc6845->last_max_x))
			x = mc6845->last_max_x;

		if (y > mc6845->last_max_y)
			y = mc6845->last_max_y;

		ret = (mc6845->start_addr +
			   (y / (mc6845->max_ras_addr + 1)) * mc6845->horiz_disp +
			   (x / mc6845->intf->hpixels_per_column)) & 0x3fff;
	}
	else
		ret = 0;

	return ret;
}


UINT8 mc6845_get_ra(mc6845_t *mc6845)
{
	UINT8 ret;

	if (mc6845->has_valid_parameters)
	{
		/* get the current vertical raster position and clamp it to the visible region */
		int y = video_screen_get_vpos(0);

		if (y > mc6845->last_max_y)
			y = mc6845->last_max_y;

		ret = y % (mc6845->max_ras_addr + 1);
	}
	else
		ret = 0;

	return ret;
}



static void update_cursor_state(mc6845_t *mc6845)
{
	/* save and increment cursor counter */
	UINT8 last_cursor_blink_count = mc6845->cursor_blink_count;
	mc6845->cursor_blink_count = mc6845->cursor_blink_count + 1;

	/* switch on cursor blinking mode */
	switch (mc6845->cursor_start_ras & 0x60)
	{
		/* always on */
		case 0x00: mc6845->cursor_state = TRUE; break;

		/* always off */
		default:
		case 0x20: mc6845->cursor_state = FALSE; break;

		/* fast blink */
		case 0x40:
			if ((last_cursor_blink_count & 0x10) != (mc6845->cursor_blink_count & 0x10))
				mc6845->cursor_state = !mc6845->cursor_state;
			break;

		/* slow blink */
		case 0x60:
			if ((last_cursor_blink_count & 0x20) != (mc6845->cursor_blink_count & 0x20))
				mc6845->cursor_state = !mc6845->cursor_state;
			break;
	}
}


void mc6845_update(mc6845_t *mc6845, mame_bitmap *bitmap, const rectangle *cliprect)
{
	if (mc6845->has_valid_parameters)
	{
		UINT16 y;

		/* call the set up function if any */
		void *param = NULL;

		if (mc6845->intf->begin_update)
			param = mc6845->intf->begin_update(mc6845->machine, mc6845, bitmap, cliprect);

		if (cliprect->min_y == 0)
		{
			/* read the start address at the beginning of the frame */
			mc6845->current_ma = mc6845->start_addr;

			/* also update the cursor state now */
			update_cursor_state(mc6845);
		}

		/* for each row in the visible region */
		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			/* compute the current raster line */
			UINT8 ra = y % (mc6845->max_ras_addr + 1);

			/* check if the cursor is visible and is on this scanline */
			int cursor_visible = mc6845->cursor_state &&
							 	(ra >= (mc6845->cursor_start_ras & 0x1f)) &&
							 	(ra <= mc6845->cursor_end_ras) &&
							 	(mc6845->cursor >= mc6845->current_ma) &&
							 	(mc6845->cursor < (mc6845->current_ma + mc6845->horiz_disp));

			/* compute the cursor X position, or -1 if not visible */
			INT8 cursor_x = cursor_visible ? (mc6845->cursor - mc6845->current_ma) : -1;

			/* call the external system to draw it */
			mc6845->intf->update_row(mc6845->machine, mc6845, bitmap, cliprect, mc6845->current_ma, ra, y, mc6845->horiz_disp, cursor_x, param);

			/* update MA if the last raster address */
			if (ra == mc6845->max_ras_addr)
				mc6845->current_ma = (mc6845->current_ma + mc6845->horiz_disp) & 0x3fff;
		}

		/* call the tear down function if any */
		if (mc6845->intf->end_update)
			mc6845->intf->end_update(mc6845->machine, mc6845, bitmap, cliprect, param);
	}
}


/* device interface */
static void *mc6845_start(running_machine *machine, const char *tag, const void *static_config, const void *inline_config)
{
	mc6845_t *mc6845;
	char unique_tag[30];

	/* validate arguments */
	assert(machine != NULL);
	assert(tag != NULL);
	assert(strlen(tag) < 20);

	/* allocate the object that holds the state */
	mc6845 = auto_malloc(sizeof(*mc6845));
	memset(mc6845, 0, sizeof(*mc6845));

	mc6845->machine = machine;
	mc6845->intf = static_config;

	/* create the timer if the user is interested in getting display enable
       notifications */
	if (mc6845->intf && mc6845->intf->display_enable_changed)
		mc6845->display_enable_changed_timer = timer_alloc(display_enable_changed_timer_cb, mc6845);

	/* register for state saving */
	state_save_combine_module_and_tag(unique_tag, "mc6845", tag);

	state_save_register_func_postload_ptr(mc6845_state_save_postload, mc6845);

	state_save_register_item(unique_tag, 0, mc6845->address_latch);
	state_save_register_item(unique_tag, 0, mc6845->horiz_total);
	state_save_register_item(unique_tag, 0, mc6845->horiz_disp);
	state_save_register_item(unique_tag, 0, mc6845->horiz_sync_pos);
	state_save_register_item(unique_tag, 0, mc6845->sync_width);
	state_save_register_item(unique_tag, 0, mc6845->vert_total);
	state_save_register_item(unique_tag, 0, mc6845->vert_total_adj);
	state_save_register_item(unique_tag, 0, mc6845->vert_disp);
	state_save_register_item(unique_tag, 0, mc6845->vert_sync_pos);
	state_save_register_item(unique_tag, 0, mc6845->intl_skew);
	state_save_register_item(unique_tag, 0, mc6845->max_ras_addr);
	state_save_register_item(unique_tag, 0, mc6845->cursor_start_ras);
	state_save_register_item(unique_tag, 0, mc6845->cursor_end_ras);
	state_save_register_item(unique_tag, 0, mc6845->start_addr);
	state_save_register_item(unique_tag, 0, mc6845->cursor);
	state_save_register_item(unique_tag, 0, mc6845->light_pen);
	state_save_register_item(unique_tag, 0, mc6845->cursor_state);
	state_save_register_item(unique_tag, 0, mc6845->cursor_blink_count);

	return mc6845;
}


static void mc6845_set_info(running_machine *machine, void *token, UINT32 state, const deviceinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void mc6845_get_info(running_machine *machine, void *token, UINT32 state, deviceinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_SET_INFO:						info->set_info = mc6845_set_info;		break;
		case DEVINFO_FCT_START:							info->start = mc6845_start;				break;
		case DEVINFO_FCT_STOP:							/* Nothing */							break;
		case DEVINFO_FCT_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							info->s = "MC6845";						break;
		case DEVINFO_STR_FAMILY:						info->s = "MC6845 CRTC";				break;
		case DEVINFO_STR_VERSION:						info->s = "1.5";						break;
		case DEVINFO_STR_SOURCE_FILE:					info->s = __FILE__;						break;
		case DEVINFO_STR_CREDITS:						info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}


void r6545_get_info(running_machine *machine, void *token, UINT32 state, deviceinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							info->s = "R6545";						break;
		default: mc6845_get_info(machine, token, state, info);									break;
	}
}
