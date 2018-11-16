// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    National Semiconductor DP8350 Series CRT Controllers

    These dedicated CRTC devices are “programmable” by specifying a
    long list of mask parameters. Since the dot clock is generated
    internally, these parameters determine the character field size
    (16 x 16 maximum) as well as the number of characters displayed
    on screen and the timing, width and polarity of the sync pulses
    (though two different vertical timings are incorporated so the
    refresh rate can be switched between 60 Hz and 50 Hz).

    The number of parameters adjustable through software control is
    relatively small, there being no dedicated microprocessor data bus.
    In practice, however, the CPU's D0 and D1 are usually tied to the
    Register Select lines, due to shared use of the address bus for
    DMA purposes. A per-row interrupt can be derived in various ways
    from the timing outputs; this can be used to reload the Row Start
    Register to point to display memory at nonconsecutive addresses.

    National released the DP8350 and a few other standard versions.
    DP8367 appears to have originally been a custom mask variant
    ordered by Hewlett-Packard, though it is listed as option “I” in
    later datasheets; its parameters have been derived from the
    information presented in Manual Part No. 13220-91087. (The 8367
    designation also comes from there, since the actual IC is merely
    marked with its HP part number, 1820-2373.) DP8369 is another
    variant ordered by General Terminal Corp. for their SW10 terminal;
    option “K” is its National designation. Sperry Univac ordered
    another variant whose actual number is unknown but appears to use
    the option “A” parameters.

        Variant     Dot rate      Monitor type
        -------     --------      ------------
        DP8350      10.92 MHz     Ball Brothers TV-12, TV-120, etc.
        DP8352      7.02 MHz      RS-170 compatible
        DP8353      17.6256 MHz   Motorola M3003
        DP8367      25.7715 MHz   HP 13220
        DP8369      12.2472 MHz   GTC SW10
        DP????      19.98 MHz     Sperry Univac UTS

**********************************************************************/

#include "emu.h"
#include "video/dp8350.h"
#include "screen.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(DP8350, dp8350_device, "dp8350", "DP8350 CRTC")
DEFINE_DEVICE_TYPE(DP8367, dp8367_device, "dp8367", "DP8367 CRTC")
DEFINE_DEVICE_TYPE(DP835X_A, dp835x_a_device, "dp835x_a", "DP835X CRTC (option A)")


//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  dp835x_device - constructor
//-------------------------------------------------

dp835x_device::dp835x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock,
							 int char_width, int char_height, int chars_per_row, int rows_per_frame,
							 int vsync_delay_f1, int vsync_width_f1, int vblank_interval_f1,
							 int vsync_delay_f0, int vsync_width_f0, int vblank_interval_f0,
							 int chars_per_line, int hsync_delay, int hsync_width, int vblank_stop,
							 bool cursor_on_all_lines, int lbc_0_width, int hsync_serration,
							 bool hsync_active, bool vsync_active, bool vblank_active)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_char_width(char_width)
	, m_char_height(char_height)
	, m_chars_per_row(chars_per_row)
	, m_rows_per_frame(rows_per_frame)
	, m_vsync_delay{vsync_delay_f0, vsync_delay_f1}
	, m_vsync_width{vsync_width_f0, vsync_width_f1}
	, m_vblank_interval{vblank_interval_f0, vblank_interval_f1}
	, m_chars_per_line(chars_per_line)
	, m_hsync_delay(hsync_delay)
	, m_hsync_width(hsync_width)
	, m_vblank_stop(vblank_stop)
	, m_cursor_on_all_lines(cursor_on_all_lines)
	, m_lbc_0_width(lbc_0_width)
	, m_hsync_serration(hsync_serration)
	, m_hsync_active(hsync_active)
	, m_vsync_active(vsync_active)
	, m_vblank_active(vblank_active)
	, m_dots_per_line(char_width * chars_per_line)
	, m_dots_per_row(char_width * chars_per_row)
	, m_video_scan_lines(char_height * rows_per_frame)
	, m_lrc_callback(*this)
	, m_clc_callback(*this)
	, m_lc_callback(*this)
	, m_lbre_callback(*this)
	, m_hsync_callback(*this)
	, m_vsync_callback(*this)
	, m_vblank_callback(*this)
	, m_60hz_refresh(true)
	, m_cgpi(false)
	, m_topr(0)
	, m_rsr(0)
	, m_cr(0)
	, m_row_start(0)
	, m_lc(0)
{
	// some parameters are not used (at least not directly)
	(void)m_rows_per_frame;
	(void)m_chars_per_line;
	(void)m_cursor_on_all_lines;
	(void)m_lbc_0_width;
	(void)m_hsync_serration;
}


//-------------------------------------------------
//  dp8350_device - constructor
//-------------------------------------------------

dp8350_device::dp8350_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dp835x_device(mconfig, DP8350, tag, owner, clock,
						7, 10, 80, 24,
						4, 10, 20,
						30, 10, 72,
						100, 0, 43, 1, // yes, the horizontal sync pulse is more than twice as long as the blanking period
						true, 4, 0,
						true, false, true)
{
}


//-------------------------------------------------
//  dp8367_device - constructor
//-------------------------------------------------

dp8367_device::dp8367_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dp835x_device(mconfig, DP8367, tag, owner, clock,
						9, 15, 80, 26,
						0, 19, 25,
						38, 64, 108,
						115, -16, 7, 1, // value of vblank_stop assumed
						true, 5, 0, // values assumed
						true, true, true)
{
}


//-------------------------------------------------
//  dp835x_a_device - constructor
//-------------------------------------------------

dp835x_a_device::dp835x_a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dp835x_device(mconfig, DP835X_A, tag, owner, clock,
						9, 14, 80, 25,
						0, 14, 20, // sync parameters guessed
						38, 59, 94, // sync parameters guessed
						100, 4, 8, 1, // sync parameters and vblank_stop guessed
						true, 5, 0, // values assumed
						true, true, true) // values assumed
{
}



//-------------------------------------------------
//  device_config_complete - finalise device
//  configuration
//-------------------------------------------------

void dp835x_device::device_config_complete()
{
	if (has_screen() && screen().refresh_attoseconds() == 0)
	{
		int lines_per_frame = m_video_scan_lines + m_vblank_interval[m_60hz_refresh ? 1 : 0];
		if (m_half_shift)
			screen().set_raw(clock() * 2, m_dots_per_line * 2, 0, m_dots_per_row * 2, lines_per_frame, 0, m_video_scan_lines);
		else
			screen().set_raw(clock(), m_dots_per_line, 0, m_dots_per_row, lines_per_frame, 0, m_video_scan_lines);
	}
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void dp835x_device::device_resolve_objects()
{
	m_lrc_callback.resolve_safe();
	m_clc_callback.resolve_safe();
	m_lc_callback.resolve_safe();
	m_lbre_callback.resolve_safe();
	m_hsync_callback.resolve_safe();
	m_vsync_callback.resolve_safe();
	m_vblank_callback.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dp835x_device::device_start()
{
	// create timers
	m_hblank_start_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dp835x_device::hblank_start), this));
	m_hblank_near_end_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dp835x_device::hblank_near_end), this));
	m_hsync_on_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dp835x_device::hsync_update), this));
	m_hsync_off_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dp835x_device::hsync_update), this));

	// save state
	save_item(NAME(m_60hz_refresh));
	save_item(NAME(m_cgpi));
	save_item(NAME(m_lc));
	save_item(NAME(m_topr));
	save_item(NAME(m_rsr));
	save_item(NAME(m_cr));
	save_item(NAME(m_row_start));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dp835x_device::device_reset()
{
	m_topr = 0;
	m_rsr = 0;
	m_cr = 0;
	m_row_start = 0;
}


//-------------------------------------------------
//  device_clock_changed - called when the
//  device clock is altered in any way
//-------------------------------------------------

void dp835x_device::device_clock_changed()
{
	reconfigure_screen();
}


//-------------------------------------------------
//  reconfigure_screen - update screen parameters
//-------------------------------------------------

void dp835x_device::reconfigure_screen()
{
	int lines_per_frame = m_video_scan_lines + m_vblank_interval[m_60hz_refresh ? 1 : 0];
	attotime scan_period = clocks_to_attotime(m_dots_per_line);
	attotime refresh = clocks_to_attotime(lines_per_frame * m_dots_per_line);

	if (m_half_shift)
		screen().configure(2 * m_dots_per_line, lines_per_frame, screen().visible_area(), refresh.as_attoseconds());
	else
		screen().configure(m_dots_per_line, lines_per_frame, screen().visible_area(), refresh.as_attoseconds());

	logerror("Frame rate refresh: %.2f Hz (f%d); horizontal rate scan: %.4f kHz; character rate: %.4f MHz; dot rate: %.5f MHz\n",
		refresh.as_hz(),
		m_60hz_refresh ? 1 : 0,
		clock() / (m_dots_per_line * 1000.0),
		clock() / (m_char_width * 1000000.0),
		clock() / 1000000.0);

	// get current screen position (note that this method is more accurate than calling hpos and vpos separately)
	u32 dpos = attotime_to_clocks(refresh - screen().time_until_pos(lines_per_frame - 1, m_dots_per_row * (m_half_shift ? 2 : 1)));
	int hpos = (dpos + m_dots_per_row) % m_dots_per_line;
	int vpos = dpos / m_dots_per_line;

	// set line rate clock timers
	int hblank_begin = m_dots_per_row;
	int hblank_near_end = m_dots_per_line - m_char_width * 5;
	if (hpos >= hblank_begin)
		hblank_begin += m_dots_per_line;
	m_hblank_start_timer->adjust(clocks_to_attotime(hblank_begin - hpos), 0, scan_period);
	if (hpos >= hblank_near_end)
		hblank_near_end += m_dots_per_line;
	m_hblank_near_end_timer->adjust(clocks_to_attotime(hblank_near_end - hpos), 0, scan_period);

	// set horizontal sync timers (note that HSYNC may precede horizontal blanking or even outlast it, as on the DP8350)
	int hsync_begin = m_dots_per_row + m_char_width * m_hsync_delay;
	int hsync_end = hsync_begin + m_char_width * m_hsync_width;
	if (hpos >= hsync_begin)
		hsync_begin += m_dots_per_line;
	m_hsync_on_timer->adjust(clocks_to_attotime(hsync_begin - hpos), m_hsync_active, scan_period);
	if (hpos >= hsync_end)
		hsync_end += m_dots_per_line;
	else if (hpos < hsync_end - m_dots_per_line)
		hsync_end -= m_dots_per_line;
	m_hsync_off_timer->adjust(clocks_to_attotime(hsync_end - hpos), !m_hsync_active, scan_period);
	logerror("hblank_begin: %d; hsync_begin: %d; hsync_end: %d; hpos: %d\n", hblank_begin, hsync_begin, hsync_end, hpos);

	// calculate vertical sync and blanking parameters
	int vsync_begin = m_video_scan_lines + m_vsync_delay[m_60hz_refresh ? 1 : 0];
	int vsync_end = vsync_begin + m_vsync_width[m_60hz_refresh ? 1 : 0];
	int vblank_end = lines_per_frame - m_vblank_stop;
	logerror("vblank_begin: %d; vsync_begin: %d; vsync_end: %d; vblank_end: %d; vpos: %d\n", m_video_scan_lines, vsync_begin, vsync_end, vblank_end, vpos);

	// set counters
	m_line = vpos;
	if (m_line >= lines_per_frame - m_char_height)
		m_lc = m_line - (lines_per_frame - m_char_height);
	else
		m_lc = m_line % m_char_height;
}


//-------------------------------------------------
//  refresh_control - set or configure one of two
//  field refresh rates (f1 = 60 Hz, f0 = 50 Hz)
//-------------------------------------------------

WRITE_LINE_MEMBER(dp835x_device::refresh_control)
{
	if (m_60hz_refresh != bool(state))
	{
		m_60hz_refresh = state;
		if (started())
			reconfigure_screen();
	}
}


//-------------------------------------------------
//  character_generator_program - set or configure
//  the character generator program input (CGPI):
//  0 = video begins on scan line 0; new addresses
//  loaded on last scan line of previous character
//  1 = video begins on scan line 1; new addresses
//  loaded on scan line 0
//-------------------------------------------------

WRITE_LINE_MEMBER(dp835x_device::character_generator_program)
{
	m_cgpi = bool(state);
}


//-------------------------------------------------
//  register_load - write to one of three CRTC
//  address registers
//-------------------------------------------------

void dp835x_device::register_load(u8 rs, u16 addr)
{
	addr &= 0xfff;

	switch (rs & 3)
	{
	case 0:
	default:
		// A = 0, B = 0: No Select
		break;

	case 1:
		// A = 0, B = 1: Top-of-Page
		LOG("Top-of-Page Register = %03X\n", addr);
		m_topr = addr;
		break;

	case 2:
		// A = 1, B = 0: Row Start (also Top-of-Page during vertical blanking)
		if (m_line >= m_video_scan_lines)
		{
			LOG("Row Start Register = %03X (redirected to Top-of-Page)\n", addr);
			m_topr = addr;
		}
		else
		{
			LOG("Row Start Register = %03X\n", addr);
			m_rsr = addr;
		}
		break;

	case 3:
		// A = 1, B = 1: Cursor
		LOG("Cursor Register = %03X\n", addr);
		m_cr = addr;
		break;
	}
}


//-------------------------------------------------
//  lrc_r - poll line rate clock state
//-------------------------------------------------

READ_LINE_MEMBER(dp835x_device::lrc_r)
{
	if (m_hblank_start_timer->remaining() > m_hblank_near_end_timer->remaining())
		return 0;
	else
		return 1;
}


//-------------------------------------------------
//  lbre_r - poll line buffer recirculate enable
//  state
//-------------------------------------------------

READ_LINE_MEMBER(dp835x_device::lbre_r)
{
	if (m_lc == (m_cgpi ? 0 : m_char_height - 1))
		return 0;
	else
		return 1;
}



//-------------------------------------------------
//  hsync_r - poll horizontal sync state
//-------------------------------------------------

READ_LINE_MEMBER(dp835x_device::hsync_r)
{
	if (m_hsync_on_timer->remaining() > m_hsync_off_timer->remaining())
		return m_hsync_active;
	else
		return !m_hsync_active;
}


//-------------------------------------------------
//  vsync_r - poll vertical sync state
//-------------------------------------------------

READ_LINE_MEMBER(dp835x_device::vsync_r)
{
	int vsync_begin = m_video_scan_lines + m_vsync_delay[m_60hz_refresh ? 1 : 0];
	int vsync_end = vsync_begin + m_vsync_width[m_60hz_refresh ? 1 : 0];
	return (m_line >= vsync_begin && m_line < vsync_end) ? m_vsync_active : !m_vsync_active;
}


//-------------------------------------------------
//  vblank_r - poll vertical blanking state
//-------------------------------------------------

READ_LINE_MEMBER(dp835x_device::vblank_r)
{
	int vblank_end = m_video_scan_lines + m_vblank_interval[m_60hz_refresh ? 1 : 0] - m_vblank_stop;
	return (m_line >= m_video_scan_lines && m_line < vblank_end) ? m_vblank_active : !m_vblank_active;
}


//-------------------------------------------------
//  hblank_start - update timing outputs at the
//  start of horizontal blanking
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(dp835x_device::hblank_start)
{
	int lines_per_frame = m_video_scan_lines + m_vblank_interval[m_60hz_refresh ? 1 : 0];
	int vsync_begin = m_video_scan_lines + m_vsync_delay[m_60hz_refresh ? 1 : 0];
	int vsync_end = vsync_begin + m_vsync_width[m_60hz_refresh ? 1 : 0];
	int vblank_end = lines_per_frame - m_vblank_stop;

	// falling edge of line rate clock
	m_lrc_callback(0);

	// increment line counter or reset it
	if (m_lc < m_char_height - 1 && m_line != lines_per_frame - m_char_height)
		m_lc++;
	else
	{
		m_lc = 0;
		m_clc_callback(0);
	}
	m_lc_callback(m_lc);

	// increment internal 9-bit scan counter
	m_line++;
	//assert(m_line == screen().vpos() + 1);

	// update line buffer recirculate enable output based on address mode
	bool lbre = m_lc != (m_cgpi ? 0 : m_char_height - 1);
	m_lbre_callback(lbre ? 1 : 0);

	if (m_line >= lines_per_frame - m_char_height && m_line < lines_per_frame)
	{
		m_row_start = m_rsr = m_topr;
	}
	else if (!lbre || m_line >= m_video_scan_lines)
	{
		// calculate starting address of next row (address counter runs continuously during VBLANK)
		m_row_start = m_rsr;
		m_rsr = (m_row_start + m_chars_per_row) & 0xfff;
	}

	// update vertical blanking output
	if (m_line == m_video_scan_lines)
		m_vblank_callback(m_vblank_active);
	else if (m_line == vblank_end)
		m_vblank_callback(!m_vblank_active);

	// update vertical sync output
	if (m_line == vsync_begin)
		m_vsync_callback(m_vsync_active);
	else if (m_line == vsync_end)
		m_vsync_callback(!m_vsync_active);

	if (m_line == lines_per_frame)
		m_line = 0;
}


//-------------------------------------------------
//  hblank_near_end - update line rate outputs
//  five chars before horizontal blanking ends
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(dp835x_device::hblank_near_end)
{
	// rising edge of line rate clock
	m_lrc_callback(1);
	if (m_lc == 0)
		m_clc_callback(1);
}


//-------------------------------------------------
//  hsync_update - update state of horizontal
//  sync output
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(dp835x_device::hsync_update)
{
	m_hsync_callback(param);
}
