/**********************************************************************

    Motorola 6845 CRT controller emulation

**********************************************************************/

#include "driver.h"
#include "m6845.h"


#define LOG		(0)


struct _m6845_t
{
	const m6845_interface *intf;

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


static void m6845_state_save_postload(void *param);
static void configure_screen(m6845_t *m6845, int postload);
static void update_timer(m6845_t *m6845);
static TIMER_CALLBACK( display_enable_changed_timer_cb );


m6845_t *m6845_config(const m6845_interface *intf)
{
	m6845_t *m6845;

	/* allocate the object that holds the state */
	m6845 = auto_malloc(sizeof(*m6845));
	memset(m6845, 0, sizeof(*m6845));

	m6845->intf = intf;

	/* create the timer if the user is interested in getting display enable
       notifications */
	if (intf && intf->display_enable_changed)
		m6845->display_enable_changed_timer = timer_alloc(display_enable_changed_timer_cb, m6845);

	/* register for state saving */
	state_save_register_func_postload_ptr(m6845_state_save_postload, m6845);

	state_save_register_item("m6845", 0, m6845->address_latch);
	state_save_register_item("m6845", 0, m6845->horiz_total);
	state_save_register_item("m6845", 0, m6845->horiz_disp);
	state_save_register_item("m6845", 0, m6845->horiz_sync_pos);
	state_save_register_item("m6845", 0, m6845->sync_width);
	state_save_register_item("m6845", 0, m6845->vert_total);
	state_save_register_item("m6845", 0, m6845->vert_total_adj);
	state_save_register_item("m6845", 0, m6845->vert_disp);
	state_save_register_item("m6845", 0, m6845->vert_sync_pos);
	state_save_register_item("m6845", 0, m6845->intl_skew);
	state_save_register_item("m6845", 0, m6845->max_ras_addr);
	state_save_register_item("m6845", 0, m6845->cursor_start_ras);
	state_save_register_item("m6845", 0, m6845->cursor_end_ras);
	state_save_register_item("m6845", 0, m6845->start_addr);
	state_save_register_item("m6845", 0, m6845->cursor);
	state_save_register_item("m6845", 0, m6845->light_pen);

	return m6845;
}


static void m6845_state_save_postload(void *param)
{
	m6845_t *m6845 = (m6845_t *)param;

	configure_screen(m6845, TRUE);
}


void m6845_address_w(m6845_t *m6845, UINT8 data)
{
	m6845->address_latch = data & 0x1f;
}


UINT8 m6845_register_r(m6845_t *m6845)
{
	UINT8 ret = 0xff;

	switch (m6845->address_latch)
	{
		case 14:
			ret = m6845->cursor >> 8;
			break;
		case 15:
			ret = m6845->cursor;
			break;
		case 16:
			ret = m6845->light_pen >> 8;
			break;
		case 17:
			ret = m6845->light_pen;
			break;
		default:
			/* all other registers are write only */
			break;
	}

	return ret;
}


void m6845_register_w(m6845_t *m6845, UINT8 data)
{
	int call_configure_screen = FALSE;
	if (LOG)  logerror("CRT #0 PC %04x: CRTC6845 reg 0x%02x = 0x%02x\n", activecpu_get_pc(), m6845->address_latch, data);

	switch (m6845->address_latch)
	{
		case 0:
			m6845->horiz_total = data;
			call_configure_screen = TRUE;
			break;
		case 1:
			m6845->horiz_disp = data;
			call_configure_screen = TRUE;
			break;
		case 2:
			m6845->horiz_sync_pos = data;
			break;
		case 3:
			m6845->sync_width = data & 0x0f;
			break;
		case 4:
			m6845->vert_total = data & 0x7f;
			call_configure_screen = TRUE;
			break;
		case 5:
			m6845->vert_total_adj = data & 0x1f;
			call_configure_screen = TRUE;
			break;
		case 6:
			m6845->vert_disp = data & 0x7f;
			call_configure_screen = TRUE;
			break;
		case 7:
			m6845->vert_sync_pos = data & 0x7f;
			break;
		case 8:
			m6845->intl_skew = data & 0x03;
			break;
		case 9:
			m6845->max_ras_addr = data & 0x1f;
			call_configure_screen = TRUE;
			break;
		case 10:
			m6845->cursor_start_ras = data & 0x7f;
			break;
		case 11:
			m6845->cursor_end_ras = data & 0x1f;
			break;
		case 12:
			m6845->start_addr &= 0x00ff;
			m6845->start_addr |= (data & 0x3f) << 8;
			call_configure_screen = TRUE;
			break;
		case 13:
			m6845->start_addr &= 0xff00;
			m6845->start_addr |= data;
			call_configure_screen = TRUE;
			break;
		case 14:
			m6845->cursor &= 0x00ff;
			m6845->cursor |= (data & 0x3f) << 8;
			break;
		case 15:
			m6845->cursor &= 0xff00;
			m6845->cursor |= data;
			break;
		case 16:	/* read-only */
			break;
		case 17:	/* read-only */
			break;
		default:
			break;
	}

	if (call_configure_screen)
		configure_screen(m6845, FALSE);
}


static void configure_screen(m6845_t *m6845, int postload)
{
	if (m6845->intf)
	{
		/* compute the screen sizes */
		UINT16 horiz_total = (m6845->horiz_total + 1) * m6845->intf->hpixels_per_column;
		UINT16 vert_total = (m6845->vert_total + 1) * (m6845->max_ras_addr + 1) + m6845->vert_total_adj;

		/* determine the visible area, avoid division by 0 */
		UINT16 max_x = m6845->horiz_disp * m6845->intf->hpixels_per_column - 1;
		UINT16 max_y = m6845->vert_disp * (m6845->max_ras_addr + 1) - 1;

		/* update only if screen parameters changed, unless we are coming here after loading the saved state */
		if (postload ||
		    (horiz_total != m6845->last_horiz_total) || (vert_total != m6845->last_vert_total) ||
			(max_x != m6845->last_max_x) || (max_y != m6845->last_max_y))
		{
			/* update the screen only if we have valid data */
			if ((m6845->horiz_total > 0) && (max_x < horiz_total) && (m6845->vert_total > 0) && (max_y < vert_total))
			{
				rectangle visarea;

				attoseconds_t refresh = HZ_TO_ATTOSECONDS(m6845->intf->clock) * (m6845->horiz_total + 1) * vert_total;

				visarea.min_x = 0;
				visarea.min_y = 0;
				visarea.max_x = max_x;
				visarea.max_y = max_y;

				if (LOG) logerror("CRTC6845 config screen: HTOTAL: %x  VTOTAL: %x  MAX_X: %x  MAX_Y: %x  FPS: %f\n",
								  horiz_total, vert_total, max_x, max_y, 1 / ATTOSECONDS_TO_DOUBLE(refresh));

				video_screen_configure(m6845->intf->scrnum, horiz_total, vert_total, &visarea, refresh);

				m6845->has_valid_parameters = TRUE;
			}
			else
				m6845->has_valid_parameters = FALSE;

			m6845->last_horiz_total = horiz_total;
			m6845->last_vert_total = vert_total;
			m6845->last_max_x = max_x;
			m6845->last_max_y = max_y;

			update_timer(m6845);
		}
	}
}


static int is_display_enabled(m6845_t *m6845)
{
	UINT16 y = video_screen_get_vpos(m6845->intf->scrnum);
	UINT16 x = video_screen_get_hpos(m6845->intf->scrnum);

	return (y <= m6845->last_max_y) && (x <= m6845->last_max_x);
}


static void update_timer(m6845_t *m6845)
{
	INT16 next_y;
	UINT16 next_x;
	attotime duration;

	if (m6845->has_valid_parameters && (m6845->display_enable_changed_timer != 0))
	{
		if (is_display_enabled(m6845))
		{
			/* we are in a display region,
               get the location of the next blanking start */

			/* normally, it's at end the current raster line */
			next_y = video_screen_get_vpos(m6845->intf->scrnum);
			next_x = m6845->last_max_x + 1;

			/* but if visible width = horiz_total, then we need
               to go to the beginning of VBLANK */
			if (next_x == m6845->last_horiz_total)
			{
				next_y = m6845->last_max_y + 1;
				next_x = 0;

				/* abnormal case, no vertical blanking, either */
				if (next_y == m6845->last_vert_total)
					next_y = -1;
			}
		}
		else
		{
			/* we are in a blanking region,
               get the location of the next display start */
			next_x = 0;
			next_y = (video_screen_get_vpos(m6845->intf->scrnum) + 1) % m6845->last_vert_total;

			/* if we would now fall in the vertical blanking, we need
               to go to the top of the screen */
			if (next_y > m6845->last_max_y)
				next_y = 0;
		}

		if (next_y != -1)
			duration = video_screen_get_time_until_pos(m6845->intf->scrnum, next_y, next_x);
		else
			duration = attotime_never;

		timer_adjust_oneshot(m6845->display_enable_changed_timer, duration, 0);
	}
}


static TIMER_CALLBACK( display_enable_changed_timer_cb )
{
	m6845_t *m6845 = ptr;

	/* call the callback function -- we know it exists */
	m6845->intf->display_enable_changed(is_display_enabled(m6845));

	update_timer(m6845);
}


UINT16 m6845_get_ma(m6845_t *m6845)
{
	UINT16 ret;

	if (m6845->has_valid_parameters)
	{
		/* get the current raster positions and clamp them to the visible region */
		int y = video_screen_get_vpos(0);
		int x = video_screen_get_hpos(0);

		/* since the MA counter stops in the blanking regions, if we are in a
           VBLANK, both X and Y are at their max */
		if ((y > m6845->last_max_y) || (x > m6845->last_max_x))
			x = m6845->last_max_x;

		if (y > m6845->last_max_y)
			y = m6845->last_max_y;

		ret = (m6845->start_addr +
			   (y / (m6845->max_ras_addr + 1)) * m6845->horiz_disp +
			   (x / m6845->intf->hpixels_per_column)) & 0x3fff;
	}
	else
		ret = 0;

	return ret;
}


UINT8 m6845_get_ra(m6845_t *m6845)
{
	UINT8 ret;

	if (m6845->has_valid_parameters)
	{
		/* get the current vertical raster position and clamp it to the visible region */
		int y = video_screen_get_vpos(0);

		if (y > m6845->last_max_y)
			y = m6845->last_max_y;

		ret = y % (m6845->max_ras_addr + 1);
	}
	else
		ret = 0;

	return ret;
}


void m6845_update(m6845_t *m6845, mame_bitmap *bitmap, const rectangle *cliprect)
{
	if (m6845->has_valid_parameters)
	{
		UINT16 y;

		/* call the set up function if any */
		void *param = 0;

		if (m6845->intf->begin_update)
			param = m6845->intf->begin_update(bitmap, cliprect);

		/* read the start address at the beginning of the frame */
		if (cliprect->min_y == 0)
			m6845->current_ma = m6845->start_addr;

		/* for each row in the visible region */
		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			UINT8 ra = y % (m6845->max_ras_addr + 1);

			/* call the external system to draw it */
			m6845->intf->update_row(bitmap, cliprect, m6845->current_ma, ra, y, m6845->horiz_disp, param);

			/* update MA if the last raster address */
			if (ra == m6845->max_ras_addr)
				m6845->current_ma = (m6845->current_ma + m6845->horiz_disp) & 0x3fff;
		}

		/* call the tear down function if any */
		if (m6845->intf->end_update)
			m6845->intf->end_update(bitmap, cliprect, param);
	}
}
