/**********************************************************************

    Motorola 6845 CRT controller emulation

**********************************************************************/

#include "driver.h"
#include "crtc6845.h"


#define LOG		(0)


struct _crtc6845_t
{
	const crtc6845_interface *intf;

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


static void crtc6845_state_save_postload(void *param);
static void configure_screen(crtc6845_t *crtc6845, int postload);
static void update_timer(crtc6845_t *crtc6845);
static TIMER_CALLBACK( display_enable_changed_timer_cb );


crtc6845_t *crtc6845_config(const crtc6845_interface *intf)
{
	crtc6845_t *crtc6845;

	/* allocate the object that holds the state */
	crtc6845 = auto_malloc(sizeof(*crtc6845));
	memset(crtc6845, 0, sizeof(*crtc6845));

	crtc6845->intf = intf;

	/* create the timer if the user is interested in getting display enable
       notifications */
	if (intf && intf->display_enable_changed)
		crtc6845->display_enable_changed_timer = timer_alloc(display_enable_changed_timer_cb, crtc6845);

	/* register for state saving */
	state_save_register_func_postload_ptr(crtc6845_state_save_postload, crtc6845);

	state_save_register_item("crtc6845", 0, crtc6845->address_latch);
	state_save_register_item("crtc6845", 0, crtc6845->horiz_total);
	state_save_register_item("crtc6845", 0, crtc6845->horiz_disp);
	state_save_register_item("crtc6845", 0, crtc6845->horiz_sync_pos);
	state_save_register_item("crtc6845", 0, crtc6845->sync_width);
	state_save_register_item("crtc6845", 0, crtc6845->vert_total);
	state_save_register_item("crtc6845", 0, crtc6845->vert_total_adj);
	state_save_register_item("crtc6845", 0, crtc6845->vert_disp);
	state_save_register_item("crtc6845", 0, crtc6845->vert_sync_pos);
	state_save_register_item("crtc6845", 0, crtc6845->intl_skew);
	state_save_register_item("crtc6845", 0, crtc6845->max_ras_addr);
	state_save_register_item("crtc6845", 0, crtc6845->cursor_start_ras);
	state_save_register_item("crtc6845", 0, crtc6845->cursor_end_ras);
	state_save_register_item("crtc6845", 0, crtc6845->start_addr);
	state_save_register_item("crtc6845", 0, crtc6845->cursor);
	state_save_register_item("crtc6845", 0, crtc6845->light_pen);

	return crtc6845;
}


static void crtc6845_state_save_postload(void *param)
{
	crtc6845_t *crtc6845 = (crtc6845_t *)param;

	configure_screen(crtc6845, TRUE);
}


void crtc6845_address_w(crtc6845_t *crtc6845, UINT8 data)
{
	crtc6845->address_latch = data & 0x1f;
}


UINT8 crtc6845_register_r(crtc6845_t *crtc6845)
{
	UINT8 ret = 0xff;

	switch (crtc6845->address_latch)
	{
		case 14:
			ret = crtc6845->cursor >> 8;
			break;
		case 15:
			ret = crtc6845->cursor;
			break;
		case 16:
			ret = crtc6845->light_pen >> 8;
			break;
		case 17:
			ret = crtc6845->light_pen;
			break;
		default:
			/* all other registers are write only */
			break;
	}

	return ret;
}


void crtc6845_register_w(crtc6845_t *crtc6845, UINT8 data)
{
	int call_configure_screen = FALSE;
	if (LOG)  logerror("CRT #0 PC %04x: CRTC6845 reg 0x%02x = 0x%02x\n", activecpu_get_pc(), crtc6845->address_latch, data);

	switch (crtc6845->address_latch)
	{
		case 0:
			crtc6845->horiz_total = data;
			call_configure_screen = TRUE;
			break;
		case 1:
			crtc6845->horiz_disp = data;
			call_configure_screen = TRUE;
			break;
		case 2:
			crtc6845->horiz_sync_pos = data;
			break;
		case 3:
			crtc6845->sync_width = data & 0x0f;
			break;
		case 4:
			crtc6845->vert_total = data & 0x7f;
			call_configure_screen = TRUE;
			break;
		case 5:
			crtc6845->vert_total_adj = data & 0x1f;
			call_configure_screen = TRUE;
			break;
		case 6:
			crtc6845->vert_disp = data & 0x7f;
			call_configure_screen = TRUE;
			break;
		case 7:
			crtc6845->vert_sync_pos = data & 0x7f;
			break;
		case 8:
			crtc6845->intl_skew = data & 0x03;
			break;
		case 9:
			crtc6845->max_ras_addr = data & 0x1f;
			call_configure_screen = TRUE;
			break;
		case 10:
			crtc6845->cursor_start_ras = data & 0x7f;
			break;
		case 11:
			crtc6845->cursor_end_ras = data & 0x1f;
			break;
		case 12:
			crtc6845->start_addr &= 0x00ff;
			crtc6845->start_addr |= (data & 0x3f) << 8;
			call_configure_screen = TRUE;
			break;
		case 13:
			crtc6845->start_addr &= 0xff00;
			crtc6845->start_addr |= data;
			call_configure_screen = TRUE;
			break;
		case 14:
			crtc6845->cursor &= 0x00ff;
			crtc6845->cursor |= (data & 0x3f) << 8;
			break;
		case 15:
			crtc6845->cursor &= 0xff00;
			crtc6845->cursor |= data;
			break;
		case 16:	/* read-only */
			break;
		case 17:	/* read-only */
			break;
		default:
			break;
	}

	if (call_configure_screen)
		configure_screen(crtc6845, FALSE);
}


static void configure_screen(crtc6845_t *crtc6845, int postload)
{
	if (crtc6845->intf)
	{
		/* compute the screen sizes */
		UINT16 horiz_total = (crtc6845->horiz_total + 1) * crtc6845->intf->hpixels_per_column;
		UINT16 vert_total = (crtc6845->vert_total + 1) * (crtc6845->max_ras_addr + 1) + crtc6845->vert_total_adj;

		/* determine the visible area, avoid division by 0 */
		UINT16 max_x = crtc6845->horiz_disp * crtc6845->intf->hpixels_per_column - 1;
		UINT16 max_y = crtc6845->vert_disp * (crtc6845->max_ras_addr + 1) - 1;

		/* update only if screen parameters changed, unless we are coming here after loading the saved state */
		if (postload ||
		    (horiz_total != crtc6845->last_horiz_total) || (vert_total != crtc6845->last_vert_total) ||
			(max_x != crtc6845->last_max_x) || (max_y != crtc6845->last_max_y))
		{
			/* update the screen only if we have valid data */
			if ((crtc6845->horiz_total > 0) && (max_x < horiz_total) && (crtc6845->vert_total > 0) && (max_y < vert_total))
			{
				rectangle visarea;

				attoseconds_t refresh = HZ_TO_ATTOSECONDS(crtc6845->intf->clock) * (crtc6845->horiz_total + 1) * vert_total;

				visarea.min_x = 0;
				visarea.min_y = 0;
				visarea.max_x = max_x;
				visarea.max_y = max_y;

				if (LOG) logerror("CRTC6845 config screen: HTOTAL: %x  VTOTAL: %x  MAX_X: %x  MAX_Y: %x  FPS: %f\n",
								  horiz_total, vert_total, max_x, max_y, 1 / ATTOSECONDS_TO_DOUBLE(refresh));

				video_screen_configure(crtc6845->intf->scrnum, horiz_total, vert_total, &visarea, refresh);

				crtc6845->has_valid_parameters = TRUE;
			}
			else
				crtc6845->has_valid_parameters = FALSE;

			crtc6845->last_horiz_total = horiz_total;
			crtc6845->last_vert_total = vert_total;
			crtc6845->last_max_x = max_x;
			crtc6845->last_max_y = max_y;

			update_timer(crtc6845);
		}
	}
}


static int is_display_enabled(crtc6845_t *crtc6845)
{
	UINT16 y = video_screen_get_vpos(crtc6845->intf->scrnum);
	UINT16 x = video_screen_get_hpos(crtc6845->intf->scrnum);

	return (y <= crtc6845->last_max_y) && (x <= crtc6845->last_max_x);
}


static void update_timer(crtc6845_t *crtc6845)
{
	INT16 next_y;
	UINT16 next_x;
	attotime duration;

	if (crtc6845->has_valid_parameters && (crtc6845->display_enable_changed_timer != 0))
	{
		if (is_display_enabled(crtc6845))
		{
			/* we are in a display region,
               get the location of the next blanking start */

			/* normally, it's at end the current raster line */
			next_y = video_screen_get_vpos(crtc6845->intf->scrnum);
			next_x = crtc6845->last_max_x + 1;

			/* but if visible width = horiz_total, then we need
               to go to the beginning of VBLANK */
			if (next_x == crtc6845->last_horiz_total)
			{
				next_y = crtc6845->last_max_y + 1;
				next_x = 0;

				/* abnormal case, no vertical blanking, either */
				if (next_y == crtc6845->last_vert_total)
					next_y = -1;
			}
		}
		else
		{
			/* we are in a blanking region,
               get the location of the next display start */
			next_x = 0;
			next_y = (video_screen_get_vpos(crtc6845->intf->scrnum) + 1) % crtc6845->last_vert_total;

			/* if we would now fall in the vertical blanking, we need
               to go to the top of the screen */
			if (next_y > crtc6845->last_max_y)
				next_y = 0;
		}

		if (next_y != -1)
			duration = video_screen_get_time_until_pos(crtc6845->intf->scrnum, next_y, next_x);
		else
			duration = attotime_never;

		timer_adjust_oneshot(crtc6845->display_enable_changed_timer, duration, 0);
	}
}


static TIMER_CALLBACK( display_enable_changed_timer_cb )
{
	crtc6845_t *crtc6845 = ptr;

	/* call the callback function -- we know it exists */
	crtc6845->intf->display_enable_changed(is_display_enabled(crtc6845));

	update_timer(crtc6845);
}


UINT16 crtc6845_get_ma(crtc6845_t *crtc6845)
{
	UINT16 ret;

	if (crtc6845->has_valid_parameters)
	{
		/* get the current raster positions and clamp them to the visible region */
		int y = video_screen_get_vpos(0);
		int x = video_screen_get_hpos(0);

		/* since the MA counter stops in the blanking regions, if we are in a
           VBLANK, both X and Y are at their max */
		if ((y > crtc6845->last_max_y) || (x > crtc6845->last_max_x))
			x = crtc6845->last_max_x;

		if (y > crtc6845->last_max_y)
			y = crtc6845->last_max_y;

		ret = (crtc6845->start_addr +
			   (y / (crtc6845->max_ras_addr + 1)) * crtc6845->horiz_disp +
			   (x / crtc6845->intf->hpixels_per_column)) & 0x3fff;
	}
	else
		ret = 0;

	return ret;
}


UINT8 crtc6845_get_ra(crtc6845_t *crtc6845)
{
	UINT8 ret;

	if (crtc6845->has_valid_parameters)
	{
		/* get the current vertical raster position and clamp it to the visible region */
		int y = video_screen_get_vpos(0);

		if (y > crtc6845->last_max_y)
			y = crtc6845->last_max_y;

		ret = y % (crtc6845->max_ras_addr + 1);
	}
	else
		ret = 0;

	return ret;
}


void crtc6845_update(crtc6845_t *crtc6845, mame_bitmap *bitmap, const rectangle *cliprect)
{
	if (crtc6845->has_valid_parameters)
	{
		UINT16 y;

		/* call the set up function if any */
		void *param = 0;

		if (crtc6845->intf->begin_update)
			param = crtc6845->intf->begin_update(bitmap, cliprect);

		/* read the start address at the beginning of the frame */
		if (cliprect->min_y == 0)
			crtc6845->current_ma = crtc6845->start_addr;

		/* for each row in the visible region */
		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			UINT8 ra = y % (crtc6845->max_ras_addr + 1);

			/* call the external system to draw it */
			crtc6845->intf->update_row(bitmap, cliprect, crtc6845->current_ma, ra, y, crtc6845->horiz_disp, param);

			/* update MA if the last raster address */
			if (ra == crtc6845->max_ras_addr)
				crtc6845->current_ma = (crtc6845->current_ma + crtc6845->horiz_disp) & 0x3fff;
		}

		/* call the tear down function if any */
		if (crtc6845->intf->end_update)
			crtc6845->intf->end_update(bitmap, cliprect, param);
	}
}
