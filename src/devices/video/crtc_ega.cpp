// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    IBM EGA CRT Controller emulation

    This controller is very loosely based on the mc6845.

**********************************************************************/

#include "emu.h"
#include "crtc_ega.h"


#define LOG     (1)


const device_type CRTC_EGA = &device_creator<crtc_ega_device>;


crtc_ega_device::crtc_ega_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CRTC_EGA, "crtc_EGA", tag, owner, clock, "crtc_ega", __FILE__),
		device_video_interface(mconfig, *this, false),
		m_res_out_de_cb(*this),
		m_res_out_hsync_cb(*this),
		m_res_out_vsync_cb(*this),
		m_res_out_vblank_cb(*this), m_horiz_char_total(0), m_horiz_disp(0), m_horiz_blank_start(0), m_horiz_blank_end(0), m_ena_vert_access(0), m_de_skew(0), m_horiz_retr_start(0), m_horiz_retr_end(0),
	m_horiz_retr_skew(0), m_vert_total(0), m_preset_row_scan(0), m_byte_panning(0), m_max_ras_addr(0), m_scan_doubling(0), m_cursor_start_ras(0), m_cursor_disable(0), m_cursor_end_ras(0), m_cursor_skew(0),
	m_disp_start_addr(0), m_cursor_addr(0), m_light_pen_addr(0), m_vert_retr_start(0), m_vert_retr_end(0), m_protect(0), m_bandwidth(0), m_vert_disp_end(0), m_offset(0), m_underline_loc(0), m_vert_blank_start(0),
	m_vert_blank_end(0), m_mode_control(0), m_line_compare(0), m_register_address_latch(0), m_cursor_state(false), m_cursor_blink_count(0),
		m_hpixels_per_column(0), m_cur(0), m_hsync(0), m_vsync(0), m_vblank(0), m_de(0), m_character_counter(0), m_hsync_width_counter(0), m_line_counter(0), m_raster_counter(0), m_vsync_width_counter(0),
	m_line_enable_ff(false), m_vsync_ff(0), m_adjust_active(0), m_line_address(0), m_cursor_x(0), m_line_timer(nullptr), m_de_off_timer(nullptr), m_cur_on_timer(nullptr), m_cur_off_timer(nullptr),
	m_hsync_on_timer(nullptr), m_hsync_off_timer(nullptr), m_light_pen_latch_timer(nullptr), m_horiz_pix_total(0), m_vert_pix_total(0), m_max_visible_x(0), m_max_visible_y(0), m_hsync_on_pos(0),
	m_hsync_off_pos(0), m_vsync_on_pos(0), m_vsync_off_pos(0), m_current_disp_addr(0), m_light_pen_latched(0), m_has_valid_parameters(false)
{
}


void crtc_ega_device::device_post_load()
{
	recompute_parameters(true);
}


WRITE8_MEMBER( crtc_ega_device::address_w )
{
	m_register_address_latch = data & 0x1f;
}


READ8_MEMBER( crtc_ega_device::register_r )
{
	UINT8 ret = 0;

	switch (m_register_address_latch)
	{
		case 0x0c:  ret = (m_disp_start_addr >> 8) & 0xff; break;
		case 0x0d:  ret = (m_disp_start_addr >> 0) & 0xff; break;
		case 0x0e:  ret = (m_cursor_addr    >> 8) & 0xff; break;
		case 0x0f:  ret = (m_cursor_addr    >> 0) & 0xff; break;
		case 0x10:  ret = (m_light_pen_addr >> 8) & 0xff; m_light_pen_latched = FALSE; break;
		case 0x11:  ret = (m_light_pen_addr >> 0) & 0xff; m_light_pen_latched = FALSE; break;

		/* all other registers are write only and return 0 */
		default: break;
	}

	return ret;
}


WRITE8_MEMBER( crtc_ega_device::register_w )
{
	if (LOG)  logerror("%s CRTC_EGA: reg 0x%02x = 0x%02x\n", machine().describe_context(), m_register_address_latch, data);

	switch (m_register_address_latch)
	{
		case 0x00:  m_horiz_char_total  =   data & 0xff; break;
		case 0x01:  m_horiz_disp        =   data & 0xff; break;
		case 0x02:  m_horiz_blank_start =   data & 0xff; break;
		case 0x03:  m_horiz_blank_end   =  ((data & 0x1f) << 0) | (m_horiz_blank_end & 0x20);
					m_de_skew           =  ((data & 0x60) >> 5);
					m_ena_vert_access   =  data & 0x80;
					break;
		case 0x04:  m_horiz_retr_start  =   data & 0xff; break;
		case 0x05:  m_horiz_retr_end    =   data & 0x1f;
					m_horiz_retr_skew   = ((data & 0x60) >> 5);
					m_horiz_blank_end   = ((data & 0x80) >> 2) | (m_horiz_blank_end & 0x1f);
					break;
		case 0x06:  m_vert_total        = ((data & 0xff) << 0) | (m_vert_total & 0x0300); break;
		case 0x07:  m_vert_total        = ((data & 0x01) << 8) | (m_vert_total & 0x02ff);
					m_vert_disp_end     = ((data & 0x02) << 7) | (m_vert_disp_end & 0x02ff);
					m_vert_retr_start   = ((data & 0x04) << 6) | (m_vert_retr_start & 0x02ff);
					m_vert_blank_start  = ((data & 0x08) << 5) | (m_vert_blank_start & 0x02ff);
					m_line_compare      = ((data & 0x10) << 4) | (m_line_compare & 0x02ff);
					m_vert_total        = ((data & 0x20) << 4) | (m_vert_total & 0x01ff);
					m_vert_disp_end     = ((data & 0x40) << 3) | (m_vert_disp_end & 0x1ff);
					m_vert_retr_start   = ((data & 0x80) << 2) | (m_vert_retr_start & 0x01ff);
					break;
		case 0x08:  m_preset_row_scan   =   data & 0x1f;
					m_byte_panning      = ((data & 0x60) >> 5);
					break;
		case 0x09:  m_max_ras_addr      =   data & 0x1f;
					m_vert_blank_start  = ((data & 0x20) << 4) | (m_vert_blank_start & 0x01ff);
					m_line_compare      = ((data & 0x40) << 3) | (m_line_compare & 0x01ff);
					m_scan_doubling     =   data & 0x80;
					break;
		case 0x0a:  m_cursor_start_ras  =   data & 0x1f;
					m_cursor_disable    =   data & 0x20;
					break;
		case 0x0b:  m_cursor_end_ras    =   data & 0x1f;
					m_cursor_skew       = ((data & 0x60) >> 5);
					break;
		case 0x0c:  m_disp_start_addr   = ((data & 0xff) << 8) | (m_disp_start_addr & 0x00ff); break;
		case 0x0d:  m_disp_start_addr   = ((data & 0xff) << 0) | (m_disp_start_addr & 0xff00); break;
		case 0x0e:  m_cursor_addr       = ((data & 0xff) << 8) | (m_cursor_addr & 0x00ff); break;
		case 0x0f:  m_cursor_addr       = ((data & 0xff) << 0) | (m_cursor_addr & 0xff00); break;
		case 0x10:  m_vert_retr_start   = ((data & 0xff) << 0) | (m_vert_retr_start & 0x0300); break;
		case 0x11:  m_vert_retr_end     =   data & 0x0f;
					m_bandwidth         =   data & 0x40;
					m_protect           =   data & 0x80;
					break;
		case 0x12:  m_vert_disp_end     = ((data & 0xff) << 0) | (m_vert_disp_end & 0x0300); break;
		case 0x13:  m_offset            =  data & 0xff; break;
		case 0x14:  m_underline_loc     =  data & 0x7f; break;
		case 0x15:  m_vert_blank_start  = ((data & 0xff) << 0) | (m_vert_blank_start & 0x0300); break;
		case 0x16:  m_vert_blank_end    =   data & 0x7f; break;
		case 0x17:  m_mode_control      =   data & 0xff; break;
		case 0x18:  m_line_compare      = ((data & 0xff) << 0) | (m_line_compare & 0x0300); break;
		default:    break;
	}

	recompute_parameters(false);
}


void crtc_ega_device::recompute_parameters(bool postload)
{
	UINT16 hsync_on_pos, hsync_off_pos, vsync_on_pos, vsync_off_pos;

	/* compute the screen sizes */
	UINT16 horiz_pix_total = (m_horiz_char_total + 2) * m_hpixels_per_column;
	UINT16 vert_pix_total = m_vert_total + 1;

	/* determine the visible area, avoid division by 0 */
	UINT16 max_visible_x = ( m_horiz_disp + 1 ) * m_hpixels_per_column - 1;
	UINT16 max_visible_y = m_vert_disp_end;

	/* determine the syncing positions */
	int horiz_sync_char_width = ( m_horiz_retr_end + 1 ) - ( m_horiz_retr_start & 0x1f );
	int vert_sync_pix_width = m_vert_retr_end - ( m_vert_retr_start & 0x0f );

	if (horiz_sync_char_width <= 0)
		horiz_sync_char_width += 0x10;

	if (vert_sync_pix_width <= 0)
		vert_sync_pix_width += 0x10;

	hsync_on_pos = m_horiz_retr_start * m_hpixels_per_column;
	hsync_off_pos = hsync_on_pos + (horiz_sync_char_width * m_hpixels_per_column);
	vsync_on_pos = m_vert_retr_start;       /* + 1 ?? */
	vsync_off_pos = vsync_on_pos + vert_sync_pix_width;

	if (hsync_off_pos > horiz_pix_total)
		hsync_off_pos = horiz_pix_total;

	if (vsync_off_pos > vert_pix_total)
		vsync_off_pos = vert_pix_total;

	if ( vsync_on_pos >= vsync_off_pos )
	{
		vsync_on_pos = vsync_off_pos - 2;
	}

	/* update only if screen parameters changed, unless we are coming here after loading the saved state */
	if (postload ||
		(horiz_pix_total != m_horiz_pix_total) || (vert_pix_total != m_vert_pix_total) ||
		(max_visible_x != m_max_visible_x) || (max_visible_y != m_max_visible_y) ||
		(hsync_on_pos != m_hsync_on_pos) || (vsync_on_pos != m_vsync_on_pos) ||
		(hsync_off_pos != m_hsync_off_pos) || (vsync_off_pos != m_vsync_off_pos))
	{
		/* update the screen if we have valid data */
		if ((horiz_pix_total > 0) && (max_visible_x < horiz_pix_total) &&
			(vert_pix_total > 0) && (max_visible_y < vert_pix_total) &&
			(hsync_on_pos <= horiz_pix_total) && (vsync_on_pos <= vert_pix_total) &&
			(hsync_on_pos != hsync_off_pos))
		{
			attoseconds_t refresh = HZ_TO_ATTOSECONDS(m_clock) * (m_horiz_char_total + 2) * vert_pix_total;

			rectangle visarea(0, max_visible_x, 0, max_visible_y);

			if (LOG) logerror("CRTC_EGA config screen: HTOTAL: 0x%x  VTOTAL: 0x%x  MAX_X: 0x%x  MAX_Y: 0x%x  HSYNC: 0x%x-0x%x  VSYNC: 0x%x-0x%x  Freq: %ffps\n",
								horiz_pix_total, vert_pix_total, max_visible_x, max_visible_y, hsync_on_pos, hsync_off_pos - 1, vsync_on_pos, vsync_off_pos - 1, 1 / ATTOSECONDS_TO_DOUBLE(refresh));

			if ( m_screen != nullptr )
				m_screen->configure(horiz_pix_total, vert_pix_total, visarea, refresh);

			m_has_valid_parameters = true;
		}
		else
		{
			m_has_valid_parameters = false;
			if (LOG) logerror("CRTC_EGA bad config screen: HTOTAL: 0x%x  VTOTAL: 0x%x  MAX_X: 0x%x  MAX_Y: 0x%x  HSYNC: 0x%x-0x%x  VSYNC: 0x%x-0x%x\n",
								horiz_pix_total, vert_pix_total, max_visible_x, max_visible_y, hsync_on_pos, hsync_off_pos - 1, vsync_on_pos, vsync_off_pos - 1);

		}

		m_horiz_pix_total = horiz_pix_total;
		m_vert_pix_total = vert_pix_total;
		m_max_visible_x = max_visible_x;
		m_max_visible_y = max_visible_y;
		m_hsync_on_pos = hsync_on_pos;
		m_hsync_off_pos = hsync_off_pos;
		m_vsync_on_pos = vsync_on_pos;
		m_vsync_off_pos = vsync_off_pos;
	}
}


void crtc_ega_device::update_counters()
{
	m_character_counter = m_line_timer->elapsed().as_ticks( m_clock );

	if ( m_hsync_off_timer->enabled() )
	{
		m_hsync_width_counter = m_hsync_off_timer->elapsed().as_ticks( m_clock );
	}
}


void crtc_ega_device::set_de(int state)
{
	if (m_de != state)
	{
		m_de = state;

		if (!m_res_out_de_cb.isnull())
			m_res_out_de_cb(m_de);
	}
}


void crtc_ega_device::set_hsync(int state)
{
	if (m_hsync != state)
	{
		m_hsync = state;

		if (!m_res_out_hsync_cb.isnull())
			m_res_out_hsync_cb(m_hsync);
	}
}


void crtc_ega_device::set_vsync(int state)
{
	if (m_vsync != state)
	{
		m_vsync = state;

		if (!m_res_out_vsync_cb.isnull())
			m_res_out_vsync_cb(m_vsync);
	}
}


void crtc_ega_device::set_vblank(int state)
{
	if (m_vblank != state)
	{
		m_vblank = state;

		if (!m_res_out_vblank_cb.isnull())
			m_res_out_vblank_cb(m_vblank);
	}
}


void crtc_ega_device::set_cur(int state)
{
	if (m_cur != state)
	{
		m_cur = state;

//      if (!m_res_out_cur_cb.isnull())
//          m_res_out_cur_cb(m_cur);
	}
}


void crtc_ega_device::handle_line_timer()
{
	int new_vsync = m_vsync;

	m_character_counter = 0;
	m_cursor_x = -1;

	/* Check if VSYNC is active */
	if ( m_vsync_ff )
	{
		m_vsync_width_counter = ( m_vsync_width_counter + 1 ) & 0x0F;

		/* Check if we've reached end of VSYNC */
		if ( m_vsync_width_counter == m_vert_retr_end )
		{
			m_vsync_ff = 0;

			new_vsync = FALSE;
		}
	}

	if ( m_raster_counter == m_max_ras_addr )
	{
		m_raster_counter = 0;
		m_line_address = ( m_line_address + m_horiz_disp + 1 ) & 0xffff;
	}
	else
	{
		m_raster_counter = ( m_raster_counter + 1 ) & 0x1F;
	}

	m_line_counter = ( m_line_counter + 1 ) & 0x3ff;

	/* Check if we've reached the end of active display */
	if ( m_line_counter == m_vert_disp_end )
	{
		m_line_enable_ff = false;
	}

	/* Check if VSYNC should be enabled */
	if ( m_line_counter == m_vert_retr_start )
	{
		m_vsync_width_counter = 0;
		m_vsync_ff = 1;

		new_vsync = TRUE;
	}

	/* Check if we have reached the end of the vertical area */
	if ( m_line_counter == m_vert_total )
	{
		m_line_counter = 0;
		m_line_address = m_disp_start_addr;
		m_line_enable_ff = true;
		set_vblank( FALSE );
		/* also update the cursor state now */
		update_cursor_state();

		if (m_screen != nullptr)
			m_screen->reset_origin();
	}

	if ( m_line_enable_ff )
	{
		/* Schedule DE off signal change */
		m_de_off_timer->adjust(attotime::from_ticks( m_horiz_disp + 1, m_clock ));

		/* Is cursor visible on this line? */
		if ( m_cursor_state &&
			(m_raster_counter >= (m_cursor_start_ras & 0x1f)) &&
			(m_raster_counter <= m_cursor_end_ras) &&
			(m_cursor_addr >= m_line_address) &&
			(m_cursor_addr < (m_line_address + m_horiz_disp + 1)) )
		{
			m_cursor_x = m_cursor_addr - m_line_address;

			/* Schedule CURSOR ON signal */
			m_cur_on_timer->adjust( attotime::from_ticks( m_cursor_x, m_clock ) );
		}
	}

	/* Schedule HSYNC on signal */
	m_hsync_on_timer->adjust( attotime::from_ticks( m_horiz_blank_start, m_clock ) );

	/* Set VBlank signal */
	if ( m_line_counter == m_vert_disp_end + 1 )
	{
		set_vblank( TRUE );
	}

	/* Schedule our next callback */
	m_line_timer->adjust( attotime::from_ticks( m_horiz_char_total + 2, m_clock ) );

	/* Set VSYNC and DE signals */
	set_vsync( new_vsync );
	set_de( m_line_enable_ff ? TRUE : FALSE );
}


void crtc_ega_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_LINE:
		handle_line_timer();
		break;

	case TIMER_DE_OFF:
		set_de( FALSE );
		break;

	case TIMER_CUR_ON:
		set_cur( TRUE );

		/* Schedule CURSOR off signal */
		m_cur_off_timer->adjust( attotime::from_ticks( 1, m_clock ) );
		break;

	case TIMER_CUR_OFF:
		set_cur( FALSE );
		break;

	case TIMER_HSYNC_ON:
		{
			INT8 hsync_width = ( 0x20 | m_horiz_blank_end ) - ( m_horiz_blank_start & 0x1f );

			if ( hsync_width <= 0 )
			{
				hsync_width += 0x20;
			}

			m_hsync_width_counter = 0;
			set_hsync( TRUE );

			/* Schedule HSYNC off signal */
			m_hsync_off_timer->adjust( attotime::from_ticks( hsync_width, m_clock ) );
		}
		break;

	case TIMER_HSYNC_OFF:
		set_hsync( FALSE );
		break;

	case TIMER_LIGHT_PEN_LATCH:
		m_light_pen_addr = get_ma();
		m_light_pen_latched = true;
		break;
	}
}


UINT16 crtc_ega_device::get_ma()
{
	update_counters();

	return m_line_address + m_character_counter;
}


UINT8 crtc_ega_device::get_ra()
{
	return m_raster_counter;
}


void crtc_ega_device::assert_light_pen_input()
{
	/* compute the pixel coordinate of the NEXT character -- this is when the light pen latches */
	/* set the timer that will latch the display address into the light pen registers */
	m_light_pen_latch_timer->adjust(attotime::from_ticks( 1, m_clock ));
}


void crtc_ega_device::set_clock(int clock)
{
	/* validate arguments */
	assert(clock > 0);

	if (clock != m_clock)
	{
		m_clock = clock;
		recompute_parameters(true);
	}
}


void crtc_ega_device::set_hpixels_per_column(int hpixels_per_column)
{
	/* validate arguments */
	assert(hpixels_per_column > 0);

	if (hpixels_per_column != m_hpixels_per_column)
	{
		m_hpixels_per_column = hpixels_per_column;
		recompute_parameters(true);
	}
}


void crtc_ega_device::update_cursor_state()
{
	/* save and increment cursor counter */
	UINT8 last_cursor_blink_count = m_cursor_blink_count;
	m_cursor_blink_count = m_cursor_blink_count + 1;

	/* switch on cursor blinking mode */
	switch (m_cursor_start_ras & 0x60)
	{
		/* always on */
		case 0x00: m_cursor_state = true; break;

		/* always off */
		case 0x20: m_cursor_state = false; break;

		/* fast blink */
		case 0x40:
			if ((last_cursor_blink_count & 0x10) != (m_cursor_blink_count & 0x10))
			{
				m_cursor_state = !m_cursor_state;
			}
			break;

		/* slow blink */
		case 0x60:
			if ((last_cursor_blink_count & 0x20) != (m_cursor_blink_count & 0x20))
			{
				m_cursor_state = !m_cursor_state;
			}
			break;
	}
}


UINT32 crtc_ega_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	assert(bitmap.valid());

	if (m_has_valid_parameters)
	{
		UINT16 y;

		assert(!m_row_update_cb.isnull());

		/* call the set up function if any */
		if (!m_begin_update_cb.isnull())
			m_begin_update_cb(bitmap, cliprect);

		if (cliprect.min_y == 0)
		{
			/* read the start address at the beginning of the frame */
			m_current_disp_addr = m_disp_start_addr;
		}

		/* for each row in the visible region */
		for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			/* compute the current raster line */
			UINT8 ra = y % (m_max_ras_addr + 1);

			/* check if the cursor is visible and is on this scanline */
			int cursor_visible = m_cursor_state &&
								(ra >= (m_cursor_start_ras & 0x1f)) &&
								( (ra <= (m_cursor_end_ras & 0x1f)) || ((m_cursor_end_ras & 0x1f) == 0x00 )) &&
								(m_cursor_addr >= m_current_disp_addr) &&
								(m_cursor_addr < (m_current_disp_addr + ( m_horiz_disp + 1 )));

			/* compute the cursor X position, or -1 if not visible */
			INT8 cursor_x = cursor_visible ? (m_cursor_addr - m_current_disp_addr) : -1;

			/* call the external system to draw it */
			m_row_update_cb(bitmap, cliprect, m_current_disp_addr, ra, y, m_horiz_disp + 1, cursor_x);

			/* update MA if the last raster address */
			if (ra == m_max_ras_addr)
				m_current_disp_addr = (m_current_disp_addr + m_horiz_disp + 1) & 0xffff;
		}

		/* call the tear down function if any */
		if (!m_end_update_cb.isnull())
			m_end_update_cb(bitmap, cliprect);
	}
	else
		logerror("Invalid crtc_ega screen parameters - display disabled!!!\n");

	return 0;
}


/* device interface */
void crtc_ega_device::device_start()
{
	/* validate arguments */
	assert(m_clock > 0);
	assert(m_hpixels_per_column > 0);

	/* resolve callbacks */
	m_res_out_de_cb.resolve();
	m_res_out_hsync_cb.resolve();
	m_res_out_vsync_cb.resolve();
	m_res_out_vblank_cb.resolve();

	/* bind delegates */
	m_begin_update_cb.bind_relative_to(*owner());
	m_row_update_cb.bind_relative_to(*owner());
	m_end_update_cb.bind_relative_to(*owner());

	/* create the timers */
	m_line_timer = timer_alloc(TIMER_LINE);
	m_de_off_timer = timer_alloc(TIMER_DE_OFF);
	m_cur_on_timer = timer_alloc(TIMER_CUR_ON);
	m_cur_off_timer = timer_alloc(TIMER_CUR_OFF);
	m_hsync_on_timer = timer_alloc(TIMER_HSYNC_ON);
	m_hsync_off_timer = timer_alloc(TIMER_HSYNC_OFF);
	m_light_pen_latch_timer = timer_alloc(TIMER_LIGHT_PEN_LATCH);

	/* Use some large startup values */
	m_horiz_char_total = 0xff;
	m_max_ras_addr = 0x1f;
	m_vert_total = 0x3ff;

	m_ena_vert_access = 0;
	m_de_skew = 0;
	m_horiz_retr_start = 0;
	m_horiz_retr_end = 0;
	m_horiz_retr_skew = 0;
	m_preset_row_scan = 0;
	m_byte_panning = 0;
	m_scan_doubling = 0;
	m_cursor_start_ras = 0x20;
	m_cursor_disable = 0;
	m_cursor_end_ras = 0;
	m_cursor_skew = 0;
	m_disp_start_addr = 0;
	m_light_pen_addr = 0;
	m_vert_retr_end = 0;
	m_protect = 0;
	m_bandwidth = 0;
	m_offset = 0;
	m_underline_loc = 0;
	m_vert_blank_end = 0;
	m_mode_control = 0;
	m_line_compare = 0;
	m_register_address_latch = 0;
	m_cursor_state = false;
	m_cursor_blink_count = 0;
	m_cur = 0;
	m_hsync = 0;
	m_vsync = 0;
	m_vblank = 0;
	m_de = 0;
	m_character_counter = 0;
	m_hsync_width_counter = 0;
	m_vsync_width_counter = 0;
	m_line_enable_ff = false;
	m_vsync_ff = 0;
	m_adjust_active = 0;

	m_current_disp_addr = 0;
	m_light_pen_latched = FALSE;
	m_has_valid_parameters = false;

	/* register for state saving */
	save_item(NAME(m_hpixels_per_column));
	save_item(NAME(m_register_address_latch));
	save_item(NAME(m_horiz_char_total));
	save_item(NAME(m_horiz_disp));
	save_item(NAME(m_horiz_blank_start));
	save_item(NAME(m_mode_control));
	save_item(NAME(m_cursor_start_ras));
	save_item(NAME(m_cursor_end_ras));
	save_item(NAME(m_disp_start_addr));
	save_item(NAME(m_cursor_addr));
	save_item(NAME(m_light_pen_addr));
	save_item(NAME(m_light_pen_latched));
	save_item(NAME(m_cursor_state));
	save_item(NAME(m_cursor_blink_count));
	save_item(NAME(m_horiz_blank_end));
	save_item(NAME(m_ena_vert_access));
	save_item(NAME(m_de_skew));
	save_item(NAME(m_horiz_retr_start));
	save_item(NAME(m_horiz_retr_end));
	save_item(NAME(m_horiz_retr_skew));
	save_item(NAME(m_vert_total));
	save_item(NAME(m_preset_row_scan));
	save_item(NAME(m_byte_panning));
	save_item(NAME(m_max_ras_addr));
	save_item(NAME(m_scan_doubling));
	save_item(NAME(m_cursor_disable));
	save_item(NAME(m_cursor_skew));
	save_item(NAME(m_vert_retr_start));
	save_item(NAME(m_vert_retr_end));
	save_item(NAME(m_protect));
	save_item(NAME(m_bandwidth));
	save_item(NAME(m_vert_disp_end));
	save_item(NAME(m_offset));
	save_item(NAME(m_underline_loc));
	save_item(NAME(m_vert_blank_start));
	save_item(NAME(m_vert_blank_end));
	save_item(NAME(m_line_compare));
}


void crtc_ega_device::device_reset()
{
	/* internal registers other than status remain unchanged, all outputs go low */
	if (!m_res_out_de_cb.isnull())
		m_res_out_de_cb(false);

	if (!m_res_out_hsync_cb.isnull())
		m_res_out_hsync_cb(false);

	if (!m_res_out_vsync_cb.isnull())
		m_res_out_vsync_cb(false);

	if (!m_res_out_vblank_cb.isnull())
		m_res_out_vblank_cb(false);

	if (!m_line_timer->enabled())
	{
		m_line_timer->adjust( attotime::from_ticks( m_horiz_char_total + 2, m_clock ) );
	}

	m_light_pen_latched = false;

	m_cursor_addr = 0;
	m_line_address = 0;
	m_horiz_disp = 0;
	m_cursor_x = 0;
	m_horiz_blank_start = 0;
	m_horiz_blank_end = 0;
	m_vert_disp_end = 0;
	m_vert_retr_start = 0;
	m_vert_blank_start = 0;
	m_line_counter = 0;
	m_raster_counter = 0;
	m_horiz_pix_total = 0;
	m_vert_pix_total = 0;
	m_max_visible_x = 0;
	m_max_visible_y = 0;
	m_hsync_on_pos = 0;
	m_vsync_on_pos = 0;
	m_hsync_off_pos = 0;
	m_vsync_off_pos = 0;
}
