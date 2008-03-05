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


/* device types */
enum
{
	TYPE_MC6845,
	TYPE_C6545_1,
	TYPE_R6545_1,

	NUM_TYPES
};

/* tags for state saving */
const char * const device_tags[NUM_TYPES] = { "mc6845", "c6545-1", "r6545-1" };

/* capabilities */
static const int supports_disp_start_addr_r[NUM_TYPES] = {  TRUE, FALSE, FALSE };
static const int supports_vert_sync_width[NUM_TYPES]   = { FALSE,  TRUE,  TRUE };
static const int supports_status_reg_d5[NUM_TYPES]     = { FALSE,  TRUE,  TRUE };
static const int supports_status_reg_d6[NUM_TYPES]     = { FALSE,  TRUE,  TRUE };


struct _mc6845_t
{
	int device_type;
	running_machine *machine;
	const mc6845_interface *intf;

	/* register file */
	UINT8	horiz_char_total;	/* 0x00 */
	UINT8	horiz_disp;			/* 0x01 */
	UINT8	horiz_sync_pos;		/* 0x02 */
	UINT8	sync_width;			/* 0x03 */
	UINT8	vert_char_total;	/* 0x04 */
	UINT8	vert_total_adj;		/* 0x05 */
	UINT8	vert_disp;			/* 0x06 */
	UINT8	vert_sync_pos;		/* 0x07 */
	UINT8	mode_control;		/* 0x08 */
	UINT8	max_ras_addr;		/* 0x09 */
	UINT8	cursor_start_ras;	/* 0x0a */
	UINT8	cursor_end_ras;		/* 0x0b */
	UINT16	disp_start_addr;	/* 0x0c/0x0d */
	UINT16	cursor_addr;		/* 0x0e/0x0f */
	UINT16	light_pen_addr;		/* 0x10/0x11 */

	/* other internal state */
	UINT8	register_address_latch;
	UINT8	cursor_state;	/* 0 = off, 1 = on */
	UINT8	cursor_blink_count;

	/* timers */
	emu_timer *de_changed_timer;
	emu_timer *hsync_on_timer;
	emu_timer *hsync_off_timer;
	emu_timer *vsync_on_timer;
	emu_timer *vsync_off_timer;
	emu_timer *light_pen_latch_timer;

	/* computed values - do NOT state save these! */
	UINT16	horiz_pix_total;
	UINT16	vert_pix_total;
	UINT16	max_visible_x;
	UINT16	max_visible_y;
	UINT16	hsync_on_pos;
	UINT16	hsync_off_pos;
	UINT16	vsync_on_pos;
	UINT16	vsync_off_pos;
	UINT16  current_disp_addr;	/* the display address currently drawn */
	UINT8	light_pen_latched;
	int		has_valid_parameters;
};


static void mc6845_state_save_postload(void *param);
static void recompute_parameters(mc6845_t *mc6845, int postload);
static void update_de_changed_timer(mc6845_t *mc6845);
static void update_hsync_changed_timers(mc6845_t *mc6845);
static void update_vsync_changed_timers(mc6845_t *mc6845);


static void mc6845_state_save_postload(void *param)
{
	recompute_parameters(param, TRUE);
}


WRITE8_DEVICE_HANDLER( mc6845_address_w )
{
	mc6845_t *mc6845 = device->token;

	assert(mc6845 != NULL);

	mc6845->register_address_latch = data & 0x1f;
}


READ8_DEVICE_HANDLER( mc6845_status_r )
{
	mc6845_t *mc6845 = device->token;
	UINT8 ret = 0;

	assert(mc6845 != NULL);

	/* VBLANK bit */
	if (supports_status_reg_d5[mc6845->device_type] &&
	   (video_screen_get_vpos(mc6845->intf->scrnum) > mc6845->max_visible_y))
	   ret = ret | 0x20;

	/* light pen latched */
	if (supports_status_reg_d6[mc6845->device_type] && mc6845->light_pen_latched)
	   ret = ret | 0x40;

	return ret;
}


READ8_DEVICE_HANDLER( mc6845_register_r )
{
	mc6845_t *mc6845 = device->token;
	UINT8 ret = 0;

	assert(mc6845 != NULL);

	switch (mc6845->register_address_latch)
	{
		case 0x0c:  ret = supports_disp_start_addr_r[mc6845->device_type] ? (mc6845->disp_start_addr >> 8) & 0xff : 0; break;
		case 0x0d:  ret = supports_disp_start_addr_r[mc6845->device_type] ? (mc6845->disp_start_addr >> 0) & 0xff : 0; break;
		case 0x0e:  ret = (mc6845->cursor_addr    >> 8) & 0xff; break;
		case 0x0f:  ret = (mc6845->cursor_addr    >> 0) & 0xff; break;
		case 0x10:  ret = (mc6845->light_pen_addr >> 8) & 0xff; mc6845->light_pen_latched = FALSE; break;
		case 0x11:  ret = (mc6845->light_pen_addr >> 0) & 0xff; mc6845->light_pen_latched = FALSE; break;

		/* all other registers are write only and return 0 */
		default: break;
	}

	return ret;
}


WRITE8_DEVICE_HANDLER( mc6845_register_w )
{
	mc6845_t *mc6845 = device->token;

	assert(mc6845 != NULL);

	if (LOG)  logerror("M6845 PC %04x: reg 0x%02x = 0x%02x\n", activecpu_get_pc(), mc6845->register_address_latch, data);

	switch (mc6845->register_address_latch)
	{
		case 0x00:  mc6845->horiz_char_total =   data & 0xff; break;
		case 0x01:  mc6845->horiz_disp       =   data & 0xff; break;
		case 0x02:  mc6845->horiz_sync_pos   =   data & 0xff; break;
		case 0x03:  mc6845->sync_width       =   data & 0xff; break;
		case 0x04:  mc6845->vert_char_total  =   data & 0x7f; break;
		case 0x05:  mc6845->vert_total_adj   =   data & 0x1f; break;
		case 0x06:  mc6845->vert_disp        =   data & 0x7f; break;
		case 0x07:  mc6845->vert_sync_pos    =   data & 0x7f; break;
		case 0x08:  mc6845->mode_control     =   data & 0xff; break;
		case 0x09:  mc6845->max_ras_addr     =   data & 0x1f; break;
		case 0x0a:  mc6845->cursor_start_ras =   data & 0x7f; break;
		case 0x0b:  mc6845->cursor_end_ras   =   data & 0x1f; break;
		case 0x0c:  mc6845->disp_start_addr  = ((data & 0x3f) << 8) | (mc6845->disp_start_addr & 0x00ff); break;
		case 0x0d:  mc6845->disp_start_addr  = ((data & 0xff) << 0) | (mc6845->disp_start_addr & 0xff00); break;
		case 0x0e:  mc6845->cursor_addr      = ((data & 0x3f) << 8) | (mc6845->cursor_addr & 0x00ff); break;
		case 0x0f:  mc6845->cursor_addr      = ((data & 0xff) << 0) | (mc6845->cursor_addr & 0xff00); break;
		case 0x10: /* read-only */ break;
		case 0x11: /* read-only */ break;
		default: break;
	}

	/* display message if the Mode Control register is not zero */
	if ((mc6845->register_address_latch == 0x08) && (mc6845->mode_control != 0))
		popmessage("Mode Control %02X is not supported!!!", mc6845->mode_control);

	recompute_parameters(mc6845, FALSE);
}


static void recompute_parameters(mc6845_t *mc6845, int postload)
{
	if (mc6845->intf)
	{
		UINT16 hsync_on_pos, hsync_off_pos, vsync_on_pos, vsync_off_pos;

		/* compute the screen sizes */
		UINT16 horiz_pix_total = (mc6845->horiz_char_total + 1) * mc6845->intf->hpixels_per_column;
		UINT16 vert_pix_total = (mc6845->vert_char_total + 1) * (mc6845->max_ras_addr + 1) + mc6845->vert_total_adj;

		/* determine the visible area, avoid division by 0 */
		UINT16 max_visible_x = mc6845->horiz_disp * mc6845->intf->hpixels_per_column - 1;
		UINT16 max_visible_y = mc6845->vert_disp * (mc6845->max_ras_addr + 1) - 1;

		/* determine the syncing positions */
		UINT8 horiz_sync_char_width = mc6845->sync_width & 0x0f;
		UINT8 vert_sync_pix_width = supports_vert_sync_width[mc6845->device_type] ? (mc6845->sync_width >> 4) & 0x0f : 0x10;

		if (horiz_sync_char_width == 0)
			horiz_sync_char_width = 0x10;

		if (vert_sync_pix_width == 0)
			vert_sync_pix_width = 0x10;

		hsync_on_pos = mc6845->horiz_sync_pos * mc6845->intf->hpixels_per_column;
		hsync_off_pos = hsync_on_pos + (horiz_sync_char_width * mc6845->intf->hpixels_per_column);
		vsync_on_pos = mc6845->vert_sync_pos * (mc6845->max_ras_addr + 1);
		vsync_off_pos = vsync_on_pos + vert_sync_pix_width;

		/* the Commodore PET computers program a horizontal synch pulse that extends
           past the scanline width.  I assume that the real device will clamp it */
		if (hsync_off_pos > horiz_pix_total)
			hsync_off_pos = horiz_pix_total;

		if (vsync_off_pos > vert_pix_total)
			vsync_off_pos = vert_pix_total;

		/* update only if screen parameters changed, unless we are coming here after loading the saved state */
		if (postload ||
		    (horiz_pix_total != mc6845->horiz_pix_total) || (vert_pix_total != mc6845->vert_pix_total) ||
			(max_visible_x != mc6845->max_visible_x) || (max_visible_y != mc6845->max_visible_y) ||
			(hsync_on_pos != mc6845->hsync_on_pos) || (vsync_on_pos != mc6845->vsync_on_pos) ||
			(hsync_off_pos != mc6845->hsync_off_pos) || (vsync_off_pos != mc6845->vsync_off_pos))
		{
			/* update the screen if we have valid data */
			if ((horiz_pix_total > 0) && (max_visible_x < horiz_pix_total) &&
				(vert_pix_total > 0) && (max_visible_y < vert_pix_total) &&
				(hsync_on_pos <= horiz_pix_total) && (vsync_on_pos <= vert_pix_total) &&
				(hsync_on_pos != hsync_off_pos))
			{
				rectangle visarea;

				attoseconds_t refresh = HZ_TO_ATTOSECONDS(mc6845->intf->clock) * (mc6845->horiz_char_total + 1) * vert_pix_total;

				visarea.min_x = 0;
				visarea.min_y = 0;
				visarea.max_x = max_visible_x;
				visarea.max_y = max_visible_y;

				if (LOG) logerror("M6845 config screen: HTOTAL: 0x%x  VTOTAL: 0x%x  MAX_X: 0x%x  MAX_Y: 0x%x  HSYNC: 0x%x-0x%x  VSYNC: 0x%x-0x%x  Freq: %ffps\n",
								  horiz_pix_total, vert_pix_total, max_visible_x, max_visible_y, hsync_on_pos, hsync_off_pos - 1, vsync_on_pos, vsync_off_pos - 1, 1 / ATTOSECONDS_TO_DOUBLE(refresh));

				video_screen_configure(mc6845->intf->scrnum, horiz_pix_total, vert_pix_total, &visarea, refresh);

				mc6845->has_valid_parameters = TRUE;
			}
			else
				mc6845->has_valid_parameters = FALSE;

			mc6845->horiz_pix_total = horiz_pix_total;
			mc6845->vert_pix_total = vert_pix_total;
			mc6845->max_visible_x = max_visible_x;
			mc6845->max_visible_y = max_visible_y;
			mc6845->hsync_on_pos = hsync_on_pos;
			mc6845->hsync_off_pos = hsync_off_pos;
			mc6845->vsync_on_pos = vsync_on_pos;
			mc6845->vsync_off_pos = vsync_off_pos;

			update_de_changed_timer(mc6845);
			update_hsync_changed_timers(mc6845);
			update_vsync_changed_timers(mc6845);
		}
	}
}


static int is_display_enabled(mc6845_t *mc6845)
{
	UINT16 y = video_screen_get_vpos(mc6845->intf->scrnum);
	UINT16 x = video_screen_get_hpos(mc6845->intf->scrnum);

	return (y <= mc6845->max_visible_y) && (x <= mc6845->max_visible_x);
}


static void update_de_changed_timer(mc6845_t *mc6845)
{
	if (mc6845->has_valid_parameters && (mc6845->de_changed_timer != NULL))
	{
		INT16 next_y;
		UINT16 next_x;
		attotime duration;

		/* we are in a display region, get the location of the next blanking start */
		if (is_display_enabled(mc6845))
		{
			/* normally, it's at end the current raster line */
			next_y = video_screen_get_vpos(mc6845->intf->scrnum);
			next_x = mc6845->max_visible_x + 1;

			/* but if visible width = horiz_pix_total, then we need
               to go to the beginning of VBLANK */
			if (next_x == mc6845->horiz_pix_total)
			{
				next_y = mc6845->max_visible_y + 1;
				next_x = 0;

				/* abnormal case, no vertical blanking, either */
				if (next_y == mc6845->vert_pix_total)
					next_y = -1;
			}
		}

		/* we are in a blanking region, get the location of the next display start */
		else
		{
			next_x = 0;
			next_y = (video_screen_get_vpos(mc6845->intf->scrnum) + 1) % mc6845->vert_pix_total;

			/* if we would now fall in the vertical blanking, we need
               to go to the top of the screen */
			if (next_y > mc6845->max_visible_y)
				next_y = 0;
		}

		if (next_y != -1)
			duration = video_screen_get_time_until_pos(mc6845->intf->scrnum, next_y, next_x);
		else
			duration = attotime_never;

		timer_adjust_oneshot(mc6845->de_changed_timer, duration, 0);
	}
}


static void update_hsync_changed_timers(mc6845_t *mc6845)
{
	if (mc6845->has_valid_parameters && (mc6845->hsync_on_timer != NULL))
	{
		UINT16 next_y;

		/* we are before the HSYNC position, we trigger on the current line */
		if (video_screen_get_hpos(mc6845->intf->scrnum) < mc6845->hsync_on_pos)
			next_y = video_screen_get_vpos(mc6845->intf->scrnum);

		/* trigger on the next line */
		else
			next_y = (video_screen_get_vpos(mc6845->intf->scrnum) + 1) % mc6845->vert_pix_total;

		/* if the next line is not in the visible region, go to the beginning of the screen */
		if (next_y > mc6845->max_visible_y)
			next_y = 0;

		timer_adjust_oneshot(mc6845->hsync_on_timer,  video_screen_get_time_until_pos(mc6845->intf->scrnum, next_y, mc6845->hsync_on_pos) , 0);
		timer_adjust_oneshot(mc6845->hsync_off_timer, video_screen_get_time_until_pos(mc6845->intf->scrnum, next_y, mc6845->hsync_off_pos), 0);
	}
}


static void update_vsync_changed_timers(mc6845_t *mc6845)
{
	if (mc6845->has_valid_parameters && (mc6845->vsync_on_timer != NULL))
	{
		timer_adjust_oneshot(mc6845->vsync_on_timer,  video_screen_get_time_until_pos(mc6845->intf->scrnum, mc6845->vsync_on_pos,  0), 0);
		timer_adjust_oneshot(mc6845->vsync_off_timer, video_screen_get_time_until_pos(mc6845->intf->scrnum, mc6845->vsync_off_pos, 0), 0);
	}
}


static TIMER_CALLBACK( de_changed_timer_cb )
{
	mc6845_t *mc6845 = ptr;

	/* call the callback function -- we know it exists */
	mc6845->intf->on_de_changed(mc6845->machine, mc6845, is_display_enabled(mc6845));

	update_de_changed_timer(mc6845);
}


static TIMER_CALLBACK( vsync_on_timer_cb )
{
	mc6845_t *mc6845 = ptr;

	/* call the callback function -- we know it exists */
	mc6845->intf->on_vsync_changed(mc6845->machine, mc6845, TRUE);
}


static TIMER_CALLBACK( vsync_off_timer_cb )
{
	mc6845_t *mc6845 = ptr;

	/* call the callback function -- we know it exists */
	mc6845->intf->on_vsync_changed(mc6845->machine, mc6845, FALSE);

	update_vsync_changed_timers(mc6845);
}


static TIMER_CALLBACK( hsync_on_timer_cb )
{
	mc6845_t *mc6845 = ptr;

	/* call the callback function -- we know it exists */
	mc6845->intf->on_hsync_changed(mc6845->machine, mc6845, TRUE);
}


static TIMER_CALLBACK( hsync_off_timer_cb )
{
	mc6845_t *mc6845 = ptr;

	/* call the callback function -- we know it exists */
	mc6845->intf->on_hsync_changed(mc6845->machine, mc6845, FALSE);

	update_hsync_changed_timers(mc6845);
}



UINT16 mc6845_get_ma(mc6845_t *mc6845)
{
	UINT16 ret;

	assert(mc6845 != NULL);

	if (mc6845->has_valid_parameters)
	{
		/* clamp Y/X to the visible region */
		int y = video_screen_get_vpos(mc6845->intf->scrnum);
		int x = video_screen_get_hpos(mc6845->intf->scrnum);

		/* since the MA counter stops in the blanking regions, if we are in a
           VBLANK, both X and Y are at their max */
		if ((y > mc6845->max_visible_y) || (x > mc6845->max_visible_x))
			x = mc6845->max_visible_x;

		if (y > mc6845->max_visible_y)
			y = mc6845->max_visible_y;

		ret = (mc6845->disp_start_addr +
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

	assert(mc6845 != NULL);

	if (mc6845->has_valid_parameters)
	{
		/* get the current vertical raster position and clamp it to the visible region */
		int y = video_screen_get_vpos(mc6845->intf->scrnum);

		if (y > mc6845->max_visible_y)
			y = mc6845->max_visible_y;

		ret = y % (mc6845->max_ras_addr + 1);
	}
	else
		ret = 0;

	return ret;
}


static TIMER_CALLBACK( light_pen_latch_timer_cb )
{
	mc6845_t *mc6845 = ptr;

	mc6845->light_pen_addr = mc6845_get_ma(mc6845);
	mc6845->light_pen_latched = TRUE;
}


void mc6845_assert_light_pen_input(mc6845_t *mc6845)
{
	int y, x;
	int char_x;

	assert(mc6845 != NULL);

	if (mc6845->has_valid_parameters)
	{
		/* get the current pixel coordinates */
		y = video_screen_get_vpos(mc6845->intf->scrnum);
		x = video_screen_get_hpos(mc6845->intf->scrnum);

		/* compute the pixel coordinate of the NEXT character -- this is when the light pen latches */
		char_x = x / mc6845->intf->hpixels_per_column;
		x = (char_x + 1) * mc6845->intf->hpixels_per_column;

		/* adjust if we are passed the boundaries of the screen */
		if (x == mc6845->horiz_pix_total)
		{
			y = y + 1;
			x = 0;

			if (y == mc6845->vert_pix_total)
				y = 0;
		}

		/* set the timer that will latch the display address into the light pen registers */
		timer_adjust_oneshot(mc6845->light_pen_latch_timer, video_screen_get_time_until_pos(mc6845->intf->scrnum, y, x), 0);
	}
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


void mc6845_update(mc6845_t *mc6845, bitmap_t *bitmap, const rectangle *cliprect)
{
	assert(mc6845 != NULL);
	assert(bitmap != NULL);
	assert(cliprect != NULL);

	if (mc6845->has_valid_parameters)
	{
		UINT16 y;

		void *param = NULL;

		assert(mc6845->intf != NULL);
		assert(mc6845->intf->update_row != NULL);

		/* call the set up function if any */
		if (mc6845->intf->begin_update != NULL)
			param = mc6845->intf->begin_update(mc6845->machine, mc6845, bitmap, cliprect);

		if (cliprect->min_y == 0)
		{
			/* read the start address at the beginning of the frame */
			mc6845->current_disp_addr = mc6845->disp_start_addr;

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
							 	(mc6845->cursor_addr >= mc6845->current_disp_addr) &&
							 	(mc6845->cursor_addr < (mc6845->current_disp_addr + mc6845->horiz_disp));

			/* compute the cursor X position, or -1 if not visible */
			INT8 cursor_x = cursor_visible ? (mc6845->cursor_addr - mc6845->current_disp_addr) : -1;

			/* call the external system to draw it */
			mc6845->intf->update_row(mc6845->machine, mc6845, bitmap, cliprect, mc6845->current_disp_addr, ra, y, mc6845->horiz_disp, cursor_x, param);

			/* update MA if the last raster address */
			if (ra == mc6845->max_ras_addr)
				mc6845->current_disp_addr = (mc6845->current_disp_addr + mc6845->horiz_disp) & 0x3fff;
		}

		/* call the tear down function if any */
		if (mc6845->intf->end_update != NULL)
			mc6845->intf->end_update(mc6845->machine, mc6845, bitmap, cliprect, param);

		popmessage(NULL);
	}
	else
		popmessage("Invalid MC6845 screen parameters - display disabled!!!");
}


/* device interface */
static void *common_start(running_machine *machine, const char *tag, const void *static_config, const void *inline_config, int device_type)
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

	mc6845->device_type = device_type;
	mc6845->machine = machine;
	mc6845->intf = static_config;

	/* create the timers */
	if (mc6845->intf != NULL)
	{
		if (mc6845->intf->on_de_changed != NULL)
			mc6845->de_changed_timer = timer_alloc(de_changed_timer_cb, mc6845);

		if (mc6845->intf->on_hsync_changed != NULL)
		{
			mc6845->hsync_on_timer = timer_alloc(hsync_on_timer_cb, mc6845);
			mc6845->hsync_off_timer = timer_alloc(hsync_off_timer_cb, mc6845);
		}

		if (mc6845->intf->on_vsync_changed != NULL)
		{
			mc6845->vsync_on_timer = timer_alloc(vsync_on_timer_cb, mc6845);
			mc6845->vsync_off_timer = timer_alloc(vsync_off_timer_cb, mc6845);
		}
	}

	mc6845->light_pen_latch_timer = timer_alloc(light_pen_latch_timer_cb, mc6845);

	/* register for state saving */
	state_save_combine_module_and_tag(unique_tag, device_tags[device_type], tag);

	state_save_register_func_postload_ptr(mc6845_state_save_postload, mc6845);

	state_save_register_item(unique_tag, 0, mc6845->register_address_latch);
	state_save_register_item(unique_tag, 0, mc6845->horiz_char_total);
	state_save_register_item(unique_tag, 0, mc6845->horiz_disp);
	state_save_register_item(unique_tag, 0, mc6845->horiz_sync_pos);
	state_save_register_item(unique_tag, 0, mc6845->sync_width);
	state_save_register_item(unique_tag, 0, mc6845->vert_char_total);
	state_save_register_item(unique_tag, 0, mc6845->vert_total_adj);
	state_save_register_item(unique_tag, 0, mc6845->vert_disp);
	state_save_register_item(unique_tag, 0, mc6845->vert_sync_pos);
	state_save_register_item(unique_tag, 0, mc6845->mode_control);
	state_save_register_item(unique_tag, 0, mc6845->max_ras_addr);
	state_save_register_item(unique_tag, 0, mc6845->cursor_start_ras);
	state_save_register_item(unique_tag, 0, mc6845->cursor_end_ras);
	state_save_register_item(unique_tag, 0, mc6845->disp_start_addr);
	state_save_register_item(unique_tag, 0, mc6845->cursor_addr);
	state_save_register_item(unique_tag, 0, mc6845->light_pen_addr);
	state_save_register_item(unique_tag, 0, mc6845->light_pen_latched);
	state_save_register_item(unique_tag, 0, mc6845->cursor_state);
	state_save_register_item(unique_tag, 0, mc6845->cursor_blink_count);

	return mc6845;
}

static DEVICE_START( mc6845 )
{
	return common_start(machine, tag, static_config, inline_config, TYPE_MC6845);
}

static DEVICE_START( c6545_1 )
{
	return common_start(machine, tag, static_config, inline_config, TYPE_C6545_1);
}

static DEVICE_START( r6545_1 )
{
	return common_start(machine, tag, static_config, inline_config, TYPE_R6545_1);
}


static DEVICE_RESET( mc6845 )
{
	mc6845_t *mc6845 = token;

	/* internal registers other than status remain unchanged, all outputs go low */
	if (mc6845->intf != NULL)
	{
		if (mc6845->intf->on_de_changed != NULL)
			mc6845->intf->on_de_changed(machine, mc6845, FALSE);

		if (mc6845->intf->on_hsync_changed != NULL)
			mc6845->intf->on_hsync_changed(machine, mc6845, FALSE);

		if (mc6845->intf->on_vsync_changed != NULL)
			mc6845->intf->on_vsync_changed(machine, mc6845, FALSE);
	}

	mc6845->light_pen_latched = FALSE;
}


static DEVICE_SET_INFO( mc6845 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


DEVICE_GET_INFO( mc6845 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;								break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_SET_INFO:						info->set_info = DEVICE_SET_INFO_NAME(mc6845); break;
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(mc6845);	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(mc6845);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							info->s = "Motorola 6845";					break;
		case DEVINFO_STR_FAMILY:						info->s = "MC6845 CRTC";					break;
		case DEVINFO_STR_VERSION:						info->s = "1.6";							break;
		case DEVINFO_STR_SOURCE_FILE:					info->s = __FILE__;							break;
		case DEVINFO_STR_CREDITS:						info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}


DEVICE_GET_INFO( c6545_1 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							info->s = "Commodore 6545-1";				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(c6545_1);	break;

		default: 										DEVICE_GET_INFO_CALL(mc6845);				break;
	}
}


DEVICE_GET_INFO( r6545_1 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							info->s = "Rockwell 6545-1";				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(r6545_1);	break;

		default: 										DEVICE_GET_INFO_CALL(mc6845);				break;
	}
}
