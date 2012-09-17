/***************************************************************************

  cgenie.c

  Functions to emulate the video controller 6845.

***************************************************************************/

#include "emu.h"
#include "includes/cgenie.h"





/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
void cgenie_state::video_start()
{
	screen_device *screen = machine().first_screen();

	screen->register_screen_bitmap(m_dlybitmap);
	screen->register_screen_bitmap(m_bitmap);
}

/***************************************************************************

  Calculate the horizontal and vertical offset for the
  current register settings of the 6845 CRTC

***************************************************************************/
static void cgenie_offset_xy(cgenie_state *state)
{
	if( state->m_crt.horizontal_sync_pos )
		state->m_off_x = state->m_crt.horizontal_total - state->m_crt.horizontal_sync_pos - 14;
	else
		state->m_off_x = -15;

	state->m_off_y = (state->m_crt.vertical_total - state->m_crt.vertical_sync_pos) *
		(state->m_crt.scan_lines + 1) + state->m_crt.vertical_adjust
		- 32;

	if( state->m_off_y < 0 )
		state->m_off_y = 0;

	if( state->m_off_y > 128 )
		state->m_off_y = 128;

// logerror("cgenie offset x:%d  y:%d\n", state->m_off_x, state->m_off_y);
}


/***************************************************************************
  Write to an indexed register of the 6845 CRTC
***************************************************************************/
WRITE8_HANDLER ( cgenie_register_w )
{
	cgenie_state *state = space.machine().driver_data<cgenie_state>();
	//int addr;

	switch (state->m_crt.idx)
	{
		case 0:
			if( state->m_crt.horizontal_total == data )
				break;
			state->m_crt.horizontal_total = data;
			cgenie_offset_xy(state);
			break;
		case 1:
			if( state->m_crt.horizontal_displayed == data )
				break;
			state->m_crt.horizontal_displayed = data;
			break;
		case 2:
			if( state->m_crt.horizontal_sync_pos == data )
				break;
			state->m_crt.horizontal_sync_pos = data;
			cgenie_offset_xy(state);
			break;
		case 3:
			state->m_crt.horizontal_length = data;
			break;
		case 4:
			if( state->m_crt.vertical_total == data )
				break;
			state->m_crt.vertical_total = data;
			cgenie_offset_xy(state);
			break;
		case 5:
			if( state->m_crt.vertical_adjust == data )
				break;
			state->m_crt.vertical_adjust = data;
			cgenie_offset_xy(state);
			break;
		case 6:
			if( state->m_crt.vertical_displayed == data )
				break;
			state->m_crt.vertical_displayed = data;
			break;
		case 7:
			if( state->m_crt.vertical_sync_pos == data )
				break;
			state->m_crt.vertical_sync_pos = data;
			cgenie_offset_xy(state);
			break;
		case 8:
			state->m_crt.crt_mode = data;
			break;
		case 9:
			data &= 15;
			if( state->m_crt.scan_lines == data )
				break;
			state->m_crt.scan_lines = data;
			cgenie_offset_xy(state);
			break;
		case 10:
			if( state->m_crt.cursor_top == data )
				break;
			state->m_crt.cursor_top = data;
			//addr = 256 * state->m_crt.cursor_address_hi + state->m_crt.cursor_address_lo;
            break;
		case 11:
			if( state->m_crt.cursor_bottom == data )
				break;
			state->m_crt.cursor_bottom = data;
			//addr = 256 * state->m_crt.cursor_address_hi + state->m_crt.cursor_address_lo;
            break;
		case 12:
			data &= 63;
			if( state->m_crt.screen_address_hi == data )
				break;
			state->m_crt.screen_address_hi = data;
			break;
		case 13:
			if( state->m_crt.screen_address_lo == data )
				break;
			state->m_crt.screen_address_lo = data;
			break;
		case 14:
			data &= 63;
			if( state->m_crt.cursor_address_hi == data )
				break;
			state->m_crt.cursor_address_hi = data;
			//addr = 256 * state->m_crt.cursor_address_hi + state->m_crt.cursor_address_lo;
            break;
		case 15:
			if( state->m_crt.cursor_address_lo == data )
				break;
			state->m_crt.cursor_address_lo = data;
			//addr = 256 * state->m_crt.cursor_address_hi + state->m_crt.cursor_address_lo;
            break;
	}
}

/***************************************************************************
  Write to the index register of the 6845 CRTC
***************************************************************************/
WRITE8_HANDLER ( cgenie_index_w )
{
	cgenie_state *state = space.machine().driver_data<cgenie_state>();
	state->m_crt.idx = data & 15;
}

/***************************************************************************
  Read from an indexed register of the 6845 CRTC
***************************************************************************/
 READ8_HANDLER ( cgenie_register_r )
{
	cgenie_state *state = space.machine().driver_data<cgenie_state>();
	return cgenie_get_register(space.machine(), state->m_crt.idx);
}

/***************************************************************************
  Read from a register of the 6845 CRTC
***************************************************************************/
int cgenie_get_register(running_machine &machine, int indx)
{
	cgenie_state *state = machine.driver_data<cgenie_state>();
	switch (indx)
	{
		case 0:
			return state->m_crt.horizontal_total;
		case 1:
			return state->m_crt.horizontal_displayed;
		case 2:
			return state->m_crt.horizontal_sync_pos;
		case 3:
			return state->m_crt.horizontal_length;
		case 4:
			return state->m_crt.vertical_total;
		case 5:
			return state->m_crt.vertical_adjust;
		case 6:
			return state->m_crt.vertical_displayed;
		case 7:
			return state->m_crt.vertical_sync_pos;
		case 8:
			return state->m_crt.crt_mode;
		case 9:
			return state->m_crt.scan_lines;
		case 10:
			return state->m_crt.cursor_top;
		case 11:
			return state->m_crt.cursor_bottom;
		case 12:
			return state->m_crt.screen_address_hi;
		case 13:
			return state->m_crt.screen_address_lo;
		case 14:
			return state->m_crt.cursor_address_hi;
		case 15:
			return state->m_crt.cursor_address_lo;
	}
	return 0;
}

/***************************************************************************
  Read the index register of the 6845 CRTC
***************************************************************************/
 READ8_HANDLER ( cgenie_index_r )
{
	cgenie_state *state = space.machine().driver_data<cgenie_state>();
	return state->m_crt.idx;
}

/***************************************************************************
  Switch mode between character generator and graphics
***************************************************************************/
void cgenie_mode_select(running_machine &machine, int mode)
{
	cgenie_state *state = machine.driver_data<cgenie_state>();
	state->m_graphics = (mode) ? 1 : 0;
}


static void cgenie_refresh_monitor(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	cgenie_state *state = machine.driver_data<cgenie_state>();
	UINT8 *videoram = state->m_videoram;
	int i, address, offset, cursor, size, code, x, y;
    rectangle r;

	bitmap.fill(get_black_pen(machine), cliprect);

	if(state->m_crt.vertical_displayed || state->m_crt.horizontal_displayed)
	{
		offset = 256 * state->m_crt.screen_address_hi + state->m_crt.screen_address_lo;
		size = state->m_crt.horizontal_displayed * state->m_crt.vertical_displayed;
		cursor = 256 * state->m_crt.cursor_address_hi + state->m_crt.cursor_address_lo;

		/*
         * for every character in the Video RAM, check if it has been modified since
         * last time and update it accordingly.
         */
		for( address = 0; address < size; address++ )
		{
			i = (offset + address) & 0x3fff;
			x = address % state->m_crt.horizontal_displayed + state->m_off_x;
			y = address / state->m_crt.horizontal_displayed;

			r.min_x = x * 8;
			r.max_x = r.min_x + 7;
			r.min_y = y * (state->m_crt.scan_lines + 1) + state->m_off_y;
			r.max_y = r.min_y + state->m_crt.scan_lines;

			if( state->m_graphics )
			{
				/* get graphics code */
				code = videoram[i];
				drawgfx_opaque(bitmap, r, machine.gfx[1], code, 0,
					0, 0, r.min_x, r.min_y);
			}
			else
			{
				/* get character code */
				code = videoram[i];

				/* translate defined character sets */
				code += state->m_font_offset[(code >> 6) & 3];
				drawgfx_opaque(bitmap, r, machine.gfx[0], code, state->m_colorram[i&0x3ff],
					0, 0, r.min_x, r.min_y);
			}

			if( i == cursor )
			{
			rectangle rc;

			/* check if cursor turned off */
				if( (state->m_crt.cursor_top & 0x60) == 0x20 )
					continue;

				if( (state->m_crt.cursor_top & 0x60) == 0x60 )
				{
					state->m_crt.cursor_visible = 1;
				}
				else
				{
					state->m_crt.cursor_phase++;
					state->m_crt.cursor_visible = (state->m_crt.cursor_phase >> 3) & 1;
				}

				if( !state->m_crt.cursor_visible )
					continue;

				rc.min_x = r.min_x;
				rc.max_x = r.max_x;
				rc.min_y = r.min_y + (state->m_crt.cursor_top & 15);
				rc.max_y = r.min_y + (state->m_crt.cursor_bottom & 15);
				drawgfx_opaque(bitmap, rc, machine.gfx[0], 0x7f, state->m_colorram[i&0x3ff],
					0, 0, rc.min_x, rc.min_y);
			}
		}
	}
}

static void cgenie_refresh_tv_set(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	cgenie_state *state = machine.driver_data<cgenie_state>();
	UINT8 *videoram = state->m_videoram;
	int i, address, offset, cursor, size, code, x, y;
	rectangle r;

	state->m_bitmap.fill(get_black_pen(machine), cliprect);
	state->m_dlybitmap.fill(get_black_pen(machine), cliprect);

	if(state->m_crt.vertical_displayed || state->m_crt.horizontal_displayed)
	{
		offset = 256 * state->m_crt.screen_address_hi + state->m_crt.screen_address_lo;
		size = state->m_crt.horizontal_displayed * state->m_crt.vertical_displayed;
		cursor = 256 * state->m_crt.cursor_address_hi + state->m_crt.cursor_address_lo;

		/*
         * for every character in the Video RAM, check if it has been modified since
         * last time and update it accordingly.
         */
		for( address = 0; address < size; address++ )
		{
			i = (offset + address) & 0x3fff;
			x = address % state->m_crt.horizontal_displayed + state->m_off_x;
			y = address / state->m_crt.horizontal_displayed;

			r.min_x = x * 8;
			r.max_x = r.min_x + 7;
			r.min_y = y * (state->m_crt.scan_lines + 1) + state->m_off_y;
			r.max_y = r.min_y + state->m_crt.scan_lines;

			if( state->m_graphics )
			{
				/* get graphics code */
				code = videoram[i];
				drawgfx_opaque(state->m_bitmap, r, machine.gfx[1], code, 1,
					0, 0, r.min_x, r.min_y);
				drawgfx_opaque(state->m_dlybitmap, r, machine.gfx[1], code, 2,
					0, 0, r.min_x, r.min_y);
			}
			else
			{
				/* get character code */
				code = videoram[i];

				/* translate defined character sets */
				code += state->m_font_offset[(code >> 6) & 3];
				drawgfx_opaque(state->m_bitmap, r, machine.gfx[0], code, state->m_colorram[i&0x3ff] + 16,
					0, 0, r.min_x, r.min_y);
				drawgfx_opaque(state->m_dlybitmap, r, machine.gfx[0], code, state->m_colorram[i&0x3ff] + 32,
					0, 0, r.min_x, r.min_y);
			}

			if( i == cursor )
			{
				rectangle rc;

				/* check if cursor turned off */
				if( (state->m_crt.cursor_top & 0x60) == 0x20 )
					continue;

				if( (state->m_crt.cursor_top & 0x60) == 0x60 )
				{
					state->m_crt.cursor_visible = 1;
				}
				else
				{
					state->m_crt.cursor_phase++;
					state->m_crt.cursor_visible = (state->m_crt.cursor_phase >> 3) & 1;
				}

				if( !state->m_crt.cursor_visible )
					continue;

				rc.min_x = r.min_x;
				rc.max_x = r.max_x;
				rc.min_y = r.min_y + (state->m_crt.cursor_top & 15);
				rc.max_y = r.min_y + (state->m_crt.cursor_bottom & 15);

				drawgfx_opaque(state->m_bitmap, rc, machine.gfx[0], 0x7f, state->m_colorram[i&0x3ff] + 16,
					0, 0, rc.min_x, rc.min_y);
				drawgfx_opaque(state->m_dlybitmap, rc, machine.gfx[0], 0x7f, state->m_colorram[i&0x3ff] + 32,
					0, 0, rc.min_x, rc.min_y);
			}
		}
	}

	copybitmap(bitmap, state->m_bitmap, 0, 0, 0, 0, cliprect);
	copybitmap_trans(bitmap, state->m_dlybitmap, 0, 0, 1, 0, cliprect, 0);
}

/***************************************************************************
  Draw the game screen in the given bitmap_ind16.
***************************************************************************/
SCREEN_UPDATE_IND16( cgenie )
{
	cgenie_state *state = screen.machine().driver_data<cgenie_state>();
    if( state->m_tv_mode )
		cgenie_refresh_tv_set(screen.machine(), bitmap, cliprect);
	else
		cgenie_refresh_monitor(screen.machine(), bitmap, cliprect);
	return 0;
}
