// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    National Semiconductor DP8350 Series CRT Controllers

    These dedicated CRTC devices are “programmable” by specifying a
    long list of mask parameters. The number of parameters adjustable
    through software is relatively small, there being no dedicated
    microprocessor data bus.

    National released the DP8350 and a few other standard versions.
    DP8367 appears to have originally been a custom mask variant
    ordered by Hewlett-Packard, though it is listed as option “I” in
    later datasheets; its parameters have been derived from the
    information presented in Manual Part No. 13220-91087.

        Variant     Dot rate      Monitor type
        -------     --------      ------------
        DP8350      10.92 MHz     Ball Brothers TV-12, TV-120, etc.
        DP8352      7.02 MHz      RS-170 compatible
        DP8353      17.6256 MHz   Motorola M3003
        DP8367      25.7715 MHz   HP 13220

**********************************************************************/

#include "emu.h"
#include "video/dp8350.h"
#include "screen.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(DP8350, dp8350_device, "dp8350", "DP8350 CRTC")
DEFINE_DEVICE_TYPE(DP8367, dp8367_device, "dp8367", "DP8367 CRTC")


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
	, m_vblank_callback(*this)
	, m_60hz_refresh(true)
{
	// many parameters are not used yet
	(void)m_vsync_delay;
	(void)m_vsync_width;
	(void)m_hsync_delay;
	(void)m_hsync_width;
	(void)m_vblank_stop;
	(void)m_cursor_on_all_lines;
	(void)m_lbc_0_width;
	(void)m_hsync_serration;
	(void)m_hsync_active;
	(void)m_vsync_active;
}


//-------------------------------------------------
//  dp8350_device - constructor
//-------------------------------------------------

dp8350_device::dp8350_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dp835x_device(mconfig, DP8350, tag, owner, clock,
                        7, 10, 80, 24,
                        4, 10, 20,
                        30, 10, 72,
                        100, 0, 4, 1, // HSYNC width given in datasheet as an impossible 43 character times
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
//  device_config_complete - finalise device
//  configuration
//-------------------------------------------------

void dp835x_device::device_config_complete()
{
	if (has_screen() && screen().refresh_attoseconds() == 0)
	{
		int dots_per_line = m_char_width * m_chars_per_line;
		int dots_per_row = m_char_width * m_chars_per_row;
		int lines_per_frame = m_char_height * m_rows_per_frame + m_vblank_interval[m_60hz_refresh ? 1 : 0];

		if (m_half_shift)
			screen().set_raw(clock() * 2, dots_per_line * 2, 0, dots_per_row * 2, lines_per_frame, 0, m_char_height * m_rows_per_frame);
		else
			screen().set_raw(clock(), dots_per_line, 0, dots_per_row, lines_per_frame, 0, m_char_height * m_rows_per_frame);
	}
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void dp835x_device::device_resolve_objects()
{
	//m_hsync_callback.resolve_safe();
	//m_vsync_callback.resolve_safe();
	m_vblank_callback.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dp835x_device::device_start()
{
	screen().register_vblank_callback(vblank_state_delegate(&dp835x_device::screen_vblank, this));

	save_item(NAME(m_60hz_refresh));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dp835x_device::device_reset()
{
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
	int dots_per_line = m_char_width * m_chars_per_line;
	int dots_per_row = m_char_width * m_chars_per_row;
	int lines_per_frame = m_char_height * m_rows_per_frame + m_vblank_interval[m_60hz_refresh ? 1 : 0];
	attoseconds_t refresh = clocks_to_attotime(lines_per_frame * dots_per_line).as_attoseconds();

	if (m_half_shift)
	{
		rectangle visarea(0, 2 * dots_per_row - 1, 0, m_char_height * m_rows_per_frame - 1);
		screen().configure(2 * dots_per_line, lines_per_frame, visarea, refresh);
	}
	else
	{
		rectangle visarea(0, dots_per_row - 1, 0, m_char_height * m_rows_per_frame - 1);
		screen().configure(dots_per_line, lines_per_frame, visarea, refresh);
	}

	logerror("Frame rate refresh: %.2f Hz (f%d); horizontal rate scan: %.4f kHz; character rate: %.4f MHz; dot rate: %.5f MHz\n",
		ATTOSECONDS_TO_HZ(refresh),
		m_60hz_refresh ? 1 : 0,
		clock() / (dots_per_line * 1000.0),
		clock() / (m_char_width * 1000000.0),
		clock() / 1000000.0);
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
		// No Select
		break;

	case 1:
		// Top-of-Page
		logerror("Top-of-Page Register = %03X\n", addr);
		break;

	case 2:
		// Row Start
		logerror("Row Start Register = %03X\n", addr);
		break;

	case 3:
		// Cursor
		logerror("Cursor Register = %03X\n", addr);
		break;
	}
}


//-------------------------------------------------
//  vblank_r - report vertical blanking state
//-------------------------------------------------

READ_LINE_MEMBER(dp835x_device::vblank_r)
{
	return bool(screen().vblank()) == m_vblank_active;
}


//-------------------------------------------------
//  screen_vblank - vertical blanking handler
//-------------------------------------------------

void dp835x_device::screen_vblank(screen_device &screen, bool state)
{
	m_vblank_callback(state == m_vblank_active);
}
