/**********************************************************************

    Motorola MC6845 and compatible CRT controller emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

    The following variations exist that are different in
    functionality and not just in speed rating(1):
        * Motorola 6845, 6845-1
        * Hitachi 46505
        * Rockwell 6545, 6545-1 (= Synertek SY6545-1)
        * MOS Technology 6545-1

    (1) as per the document at
    http://www.6502.org/users/andre/hwinfo/crtc/diffs.html

    The various speed rated devices are identified by a letter,
    for example M68A45, M68B45, etc.

    The chip is originally designed by Hitachi, not by Motorola.

**********************************************************************/

#include "emu.h"
#include "mc6845.h"


#define LOG		(1)


/* device types */
enum
{
	TYPE_MC6845 = 0,
	TYPE_MC6845_1,
	TYPE_C6545_1,
	TYPE_R6545_1,
	TYPE_H46505,
	TYPE_HD6845,
	TYPE_SY6545_1,
	TYPE_SY6845E,

	NUM_TYPES
};

/* mode macros */
#define MODE_TRANSPARENT(d)				(((d)->mode_control & 0x08) != 0)
#define MODE_TRANSPARENT_PHI2(d)		(((d)->mode_control & 0x88) == 0x88)
/* FIXME: not supported yet */
#define MODE_TRANSPARENT_BLANK(d)		(((d)->mode_control & 0x88) == 0x08)
#define MODE_UPDATE_STROBE(d)			(((d)->mode_control & 0x40) != 0)
#define MODE_CURSOR_SKEW(d)				(((d)->mode_control & 0x20) != 0)
#define MODE_DISPLAY_ENABLE_SKEW(d)		(((d)->mode_control & 0x10) != 0)
#define MODE_ROW_COLUMN_ADDRESSING(d)	(((d)->mode_control & 0x04) != 0)

/* capabilities */                                     /* MC6845    MC6845-1    C6545-1    R6545-1    H46505    HD6845    SY6545-1    SY6845E */
static const int supports_disp_start_addr_r[NUM_TYPES] = {  TRUE,       TRUE,     FALSE,     FALSE,    FALSE,    FALSE,      FALSE,		FALSE };
static const int supports_vert_sync_width[NUM_TYPES]   = { FALSE,       TRUE,      TRUE,      TRUE,    FALSE,     TRUE,       TRUE,		 TRUE };
static const int supports_status_reg_d5[NUM_TYPES]     = { FALSE,      FALSE,      TRUE,      TRUE,    FALSE,    FALSE,       TRUE,		 TRUE };
static const int supports_status_reg_d6[NUM_TYPES]     = { FALSE,      FALSE,      TRUE,      TRUE,    FALSE,    FALSE,       TRUE,		 TRUE };

/* FIXME: check other variants */
static const int supports_status_reg_d7[NUM_TYPES]     = { FALSE,      FALSE,     FALSE,      TRUE,    FALSE,    FALSE,       TRUE,		 TRUE };

/* FIXME: check other variants */
static const int supports_transparent[NUM_TYPES]       = { FALSE,      FALSE,     FALSE,      TRUE,    FALSE,    FALSE,       TRUE,		 TRUE };


typedef struct _mc6845_t mc6845_t;
struct _mc6845_t
{
	devcb_resolved_write_line			out_de_func;
	devcb_resolved_write_line			out_cur_func;
	devcb_resolved_write_line			out_hsync_func;
	devcb_resolved_write_line			out_vsync_func;

	int device_type;
	const mc6845_interface *intf;
	screen_device *screen;

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
	UINT16	update_addr;		/* 0x12/0x13 */

	/* other internal state */
	UINT64	clock;
	UINT8	register_address_latch;
	UINT8	hpixels_per_column;
	UINT8	cursor_state;	/* 0 = off, 1 = on */
	UINT8	cursor_blink_count;
	UINT8	update_ready_bit;
	/* output signals */
	int		cur;
	int		hsync;
	int		vsync;
	int		de;

	/* internal counters */
	UINT8	character_counter;		/* Not used yet */
	UINT8	hsync_width_counter;	/* Not used yet */
	UINT8	line_counter;
	UINT8	raster_counter;
	UINT8	adjust_counter;
	UINT8	vsync_width_counter;

	UINT8	line_enable_ff;		/* Internal flip flop which is set when the line_counter is reset and reset when vert_disp is reached */
	UINT8	vsync_ff;
	UINT8	adjust_active;
	UINT16	line_address;
	INT16	cursor_x;

	/* timers */
	emu_timer *line_timer;
	emu_timer *de_off_timer;
	emu_timer *cur_on_timer;
	emu_timer *cur_off_timer;
	emu_timer *hsync_on_timer;
	emu_timer *hsync_off_timer;
	emu_timer *light_pen_latch_timer;
	emu_timer *upd_adr_timer;

	/* computed values - do NOT state save these! */
	/* These computed are used to define the screen parameters for a driver */
	UINT16	horiz_pix_total;
	UINT16	vert_pix_total;
	UINT16	max_visible_x;
	UINT16	max_visible_y;
	UINT16	hsync_on_pos;
	UINT16	hsync_off_pos;
	UINT16	vsync_on_pos;
	UINT16	vsync_off_pos;
	int		has_valid_parameters;

	UINT16   current_disp_addr;	/* the display address currently drawn (used only in mc6845_update) */

	UINT8	 light_pen_latched;
	attotime upd_time;
};


static STATE_POSTLOAD( mc6845_state_save_postload );
static void recompute_parameters(mc6845_t *mc6845, int postload);
static void update_upd_adr_timer(mc6845_t *mc6845);
static void update_cursor_state(mc6845_t *mc6845);


const mc6845_interface mc6845_null_interface = { 0 };


/* makes sure that the passed in device is the right type */
INLINE mc6845_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert((device->type() == MC6845) ||
		   (device->type() == MC6845_1) ||
		   (device->type() == C6545_1) ||
		   (device->type() == R6545_1) ||
		   (device->type() == H46505) ||
		   (device->type() == HD6845) ||
		   (device->type() == SY6545_1) ||
		   (device->type() == SY6845E));

	return (mc6845_t *)downcast<legacy_device_base *>(device)->token();
}


static STATE_POSTLOAD( mc6845_state_save_postload )
{
	recompute_parameters((mc6845_t *)param, TRUE);
}


static TIMER_CALLBACK( on_update_address_cb )
{
	device_t *device = (device_t *)ptr;
	mc6845_t *mc6845 = get_safe_token(device);
	int addr = (param >> 8);
	int strobe = (param & 0xff);

	/* call the callback function -- we know it exists */
	mc6845->intf->on_update_addr_changed(device, addr, strobe);

	if(!mc6845->update_ready_bit && MODE_TRANSPARENT_BLANK(mc6845))
	{
		mc6845->update_addr++;
		mc6845->update_addr &= 0x3fff;
		mc6845->update_ready_bit = 1;
	}
}

INLINE void call_on_update_address(device_t *device, int strobe)
{
	mc6845_t *mc6845 = get_safe_token(device);

	if (mc6845->intf->on_update_addr_changed)
		timer_set(device->machine, attotime_zero, (void *) device, (mc6845->update_addr << 8) | strobe, on_update_address_cb);
	else
		fatalerror("M6845: transparent memory mode without handler\n");
}


WRITE8_DEVICE_HANDLER( mc6845_address_w )
{
	mc6845_t *mc6845 = get_safe_token(device);

	mc6845->register_address_latch = data & 0x1f;
}


READ8_DEVICE_HANDLER( mc6845_status_r )
{
	mc6845_t *mc6845 = get_safe_token(device);
	UINT8 ret = 0;

	/* VBLANK bit */
	if (supports_status_reg_d5[mc6845->device_type] && ( mc6845->line_enable_ff == 0 ))
	   ret = ret | 0x20;

	/* light pen latched */
	if (supports_status_reg_d6[mc6845->device_type] && mc6845->light_pen_latched)
	   ret = ret | 0x40;

	/* UPDATE ready */
	if (supports_status_reg_d7[mc6845->device_type] && mc6845->update_ready_bit)
	   ret = ret | 0x80;

	return ret;
}


READ8_DEVICE_HANDLER( mc6845_register_r )
{
	mc6845_t *mc6845 = get_safe_token(device);
	UINT8 ret = 0;

	switch (mc6845->register_address_latch)
	{
		case 0x0c:  ret = supports_disp_start_addr_r[mc6845->device_type] ? (mc6845->disp_start_addr >> 8) & 0xff : 0; break;
		case 0x0d:  ret = supports_disp_start_addr_r[mc6845->device_type] ? (mc6845->disp_start_addr >> 0) & 0xff : 0; break;
		case 0x0e:  ret = (mc6845->cursor_addr    >> 8) & 0xff; break;
		case 0x0f:  ret = (mc6845->cursor_addr    >> 0) & 0xff; break;
		case 0x10:  ret = (mc6845->light_pen_addr >> 8) & 0xff; mc6845->light_pen_latched = FALSE; break;
		case 0x11:  ret = (mc6845->light_pen_addr >> 0) & 0xff; mc6845->light_pen_latched = FALSE; break;
		case 0x1f:
			if (supports_transparent[mc6845->device_type] && MODE_TRANSPARENT(mc6845))
			{
				if(MODE_TRANSPARENT_PHI2(mc6845))
				{
					mc6845->update_addr++;
					mc6845->update_addr &= 0x3fff;
					call_on_update_address(device, 0);
				}
				else
				{
					/* MODE_TRANSPARENT_BLANK */
					if(mc6845->update_ready_bit)
					{
						mc6845->update_ready_bit = 0;
						update_upd_adr_timer(mc6845);
					}
				}
			}
			break;

		/* all other registers are write only and return 0 */
		default: break;
	}

	return ret;
}


WRITE8_DEVICE_HANDLER( mc6845_register_w )
{
	mc6845_t *mc6845 = get_safe_token(device);

	if (LOG)  logerror("%s:M6845 reg 0x%02x = 0x%02x\n", cpuexec_describe_context(device->machine), mc6845->register_address_latch, data);

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
		case 0x12:
			if (supports_transparent[mc6845->device_type])
			{
				mc6845->update_addr = ((data & 0x3f) << 8) | (mc6845->update_addr & 0x00ff);
				if(MODE_TRANSPARENT_PHI2(mc6845))
					call_on_update_address(device, 0);
			}
			break;
		case 0x13:
			if (supports_transparent[mc6845->device_type])
			{
				mc6845->update_addr = ((data & 0xff) << 0) | (mc6845->update_addr & 0xff00);
				if(MODE_TRANSPARENT_PHI2(mc6845))
					call_on_update_address(device, 0);
			}
			break;
		case 0x1f:
			if (supports_transparent[mc6845->device_type] && MODE_TRANSPARENT(mc6845))
			{
				if(MODE_TRANSPARENT_PHI2(mc6845))
				{
					mc6845->update_addr++;
					mc6845->update_addr &= 0x3fff;
					call_on_update_address(device, 0);
				}
				else
				{
					/* MODE_TRANSPARENT_BLANK */
					if(mc6845->update_ready_bit)
					{
						mc6845->update_ready_bit = 0;
						update_upd_adr_timer(mc6845);
					}
				}
			}
			break;
		default: break;
	}

	/* display message if the Mode Control register is not zero */
	if ((mc6845->register_address_latch == 0x08) && (mc6845->mode_control != 0))
		if (!supports_transparent[mc6845->device_type])
			popmessage("Mode Control %02X is not supported!!!", mc6845->mode_control);

	recompute_parameters(mc6845, FALSE);
}


static void recompute_parameters(mc6845_t *mc6845, int postload)
{
	if (mc6845->intf != NULL)
	{
		UINT16 hsync_on_pos, hsync_off_pos, vsync_on_pos, vsync_off_pos;

		/* compute the screen sizes */
		UINT16 horiz_pix_total = (mc6845->horiz_char_total + 1) * mc6845->hpixels_per_column;
		UINT16 vert_pix_total = (mc6845->vert_char_total + 1) * (mc6845->max_ras_addr + 1) + mc6845->vert_total_adj;

		/* determine the visible area, avoid division by 0 */
		UINT16 max_visible_x = mc6845->horiz_disp * mc6845->hpixels_per_column - 1;
		UINT16 max_visible_y = mc6845->vert_disp * (mc6845->max_ras_addr + 1) - 1;

		/* determine the syncing positions */
		UINT8 horiz_sync_char_width = mc6845->sync_width & 0x0f;
		UINT8 vert_sync_pix_width = supports_vert_sync_width[mc6845->device_type] ? (mc6845->sync_width >> 4) & 0x0f : 0x10;

		if (horiz_sync_char_width == 0)
			horiz_sync_char_width = 0x10;

		if (vert_sync_pix_width == 0)
			vert_sync_pix_width = 0x10;

		/* determine the transparent update cycle time, 1 update every 4 character clocks */
		mc6845->upd_time = attotime_mul(ATTOTIME_IN_HZ(mc6845->clock), 4 * mc6845->hpixels_per_column);

		hsync_on_pos = mc6845->horiz_sync_pos * mc6845->hpixels_per_column;
		hsync_off_pos = hsync_on_pos + (horiz_sync_char_width * mc6845->hpixels_per_column);
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

				attoseconds_t refresh = HZ_TO_ATTOSECONDS(mc6845->clock) * (mc6845->horiz_char_total + 1) * vert_pix_total;

				visarea.min_x = 0;
				visarea.min_y = 0;
				visarea.max_x = max_visible_x;
				visarea.max_y = max_visible_y;

				if (LOG) logerror("M6845 config screen: HTOTAL: 0x%x  VTOTAL: 0x%x  MAX_X: 0x%x  MAX_Y: 0x%x  HSYNC: 0x%x-0x%x  VSYNC: 0x%x-0x%x  Freq: %ffps\n",
								  horiz_pix_total, vert_pix_total, max_visible_x, max_visible_y, hsync_on_pos, hsync_off_pos - 1, vsync_on_pos, vsync_off_pos - 1, 1 / ATTOSECONDS_TO_DOUBLE(refresh));

				if ( mc6845->screen != NULL )
					mc6845->screen->configure(horiz_pix_total, vert_pix_total, visarea, refresh);

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
		}
	}
}


INLINE void mc6845_update_counters(mc6845_t *mc6845)
{
	mc6845->character_counter = attotime_to_ticks( timer_timeelapsed( mc6845->line_timer ), mc6845->clock );

	if ( timer_enabled( mc6845->hsync_off_timer ) )
	{
		mc6845->hsync_width_counter = attotime_to_ticks( timer_timeelapsed( mc6845->hsync_off_timer ), mc6845->clock );
	}
}


INLINE void mc6845_set_de(mc6845_t *mc6845, int state)
{
	if ( mc6845->de != state )
	{
		mc6845->de = state;

		if ( mc6845->de )
		{
			/* If the upd_adr_timer was running, cancel it */
			timer_adjust_oneshot(mc6845->upd_adr_timer,  attotime_never, 0);
		}
		else
		{
			/* if transparent update was requested fire the update timer */
			if(!mc6845->update_ready_bit)
				update_upd_adr_timer(mc6845);
		}

		if ( mc6845->out_de_func.target != NULL )
			devcb_call_write_line( &mc6845->out_de_func, mc6845->de );
	}
}


INLINE void mc6845_set_hsync(mc6845_t *mc6845, int state)
{
	if ( mc6845->hsync != state )
	{
		mc6845->hsync = state;

		if ( mc6845->out_hsync_func.target != NULL )
			devcb_call_write_line( &mc6845->out_hsync_func, mc6845->hsync );
	}
}


INLINE void mc6845_set_vsync(mc6845_t *mc6845, int state)
{
	if ( mc6845->vsync != state )
	{
		mc6845->vsync = state;

		if ( mc6845->out_vsync_func.target != NULL )
			devcb_call_write_line( &mc6845->out_vsync_func, mc6845->vsync );
	}
}


INLINE void mc6845_set_cur(mc6845_t *mc6845, int state)
{
	if ( mc6845->cur != state )
	{
		mc6845->cur = state;

		if ( mc6845->out_cur_func.target != NULL )
			devcb_call_write_line( &mc6845->out_cur_func, mc6845->cur );
	}
}


static void update_upd_adr_timer(mc6845_t *mc6845)
{
	if (! mc6845->de && supports_transparent[mc6845->device_type])
		timer_adjust_oneshot(mc6845->upd_adr_timer,  mc6845->upd_time, 0);
}


static TIMER_CALLBACK( upd_adr_timer_cb )
{
	device_t *device = (device_t *)ptr;

	/* fire a update address strobe */
	call_on_update_address(device, 0);
}


static TIMER_CALLBACK( de_off_timer_cb )
{
	device_t *device = (device_t *)ptr;
	mc6845_t *mc6845 = get_safe_token(device);

	mc6845_set_de( mc6845, FALSE );
}


static TIMER_CALLBACK( cur_on_timer_cb )
{
	device_t *device = (device_t *)ptr;
	mc6845_t *mc6845 = get_safe_token(device);

	mc6845_set_cur( mc6845, TRUE );

	/* Schedule CURSOR off signal */
	timer_adjust_oneshot( mc6845->cur_off_timer, ticks_to_attotime( 1, mc6845->clock ), 0 );
}


static TIMER_CALLBACK( cur_off_timer_cb )
{
	device_t *device = (device_t *)ptr;
	mc6845_t *mc6845 = get_safe_token(device);

	mc6845_set_cur( mc6845, FALSE );
}


static TIMER_CALLBACK( hsync_on_timer_cb )
{
	device_t *device = (device_t *)ptr;
	mc6845_t *mc6845 = get_safe_token(device);
	UINT8 hsync_width = ( mc6845->sync_width & 0x0f ) ? ( mc6845->sync_width & 0x0f ) : 0x10;

	mc6845->hsync_width_counter = 0;
	mc6845_set_hsync( mc6845, TRUE );

	/* Schedule HSYNC off signal */
	timer_adjust_oneshot( mc6845->hsync_off_timer, ticks_to_attotime( hsync_width, mc6845->clock ), 0 );
}


static TIMER_CALLBACK( hsync_off_timer_cb )
{
	device_t *device = (device_t *)ptr;
	mc6845_t *mc6845 = get_safe_token(device);

	mc6845_set_hsync( mc6845, FALSE );
}


static TIMER_CALLBACK( line_timer_cb )
{
	device_t *device = (device_t *)ptr;
	mc6845_t *mc6845 = get_safe_token(device);
	int new_vsync = mc6845->vsync;

	mc6845->character_counter = 0;
	mc6845->cursor_x = -1;

	/* Check if VSYNC is active */
	if ( mc6845->vsync_ff )
	{
		UINT8 vsync_width = supports_vert_sync_width[mc6845->device_type] ? (mc6845->sync_width >> 4) & 0x0f : 0;

		mc6845->vsync_width_counter = ( mc6845->vsync_width_counter + 1 ) & 0x0F;

		/* Check if we've reached end of VSYNC */
		if ( mc6845->vsync_width_counter == vsync_width )
		{
			mc6845->vsync_ff = 0;

			new_vsync = FALSE;
		}
	}

	if ( mc6845->raster_counter == mc6845->max_ras_addr )
	{
		/* Check if we have reached the end of the vertical area */
		if ( mc6845->line_counter == mc6845->vert_char_total )
		{
			mc6845->adjust_counter = 0;
			mc6845->adjust_active = 1;
		}

		mc6845->raster_counter = 0;
		mc6845->line_counter = ( mc6845->line_counter + 1 ) & 0x7F;
		mc6845->line_address = ( mc6845->line_address + mc6845->horiz_disp ) & 0x3fff;

		/* Check if we've reached the end of active display */
		if ( mc6845->line_counter == mc6845->vert_disp )
		{
			mc6845->line_enable_ff = 0;
		}

		/* Check if VSYNC should be enabled */
		if ( mc6845->line_counter == mc6845->vert_sync_pos )
		{
			mc6845->vsync_width_counter = 0;
			mc6845->vsync_ff = 1;

			new_vsync = TRUE;
		}
	}
	else
	{
		mc6845->raster_counter = ( mc6845->raster_counter + 1 ) & 0x1F;
	}

	if ( mc6845->adjust_active )
	{
		/* Check if we have reached the end of a full cycle */
		if ( mc6845->adjust_counter == mc6845->vert_total_adj )
		{
			mc6845->adjust_active = 0;
			mc6845->raster_counter = 0;
			mc6845->line_counter = 0;
			mc6845->line_address = mc6845->disp_start_addr;
			mc6845->line_enable_ff = 1;
			/* also update the cursor state now */
			update_cursor_state(mc6845);

			if (mc6845->screen != NULL)
				mc6845->screen->reset_origin();
		}
		else
		{
			mc6845->adjust_counter = ( mc6845->adjust_counter + 1 ) & 0x1F;
		}
	}

	if ( mc6845->line_enable_ff )
	{
		/* Schedule DE off signal change */
		timer_adjust_oneshot(mc6845->de_off_timer, ticks_to_attotime( mc6845->horiz_disp, mc6845->clock ), 0);

		/* Is cursor visible on this line? */
		if ( mc6845->cursor_state &&
			(mc6845->raster_counter >= (mc6845->cursor_start_ras & 0x1f)) &&
			(mc6845->raster_counter <= mc6845->cursor_end_ras) &&
			(mc6845->cursor_addr >= mc6845->line_address) &&
			(mc6845->cursor_addr < (mc6845->line_address + mc6845->horiz_disp)) )
		{
			mc6845->cursor_x = mc6845->cursor_addr - mc6845->line_address;

			/* Schedule CURSOR ON signal */
			timer_adjust_oneshot( mc6845->cur_on_timer, ticks_to_attotime( mc6845->cursor_x, mc6845->clock ), 0 );
		}
	}

	/* Schedule HSYNC on signal */
	timer_adjust_oneshot( mc6845->hsync_on_timer, ticks_to_attotime( mc6845->horiz_sync_pos, mc6845->clock ), 0 );

	/* Schedule our next callback */
	timer_adjust_oneshot( mc6845->line_timer, ticks_to_attotime( mc6845->horiz_char_total + 1, mc6845->clock ), 0 );

	/* Set VSYNC and DE signals */
	mc6845_set_vsync( mc6845, new_vsync );
	mc6845_set_de( mc6845, mc6845->line_enable_ff ? TRUE : FALSE );
}


UINT16 mc6845_get_ma(device_t *device)
{
	mc6845_t *mc6845 = get_safe_token(device);

	mc6845_update_counters( mc6845 );

	return ( mc6845->line_address + mc6845->character_counter ) & 0x3fff;
}


UINT8 mc6845_get_ra(device_t *device)
{
	mc6845_t *mc6845 = get_safe_token(device);

	return mc6845->raster_counter;
}


static TIMER_CALLBACK( light_pen_latch_timer_cb )
{
	device_t *device = (device_t *)ptr;
	mc6845_t *mc6845 = get_safe_token(device);

	mc6845->light_pen_addr = mc6845_get_ma(device);
	mc6845->light_pen_latched = TRUE;
}


void mc6845_assert_light_pen_input(device_t *device)
{
	mc6845_t *mc6845 = get_safe_token(device);

	/* compute the pixel coordinate of the NEXT character -- this is when the light pen latches */
	/* set the timer that will latch the display address into the light pen registers */
	timer_adjust_oneshot(mc6845->light_pen_latch_timer, ticks_to_attotime( 1, mc6845->clock ), 0);
}


void mc6845_set_clock(device_t *device, int clock)
{
	mc6845_t *mc6845 = get_safe_token(device);

	/* validate arguments */
	assert(clock > 0);

	if (clock != mc6845->clock)
	{
		mc6845->clock = clock;
		recompute_parameters(mc6845, TRUE);
	}
}


void mc6845_set_hpixels_per_column(device_t *device, int hpixels_per_column)
{
	mc6845_t *mc6845 = get_safe_token(device);

	/* validate arguments */
	assert(hpixels_per_column > 0);

	if (hpixels_per_column != mc6845->hpixels_per_column)
	{
		mc6845->hpixels_per_column = hpixels_per_column;
		recompute_parameters(mc6845, FALSE);
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


void mc6845_update(device_t *device, bitmap_t *bitmap, const rectangle *cliprect)
{
	mc6845_t *mc6845 = get_safe_token(device);
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
			param = mc6845->intf->begin_update(device, bitmap, cliprect);

		if (cliprect->min_y == 0)
		{
			/* read the start address at the beginning of the frame */
			mc6845->current_disp_addr = mc6845->disp_start_addr;
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
			mc6845->intf->update_row(device, bitmap, cliprect, mc6845->current_disp_addr, ra, y, mc6845->horiz_disp, cursor_x, param);

			/* update MA if the last raster address */
			if (ra == mc6845->max_ras_addr)
				mc6845->current_disp_addr = (mc6845->current_disp_addr + mc6845->horiz_disp) & 0x3fff;
		}

		/* call the tear down function if any */
		if (mc6845->intf->end_update != NULL)
			mc6845->intf->end_update(device, bitmap, cliprect, param);
	}
	else
		popmessage("Invalid MC6845 screen parameters - display disabled!!!");
}


/* device interface */
static void common_start(device_t *device, int device_type)
{
	mc6845_t *mc6845 = get_safe_token(device);

	/* validate arguments */
	assert(device != NULL);

	mc6845->intf = (const mc6845_interface *)device->baseconfig().static_config();
	mc6845->device_type = device_type;

	if (mc6845->intf != NULL)
	{
		assert(device->clock() > 0);
		assert(mc6845->intf->hpixels_per_column > 0);

		/* resolve callbacks */
		devcb_resolve_write_line(&mc6845->out_de_func, &mc6845->intf->out_de_func, device);
		devcb_resolve_write_line(&mc6845->out_cur_func, &mc6845->intf->out_cur_func, device);
		devcb_resolve_write_line(&mc6845->out_hsync_func, &mc6845->intf->out_hsync_func, device);
		devcb_resolve_write_line(&mc6845->out_vsync_func, &mc6845->intf->out_vsync_func, device);

		/* copy the initial parameters */
		mc6845->clock = device->clock();
		mc6845->hpixels_per_column = mc6845->intf->hpixels_per_column;

		/* get the screen device */
		if ( mc6845->intf->screen_tag != NULL )
		{
			mc6845->screen = downcast<screen_device *>(device->machine->device(mc6845->intf->screen_tag));
			assert(mc6845->screen != NULL);
		}
		else
			mc6845->screen = NULL;

		/* create the timers */
		mc6845->line_timer = timer_alloc(device->machine, line_timer_cb, (void *)device);

		mc6845->de_off_timer = timer_alloc(device->machine, de_off_timer_cb, (void *)device);
		mc6845->upd_adr_timer = timer_alloc(device->machine, upd_adr_timer_cb, (void *)device);

		mc6845->cur_on_timer = timer_alloc(device->machine, cur_on_timer_cb, (void *)device);
		mc6845->cur_off_timer = timer_alloc(device->machine, cur_off_timer_cb, (void *)device);

		mc6845->hsync_on_timer = timer_alloc(device->machine, hsync_on_timer_cb, (void *)device);
		mc6845->hsync_off_timer = timer_alloc(device->machine, hsync_off_timer_cb, (void *)device);
	}

	mc6845->light_pen_latch_timer = timer_alloc(device->machine, light_pen_latch_timer_cb, (void *)device);

	/* Use some large startup values */
	mc6845->horiz_char_total = 0xff;
	mc6845->max_ras_addr = 0x1f;
	mc6845->vert_char_total = 0x7f;

	/* register for state saving */
	state_save_register_postload(device->machine, mc6845_state_save_postload, mc6845);

	state_save_register_device_item(device, 0, mc6845->clock);
	state_save_register_device_item(device, 0, mc6845->hpixels_per_column);
	state_save_register_device_item(device, 0, mc6845->register_address_latch);
	state_save_register_device_item(device, 0, mc6845->horiz_char_total);
	state_save_register_device_item(device, 0, mc6845->horiz_disp);
	state_save_register_device_item(device, 0, mc6845->horiz_sync_pos);
	state_save_register_device_item(device, 0, mc6845->sync_width);
	state_save_register_device_item(device, 0, mc6845->vert_char_total);
	state_save_register_device_item(device, 0, mc6845->vert_total_adj);
	state_save_register_device_item(device, 0, mc6845->vert_disp);
	state_save_register_device_item(device, 0, mc6845->vert_sync_pos);
	state_save_register_device_item(device, 0, mc6845->mode_control);
	state_save_register_device_item(device, 0, mc6845->max_ras_addr);
	state_save_register_device_item(device, 0, mc6845->cursor_start_ras);
	state_save_register_device_item(device, 0, mc6845->cursor_end_ras);
	state_save_register_device_item(device, 0, mc6845->disp_start_addr);
	state_save_register_device_item(device, 0, mc6845->cursor_addr);
	state_save_register_device_item(device, 0, mc6845->light_pen_addr);
	state_save_register_device_item(device, 0, mc6845->light_pen_latched);
	state_save_register_device_item(device, 0, mc6845->cursor_state);
	state_save_register_device_item(device, 0, mc6845->cursor_blink_count);
	state_save_register_device_item(device, 0, mc6845->update_addr);
	state_save_register_device_item(device, 0, mc6845->update_ready_bit);
	state_save_register_device_item(device, 0, mc6845->cur);
	state_save_register_device_item(device, 0, mc6845->hsync);
	state_save_register_device_item(device, 0, mc6845->vsync);
	state_save_register_device_item(device, 0, mc6845->de);
	state_save_register_device_item(device, 0, mc6845->character_counter);
	state_save_register_device_item(device, 0, mc6845->hsync_width_counter);
	state_save_register_device_item(device, 0, mc6845->line_counter);
	state_save_register_device_item(device, 0, mc6845->raster_counter);
	state_save_register_device_item(device, 0, mc6845->adjust_counter);
	state_save_register_device_item(device, 0, mc6845->vsync_width_counter);
	state_save_register_device_item(device, 0, mc6845->line_enable_ff);
	state_save_register_device_item(device, 0, mc6845->vsync_ff);
	state_save_register_device_item(device, 0, mc6845->adjust_active);
	state_save_register_device_item(device, 0, mc6845->line_address);
	state_save_register_device_item(device, 0, mc6845->cursor_x);
}

static DEVICE_START( mc6845 )
{
	common_start(device, TYPE_MC6845);
}

static DEVICE_START( mc6845_1 )
{
	common_start(device, TYPE_MC6845_1);
}

static DEVICE_START( c6545_1 )
{
	common_start(device, TYPE_C6545_1);
}

static DEVICE_START( r6545_1 )
{
	common_start(device, TYPE_R6545_1);
}

static DEVICE_START( h46505 )
{
	common_start(device, TYPE_H46505);
}

static DEVICE_START( hd6845 )
{
	common_start(device, TYPE_HD6845);
}

static DEVICE_START( sy6545_1 )
{
	common_start(device, TYPE_SY6545_1);
}

static DEVICE_START( sy6845e )
{
	common_start(device, TYPE_SY6845E);
}


static DEVICE_RESET( mc6845 )
{
	mc6845_t *mc6845 = get_safe_token(device);

	/* internal registers other than status remain unchanged, all outputs go low */
	if (mc6845->intf != NULL)
	{
		if (mc6845->out_de_func.target != NULL)
			devcb_call_write_line(&mc6845->out_de_func, FALSE);

		if (mc6845->out_hsync_func.target != NULL)
			devcb_call_write_line(&mc6845->out_hsync_func, FALSE);

		if (mc6845->out_vsync_func.target != NULL)
			devcb_call_write_line(&mc6845->out_vsync_func, FALSE);
	}

	if ( ! timer_enabled( mc6845->line_timer ) )
	{
		timer_adjust_oneshot( mc6845->line_timer, ticks_to_attotime( mc6845->horiz_char_total + 1, mc6845->clock ), 0 );
	}

	mc6845->light_pen_latched = FALSE;
}

#if 0
static DEVICE_VALIDITY_CHECK( mc6845 )
{
	int error = FALSE;
	const mc6845_interface *intf = (const mc6845_interface *) device->static_config();

	if (intf != NULL && intf->screen_tag != NULL)
	{
		if (device->clock() <= 0)
		{
			mame_printf_error("%s: %s has an mc6845 with an invalid clock\n", driver->source_file, driver->name);
			error = TRUE;
		}

		if (intf->hpixels_per_column <= 0)
		{
			mame_printf_error("%s: %s has an mc6845 with an invalid hpixels_per_column\n", driver->source_file, driver->name);
			error = TRUE;
		}
	}

	return error;
}
#endif

DEVICE_GET_INFO( mc6845 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(mc6845_t);					break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;								break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(mc6845);	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(mc6845);	break;
//      case DEVINFO_FCT_VALIDITY_CHECK:                info->validity_check = DEVICE_VALIDITY_CHECK_NAME(mc6845); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Motorola 6845");			break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "MC6845 CRTC");				break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.61");					break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEVICE_GET_INFO( mc6845_1 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Motorola 6845-1");			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(mc6845_1);	break;

		default:										DEVICE_GET_INFO_CALL(mc6845);				break;
	}
}


DEVICE_GET_INFO( c6545_1 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "MOS Technology 6545-1");	break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(c6545_1);	break;

		default:										DEVICE_GET_INFO_CALL(mc6845);				break;
	}
}


DEVICE_GET_INFO( r6545_1 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Rockwell 6545-1");			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(r6545_1);	break;

		default:										DEVICE_GET_INFO_CALL(mc6845);				break;
	}
}


DEVICE_GET_INFO( h46505 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Hitachi 46505");			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(h46505);	break;

		default:										DEVICE_GET_INFO_CALL(mc6845);				break;
	}
}


DEVICE_GET_INFO( hd6845 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Hitachi 6845");			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(hd6845);	break;

		default:										DEVICE_GET_INFO_CALL(mc6845);				break;
	}
}


DEVICE_GET_INFO( sy6545_1 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Synertek 6545-1");			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(sy6545_1);	break;

		default:										DEVICE_GET_INFO_CALL(mc6845);				break;
	}
}


DEVICE_GET_INFO( sy6845e )
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Synertek 6845E");			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(sy6845e);	break;

		default:										DEVICE_GET_INFO_CALL(mc6845);				break;
	}
}

DEFINE_LEGACY_DEVICE(MC6845, mc6845);
DEFINE_LEGACY_DEVICE(MC6845_1, mc6845_1);
DEFINE_LEGACY_DEVICE(R6545_1, r6545_1);
DEFINE_LEGACY_DEVICE(C6545_1, c6545_1);
DEFINE_LEGACY_DEVICE(H46505, h46505);
DEFINE_LEGACY_DEVICE(HD6845, hd6845);
DEFINE_LEGACY_DEVICE(SY6545_1, sy6545_1);
DEFINE_LEGACY_DEVICE(SY6845E, sy6845e);
