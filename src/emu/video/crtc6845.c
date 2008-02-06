/**********************************************************************

    Motorola 6845 CRT controller emulation

**********************************************************************/

#include "driver.h"
#include "crtc6845.h"


#define LOG		(0)


typedef struct _crtc6845_state crtc6845_state;

struct _crtc6845_state
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

static crtc6845_state crtc6845;


static void crtc6845_state_save_postload(void *param);
static void configure_screen(crtc6845_state *chip, int postload);
static void update_timer(crtc6845_state *chip);
static TIMER_CALLBACK( display_enable_changed_timer_cb );


void crtc6845_init(void)
{
	crtc6845_state *chip = &crtc6845;
	int which = 0;

	state_save_register_func_postload_ptr(crtc6845_state_save_postload, chip);

	state_save_register_item("crtc6845", which, chip->address_latch);
	state_save_register_item("crtc6845", which, chip->horiz_total);
	state_save_register_item("crtc6845", which, chip->horiz_disp);
	state_save_register_item("crtc6845", which, chip->horiz_sync_pos);
	state_save_register_item("crtc6845", which, chip->sync_width);
	state_save_register_item("crtc6845", which, chip->vert_total);
	state_save_register_item("crtc6845", which, chip->vert_total_adj);
	state_save_register_item("crtc6845", which, chip->vert_disp);
	state_save_register_item("crtc6845", which, chip->vert_sync_pos);
	state_save_register_item("crtc6845", which, chip->intl_skew);
	state_save_register_item("crtc6845", which, chip->max_ras_addr);
	state_save_register_item("crtc6845", which, chip->cursor_start_ras);
	state_save_register_item("crtc6845", which, chip->cursor_end_ras);
	state_save_register_item("crtc6845", which, chip->start_addr);
	state_save_register_item("crtc6845", which, chip->cursor);
	state_save_register_item("crtc6845", which, chip->light_pen);

	/* do not configure the screen */
	chip->intf = 0;
	chip->has_valid_parameters = FALSE;
}


void crtc6845_config(int which, const crtc6845_interface *intf)
{
	crtc6845_state *chip = &crtc6845;

	memset(chip, 0, sizeof(*chip));
	crtc6845_init();

	chip->intf = intf;

	/* create the timer if the user is interested in getting display enable
       notifications */
	if (intf->display_enable_changed)
		chip->display_enable_changed_timer = timer_alloc(display_enable_changed_timer_cb, chip);
}


static void crtc6845_state_save_postload(void *param)
{
	crtc6845_state *chip = (crtc6845_state *)param;

	configure_screen(chip, TRUE);
}


READ8_HANDLER( crtc6845_register_r )
{
	crtc6845_state *chip = &crtc6845;

	UINT8 ret = 0xff;

	switch (chip->address_latch)
	{
		case 14:
			ret = chip->cursor >> 8;
			break;
		case 15:
			ret = chip->cursor;
			break;
		case 16:
			ret = chip->light_pen >> 8;
			break;
		case 17:
			ret = chip->light_pen;
			break;
		default:
			/* all other registers are write only */
			break;
	}
	return ret;
}


WRITE8_HANDLER( crtc6845_address_w )
{
	crtc6845_state *chip = &crtc6845;

	chip->address_latch = data & 0x1f;
}


WRITE8_HANDLER( crtc6845_register_w )
{
	crtc6845_state *chip = &crtc6845;

	int call_configure_screen = FALSE;
	if (LOG)  logerror("CRT #0 PC %04x: CRTC6845 reg 0x%02x = 0x%02x\n", activecpu_get_pc(), chip->address_latch, data);

	switch (chip->address_latch)
	{
		case 0:
			chip->horiz_total = data;
			call_configure_screen = TRUE;
			break;
		case 1:
			chip->horiz_disp = data;
			call_configure_screen = TRUE;
			break;
		case 2:
			chip->horiz_sync_pos = data;
			break;
		case 3:
			chip->sync_width = data & 0x0f;
			break;
		case 4:
			chip->vert_total = data & 0x7f;
			call_configure_screen = TRUE;
			break;
		case 5:
			chip->vert_total_adj = data & 0x1f;
			call_configure_screen = TRUE;
			break;
		case 6:
			chip->vert_disp = data & 0x7f;
			call_configure_screen = TRUE;
			break;
		case 7:
			chip->vert_sync_pos = data & 0x7f;
			break;
		case 8:
			chip->intl_skew = data & 0x03;
			break;
		case 9:
			chip->max_ras_addr = data & 0x1f;
			call_configure_screen = TRUE;
			break;
		case 10:
			chip->cursor_start_ras = data & 0x7f;
			break;
		case 11:
			chip->cursor_end_ras = data & 0x1f;
			break;
		case 12:
			chip->start_addr &= 0x00ff;
			chip->start_addr |= (data & 0x3f) << 8;
			call_configure_screen = TRUE;
			break;
		case 13:
			chip->start_addr &= 0xff00;
			chip->start_addr |= data;
			call_configure_screen = TRUE;
			break;
		case 14:
			chip->cursor &= 0x00ff;
			chip->cursor |= (data & 0x3f) << 8;
			break;
		case 15:
			chip->cursor &= 0xff00;
			chip->cursor |= data;
			break;
		case 16:	/* read-only */
			break;
		case 17:	/* read-only */
			break;
		default:
			break;
	}

	if (call_configure_screen)
		configure_screen(chip, FALSE);
}


static void configure_screen(crtc6845_state *chip, int postload)
{
	if (chip->intf)
	{
		/* compute the screen sizes */
		UINT16 horiz_total = (chip->horiz_total + 1) * chip->intf->hpixels_per_column;
		UINT16 vert_total = (chip->vert_total + 1) * (chip->max_ras_addr + 1) + chip->vert_total_adj;

		/* determine the visible area, avoid division by 0 */
		UINT16 max_x = chip->horiz_disp * chip->intf->hpixels_per_column - 1;
		UINT16 max_y = chip->vert_disp * (chip->max_ras_addr + 1) - 1;

		/* update only if screen parameters changed, unless we are coming here after loading the saved state */
		if (postload ||
		    (horiz_total != chip->last_horiz_total) || (vert_total != chip->last_vert_total) ||
			(max_x != chip->last_max_x) || (max_y != chip->last_max_y))
		{
			/* update the screen only if we have valid data */
			if ((chip->horiz_total > 0) && (max_x < horiz_total) && (chip->vert_total > 0) && (max_y < vert_total))
			{
				rectangle visarea;

				attoseconds_t refresh = HZ_TO_ATTOSECONDS(chip->intf->clock) * (chip->horiz_total + 1) * vert_total;

				visarea.min_x = 0;
				visarea.min_y = 0;
				visarea.max_x = max_x;
				visarea.max_y = max_y;

				if (LOG) logerror("CRTC6845 config screen: HTOTAL: %x  VTOTAL: %x  MAX_X: %x  MAX_Y: %x  FPS: %f\n",
								  horiz_total, vert_total, max_x, max_y, 1 / ATTOSECONDS_TO_DOUBLE(refresh));

				video_screen_configure(chip->intf->scrnum, horiz_total, vert_total, &visarea, refresh);

				chip->has_valid_parameters = TRUE;
			}
			else
				chip->has_valid_parameters = FALSE;

			chip->last_horiz_total = horiz_total;
			chip->last_vert_total = vert_total;
			chip->last_max_x = max_x;
			chip->last_max_y = max_y;

			update_timer(chip);
		}
	}
}


static int is_display_enabled(crtc6845_state *chip)
{
	UINT16 y = video_screen_get_vpos(chip->intf->scrnum);
	UINT16 x = video_screen_get_hpos(chip->intf->scrnum);

	return (y <= chip->last_max_y) && (x <= chip->last_max_x);
}


static void update_timer(crtc6845_state *chip)
{
	INT16 next_y;
	UINT16 next_x;
	attotime duration;

	if (chip->has_valid_parameters && (chip->display_enable_changed_timer != 0))
	{
		if (is_display_enabled(chip))
		{
			/* we are in a display region,
               get the location of the next blanking start */

			/* normally, it's at end the current raster line */
			next_y = video_screen_get_vpos(chip->intf->scrnum);
			next_x = chip->last_max_x + 1;

			/* but if visible width = horiz_total, then we need
               to go to the beginning of VBLANK */
			if (next_x == chip->last_horiz_total)
			{
				next_y = chip->last_max_y + 1;
				next_x = 0;

				/* abnormal case, no vertical blanking, either */
				if (next_y == chip->last_vert_total)
					next_y = -1;
			}
		}
		else
		{
			/* we are in a blanking region,
               get the location of the next display start */
			next_x = 0;
			next_y = (video_screen_get_vpos(chip->intf->scrnum) + 1) % chip->last_vert_total;

			/* if we would now fall in the vertical blanking, we need
               to go to the top of the screen */
			if (next_y > chip->last_max_y)
				next_y = 0;
		}

		if (next_y != -1)
			duration = video_screen_get_time_until_pos(chip->intf->scrnum, next_y, next_x);
		else
			duration = attotime_never;

		timer_adjust_oneshot(chip->display_enable_changed_timer, duration, 0);
	}
}


static TIMER_CALLBACK( display_enable_changed_timer_cb )
{
	crtc6845_state *chip = ptr;

	/* call the callback function -- we know it exists */
	chip->intf->display_enable_changed(is_display_enabled(chip));

	update_timer(chip);
}


UINT16 crtc6845_get_ma(int which)
{
	crtc6845_state *chip = &crtc6845;

	UINT16 ret;

	if (chip->has_valid_parameters)
	{
		/* get the current raster positions and clamp them to the visible region */
		int y = video_screen_get_vpos(0);
		int x = video_screen_get_hpos(0);

		/* since the MA counter stops in the blanking regions, if we are in a
           VBLANK, both X and Y are at their max */
		if ((y > chip->last_max_y) || (x > chip->last_max_x))
			x = chip->last_max_x;

		if (y > chip->last_max_y)
			y = chip->last_max_y;

		ret = (chip->start_addr +
			   (y / (chip->max_ras_addr + 1)) * chip->horiz_disp +
			   (x / chip->intf->hpixels_per_column)) & 0x3fff;
	}
	else
		ret = 0;

	return ret;
}


UINT8 crtc6845_get_ra(int which)
{
	crtc6845_state *chip = &crtc6845;

	UINT8 ret;

	if (chip->has_valid_parameters)
	{
		/* get the current vertical raster position and clamp it to the visible region */
		int y = video_screen_get_vpos(0);

		if (y > chip->last_max_y)
			y = chip->last_max_y;

		ret = y % (chip->max_ras_addr + 1);
	}
	else
		ret = 0;

	return ret;
}


VIDEO_UPDATE( crtc6845 )
{
	crtc6845_state *chip = &crtc6845;

	if (chip->has_valid_parameters)
	{
		UINT16 y;

		/* call the set up function if any */
		void *param = 0;

		if (chip->intf->begin_update)
			param = chip->intf->begin_update(machine, screen, bitmap, cliprect);

		/* read the start address at the beginning of the frame */
		if (cliprect->min_y == 0)
			chip->current_ma = chip->start_addr;

		/* for each row in the visible region */
		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			UINT8 ra = y % (chip->max_ras_addr + 1);

			/* call the external system to draw it */
			chip->intf->update_row(bitmap, cliprect, chip->current_ma, ra, y, chip->horiz_disp, param);

			/* update MA if the last raster address */
			if (ra == chip->max_ras_addr)
				chip->current_ma = (chip->current_ma + chip->horiz_disp) & 0x3fff;
		}

		/* call the tear down function if any */
		if (chip->intf->end_update)
			chip->intf->end_update(bitmap, cliprect, param);
	}

	return 0;
}
